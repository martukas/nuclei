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
  #  PRIVATE ${CMAKE_BINARY_DIR}/build_info
)

target_link_libraries(
  ${this_target}
  PRIVATE Boost::regex
  PRIVATE QtColorWidgets
  PRIVATE Qt5::Widgets
  PRIVATE Qt5::PrintSupport
  PRIVATE Qt5::Network
  PRIVATE Qt5::Svg
  PRIVATE fmt::fmt
  PRIVATE spdlog::spdlog
  PRIVATE Threads::Threads
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE date::date-tz
  PRIVATE qt-color-widgets::qt-color-widgets
  PRIVATE ${DATE_LIBRARIES}
)

#add_dependencies(${this_target} build_time)
