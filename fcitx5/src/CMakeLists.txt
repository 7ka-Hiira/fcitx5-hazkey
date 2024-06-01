add_library(fcitx5-hazukey SHARED hazukey.cpp hazukey_candidate.cpp hazukey_state.cpp hazukey_config.cpp hazukey_preedit.cpp)
target_link_libraries(fcitx5-hazukey PRIVATE Fcitx5::Core Fcitx5::Config PkgConfig::LIBHAZUKEY)

set_target_properties(fcitx5-hazukey PROPERTIES PREFIX "")
install(TARGETS fcitx5-hazukey DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

configure_file(hazukey-addon.conf.in.in hazukey-addon.conf.in @ONLY)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/hazukey-addon.conf.in" "hazukey-addon.conf")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hazukey-addon.conf" RENAME hazukey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

fcitx5_translate_desktop_file(hazukey-im.conf.in hazukey-im.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hazukey-im.conf" RENAME hazukey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
