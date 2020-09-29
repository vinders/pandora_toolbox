# ┌──────────────────────────────────────────────────────────────────┐
# │  Manage git submodules                                           │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.14)
project(git_external LANGUAGES C CXX)

find_package(Threads REQUIRED)

include(ExternalProject)
ExternalProject_Add(git_extractor
    GIT_REPOSITORY @_GIT_EXT_REPOSITORY@
    GIT_TAG @_GIT_EXT_TAG@
    DOWNLOAD_DIR @_GIT_EXT_CACHE@
    SOURCE_DIR @_GIT_EXT_DIR@
    BINARY_DIR @_GIT_EXT_CACHE@
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)
