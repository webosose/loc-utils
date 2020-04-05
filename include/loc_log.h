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
