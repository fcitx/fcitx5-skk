#.rst:
# FindLibSKK
# -----------
#
# Try to find the LibSKK library
#
# Once done this will define
#
# ::
#
#   LIBSKK_FOUND - System has LibSKK
#   LIBSKK_INCLUDE_DIR - The LibSKK include directory
#   LIBSKK_LIBRARIES - The libraries needed to use LibSKK
#   LIBSKK_DEFINITIONS - Compiler switches required for using LibSKK
#   LIBSKK_VERSION_STRING - the version of LibSKK found (since CMake 2.8.8)

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls
find_package(PkgConfig QUIET)
pkg_check_modules(PC_LibSKK QUIET libskk)

find_path(LIBSKK_INCLUDE_DIR NAMES libskk/libskk.h
   HINTS
   ${PC_LIBSKK_INCLUDEDIR}
   ${PC_LIBSKK_INCLUDE_DIRS}
   )

find_library(LIBSKK_LIBRARIES NAMES skk
   HINTS
   ${PC_LIBSKK_LIBDIR}
   ${PC_LIBSKK_LIBRARY_DIRS}
   )

if(PC_LIBSKK_VERSION)
    set(LIBSKK_VERSION_STRING ${PC_LIBSKK_VERSION})
endif()

# handle the QUIETLY and REQUIRED arguments and set LIBSKK_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSKK
                                  REQUIRED_VARS LIBSKK_LIBRARIES LIBSKK_INCLUDE_DIR
                                  VERSION_VAR LIBSKK_VERSION_STRING)

mark_as_advanced(LIBSKK_INCLUDE_DIR LIBSKK_LIBRARIES)

if (LIBSKK_FOUND AND NOT TARGET LibSKK::LibSKK)
    add_library(LibSKK::LibSKK UNKNOWN IMPORTED)
    set_target_properties(LibSKK::LibSKK PROPERTIES
        IMPORTED_LOCATION "${LIBSKK_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBSKK_INCLUDE_DIR}"
    )
endif()

