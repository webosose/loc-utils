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
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <loc_logger.h>

#define MAX_DIR_PATH            64
#define MAX_TITLE_NAME          128
#define DEFAULT_LOG_DIR         "/media/external"
#define DEFAULT_LOG_TITLE       "unnamed"

struct _data_logger_t {
    FILE *log_handle;
    int max_rotation;
    int max_size;
    int rotation;
    char directory[MAX_DIR_PATH];
    char title_name[MAX_TITLE_NAME];
};


void rotate_file(data_logger_t *logger)
{
    char file_old[256];
    char file_new[256];

    if (!logger)
        return;

    if (logger->rotation >= logger->max_rotation) {
        logger->rotation = 1;

        while (logger->rotation < logger->max_rotation) {
            sprintf(file_old, "%s/%s.%d", logger->directory, logger->title_name, logger->rotation);
            sprintf(file_new, "%s/%s.%d", logger->directory, logger->title_name, logger->rotation - 1);

            rename(file_old, file_new);

            logger->rotation++;
        }

        logger->rotation--;
    }

    if (logger->log_handle) {
        fclose(logger->log_handle);
        logger->log_handle = NULL;
    }

    sprintf(file_old, "%s/%s", logger->directory, logger->title_name);
    sprintf(file_new, "%s/%s.%d", logger->directory, logger->title_name, logger->rotation);

    rename(file_old, file_new);

    sprintf(file_new, "%s/%s", logger->directory, logger->title_name);
    logger->log_handle = fopen(file_new, "a");

    logger->rotation++;
}

data_logger_t* loc_logger_create()
{
    data_logger_t *logger = NULL;

    logger = (data_logger_t *)malloc(sizeof(data_logger_t));
    if (logger) {
        memset(logger, 0, sizeof(data_logger_t));
    }

    return logger;
}

void loc_logger_destroy(data_logger_t **logger_ref)
{
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    loc_logger_stop_logging(logger_ref);

    free(logger);
    logger = NULL;
}

void loc_logger_start_logging(data_logger_t **logger_ref,
                              const char *directory,
                              const char *log_title)
{
    time_t rawtime;
    struct tm *timeinfo;
    char file_path[256];
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    loc_logger_stop_logging(logger_ref);

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    memset(logger->directory, 0, sizeof(char)*MAX_DIR_PATH);
    memset(logger->title_name, 0, sizeof(char)*MAX_TITLE_NAME);
    strcpy(logger->directory, directory ? directory : DEFAULT_LOG_DIR);
    strcpy(logger->title_name, log_title ? log_title : DEFAULT_LOG_TITLE);

    sprintf(file_path, "%s/%s_%04d-%02d-%02d_%02d-%02d-%02d.log",
            logger->directory,
            logger->title_name,
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
            timeinfo->tm_mday, timeinfo->tm_hour,
            timeinfo->tm_min, timeinfo->tm_sec);

    logger->log_handle = fopen(file_path, "w");
}

void loc_logger_start_logging_with_rotation(data_logger_t **logger_ref,
                                            const char *directory,
                                            const char *log_title,
                                            int max_rotation,
                                            int max_size)
{
    char file_path[256];
    int current_rotation;
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    loc_logger_stop_logging(logger_ref);

    memset(logger->directory, 0, sizeof(char)*MAX_DIR_PATH);
    memset(logger->title_name, 0, sizeof(char)*MAX_TITLE_NAME);
    strcpy(logger->directory, directory ? directory : DEFAULT_LOG_DIR);
    strcpy(logger->title_name, log_title ? log_title : DEFAULT_LOG_TITLE);
    logger->max_rotation = max_rotation;
    logger->max_size = max_size;

    current_rotation = 0;
    while (current_rotation < max_rotation) {
        sprintf(file_path, "%s/%s.%d", logger->directory, logger->title_name, current_rotation);

        if (access(file_path, R_OK) != 0)
            break;

        current_rotation++;
    }

    logger->rotation = current_rotation;

    sprintf(file_path, "%s/%s", logger->directory, logger->title_name);
    logger->log_handle = fopen(file_path, "a");
}

void loc_logger_stop_logging(data_logger_t **logger_ref)
{
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    if (logger->log_handle) {
        fclose(logger->log_handle);
        logger->log_handle = NULL;
    }

    memset(logger->directory, 0, sizeof(char)*MAX_DIR_PATH);
    memset(logger->title_name, 0, sizeof(char)*MAX_TITLE_NAME);
    logger->max_rotation = 0;
    logger->max_size = 0;
    logger->rotation = 0;
}

void loc_logger_feed_data(data_logger_t **logger_ref, char *log_data, int size)
{
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    if (!log_data || !logger->log_handle)
        return;

    fwrite(log_data, size, 1, logger->log_handle);
    fflush(logger->log_handle);
}

void loc_logger_feed_log(data_logger_t **logger_ref, char *log, int size)
{
    time_t rawtime;
    struct tm *timeinfo;
    long int file_size = 0;
    char time_info[256];
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    if (!log || !logger->log_handle)
        return;

    if (logger->max_rotation > 0 && logger->max_size > 0) {
        file_size = ftell(logger->log_handle);

        if (file_size + size > logger->max_size)
            rotate_file(logger);
    }

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(time_info, "[%04d-%02d-%02d_%02d:%02d:%02d] ",
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
            timeinfo->tm_mday, timeinfo->tm_hour,
            timeinfo->tm_min, timeinfo->tm_sec);

    fwrite(time_info, strlen(time_info), 1, logger->log_handle);
    fwrite(log, size, 1, logger->log_handle);
    fflush(logger->log_handle);
}

