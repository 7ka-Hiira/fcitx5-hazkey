cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazkey VERSION 0.0.3)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# check if vulkan headers are installed
find_package(Vulkan REQUIRED)

# build azookey-kkc library
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SWIFT_BUILD_TYPE "debug")
else()
    set(SWIFT_BUILD_TYPE "release")
endif()
execute_process(
    COMMAND swift build -c ${SWIFT_BUILD_TYPE} -Xswiftc -static-stdlib
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
        DESTINATION ${CMAKE_INSTALL_BINDIR})

# build fcitx5-hazkey
add_subdirectory(fcitx5)
