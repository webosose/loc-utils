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

#ifndef _LOC_SECURITY_H_
#define _LOC_SECURITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOC_SECURITY_ERROR_SUCCESS      0
#define LOC_SECURITY_ERROR_FAILURE      -1


// base64 - Decryption
int locSecurityBase64Decode(const char *file_path, unsigned char **decrypted);



#ifdef __cplusplus
}
#endif

#endif
