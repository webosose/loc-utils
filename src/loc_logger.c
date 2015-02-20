/*
 * Copyright (c) 2015 LG Electronics Inc. All Rights Reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <loc_logger.h>

#define MAX_PATH_LOG_FILE       256
#define DEFAULT_LOG_DIR         "/media/external"
#define DEFAULT_LOG_TITLE       "unnamed"

struct _data_logger_t {
    FILE *log_handle;
    char file_path[MAX_PATH_LOG_FILE];
};



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
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    loc_logger_stop_logging(logger_ref);

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(logger->file_path, "%s/%s_%04d-%02d-%02d_%02d-%02d-%02d.log",
            (directory ? directory : DEFAULT_LOG_DIR),
            (log_title ? log_title : DEFAULT_LOG_TITLE),
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
            timeinfo->tm_mday, timeinfo->tm_hour,
            timeinfo->tm_min, timeinfo->tm_sec);

    logger->log_handle = fopen(logger->file_path, "w");
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

    memset(logger->file_path, 0, MAX_PATH_LOG_FILE);
}

void loc_logger_feed_data(data_logger_t **logger_ref, char *log_data, int size)
{
    data_logger_t *logger = *logger_ref;

    if (!logger)
        return;

    if (!log_data || !logger->log_handle)
        return;

    fwrite(log_data, size, 1, logger->log_handle);
}

