#pragma once

#include "Energy.h"
#include <QFont>
#include <QPen>

class ActiveGraphicsItemGroup;
class QGraphicsItem;

enum ParentPosition
{
  NoParent,
  LeftParent,
  RightParent
};

class ClickableItem
{
public:
  enum Type
  {
    InvalidType,
    ParentNuclideType,
    ParentLevelType,
    DaughterNuclideType,
    DaughterLevelType,
    GammaTransitionType
  };

  ClickableItem() {}
  ClickableItem(Type type);
  virtual Type type() const;
  virtual ActiveGraphicsItemGroup * graphicsItem() const;

protected:
  ActiveGraphicsItemGroup *item {nullptr};
  Type t{InvalidType};
};

struct SchemeVisualSettings
{
  SchemeVisualSettings() {}
  void setStyle(const QFont&fontfamily, unsigned int sizePx);

  // style
  QFont stdFont, stdBoldFont, nucFont, nucIndexFont;
  QFont parentHlFont, feedIntensityFont, gammaFont;
  QPen levelPen, stableLevelPen, feedArrowPen, gammaPen;

  double outerGammaMargin {50.0}; // margin between level texts (spin, energy) and gammas
  double outerLevelTextMargin {4.0}; // level lines extend beyond the beginning/end of the level texts by this value
  double maxExtraLevelDistance {120.0}; // maximal additional distance between two level lines
  double levelToHalfLifeDistance {10.0}; // distance between level line and half life text
  double parentNuclideLevelLineLength {110.0};
  double parentNuclideLevelLineExtraLength {15.5}; // additional length of the decaying level
  double parentNuclideMinSpinEnergyDistance {15.0}; // Minumal distance between spin and energy texts on parent level lines
  double feedingArrowLineLength {110.0}; // initial (minimal) line length of feeding arrows
  double feedingArrowHeadLength {11.0};
  double feedingArrowHeadWidth {5.0};
  double feedingArrowGap {5.0};
  double feedingArrowTextMargin {4.0}; // margin around the feeding intensity text
  double parentNuclideToEnergyLevelsDistance {30.0};
  double highlightWidth {5.0};

  ParentPosition parentpos {NoParent};
};
