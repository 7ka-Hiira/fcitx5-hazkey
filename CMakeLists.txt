cmake_minimum_required(VERSION 3.21)
project(fcitx5-hazkey VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


# build azookey-kkc library
execute_process(
    COMMAND swift build -c release
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc
)

# check if built library exists
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/release/libhazkey.so)
    message(FATAL_ERROR "Failed to build libhazkey")
endif()

# install libhazkey.so
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/azookey-kkc/.build/release/libhazkey.so DESTINATION lib)

# build fcitx5-hazkey
add_subdirectory(fcitx5)