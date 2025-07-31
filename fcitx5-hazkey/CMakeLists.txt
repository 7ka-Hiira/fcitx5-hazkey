include(GNUInstallDirs)

find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5Utils REQUIRED)
find_package(Fcitx5Config REQUIRED)
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

fcitx5_add_i18n_definition()

find_package(Gettext REQUIRED)

add_subdirectory(po)
add_subdirectory(src)

fcitx5_translate_desktop_file(org.fcitx.Fcitx5.Addon.Hazkey.metainfo.xml.in
                              org.fcitx.Fcitx5.Addon.Hazkey.metainfo.xml XML)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.Fcitx5.Addon.Hazkey.metainfo.xml" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/metainfo")
