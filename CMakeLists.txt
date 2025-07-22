cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazkey VERSION 0.0.9)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SKIP_ZENZAI_INSTALL "Skip downloading zenzai.gguf" OFF)
option(SKIP_KKC_PATH_UPDATE "Skip updating path.swift" OFF)

# check if vulkan headers are installed
find_package(Vulkan REQUIRED)

# ---- zenzai.gguf ---- #
set(FILE_URL "https://huggingface.co/Miwa-Keita/zenz-v3-small-gguf/resolve/main/ggml-model-Q5_K_M.gguf")

if(NOT SKIP_ZENZAI_INSTALL)
    file(DOWNLOAD ${FILE_URL}
        ${CMAKE_CURRENT_SOURCE_DIR}/zenzai.gguf
        EXPECTED_HASH SHA256=501f605d088f5b988791a00ae19ed46985ed7c48144f364b2f3f1f951c9b2083
        SHOW_PROGRESS)
endif()

# ---- llama.cpp ---- #

execute_process(
    COMMAND cmake -B build -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS=off -DCMAKE_POSITION_INDEPENDENT_CODE=on -DGGML_VULKAN=on
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp
)

execute_process(
    COMMAND cmake --build build --config ${CMAKE_BUILD_TYPE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp
)

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
    # The third and subsequent -L options are temporary workarounds. You may need to rewrite them depending on the location of your Swift libraries.
    COMMAND swift build -c ${SWIFT_BUILD_TYPE} -Xswiftc -static-stdlib -Xlinker -L/usr/lib/swift/linux -Xlinker -L/usr/lib/swift/lib/swift/linux -Xlinker -L/usr/local/lib/swift/linux
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc
)

# ---- fcitx5-hazkey ---- #

# check if built library exists
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/${SWIFT_BUILD_TYPE}/libhazkey.so)
    message(FATAL_ERROR "Failed to build libhazkey")
endif()

# ---- install ---- #

# install libhazkey.so
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/${SWIFT_BUILD_TYPE}/libhazkey.so
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
