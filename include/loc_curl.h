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

#ifndef _LOC_CURL_H_
#define _LOC_CURL_H_

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize libcurl. Call this once at the beginning of your program. This
    function makes calls to curl_global_init() and curl_multi_init() */
void loc_curl_init();

/** Return global multi handle */
CURLM* loc_curl_handle();

/** Convenience function, just executes
    curl_multi_add_handle(loccurl_handle(), easy_handle); loccurl_start()*/
CURLMcode loc_curl_add(CURL* easy_handle);

/** Convenience function, just executes
    curl_multi_remove_handle(loccurl_handle(), easy_handle) */
CURLMcode loc_curl_remove(CURL* easy_handle);

/** Call this whenever you have added a request using
    curl_multi_add_handle(). This is necessary to start new requests. It does
    so by triggering a call to curl_multi_perform() even in the case where no
    open fds cause that function to be called anyway. The call happens
    "later", i.e. during the next iteration of the glib main loop.
    loccurl_start() only sets a flag to make it happen. */
void loc_curl_start();

/** Callback function for loccurl_set_callback */
typedef void (*LocCurlCallback)(void*);
/** Set function to call after each invocation of curl_multi_perform(). Pass
    function==0 to unregister a previously set callback. The callback
    function will be called with the supplied data pointer as its first
    argument. */
void loc_curl_set_callback(LocCurlCallback function, void* data);

/** You must call loccurl_remove() and curl_easy_cleanup() for all requests
    before calling this. This function makes calls to curl_multi_cleanup()
    and curl_global_cleanup(). */
void loc_curl_cleanup();

#ifdef __cplusplus
}
#endif

#endif
