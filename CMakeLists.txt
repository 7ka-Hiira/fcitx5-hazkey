cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazkey VERSION 0.0.4)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SKIP_ZENZAI_INSTALL "Skip downloading zenzai.gguf" OFF)
option(SKIP_KKC_PATH_UPDATE "Skip updating path.swift" OFF)

# check if vulkan headers are installed
find_package(Vulkan REQUIRED)

# ---- zenzai.gguf ---- #

set(FILE_URL "https://huggingface.co/Miwa-Keita/zenz-v1/resolve/main/ggml-model-Q8_0.gguf")

if(NOT SKIP_ZENZAI_INSTALL)
    file(DOWNLOAD ${FILE_URL}
        ${CMAKE_CURRENT_SOURCE_DIR}/zenzai.gguf
        EXPECTED_HASH SHA256=e11c68203e20bd28e155b9d8751def7e3a52bcee079e52df1145f5ce5265fd14
        SHOW_PROGRESS)
endif()

# ---- libhazkey ---- #

# build azookey-kkc library
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SWIFT_BUILD_TYPE "debug")
else()
    set(SWIFT_BUILD_TYPE "release")
endif()

if(NOT SKIP_KKC_PATH_UPDATE)
    message(STATUS "Updating path.swift")
    # write path.swift
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/Sources/azookey-kkc/path.swift "let systemResourecePath: String = \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/hazkey\"")
endif()

execute_process(
    COMMAND swift build -c ${SWIFT_BUILD_TYPE} -Xswiftc -static-stdlib 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc
)

# ---- fcitx5-hazkey ---- #

# check if built library exists
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/release/libhazkey.so)
    message(FATAL_ERROR "Failed to build libhazkey")
endif()

# ---- install ---- #

# install libhazkey.so
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/release/libhazkey.so
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(NOT SKIP_ZENZAI_INSTALL)
    # install zenzai.gguf
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/zenzai.gguf
            DESTINATION ${CMAKE_INSTALL_DATADIR}/hazkey)
endif()

# install dictionary
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azooKey_dictionary_storage/Dictionary
        DESTINATION ${CMAKE_INSTALL_DATADIR}/hazkey)

# build fcitx5-hazkey
add_subdirectory(fcitx5)
