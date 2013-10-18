set(SKK_CONFIG_SRCS
  main.cpp
  dictwidget.cpp
  adddictdialog.cpp
  dictmodel.cpp
  rulemodel.cpp
  )
fcitx_translate_add_sources(
  ${SKK_CONFIG_SRCS})

if(NOT ENABLE_QT)
  return()
endif()

include_directories(${LIBSKK_INCLUDE_DIRS})
include_directories(
  ${FCITX_QT_INCLUDE_DIRS}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

link_directories(
  ${FCITX_QT_LIBRARY_DIRS}
  ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
  ${LIBSKK_LIBRARY_DIRS}
  )
qt4_wrap_ui(SKK_CONFIG_SRCS dictwidget.ui adddictdialog.ui)
add_library(fcitx-skk-config
  MODULE ${SKK_CONFIG_SRCS})

add_definitions(-DQT_NO_KEYWORDS -std=c++11)

set_target_properties(fcitx-skk-config PROPERTIES
                      LINK_FLAGS "-Wl,--no-undefined"
                      AUTOMOC TRUE
)
target_link_libraries(fcitx-skk-config
  debug ${QT_QTCORE_LIBRARY_DEBUG}
  debug ${QT_QTDBUS_LIBRARY_DEBUG}
  debug ${QT_QTGUI_LIBRARY_DEBUG}
  optimized ${QT_QTCORE_LIBRARY_RELEASE}
  optimized ${QT_QTDBUS_LIBRARY_RELEASE}
  optimized ${QT_QTGUI_LIBRARY_RELEASE}
  ${FCITX_QT_LIBRARIES}
  ${FCITX4_FCITX_CONFIG_LIBRARIES}
  ${LIBSKK_LIBRARIES}
  )

install(TARGETS fcitx-skk-config DESTINATION ${FCITX4_ADDON_INSTALL_DIR}/qt)
