# @@@LICENSE
#
#   Copyright (c) 2015 LG Electronics Inc. All Rights Reserved
#
# LICENSE@@@

set(LOC_UTILS_NAME loc_utils)
set(LOC_UTILS_PATH ${CMAKE_INSTALL_PREFIX}/lib/)
set(LOC_UTILS_SRC loc_filter.c
                  loc_geometry.c
                  loc_http.c
                  loc_logger.c
                  loc_security.c)

set(LIBRARIES ${GLIB2_LDFLAGS} ${LIBCURL_LDFLAGS} ${GLIBCURL_LDFLAGS} ${PMLOGLIB_LDFLAGS} ${OPENSSL_LDFLAGS})
add_library(${LOC_UTILS_NAME} SHARED ${LOC_UTILS_SRC})
target_link_libraries(${LOC_UTILS_NAME} ${LIBRARIES})
set_target_properties(${LOC_UTILS_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS ${LOC_UTILS_NAME} DESTINATION ${LOC_UTILS_PATH})
