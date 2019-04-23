set(this_target ${PROJECT_NAME})
set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(${this_target}_sources
  ${dir}/DecayCascadeFilterProxyModel.cpp
  ${dir}/DecayCascadeItemModel.cpp
  ${dir}/ENSDFDataSource.cpp
  ${dir}/ENSDFTreeItem.cpp
  ${dir}/LineEdit.cpp
  ${dir}/main.cpp
  ${dir}/Nuclei.cpp
  ${dir}/ScrollZoomView.cpp
  ${dir}/TreeView.cpp
  )

set(${this_target}_headers
  ${dir}/DecayCascadeFilterProxyModel.h
  ${dir}/DecayCascadeItemModel.h
  ${dir}/ENSDFDataSource.h
  ${dir}/ENSDFTreeItem.h
  ${dir}/LineEdit.h
  ${dir}/qpx_util.h
  ${dir}/Nuclei.h
  ${dir}/ScrollZoomView.h
  ${dir}/TreeView.h
  )

set(${this_target}_ui
  ${dir}/Nuclei.ui
  ${dir}/PreferencesDialog.ui
  )

add_subdirectory(util)
add_subdirectory(ensdf)
add_subdirectory(NucData)
add_subdirectory(SchemeEditor)

set(CMAKE_AUTOUIC ON)
qt5_add_resources(${this_target}_resources
  ${dir}/resources/nuclei.qrc)

message(STATUS "sources:   ${${this_target}_sources}")
message(STATUS "headers:   ${${this_target}_headers}")
message(STATUS "ui:        ${${this_target}_ui}")

add_executable(
  ${this_target}
  ${${this_target}_resources}
  ${${this_target}_sources}
  ${${this_target}_headers}
  ${${this_target}_ui}
)

target_include_directories(
  ${this_target}
  PRIVATE ${PROJECT_SOURCE_DIR}/source
  PRIVATE ${${this_target}_include_dirs}
  #  PRIVATE ${CMAKE_BINARY_DIR}/build_info
)

target_link_libraries(
  ${this_target}
  PRIVATE Boost::system
  PRIVATE Boost::filesystem
  PRIVATE Boost::regex
  PRIVATE QtColorWidgets-Qt52
  PRIVATE Qt5::Widgets
  PRIVATE Qt5::PrintSupport
  PRIVATE Qt5::Network
  PRIVATE Qt5::Svg
  PRIVATE fmt::fmt
  PRIVATE Threads::Threads
  PRIVATE ${DATE_LIBRARIES}
  PRIVATE ${COVERAGE_LIBRARIES}
)

enable_coverage(${this_target})

#add_dependencies(${this_target} build_time)
