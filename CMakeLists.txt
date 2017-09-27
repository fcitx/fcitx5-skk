cmake_minimum_required(VERSION 3.4.0)
project(fcitx5-skk VERSION 0.0.1)

find_package(ECM REQUIRED 1.0.0)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 
option(ENABLE_QT "Enable Qt for GUI configuration" On)

include(ECMUninstallTarget)
include(FeatureSummary)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Core REQUIRED)
pkg_check_modules(GObject2 IMPORTED_TARGET "gobject-2.0" REQUIRED)
find_package(LibSKK REQUIRED)

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

option(SKK_DEFAULT_PATH "Default path of SKK" "/usr/share/skk/SKK-JISYO.L")

include(GNUInstallDirs)

#if (ENABLE_QT)
#find_package(Qt 4.8 REQUIRED)
#pkg_check_modules(FCITX_QT "fcitx-qt>=4.2.8" REQUIRED)
#endif ()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  IMMEDIATE @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#add_subdirectory(po)
add_subdirectory(src)
#add_subdirectory(data)
#add_subdirectory(gui)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
