/*
 * Copyright (c) 2012-2014 LG Electronics Inc. All Rights Reserved.
 */
#ifndef _LOC_LOG_H_
#define _LOC_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <PmLogLib.h>

#define LS_LOG_MSG_ID    "location"

extern PmLogContext gLsLogContext;

#ifdef PMLOG_USE_DEPRECATED

#define LS_LOG_HELPER(pmLogLevel__, ...) \
    PmLogPrint(gLsLogContext, (pmLogLevel__), __VA_ARGS__)
#define LS_LOG_DEBUG(...) \
    LS_LOG_HELPER(kPmLogLevel_Debug, __VA_ARGS__)
#define LS_LOG_INFO(...) \
    LS_LOG_HELPER(kPmLogLevel_Info, __VA_ARGS__)
#define LS_LOG_WARNING(...) \
    LS_LOG_HELPER(kPmLogLevel_Warning, __VA_ARGS__)
#define LS_LOG_ERROR(...) \
    LS_LOG_HELPER(kPmLogLevel_Error, __VA_ARGS__)
#define LS_LOG_CRITICAL(...) \
    LS_LOG_HELPER(kPmLogLevel_Critical, __VA_ARGS__)

#else

#define LS_LOG_DEBUG(...) \
    PmLogDebug(gLsLogContext, __VA_ARGS__)
#define LS_LOG_INFO(...) \
    PmLogInfo(gLsLogContext, LS_LOG_MSG_ID, 0, __VA_ARGS__)
#define LS_LOG_WARNING(...) \
    PmLogWarning(gLsLogContext, LS_LOG_MSG_ID, 0, __VA_ARGS__)
#define LS_LOG_ERROR(...) \
    PmLogError(gLsLogContext, LS_LOG_MSG_ID, 0, __VA_ARGS__)
#define LS_LOG_CRITICAL(...) \
    PmLogCritical(gLsLogContext, LS_LOG_MSG_ID, 0, __VA_ARGS__)

#endif

#ifdef __cplusplus
}
#endif

#endif // _LOC_LOG_H_
