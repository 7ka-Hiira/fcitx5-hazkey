configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/env_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/env_config.h
)

add_library(fcitx5-hazkey SHARED hazkey_state.cpp hazkey_engine.cpp hazkey_candidate.cpp hazkey_preedit.cpp hazkey_server_connector.cpp protocol/hazkey_server.pb.cc)

target_include_directories(fcitx5-hazkey PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(fcitx5-hazkey PRIVATE Fcitx5::Core Fcitx5::Config protobuf ${Protobuf_LIBRARIES} absl::log_internal_message absl::log_internal_check_op)


set_target_properties(fcitx5-hazkey PROPERTIES PREFIX "")
install(TARGETS fcitx5-hazkey DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

configure_file(hazkey-addon.conf.in.in hazkey-addon.conf.in @ONLY)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/hazkey-addon.conf.in" "hazkey-addon.conf")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hazkey-addon.conf" RENAME hazkey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

fcitx5_translate_desktop_file(hazkey-im.conf.in hazkey-im.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hazkey-im.conf" RENAME hazkey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
