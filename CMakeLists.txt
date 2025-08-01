cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazkey VERSION 0.0.9)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SKIP_ZENZAI_INSTALL "Skip downloading zenzai.gguf" OFF)
option(SKIP_KKC_PATH_UPDATE "Skip updating path.swift" OFF)

# check if vulkan headers are installed
find_package(Vulkan REQUIRED)
find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-hazkey\")

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
    COMMAND cmake -B build -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS=off -DCMAKE_POSITION_INDEPENDENT_CODE=on -DGGML_VULKAN=on -DLLAMA_BUILD_TESTS=OFF -DLLAMA_USE_SYSTEM_GGML=OFF -DGGML_BUILD_EXAMPLES=OFF
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp
)

execute_process(
    COMMAND cmake --build build --config ${CMAKE_BUILD_TYPE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp
)

# -- hazkey_server -- #

# build hazkey_server
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SWIFT_BUILD_TYPE "debug")
else()
    set(SWIFT_BUILD_TYPE "release")
endif()

if(NOT SKIP_KKC_PATH_UPDATE)
    message(STATUS "Updating path.swift")
    # write path.swift
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/hazkey_server/Sources/hazkey_server/path.swift "let systemResourcePath: String = \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/hazkey\"")
endif()

execute_process(
    # The third and subsequent -L options are temporary workarounds. You may need to rewrite them depending on the location of your Swift libraries.
    COMMAND swift build -c ${SWIFT_BUILD_TYPE} -Xswiftc -static-stdlib -Xlinker -L/usr/lib/swift/linux -Xlinker -L/usr/lib/swift/lib/swift/linux -Xlinker -L/usr/local/lib/swift/linux
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hazkey_server
)

# check if server binary exists
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hazkey_server/.build/${SWIFT_BUILD_TYPE}/hazkey_server)
    message(FATAL_ERROR "Failed to build hazkey_server")
endif()

# ---- install ---- #

# install hazkey_server
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/hazkey_server/.build/${SWIFT_BUILD_TYPE}/hazkey_server
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/hazkey)

if(NOT SKIP_ZENZAI_INSTALL)
    # install zenzai.gguf
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/zenzai.gguf
            DESTINATION ${CMAKE_INSTALL_DATADIR}/hazkey)
endif()

# install dictionary
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azooKey_dictionary_storage/Dictionary
        DESTINATION ${CMAKE_INSTALL_DATADIR}/hazkey)

# build fcitx5-hazkey
add_subdirectory(fcitx5-hazkey)
