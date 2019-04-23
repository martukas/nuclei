set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/DataQuality.cpp
  ${dir}/DecayInfo.cpp
  ${dir}/DecayMode.cpp
  ${dir}/DecayScheme.cpp
  ${dir}/Energy.cpp
  ${dir}/HalfLife.cpp
  ${dir}/Level.cpp
  ${dir}/Moment.cpp
  ${dir}/nid.cpp
  ${dir}/Nuclide.cpp
  ${dir}/Parity.cpp
  ${dir}/ReactionInfo.cpp
  ${dir}/Spin.cpp
  ${dir}/SpinParity.cpp
  ${dir}/Transition.cpp
  ${dir}/Uncert.cpp
  )

set(HEADERS
  ${dir}/DataQuality.h
  ${dir}/DecayInfo.h
  ${dir}/DecayMode.h
  ${dir}/DecayScheme.h
  ${dir}/Energy.h
  ${dir}/HalfLife.h
  ${dir}/Level.h
  ${dir}/Moment.h
  ${dir}/nid.h
  ${dir}/Nuclide.h
  ${dir}/Parity.h
  ${dir}/ReactionInfo.h
  ${dir}/Spin.h
  ${dir}/SpinParity.h
  ${dir}/Transition.h
  ${dir}/Uncert.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
