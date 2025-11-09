cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. "
        "Please create a separate build directory and run cmake from there.\n"
        "To clean up, remove CMakeCache.txt and CMakeFiles/ directory.")
endif()

project(hazkey-server VERSION 0.2.0)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Skip ALL targets during install to avoid swift permission issues
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

find_program(SWIFT_EXECUTABLE swift REQUIRED)

option(SKIP_KKC_PATH_UPDATE "Skip updating path.swift" OFF)
option(SKIP_INSTALL_DICTIONARY "Skip install dictionary" OFF)

# ----- llama-stub ----- #
add_subdirectory(llama-stub)

# --- hazkey-server --- #

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SWIFT_BUILD_TYPE "debug")
else()
    set(SWIFT_BUILD_TYPE "release")
endif()

if(NOT SKIP_KKC_PATH_UPDATE)
    message(STATUS "Updating path.swift")
    # write path.swift
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/Sources/hazkey-server/path.swift "let systemResourcePath: String = \"${CMAKE_INSTALL_FULL_DATADIR}/hazkey\"")
endif()

# Auto detect swift library path
# To bypass issue swiftlang/swift#78003
execute_process(
    COMMAND which swift
    OUTPUT_VARIABLE swift_path_raw
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE which_result
)
if(which_result EQUAL 0)
    execute_process(
        COMMAND readlink -f "${swift_path_raw}"
        OUTPUT_VARIABLE swift_path_resolved
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE readlink_result
    )
    if(readlink_result EQUAL 0)
        # set relative path from swift binary
        if("${swift_path_resolved}" MATCHES "/bin/swift[^/]*$")
            string(REGEX REPLACE "/bin/swift[^/]*$" "" swift_base_dir "${swift_path_resolved}")
            set(SWIFT_DETECTED_LIB_PATH "${swift_base_dir}/lib/swift/linux")

            message(STATUS "Detected Swift library path: ${SWIFT_DETECTED_LIB_PATH}")
        else()
            set(SWIFT_DETECTED_LIB_PATH "")
            message(STATUS "Resolved swift path '${swift_path_resolved}' does not end with '/bin/swift*'. SWIFT_LIB_PATH not set.")
        endif()
    else()
        set(SWIFT_DETECTED_LIB_PATH "")
        message(WARNING "Failed to execute 'readlink -f' on '${swift_path_raw}'. SWIFT_LIB_PATH not set.")
    endif()
else()
    set(SWIFT_DETECTED_LIB_PATH "")
    message(WARNING "Swift executable not found using 'which swift'. SWIFT_LIB_PATH not set.")
endif()

add_custom_target(build_hazkey_server ALL
    COMMAND ${CMAKE_COMMAND}
        -DSWIFT_BUILD_TYPE=${SWIFT_BUILD_TYPE}
        -DSWIFT_EXECUTABLE=${SWIFT_EXECUTABLE}
        -DSWIFT_DISABLE_DEPENDENCY_CACHE=${SWIFT_DISABLE_DEPENDENCY_CACHE}
        -DSWIFT_DETECTED_LIB_PATH=${SWIFT_DETECTED_LIB_PATH}
        -DSWIFT_LINK_PATH=${SWIFT_LINK_PATH}
        -DSWIFT_WORK_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DLLAMA_STUB_DIR=${CMAKE_CURRENT_BINARY_DIR}/llama-stub
        -P ${CMAKE_CURRENT_SOURCE_DIR}/build_swift.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building hazkey-server"
    VERBATIM
)

add_dependencies(build_hazkey_server llama)

# ---- install ---- #

# install hazkey-server
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/swift-build/${SWIFT_BUILD_TYPE}/hazkey-server
        DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/hazkey)

# configure and install hazkey-server wrapper script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/hazkey-server.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/hazkey-server
    @ONLY
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hazkey-server
        DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

# install llama stub library
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/llama-stub/libllama.so
        DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/hazkey/llama-stub)

# install icons
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/hazkey.svg
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/hazkey_256x256.png
        RENAME hazkey.png
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/256x256/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/hazkey_48x48.png
        RENAME hazkey.png
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/48x48/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/hazkey_32x32.png
        RENAME hazkey.png
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/32x32/apps)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/hazkey_24x24.png
        RENAME hazkey.png
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/24x24/apps)

# install dictionary
if(NOT SKIP_INSTALL_DICTIONARY)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azooKey_dictionary_storage/Dictionary
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/hazkey)
endif()

#install emoji dictionary
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/azooKey_emoji_dictionary_storage/EmojiDictionary/emoji_all_E16.0.txt
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/hazkey)
