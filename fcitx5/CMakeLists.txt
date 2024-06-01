cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazukey VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
include(GNUInstallDirs)
message("Install prefix: ${CMAKE_INSTALL_PREFIX}")

find_package(Fcitx5Core REQUIRED)
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBHAZUKEY REQUIRED IMPORTED_TARGET hazukey)

# Localization
find_package(Gettext REQUIRED)

add_subdirectory(src)
add_subdirectory(po)

fcitx5_translate_desktop_file(org.fcitx.Fcitx5.Addon.Hazukey.metainfo.xml.in
                              org.fcitx.Fcitx5.Addon.Hazukey.metainfo.xml XML)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.Fcitx5.Addon.Hazukey.metainfo.xml" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/metainfo")
