/* $Id: glibcurl.h,v 1.7 2004/12/04 13:58:29 atterer Exp $ -*- C -*-
  __   _
  |_) /|  Copyright (C) 2004  |  richard@
  | \/�|  Richard Atterer     |  atterer.net
  � '` �
  All rights reserved.

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
  USE OR OTHER DEALINGS IN THE SOFTWARE.

  Except as contained in this notice, the name of a copyright holder shall
  not be used in advertising or otherwise to promote the sale, use or other
  dealings in this Software without prior written authorization of the
  copyright holder.

*/

#include <loc_curl.h>
#include <loc_log.h>

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* Number of highest allowed fd */
#define LOCCURL_FDMAX 1023

/* Timeout for the fds passed to glib's poll() call, in millisecs.
   curl_multi_fdset(3) says we should call curl_multi_perform() at regular
   intervals. */
#define LOCCURL_TIMEOUT 1000

/* GIOCondition event masks */
#define LOCCURL_READ  (G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP)
#define LOCCURL_WRITE (G_IO_OUT | G_IO_ERR | G_IO_HUP)
#define LOCCURL_EXC   (G_IO_ERR | G_IO_HUP)

/** A structure which "derives" (in glib speak) from GSource */
typedef struct CurlGSource_ {
  GSource source; /* First: The type we're deriving from */

  CURLM* multiHandle;

  /* Previously seen FDs, for comparing with libcurl's current fd_sets */
  GPollFD lastPollFd[LOCCURL_FDMAX + 1];
  int lastPollFdMax; /* Index of highest non-empty entry in lastPollFd */

  int callPerform; /* Non-zero => curl_multi_perform() gets called */

  /* For data returned by curl_multi_fdset */
  fd_set fdRead;
  fd_set fdWrite;
  fd_set fdExc;
  int fdMax;

} CurlGSource;

/* Global state: Our CurlGSource object */
static CurlGSource* curlSrc = 0;

// Number of easy handles currently active
static int s_numEasyHandles = 0;

// Current timeout value to wait in poll
static int s_currTimeout = 0;

// Time when last dispatch was made
static struct timespec s_timeAtLastDispatch;


/* The "methods" of CurlGSource */
static gboolean prepare(GSource* source, gint* timeout);
static gboolean check(GSource* source);
static gboolean dispatch(GSource* source, GSourceFunc callback,
                         gpointer user_data);
static void finalize(GSource* source);

static GSourceFuncs curlFuncs = {
  &prepare, &check, &dispatch, &finalize, 0, 0
};
/*______________________________________________________________________*/

void loc_curl_init() {
  int fd;
  /* Create source object for curl file descriptors, and hook it into the
     default main context. */
  LS_LOG_DEBUG("Fun: %s Line: %d start\n", __FUNCTION__, __LINE__);

  GSource *gsource;
  gsource = g_source_new(&curlFuncs, sizeof(CurlGSource));
  g_source_set_priority(gsource, G_PRIORITY_DEFAULT_IDLE);
  curlSrc = (CurlGSource*)gsource;
  g_source_attach(&curlSrc->source, g_main_context_default());

  /* Init rest of our data */
  memset(&curlSrc->lastPollFd, 0, sizeof(curlSrc->lastPollFd));
  for (fd = 1; fd <= LOCCURL_FDMAX; ++fd)
    curlSrc->lastPollFd[fd].fd = fd;
  curlSrc->lastPollFdMax = 0;
  curlSrc->callPerform = 0;

  /* Init libcurl */
  curl_global_init(CURL_GLOBAL_ALL);
  curlSrc->multiHandle = curl_multi_init();
  curl_multi_setopt(curlSrc->multiHandle, CURLMOPT_MAXCONNECTS, 4);

  LS_LOG_DEBUG("Fun: %s Line: %d  events: R=%x W=%x X=%x \n",
               __FUNCTION__, __LINE__, LOCCURL_READ, LOCCURL_WRITE, LOCCURL_EXC);

  s_numEasyHandles = 0;
  s_currTimeout = 0;
  s_timeAtLastDispatch.tv_sec = 0;
  s_timeAtLastDispatch.tv_nsec = 0;

  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
}
/*______________________________________________________________________*/

CURLM* loc_curl_handle() {
  return curlSrc->multiHandle;
}
/*______________________________________________________________________*/

CURLMcode loc_curl_add(CURL *easy_handle) {
  LS_LOG_DEBUG("Fun: %s Line: %d start\n", __FUNCTION__, __LINE__);
  assert(curlSrc->multiHandle != 0);
  curlSrc->callPerform = -1;
  s_numEasyHandles++;
  CURLMcode ret = curl_multi_add_handle(curlSrc->multiHandle, easy_handle);
  g_main_context_wakeup(g_main_context_default());
  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
  return ret;
}
/*______________________________________________________________________*/

CURLMcode loc_curl_remove(CURL *easy_handle) {
  LS_LOG_DEBUG("Fun: %s Line: %d start\n", __FUNCTION__, __LINE__);
  assert(curlSrc != 0);
  assert(curlSrc->multiHandle != 0);
  assert(s_numEasyHandles > 0);
  s_numEasyHandles--;
  CURLMcode ret = curl_multi_remove_handle(curlSrc->multiHandle, easy_handle);
  g_main_context_wakeup(g_main_context_default());
  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
  return ret;
}
/*______________________________________________________________________*/

/* Call this whenever you have added a request using curl_multi_add_handle().
   This is necessary to start new requests. It does so by triggering a call
   to curl_multi_perform() even in the case where no open fds cause that
   function to be called anyway. */
void loc_curl_start() {
  LS_LOG_DEBUG("Fun: %s Line: %d start\n", __FUNCTION__, __LINE__);
  curlSrc->callPerform = -1;

  // Wake up event loop if it is suspended in a poll
  g_main_context_wakeup(g_main_context_default());
  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
}
/*______________________________________________________________________*/

void loc_curl_set_callback(LocCurlCallback function, void* data) {
  LS_LOG_DEBUG("Fun: %s Line: %d start\n", __FUNCTION__, __LINE__);
  g_source_set_callback(&curlSrc->source, (GSourceFunc)function, data,
                        NULL);
  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
}
/*______________________________________________________________________*/

void loc_curl_cleanup() {
  /* You must call curl_multi_remove_handle() and curl_easy_cleanup() for all
     requests before calling this. */
/*   assert(curlSrc->callPerform == 0); */
  LS_LOG_DEBUG("Fun: %s Line: %d start\n", __FUNCTION__, __LINE__);
  curl_multi_cleanup(curlSrc->multiHandle);
  curlSrc->multiHandle = 0;
  curl_global_cleanup();

  g_source_destroy(&curlSrc->source);
  g_source_unref(&curlSrc->source);
  curlSrc = 0;
  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
}
/*______________________________________________________________________*/

static void registerUnregisterFds() {
  int fd, fdMax;
  FD_ZERO(&curlSrc->fdRead);
  FD_ZERO(&curlSrc->fdWrite);
  FD_ZERO(&curlSrc->fdExc);
  curlSrc->fdMax = -1;
  /* What fds does libcurl want us to poll? */
  curl_multi_fdset(curlSrc->multiHandle, &curlSrc->fdRead,
                   &curlSrc->fdWrite, &curlSrc->fdExc, &curlSrc->fdMax);
  if ((curlSrc->fdMax < -1) || (curlSrc->fdMax > LOCCURL_FDMAX)) {
      LS_LOG_ERROR("Fun: %s Line: %d fdMax=%d \n", __FUNCTION__, __LINE__, curlSrc->fdMax);
  }
  /*fprintf(stderr, "registerUnregisterFds: fdMax=%d\n", curlSrc->fdMax);*/
  assert(curlSrc->fdMax >= -1 && curlSrc->fdMax <= LOCCURL_FDMAX);

  fdMax = curlSrc->fdMax;
  if (fdMax < curlSrc->lastPollFdMax) fdMax = curlSrc->lastPollFdMax;

  /* Has the list of required events for any of the fds changed? */
  for (fd = 0; fd <= fdMax; ++fd) {
    gushort events = 0;
    if (FD_ISSET(fd, &curlSrc->fdRead))  events |= LOCCURL_READ;
    if (FD_ISSET(fd, &curlSrc->fdWrite)) events |= LOCCURL_WRITE;
    if (FD_ISSET(fd, &curlSrc->fdExc))   events |= LOCCURL_EXC;

    /* List of events unchanged => no (de)registering */
    if (events == curlSrc->lastPollFd[fd].events) continue;

    LS_LOG_DEBUG("Fun: %s Line: %d registerUnregisterFds: fd %d: old events %x "
                "new events %x \n", __FUNCTION__, __LINE__, fd, curlSrc->lastPollFd[fd].events, events);

    /* fd is already a lastPollFd, but event type has changed => do nothing.
       Due to the implementation of g_main_context_query(), the new event
       flags will be picked up automatically. */
    if (events != 0 && curlSrc->lastPollFd[fd].events != 0) {
      curlSrc->lastPollFd[fd].events = events;
      continue;
    }
    curlSrc->lastPollFd[fd].events = events;

    /* Otherwise, (de)register as appropriate */
    if (events == 0) {
      g_source_remove_poll(&curlSrc->source, &curlSrc->lastPollFd[fd]);
      curlSrc->lastPollFd[fd].revents = 0;
      LS_LOG_DEBUG("Fun: %s Line: %d unregister fd %d \n", __FUNCTION__, __LINE__, fd);
    } else {
      g_source_add_poll(&curlSrc->source, &curlSrc->lastPollFd[fd]);
      LS_LOG_DEBUG("Fun: %s Line: %d register fd %d\n", __FUNCTION__, __LINE__, fd);
    }
  }

  curlSrc->lastPollFdMax = curlSrc->fdMax;
}

/* Called before all the file descriptors are polled by the glib main loop.
   We must have a look at all fds that libcurl wants polled. If any of them
   are new/no longer needed, we have to (de)register them with glib. */
gboolean prepare(GSource* source, gint* timeout) {
  assert(source == &curlSrc->source);

  if (curlSrc->multiHandle == 0) return FALSE;

  registerUnregisterFds();

  // Handle has been added. we are ready
  if (curlSrc->callPerform == -1) {
      s_currTimeout = *timeout = 0;
      return TRUE;
  }

  long curlTimeout = 0;
  curl_multi_timeout(curlSrc->multiHandle, &curlTimeout);

  // Curl tells us it is ready
  if (curlTimeout == 0) {
      s_currTimeout = *timeout = 0;
      return TRUE;
  }

  // Curl says wait forever. do it only when if we don't have pending
  // connections
  if (curlTimeout < 0) {
      s_currTimeout = *timeout = (s_numEasyHandles > 0) ? LOCCURL_TIMEOUT : -1;
      return FALSE;
  }

  s_currTimeout = *timeout = MIN(LOCCURL_TIMEOUT, curlTimeout);
  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
  return FALSE;
}
/*______________________________________________________________________*/

/* Called after all the file descriptors are polled by glib.
   g_main_context_check() has copied back the revents fields (set by glib's
   poll() call) to our GPollFD objects. How inefficient all that copying
   is... let's add some more and copy the results of these revents into
   libcurl's fd_sets! */
gboolean check(GSource* source) {
  int fd, somethingHappened = 0;
  assert(source == &curlSrc->source);

  if (curlSrc->multiHandle == 0 || s_numEasyHandles <= 0) {
      curlSrc->callPerform = 0;
      return FALSE;
  }

  FD_ZERO(&curlSrc->fdRead);
  FD_ZERO(&curlSrc->fdWrite);
  FD_ZERO(&curlSrc->fdExc);
  for (fd = 0; fd <= curlSrc->fdMax; ++fd) {
    gushort revents = curlSrc->lastPollFd[fd].revents;
    if (revents == 0) continue;
    somethingHappened = 1;
    if (revents & (G_IO_IN | G_IO_PRI))
      FD_SET((unsigned)fd, &curlSrc->fdRead);
    if (revents & G_IO_OUT)
      FD_SET((unsigned)fd, &curlSrc->fdWrite);
    if (revents & (G_IO_ERR | G_IO_HUP))
      FD_SET((unsigned)fd, &curlSrc->fdExc);
  }

/*   return TRUE; */
/*   return FALSE; */

  if (curlSrc->callPerform == -1) {
      return TRUE;
  }

  if (somethingHappened != 0) {
      return TRUE;
  }

  struct timespec currTime;
  clock_gettime(CLOCK_MONOTONIC, &currTime);

  // Curl wants us to call it regularly even if there is no data available
  if (((currTime.tv_sec - s_timeAtLastDispatch.tv_sec) * 1000 +
       (currTime.tv_nsec - s_timeAtLastDispatch.tv_nsec) / 1000000) >=
      s_currTimeout) {
      return TRUE;
  }

  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
  return FALSE;
}
/*______________________________________________________________________*/

gboolean dispatch(GSource* source, GSourceFunc callback,
                  gpointer user_data) {
  CURLMcode x;
  LS_LOG_DEBUG("Fun: %s Line: %d start \n", __FUNCTION__, __LINE__);
  assert(source == &curlSrc->source);
  assert(curlSrc->multiHandle != 0);

  clock_gettime(CLOCK_MONOTONIC, &s_timeAtLastDispatch);

  do {
    x = curl_multi_perform(curlSrc->multiHandle, &curlSrc->callPerform);
  } while (x == CURLM_CALL_MULTI_PERFORM);

  /* If no more calls to curl_multi_perform(), unregister left-over fds */
  if (curlSrc->callPerform == 0) registerUnregisterFds();

  if (callback != 0) (*callback)(user_data);

  LS_LOG_DEBUG("Fun: %s Line: %d end\n", __FUNCTION__, __LINE__);
  return TRUE; /* "Do not destroy me" */
}
/*______________________________________________________________________*/

void finalize(GSource* source) {
  assert(source == &curlSrc->source);
  registerUnregisterFds();
}

