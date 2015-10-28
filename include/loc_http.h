/*
 * Copyright (c) 2015 LG Electronics Inc. All Rights Reserved.
 */
#ifndef _LOC_HTTP_H_
#define _LOC_HTTP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <glibcurl.h>

#define HTTP_STATUS_CODE_SUCCESS    200

typedef struct {
    CURL *handle;
    struct curl_slist *headerList;
    CURLcode curlResultCode;
    char *curlResultErrorStr;
    long httpResponseCode;
    long httpConnectCode;
} CurlDesc;

typedef struct {
    CurlDesc curlDesc;
    char *post_data;
    size_t responseSize;
    char *responseData;
    void *message;
    void *recepient;
} HttpReqTask;

typedef void (*ResponseCallback)(HttpReqTask *task, void *user_data);


// create a HttpReqTask object
HttpReqTask *loc_http_task_create(const char **headers, int size);

// create a HttpReqTask object new implementation for new design
HttpReqTask *loc_create_http_task(const char **headers, int size, void *message, void *userdata);

// destroy the given request task
void loc_http_task_destroy(HttpReqTask **task_ref);

// prepare curl connection with the given url
gboolean loc_http_task_prepare_connection(HttpReqTask **task_ref, char *url);

// start http utility
void loc_http_start();

// stop http utility
void loc_http_stop();

// set response callback which will be called when an added request task is done
void loc_http_set_callback(ResponseCallback response_cb, void *user_data);

// add request task
gboolean loc_http_add_request(HttpReqTask *task, gboolean sync);

// remove request task
void loc_http_remove_request(HttpReqTask *task);



#ifdef __cplusplus
}
#endif

#endif
