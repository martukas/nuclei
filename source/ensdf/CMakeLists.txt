set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Fields.cpp
  ${dir}/LevelsData.cpp
  ${dir}/NuclideData.cpp
  ${dir}/Parser.cpp
  ${dir}/Record.cpp
  ${dir}/Translator.cpp
  )

set(HEADERS
  ${dir}/Fields.h
  ${dir}/LevelsData.h
  ${dir}/NuclideData.h
  ${dir}/Parser.h
  ${dir}/Record.h
  ${dir}/Translator.h
  )

add_subdirectory(records)

set(${this_target}_headers
  ${${this_target}_headers}
  ${records_headers}
  ${HEADERS}
  PARENT_SCOPE)

set(${this_target}_sources
  ${${this_target}_sources}
  ${records_sources}
  ${SOURCES}
  PARENT_SCOPE)
