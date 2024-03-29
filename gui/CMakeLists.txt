set(SKK_CONFIG_SRCS
  main.cpp
  dictwidget.cpp
  adddictdialog.cpp
  dictmodel.cpp
  )

if(NOT ENABLE_QT)
  return()
endif()

add_library(fcitx5-skk-config
  MODULE ${SKK_CONFIG_SRCS})

set_target_properties(fcitx5-skk-config PROPERTIES
                      AUTOMOC TRUE
                      AUTOUIC TRUE
                      AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_link_libraries(fcitx5-skk-config
  Qt${QT_MAJOR_VERSION}::Core
  Qt${QT_MAJOR_VERSION}::Widgets
  Fcitx5Qt${QT_MAJOR_VERSION}::WidgetsAddons
  Fcitx5::Utils
  PkgConfig::GObject2
  LibSKK::LibSKK
  )

install(TARGETS fcitx5-skk-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/fcitx5/qt${QT_MAJOR_VERSION})
