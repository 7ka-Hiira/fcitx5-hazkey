add_library(fcitx5-hazkey SHARED hazkey_state.cpp hazkey_engine.cpp)

#link_directories(${CMAKE_BINARY_DIR}/fcitx5-hazkey/.build/${SWIFT_BUILD_TYPE})

# TODO: fix swift library path
target_link_libraries(fcitx5-hazkey PRIVATE Fcitx5::Core Fcitx5::Config "${CMAKE_SOURCE_DIR}/fcitx5-hazkey/.build/${SWIFT_BUILD_TYPE}/libfcitx5Hazkey.a" "/usr/lib/swift/lib/swift_static/linux/libswiftCore.a" "/usr/lib/swift/lib/swift_static/linux/libswiftSwiftOnoneSupport.a")


set_target_properties(fcitx5-hazkey PROPERTIES PREFIX "")
install(TARGETS fcitx5-hazkey DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

configure_file(hazkey-addon.conf.in.in hazkey-addon.conf.in @ONLY)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/hazkey-addon.conf.in" "hazkey-addon.conf")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hazkey-addon.conf" RENAME hazkey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

fcitx5_translate_desktop_file(hazkey-im.conf.in hazkey-im.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hazkey-im.conf" RENAME hazkey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
