set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Alpha.cpp
  ${dir}/Beta.cpp
  ${dir}/Comments.cpp
  ${dir}/Continuation.cpp
  ${dir}/EC.cpp
  ${dir}/Gamma.cpp
  ${dir}/Header.cpp
  ${dir}/History.cpp
  ${dir}/LevelRec.cpp
  ${dir}/Normalization.cpp
  ${dir}/Parent.cpp
  ${dir}/Particle.cpp
  ${dir}/ProductionNorm.cpp
  ${dir}/QValue.cpp
  ${dir}/Reference.cpp
  ${dir}/XRef.cpp
  )

set(HEADERS
  ${dir}/Alpha.h
  ${dir}/Beta.h
  ${dir}/Comments.h
  ${dir}/Continuation.h
  ${dir}/EC.h
  ${dir}/Gamma.h
  ${dir}/Header.h
  ${dir}/History.h
  ${dir}/LevelRec.h
  ${dir}/Normalization.h
  ${dir}/Parent.h
  ${dir}/Particle.h
  ${dir}/ProductionNorm.h
  ${dir}/QValue.h
  ${dir}/Reference.h
  ${dir}/XRef.h
  )

set(records_headers ${HEADERS} PARENT_SCOPE)
set(records_sources ${SOURCES} PARENT_SCOPE)
