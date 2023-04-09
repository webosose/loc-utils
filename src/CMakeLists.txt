# Copyright (c) 2020 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

set(LOC_UTILS_NAME loc_utils)
set(LOC_UTILS_SRC loc_curl.c
                  loc_filter.c
                  loc_geometry.c
                  loc_http.c
                  loc_logger.c
                  loc_security.c)

set(LIBRARIES ${GLIB2_LDFLAGS} ${LIBCURL_LDFLAGS} ${PMLOGLIB_LDFLAGS} -lcrypt)
add_library(${LOC_UTILS_NAME} SHARED ${LOC_UTILS_SRC})
target_link_libraries(${LOC_UTILS_NAME} ${LIBRARIES})
set_target_properties(${LOC_UTILS_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS ${LOC_UTILS_NAME} DESTINATION ${WEBOS_INSTALL_LIBDIR})
