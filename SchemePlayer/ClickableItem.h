#ifndef CLICKABLEITEM_H
#define CLICKABLEITEM_H

#include "Energy.h"
#include <QFont>
#include <QPen>

class ActiveGraphicsItemGroup;
class QGraphicsItem;

enum ParentPosition {
  NoParent,
  LeftParent,
  RightParent
};

class ClickableItem
{
public:
  enum Type {
    EnergyLevelType,
    GammaTransitionType,
    ParentNuclideType,
    DaughterNuclideType,
    InvalidType
  };

  ClickableItem(Type type);
  virtual Type type() const;
  virtual ActiveGraphicsItemGroup * graphicsItem() const;

protected:
  ActiveGraphicsItemGroup *item;
  Type t;
};

struct SchemeVisualSettings
{
  SchemeVisualSettings();
  void setStyle(const QFont &fontfamily, unsigned int sizePx);

  // style
  QFont stdFont, stdBoldFont, nucFont, nucIndexFont, parentHlFont, feedIntensityFont, gammaFont;
  QPen levelPen, stableLevelPen, feedArrowPen, gammaPen;

  double outerGammaMargin; // margin between level texts (spin, energy) and gammas
  double outerLevelTextMargin; // level lines extend beyond the beginning/end of the level texts by this value
  double maxExtraLevelDistance; // maximal additional distance between two level lines
  double levelToHalfLifeDistance; // distance between level line and half life text
  double parentNuclideLevelLineLength;
  double parentNuclideLevelLineExtraLength; // additional length of the decaying level
  double parentNuclideMinSpinEnergyDistance; // Minumal distance between spin and energy texts on parent level lines
  double feedingArrowLineLength; // initial (minimal) line length of feeding arrows
  double feedingArrowHeadLength;
  double feedingArrowHeadWidth;
  double feedingArrowGap;
  double feedingArrowTextMargin; // margin around the feeding intensity text
  double parentNuclideToEnergyLevelsDistance;
  double highlightWidth;

  ParentPosition parentpos;
};

#endif // CLICKABLEITEM_H
