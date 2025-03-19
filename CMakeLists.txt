cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazkey VERSION 0.0.9)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER clang++)

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

# ---- libhazkey ---- #

# build fcitx5-hazkey swift library
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SWIFT_BUILD_TYPE "debug")
else()
    set(SWIFT_BUILD_TYPE "release")
endif()

if(NOT SKIP_KKC_PATH_UPDATE)
    message(STATUS "Updating path.swift")
    # write path.swift
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/fcitx5-hazkey/Sources/fcitx5Hazkey/path.swift "let systemResourecePath: String = \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/hazkey\"")
endif()

execute_process(
    # TODO: do not use absolute path
    COMMAND swift build -c ${SWIFT_BUILD_TYPE} -Xswiftc -emit-clang-header-path -Xswiftc libfcitx5Hazkey.h -Xcc -I/usr/include/Fcitx5/Core -Xcc -I/usr/include/Fcitx5/Config -Xcc -I/usr/include/Fcitx5/Utils
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/fcitx5-hazkey
)

# ---- fcitx5-hazkey ---- #

# check if built library exists
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/fcitx5-hazkey/.build/${SWIFT_BUILD_TYPE}/libfcitx5Hazkey.a)
    message(FATAL_ERROR "Failed to build fcitx5-hazkey swift library.\n\nIf the build log shows error messages like \"cannot find -lswiftCxx\" or \"cannot find -lswiftCxxStdlib\", you need to create symbolic links from swift/libswiftCxx.a to swift_static/libswiftCxx.a and from swift/libswiftCxxStdlib.a to swift_static/libswiftCxxStdlib.a [https://github.com/swiftlang/swift/issues/78003]")
endif()

# ---- install ---- #

if(NOT SKIP_ZENZAI_INSTALL)
    # install zenzai.gguf
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/zenzai.gguf
            DESTINATION ${CMAKE_INSTALL_DATADIR}/hazkey)
endif()

# install dictionary
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azooKey_dictionary_storage/Dictionary
        DESTINATION ${CMAKE_INSTALL_DATADIR}/hazkey)

# build fcitx5-hazkey
add_subdirectory(fcitx5-hazkey-interface)
