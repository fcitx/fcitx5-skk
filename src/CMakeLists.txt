include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
include_directories(${PROJECT_BINARY_DIR})
include_directories(${LIBSKK_INCLUDE_DIRS})

set(fcitx_skk_sources
  skk.c)
fcitx_add_addon_full(skk
  SOURCES ${fcitx_skk_sources}
  IM_CONFIG skk.conf
  LINK_LIBS ${LIBSKK_LIBRARIES}
  EXTRA_PO skk.h)
