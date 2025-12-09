cmake_minimum_required(VERSION 3.6.0)
project(fcitx5-skk VERSION 5.1.8)

set(REQUIRED_FCITX_VERSION 5.1.13)
find_package(ECM 1.0.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 
option(ENABLE_QT "Enable Qt for GUI configuration" On)

include(ECMUninstallTarget)
include(FeatureSummary)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Core ${REQUIRED_FCITX_VERSION} REQUIRED)
find_package(Gettext REQUIRED)
pkg_check_modules(GObject2 IMPORTED_TARGET "gobject-2.0" REQUIRED)
find_package(LibSKK REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

set(SKK_PATH "/usr/share/skk/" CACHE STRING "Default path of SKK directory that has SKK-JISYO.L, could start with \$XDG_DATA_DIRS/")
if("${SKK_DEFAULT_PATH}" MATCHES "SKK-JISYO.L$")
  message(FATAL_ERROR "SKK_DEFAULT_PATH is superseded by SKK_PATH")
endif()

include(GNUInstallDirs)

if (ENABLE_QT)
  set(QT_MAJOR_VERSION 6)
  find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
  find_package(Fcitx5Qt${QT_MAJOR_VERSION}WidgetsAddons REQUIRED)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  IMMEDIATE @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-skk\" -D_GNU_SOURCE)
add_definitions(-DQT_NO_KEYWORDS)
fcitx5_add_i18n_definition()


add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(gui)

fcitx5_translate_desktop_file(org.fcitx.Fcitx5.Addon.Skk.metainfo.xml.in
                              org.fcitx.Fcitx5.Addon.Skk.metainfo.xml XML)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.Fcitx5.Addon.Skk.metainfo.xml" DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
