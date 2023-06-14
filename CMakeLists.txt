cmake_minimum_required(VERSION 3.7.0 FATAL_ERROR)
project(nuclei
  LANGUAGES CXX C
  VERSION 0.0.1
  )

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

#=============================================================================
# ensure build type first
#=============================================================================

include(EnsureBuildType)

include(CompilerConfig)
include(OutputDirConfig)
include(InstallConfig)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)

include(QtLibraryConfig)
include(BoostLibraryConfig)

add_subdirectory(source)

