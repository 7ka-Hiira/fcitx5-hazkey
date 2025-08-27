cmake_minimum_required(VERSION 3.21)
project(llama)
include(GNUInstallDirs)

add_library(llama SHARED llama.c)

install(TARGETS llama LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/hazkey/llama-stub)
