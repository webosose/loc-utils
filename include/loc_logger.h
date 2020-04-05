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

void loc_logger_start_logging_with_rotation(data_logger_t **logger_ref,
                                            const char *directory,
                                            const char *log_title,
                                            int max_rotation,
                                            int max_size);

void loc_logger_stop_logging(data_logger_t **logger_ref);

void loc_logger_feed_data(data_logger_t **logger_ref, char *log_data, int size);

void loc_logger_feed_log(data_logger_t **logger_ref, char *log, int size);


#ifdef __cplusplus
}
#endif

#endif
