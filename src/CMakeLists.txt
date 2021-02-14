
set(SKK_SOURCES
    skk.cpp
)
add_library(skk MODULE ${SKK_SOURCES})
target_link_libraries(skk
    Fcitx5::Core
    Fcitx5::Config
    PkgConfig::GObject2
    LibSKK::LibSKK
)
set_target_properties(skk PROPERTIES PREFIX "")
install(TARGETS skk DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")
fcitx5_translate_desktop_file(skk.conf.in skk.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/skk.conf" DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/inputmethod")
fcitx5_translate_desktop_file(skk-addon.conf.in skk-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/skk-addon.conf" RENAME skk.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dictionary_list.in ${CMAKE_CURRENT_BINARY_DIR}/dictionary_list @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dictionary_list DESTINATION "${FCITX_INSTALL_PKGDATADIR}/skk")
