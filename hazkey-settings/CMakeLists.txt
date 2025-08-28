cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. "
        "Please create a separate build directory and run cmake from there.\n"
        "To clean up, remove CMakeCache.txt and CMakeFiles/ directory.")
endif()

project(hazkey-settings VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/env_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/env_config.h
)

set(TS_FILES hazkey-settings_ja_JP.ts)

set(PROJECT_SOURCES
    protocol/base.pb.cc
    protocol/commands.pb.cc
    protocol/config.pb.cc
    protocol/base.pb.h
    protocol/commands.pb.h
    protocol/config.pb.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    serverconnector.cpp
    serverconnector.h
    config_definitions.h
    config_macros.h
    ${TS_FILES}
)

qt_add_executable(hazkey-settings ${PROJECT_SOURCES})
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_include_directories(hazkey-settings PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(hazkey-settings PRIVATE
    Qt6::Widgets
    ${Protobuf_LIBRARIES}
    absl::log_internal_message
    absl::log_internal_check_op
)

include(GNUInstallDirs)
install(TARGETS hazkey-settings
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/hazkey
)
