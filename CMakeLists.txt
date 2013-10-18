cmake_minimum_required(VERSION 2.8.6)
project(fcitx-skk)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
option(ENABLE_QT "Enable Qt for GUI configuration" On)

find_package(Fcitx 4.2.8 REQUIRED)
include(FindPkgConfig)
pkg_check_modules(LIBSKK "libskk" REQUIRED)
_fcitx_add_uninstall_target()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")

if (NOT SKK_DEFAULT_PATH)
    set(SKK_DEFAULT_PATH "/usr/share/skk/SKK-JISYO.L")
endif ()

if(NOT DEFINED LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

if(NOT DEFINED LOCALEDIR)
  set(LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
endif()

if (ENABLE_QT)
find_package(Qt 4.8 REQUIRED)
pkg_check_modules(FCITX_QT "fcitx-qt>=4.2.8" REQUIRED)
endif ()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  IMMEDIATE @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(gui)
