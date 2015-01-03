/*
 * Copyright (c) 2012-2014 LG Electronics Inc. All Rights Reserved.
 */

#ifndef _LOC_LOGGER_H_
#define _LOC_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _data_logger_t        data_logger_t;


data_logger_t* loc_logger_create();

void loc_logger_destroy(data_logger_t **logger_ref);

void loc_logger_start_logging(data_logger_t **logger_ref,
                              const char *directory,
                              const char *log_title);

void loc_logger_stop_logging(data_logger_t **logger_ref);

void loc_logger_feed_data(data_logger_t **logger_ref, char *log_data, int size);



#ifdef __cplusplus
}
#endif

#endif
