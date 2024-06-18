cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazkey VERSION 0.0.3)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)

# build azookey-kkc library
execute_process(
    COMMAND swift build -c release -Xswiftc -static-stdlib
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc
)

# check if built library exists
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/release/libhazkey.so)
    message(FATAL_ERROR "Failed to build libhazkey")
endif()

# install libhazkey.so
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/release/libhazkey.so
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install dictionary
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/release/AzooKeyKanakanjiConverter_KanaKanjiConverterModuleWithDefaultDictionary.resources
        DESTINATION ${CMAKE_INSTALL_BINDIR}/AzooKeyKanakanjiConverter_KanaKanjiConverterModuleWithDefaultDictionary.resources)

# build fcitx5-hazkey
add_subdirectory(fcitx5)