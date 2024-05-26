add_library(fcitx5-azookey SHARED azookey.cpp azookey_candidate.cpp azookey_state.cpp azookey_config.cpp)
target_link_libraries(fcitx5-azookey PRIVATE Fcitx5::Core Fcitx5::Config PkgConfig::LIBAZOOKEY)

set_target_properties(fcitx5-azookey PROPERTIES PREFIX "")
install(TARGETS fcitx5-azookey DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

configure_file(azookey-addon.conf.in.in azookey-addon.conf.in @ONLY)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/azookey-addon.conf.in" "azookey-addon.conf")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/azookey-addon.conf" RENAME azookey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

fcitx5_translate_desktop_file(azookey-im.conf.in azookey-im.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/azookey-im.conf" RENAME azookey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
