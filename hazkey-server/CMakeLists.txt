cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. "
        "Please create a separate build directory and run cmake from there.\n"
        "To clean up, remove CMakeCache.txt and CMakeFiles/ directory.")
endif()

project(hazkey-server VERSION 0.0.9)
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

add_custom_target(build_hazkey_server ALL
    COMMAND ${CMAKE_COMMAND}
        -DSWIFT_BUILD_TYPE=${SWIFT_BUILD_TYPE}
        -DSWIFT_EXECUTABLE=${SWIFT_EXECUTABLE}
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

# create a symlink of hazkey-server in the binary directory
install(CODE "
    file(MAKE_DIRECTORY \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_BINDIR}\")
    file(CREATE_LINK
    \"${CMAKE_INSTALL_FULL_LIBDIR}/hazkey/hazkey-server\"
    \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_BINDIR}/hazkey-server\"
    SYMBOLIC)
")

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
