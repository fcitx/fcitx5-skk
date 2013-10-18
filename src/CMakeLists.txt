include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
include_directories(${LIBSKK_INCLUDE_DIRS})
link_directories(${LIBSKK_LIBRARY_DIRS})

set(fcitx_skk_sources
  skk.c skk-config.c)
fcitx_add_addon_full(skk
  DESC
  SOURCES ${fcitx_skk_sources}
  IM_CONFIG skk.conf
  LINK_LIBS ${LIBSKK_LIBRARIES}
  EXTRA_PO skk.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dictionary_list.in ${CMAKE_CURRENT_BINARY_DIR}/dictionary_list @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dictionary_list rule DESTINATION "${FCITX4_PREFIX}/share/fcitx/skk")
