cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(nuclei CXX)

#===========================================================
# Internal compiler options
#===========================================================
# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14)
  set(CMAKE_CXX_STANDARD 14)
endif()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# the compiler used for C++ files
MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_VERSION} )

FIND_PACKAGE ( Threads REQUIRED )

# Boost
find_package(Boost COMPONENTS
  system filesystem thread
  timer date_time log log_setup
  regex REQUIRED)
add_definitions(-DBOOST_LOG_DYN_LINK)

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets REQUIRED)
#find_package(OpenGL REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_UI *.ui)
file(GLOB_RECURSE ${PROJECT_NAME}_QRC *.qrc)
file(GLOB ${PROJECT_NAME}_SOURCES *.cpp ensdf/*.cpp ensdf/records/*.cpp
  NucData/*.cpp SchemeEditor/*.cpp SchemeEditor/Qt-Color-Widgets/src/*.cpp)
file(GLOB ${PROJECT_NAME}_HEADERS *.h ensdf/*.h ensdf/records/*.h
  NucData/*.h SchemeEditor/*.h SchemeEditor/Qt-Color-Widgets/include/*.hpp
  ${PROJECT_BINARY_DIR}/*.h)
foreach (_headerFile ${${PROJECT_NAME}_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES ${PROJECT_NAME}_INCLUDE_DIRS)

include_directories(
  ${${PROJECT_NAME}_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
#  ${OPENGL_INCLUDE_DIR}
)

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_HEADERS}
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_QRC}
  ${${PROJECT_NAME}_UI}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
#  ${OPENGL_LIBRARIES}
)

qt5_use_modules(${PROJECT_NAME} Widgets PrintSupport Svg)

if(UNIX)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
