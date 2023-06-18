cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

# This MUST run before the call to project()
# Otherwise the appropriate variables will not be set
include(ConanConfig)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/conan_toolchain.cmake)

project(nuclei
  LANGUAGES CXX C
  VERSION 0.0.1
  )

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
find_package(nlohmann_json REQUIRED)
find_package(date REQUIRED)
find_package(qt-color-widgets REQUIRED)

include(QtLibraryConfig)
include(BoostLibraryConfig)

add_subdirectory(source)

