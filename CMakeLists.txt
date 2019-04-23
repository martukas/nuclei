cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(nuclei
  LANGUAGES CXX C
  VERSION 0.0.1
  )

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})

#=============================================================================
# ensure build type first
#=============================================================================

include(EnsureBuildType)

#=============================================================================
# Conan
#=============================================================================

set(CONAN_PROFILE "default" CACHE STRING "Name of conan profile to use, uses default by default")
set(CONAN "AUTO" CACHE STRING "conan options AUTO (conan must be in path), MANUAL (expects conanbuildinfo.cmake in build directory) or DISABLE")
if(${CONAN} MATCHES "AUTO")
  include(${CMAKE_MODULE_PATH}/conan.cmake)
  conan_cmake_run(CONANFILE conanfile.txt
    PROFILE ${CONAN_PROFILE}
    BASIC_SETUP NO_OUTPUT_DIRS SKIP_RPATH
    BUILD_TYPE "None"
    BUILD outdated)
elseif(${CONAN} MATCHES "MANUAL")
  if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(NO_OUTPUT_DIRS SKIP_RPATH)
  else()
    message(FATAL_ERROR "CONAN set to MANUAL but no file named conanbuildinfo.cmake found in build directory")
  endif()
elseif(NOT ${CONAN} MATCHES "DISABLE")
  message(FATAL_ERROR "Unrecognised option for CONAN (${CONAN}), use AUTO, MANUAL or DISABLE")
endif()

include(CompilerConfig)
include(OutputDirConfig)
include(CoverageReports)

include(InstallConfig)
include(BuildInfo)

find_package(date REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
include(QtLibraryConfig)
include(BoostLibraryConfig)

add_subdirectory(source)

#include(FindGTestFix)
#if(GTest_FOUND)
#  add_subdirectory(tests)
#  add_custom_target(everything DEPENDS
#    all_tests
#    nuclei
#    )
#  create_coverage_targets(
#    coverage
#    run_unit_tests
#    ${PROJECT_BINARY_DIR}
#    ${CMAKE_SOURCE_DIR}/source/
#    ${PROJECT_BINARY_DIR}/tests/coverage
#  )
#endif()
