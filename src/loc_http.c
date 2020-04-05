// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <loc_http.h>
#include <loc_log.h>

#define MAX_HTTPHEADER          3
#define CONNECTION_TIMEOUT      60
#define NO_CERTIFICATE_VERIFY   0L
#define CERTIFICATE_VERIFY      1L
#define NO_SSL_VERIFYHOST       0L
#define SSL_VERIFYHOST          1L

static gboolean gIsInitialized = FALSE;
static ResponseCallback gResponseCb = NULL;
static GHashTable *gHttpTasks = NULL;
static const char *gHttpHeader[MAX_HTTPHEADER] = { "Accept: application/json",
                                                   "Content-Type: application/json",
                                                   "charsets: utf-8" };

static void cbLocCurl(void* data);
static size_t cbWriteMemory(char *, size_t, size_t, void *);

HttpReqTask *loc_create_http_task(const char **headers, int size, void *message, void *userdata)
{
    HttpReqTask *task = NULL;
    int i;

    task = (HttpReqTask *)malloc(sizeof(HttpReqTask));
    if (task) {
        memset(task, 0, sizeof(HttpReqTask));

        task->curlDesc.handle = curl_easy_init();
        task->message = message;
        task->recepient = userdata;
        if (headers != NULL) {
            for (i = 0; i < size; i++) {
                task->curlDesc.headerList = curl_slist_append(task->curlDesc.headerList, headers[i]);
            }
        } else {
            for (i = 0; i < MAX_HTTPHEADER; i++) {
                task->curlDesc.headerList = curl_slist_append(task->curlDesc.headerList, gHttpHeader[i]);
            }
        }
    }

    return task;
}

HttpReqTask *loc_http_task_create(const char **headers, int size)
{
    HttpReqTask *task = NULL;
    int i;

    task = (HttpReqTask *)malloc(sizeof(HttpReqTask));
    if (task) {
        memset(task, 0, sizeof(HttpReqTask));

        task->curlDesc.handle = curl_easy_init();
        if (headers != NULL) {
            for (i = 0; i < size; i++) {
                task->curlDesc.headerList = curl_slist_append(task->curlDesc.headerList, headers[i]);
            }
        } else {
            for (i = 0; i < MAX_HTTPHEADER; i++) {
                task->curlDesc.headerList = curl_slist_append(task->curlDesc.headerList, gHttpHeader[i]);
            }
        }
    }

    return task;
}

void loc_http_task_destroy(HttpReqTask **task_ref)
{
    HttpReqTask *task = *task_ref;

    if (!task)
        return;

    if (task->curlDesc.handle) {
        curl_easy_cleanup(task->curlDesc.handle);
        task->curlDesc.handle = NULL;
    }

    if (task->curlDesc.headerList) {
        curl_slist_free_all(task->curlDesc.headerList);
        task->curlDesc.headerList = NULL;
    }

    if (task->post_data) {
        free(task->post_data);
        task->post_data = NULL;
    }

    if (task->responseData) {
        free(task->responseData);
        task->responseData = NULL;
    }

    free(task);
    task = NULL;
}

gboolean loc_http_task_prepare_connection(HttpReqTask **task_ref, char *url)
{
    HttpReqTask *task = *task_ref;
    CURLcode curlRc = CURLE_OK;

    if (!task || !url)
        return FALSE;


    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_URL, url)) != CURLE_OK) {
        LS_LOG_ERROR("curl set opt: CURLOPT_URL failed [%s]\n", curl_easy_strerror(curlRc));
        return FALSE;
    }

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_WRITEFUNCTION, cbWriteMemory)) != CURLE_OK) {
        LS_LOG_ERROR("curl set opt: CURLOPT_WRITEFUNCTION failed [%s]\n", curl_easy_strerror(curlRc));
        return FALSE;
    }

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_WRITEDATA, (void *)task)) != CURLE_OK) {
        LS_LOG_ERROR("curl set opt: CURLOPT_WRITEDATA failed [%s]\n", curl_easy_strerror(curlRc));
        return FALSE;
    }

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_HTTPHEADER, task->curlDesc.headerList)) != CURLE_OK) {
        LS_LOG_ERROR("curl set opt: CURLOPT_HTTPHEADER failed [%s]\n", curl_easy_strerror(curlRc));
        return FALSE;
    }

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT)) != CURLE_OK)
        LS_LOG_WARNING("curl set opt: CURLOPT_CONNECTTIMEOUT failed [%s]\n", curl_easy_strerror(curlRc));

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_SSL_VERIFYPEER, NO_CERTIFICATE_VERIFY)) != CURLE_OK)
        LS_LOG_WARNING("curl set opt: CURLOPT_SSL_VERIFYPEER failed [%s]\n", curl_easy_strerror(curlRc));

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_SSL_VERIFYHOST, NO_SSL_VERIFYHOST)) != CURLE_OK)
        LS_LOG_WARNING("curl set opt: CURLOPT_SSL_VERIFYHOST failed [%s]\n", curl_easy_strerror(curlRc));

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_FOLLOWLOCATION, TRUE)) != CURLE_OK)
        LS_LOG_WARNING("curl set opt: CURLOPT_FOLLOWLOCATION failed [%s]\n", curl_easy_strerror(curlRc));
    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_LOW_SPEED_LIMIT,10L)) != CURLE_OK )
        LS_LOG_WARNING("curl set opt: CURLOPT_LOW_SPEED_LIMIT failed [%d]\n",curlRc);

    if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_LOW_SPEED_TIME,10L)) != CURLE_OK )
        LS_LOG_WARNING("curl set opt: CURLOPT_LOW_SPEED_TIME failed [%d]\n",curlRc);

    return TRUE;
}

void loc_http_start()
{
    if (gIsInitialized)
        return;

    loc_curl_init();
    gIsInitialized = TRUE;
}

void loc_http_stop()
{
    if (!gIsInitialized)
        return;

    loc_curl_cleanup();

    if (gHttpTasks) {
        g_hash_table_destroy(gHttpTasks);
        gHttpTasks = NULL;
    }

    gIsInitialized = FALSE;
}

void loc_http_set_callback(ResponseCallback response_cb, void *user_data)
{
    gResponseCb = response_cb;
    loc_curl_set_callback(cbLocCurl, user_data);
}

gboolean loc_http_add_request(HttpReqTask *task, gboolean sync)
{
    CURLcode curlRc = CURLE_OK;
    CURLMcode curlMRc = CURLM_OK;

    if (!task)
        return FALSE;

    if (task->curlDesc.handle == NULL)
        return FALSE;

    if (task->post_data != NULL) {
        if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_POST, 1)) != CURLE_OK) {
            LS_LOG_ERROR("curl set opt: CURLOPT_POST failed [%s]\n", curl_easy_strerror(curlRc));
            return FALSE;
        }

        if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_POSTFIELDSIZE, (long)strlen(task->post_data))) != CURLE_OK) {
            LS_LOG_ERROR("curl set opt: CURLOPT_POSTFIELDSIZE failed [%s]\n", curl_easy_strerror(curlRc));
            return FALSE;
        }

        if ((curlRc = curl_easy_setopt(task->curlDesc.handle, CURLOPT_POSTFIELDS, task->post_data)) != CURLE_OK) {
            LS_LOG_ERROR("curl set opt: CURLOPT_POSTFIELDS failed [%s]\n", curl_easy_strerror(curlRc));
            return FALSE;
        }
    }

    if (!gIsInitialized)
        loc_http_start();

    if (task->responseData) {
        free(task->responseData);
        task->responseData = NULL;
    }

    task->curlDesc.curlResultCode = CURLE_OK;
    task->curlDesc.curlResultErrorStr = NULL;
    task->curlDesc.httpResponseCode = 0;
    task->curlDesc.httpConnectCode = 0;
    task->responseSize = 0;

    if (sync) {
        if ((task->curlDesc.curlResultCode = curl_easy_perform(task->curlDesc.handle)) != CURLE_OK) {
            task->curlDesc.curlResultErrorStr = (char *)curl_easy_strerror(task->curlDesc.curlResultCode);
            LS_LOG_ERROR("curl easy perform: failed [%s]\n", task->curlDesc.curlResultErrorStr);

            return FALSE;
        }

        if ((curlRc = curl_easy_getinfo(task->curlDesc.handle,
                                        CURLINFO_RESPONSE_CODE,
                                        &(task->curlDesc.httpResponseCode))) != CURLE_OK)
            LS_LOG_WARNING("get info: CURLINFO_RESPONSE_CODE failed [%s]\n", curl_easy_strerror(curlRc));

        if ((curlRc = curl_easy_getinfo(task->curlDesc.handle,
                                        CURLINFO_HTTP_CONNECTCODE,
                                        &(task->curlDesc.httpConnectCode))) != CURLE_OK)
            LS_LOG_WARNING("get info: CURLINFO_HTTP_CONNECTCODE failed [%s]\n", curl_easy_strerror(curlRc));
    } else {
        if (gHttpTasks == NULL) {
            gHttpTasks = g_hash_table_new_full(g_direct_hash,
                                               g_direct_equal,
                                               NULL,
                                               NULL);
        }

        if ((curlMRc = loc_curl_add(task->curlDesc.handle)) != CURLM_OK) {
            LS_LOG_ERROR("loc_curl_add: failed [%s]\n", curl_multi_strerror(curlMRc));

            return FALSE;
        }

        g_hash_table_insert(gHttpTasks, task->curlDesc.handle, task);
    }

    return TRUE;
}

void loc_http_remove_request(HttpReqTask *task)
{
    if (!task)
        return;

    if (task->curlDesc.handle == NULL)
        return;

    loc_curl_remove(task->curlDesc.handle);

    if (gHttpTasks)
        g_hash_table_remove(gHttpTasks, task->curlDesc.handle);
}

static void cbLocCurl(void* data)
{
    CURLMsg* msg = NULL;
    CURLcode curlRc = CURLE_OK;
    int inQueue = 0;
    HttpReqTask *task = NULL;

    LS_LOG_DEBUG("cbLocCurl start\n");

    while (1) {
        msg = curl_multi_info_read(loc_curl_handle(), &inQueue);
        LS_LOG_DEBUG("msg = %p, inQueue = %d\n", msg, inQueue);
        if (msg == 0)
            break;

        if (msg->msg != CURLMSG_DONE)
            continue;

        LS_LOG_DEBUG("CURLMSG_DONE\n");
        if ((task = (HttpReqTask *)g_hash_table_lookup(gHttpTasks, msg->easy_handle)) != NULL) {
            if ((curlRc = curl_easy_getinfo(msg->easy_handle,
                                            CURLINFO_RESPONSE_CODE,
                                            &(task->curlDesc.httpResponseCode))) != CURLE_OK)
                LS_LOG_WARNING("get info: CURLINFO_RESPONSE_CODE failed [%s]\n", curl_easy_strerror(curlRc));

            if ((curlRc = curl_easy_getinfo(msg->easy_handle,
                                            CURLINFO_HTTP_CONNECTCODE,
                                            &(task->curlDesc.httpConnectCode))) != CURLE_OK)
                LS_LOG_WARNING("get info: CURLINFO_HTTP_CONNECTCODE failed [%s]\n", curl_easy_strerror(curlRc));

            task->curlDesc.curlResultCode = msg->data.result;
            if (msg->data.result != CURLE_OK)
                task->curlDesc.curlResultErrorStr = (char *)curl_easy_strerror(msg->data.result);

            if (gResponseCb)
                (*gResponseCb)(task, data);

        }
    }

    LS_LOG_DEBUG("cbLoccurl end\n");
}

static size_t cbWriteMemory(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    HttpReqTask *task = (HttpReqTask *)userdata;
    size_t realsize = size * nmemb;

    LS_LOG_DEBUG("cbWriteMemory called\n");

    task->responseData = (char *)realloc(task->responseData, task->responseSize + realsize + 1);
    if (task->responseData == NULL)
        return 0;

    memcpy(&(task->responseData[task->responseSize]), ptr, realsize);
    task->responseSize += realsize;
    task->responseData[task->responseSize] = 0;

    LS_LOG_DEBUG("mem: %s\n", task->responseData);

    return realsize;
}

