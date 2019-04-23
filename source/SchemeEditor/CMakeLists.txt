set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/ActiveGraphicsItemGroup.cpp
  ${dir}/ClickableItem.cpp
  ${dir}/GraphicsDropShadowEffect.cpp
  ${dir}/GraphicsHighlightItem.cpp
  ${dir}/GraphicsScene.cpp
  ${dir}/LevelItem.cpp
  ${dir}/NuclideItem.cpp
  ${dir}/SchemeEditor.cpp
  ${dir}/SchemeEditorPrefs.cpp
  ${dir}/SchemeGraphics.cpp
  ${dir}/SchemeVisualSettings.cpp
  ${dir}/TransitionItem.cpp
  )

set(UI
  ${dir}/SchemeEditor.ui
  ${dir}/SchemeEditorPrefs.ui
  )

set(HEADERS
  ${dir}/ActiveGraphicsItemGroup.h
  ${dir}/ClickableItem.h
  ${dir}/GraphicsDropShadowEffect.h
  ${dir}/GraphicsHighlightItem.h
  ${dir}/GraphicsScene.h
  ${dir}/LevelItem.h
  ${dir}/NuclideItem.h
  ${dir}/SchemeEditor.h
  ${dir}/SchemeEditorPrefs.h
  ${dir}/SchemeGraphics.h
  ${dir}/SchemeVisualSettings.h
  ${dir}/TransitionItem.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
set(${this_target}_ui ${${this_target}_ui} ${UI} PARENT_SCOPE)
