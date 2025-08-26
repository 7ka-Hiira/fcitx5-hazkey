cmake_minimum_required(VERSION 3.16)

project(hazkey-settings VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

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
        serverconnector.cpp
        mainwindow.h
        serverconnector.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(hazkey-settings
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        serverconnector.h serverconnector.cpp
    )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET hazkey-settings APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    add_executable(hazkey-settings
        ${PROJECT_SOURCES}
    )

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)
# link_directories(${Protobuf_LIBRARY_DIRS})

target_link_libraries(hazkey-settings PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${Protobuf_LIBRARIES} absl::log_internal_message absl::log_internal_check_op)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.hazkey-settings)
endif()
set_target_properties(hazkey-settings PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS hazkey-settings
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(hazkey-settings)
endif()
