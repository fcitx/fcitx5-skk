cmake_minimum_required(VERSION 3.4.0)
project(fcitx5-skk VERSION 5.0.4)

find_package(ECM 1.0.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 
option(ENABLE_QT "Enable Qt for GUI configuration" On)

include(ECMUninstallTarget)
include(FeatureSummary)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Core REQUIRED)
find_package(Gettext REQUIRED)
pkg_check_modules(GObject2 IMPORTED_TARGET "gobject-2.0" REQUIRED)
find_package(LibSKK REQUIRED)

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

set(SKK_DEFAULT_PATH "/usr/share/skk/SKK-JISYO.L" CACHE STRING "Default path of SKK")

include(GNUInstallDirs)

if (ENABLE_QT)
find_package(Qt5 5.7 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Fcitx5Qt5WidgetsAddons REQUIRED)
endif ()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  IMMEDIATE @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-skk\" -D_GNU_SOURCE)
fcitx5_add_i18n_definition()


add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(gui)

fcitx5_translate_desktop_file(org.fcitx.Fcitx5.Addon.Skk.metainfo.xml.in
                              org.fcitx.Fcitx5.Addon.Skk.metainfo.xml XML)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.Fcitx5.Addon.Skk.metainfo.xml" DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
