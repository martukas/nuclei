cmake_minimum_required(VERSION 2.8)
project(nuclei CXX)

# Common flags
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Boost
find_package(Boost COMPONENTS system filesystem thread timer date_time 
             log log_setup regex REQUIRED)
link_directories( ${BOOST_LIBRARY_DIRS} )
add_definitions(-DBOOST_LOG_DYN_LINK)

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets REQUIRED)
#find_package(OpenGL REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES *.cpp *.h *.ui *.qrc)
file(GLOB_RECURSE ${PROJECT_NAME}_HEADERS *.h ${PROJECT_BINARY_DIR}/*.h)
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
  ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
#  ${OPENGL_LIBRARIES}
)

qt5_use_modules(${PROJECT_NAME} Widgets PrintSupport Svg)

if(UNIX)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
