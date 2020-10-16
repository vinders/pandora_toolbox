#*******************************************************************************
# Author  :     Romain Vinders
# License :     MIT
# ------------------------------------------------------------------------------
# Portable and efficient libraries for video & real-time projects.
#*******************************************************************************
cmake_minimum_required(VERSION 3.14)
include("${CMAKE_CURRENT_SOURCE_DIR}/_cmake/cwork.cmake")
cwork_read_version_from_file("${CMAKE_CURRENT_SOURCE_DIR}/build_version.txt" ON)
project("pandora" VERSION ${CWORK_BUILD_VERSION})

# ┌──────────────────────────────────────────────────────────────────┐
# │  Solution settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cwork_create_solution("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/_cmake"
    system
    thread
    pattern
    memory
    logic
    time
    io
    hardware
)
