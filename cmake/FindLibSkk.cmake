# - Try to find the LibSkk libraries
# Once done this will define
#
#  LIBSKK_FOUND - system has LIBSKK
#  LIBSKK_INCLUDE_DIRS - the LIBSKK include directory
#  LIBSKK_LIBRARIES - LIBSKK library
#
# Copyright (c) 2013 Yichao Yu <yyc1992@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(LIBSKK_INCLUDE_DIR AND LIBSKK_LIBRARIES)
  # Already in cache, be silent
  set(LIBSKK_FIND_QUIETLY TRUE)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSKK "libskk")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSkk
  REQUIRED_VARS LIBSKK_LIBRARIES LIBSKK_INCLUDE_DIRS LIBSKK_FOUND
  VERSION_VAR LIBSKK_VERSION)

mark_as_advanced(LIBSKK_INCLUDE_DIRS LIBSKK_LIBRARIES)
