# @@@LICENSE
#
#   Copyright (c) 2015 LG Electronics Inc. All Rights Reserved
#
# LICENSE@@@

cmake_minimum_required(VERSION 2.8.7)
project(loc-utils)

include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC4)
#webos_component(1 0 0)

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
add_definitions(${GLIB2_CFLAGS})

pkg_check_modules(LIBCURL REQUIRED libcurl)
add_definitions(${LIBCURL_CFLAGS})

pkg_check_modules(GLIBCURL REQUIRED glibcurl)
add_definitions(${GLIBCURL_CFLAGS})

pkg_check_modules(OPENSSL REQUIRED openssl)
add_definitions(${OPENSSL_CFLAGS})

pkg_check_modules(PMLOGLIB REQUIRED PmLogLib)
if (${PMLOGLIB_VERSION} VERSION_LESS 3.0.0)
    set(PMLOG_USAGE_CFLAGS -DPMLOG_USE_DEPRECATED)
else ()
    webos_build_configured_file(files/conf/pmlog/location.conf SYSCONFDIR pmlog.d)
endif ()
add_definitions(${PMLOGLIB_CFLAGS})
add_definitions(${PMLOG_USAGE_CFLAGS})

include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(src)

webos_build_pkgconfig(files/pkgconfig/loc-utils)

install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/loc-utils
        FILES_MATCHING PATTERN "*.h")
