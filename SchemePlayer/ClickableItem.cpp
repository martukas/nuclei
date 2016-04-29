#include "ClickableItem.h"

#include "ActiveGraphicsItemGroup.h"

ClickableItem::ClickableItem(Type type)
    : item(0), t(type)
{
}

ClickableItem::Type ClickableItem::type() const
{
    return t;
}

ActiveGraphicsItemGroup *ClickableItem::graphicsItem() const
{
    return item;
}

SchemeVisualSettings::SchemeVisualSettings()
{
  outerGammaMargin = 50.0;
  outerLevelTextMargin = 4.0; // level lines extend beyond the beginning/end of the level texts by this value
  maxExtraLevelDistance = 120.0;
  levelToHalfLifeDistance = 10.0;
  parentNuclideLevelLineLength = 110.0; // initial (minimal) length of parent level lines
  parentNuclideLevelLineExtraLength = 15.5; // extra length of "active" levels (i.e. levels decaying to the daughter nuclide)
  parentNuclideMinSpinEnergyDistance = 15.0;
  feedingArrowLineLength = 110.0;
  feedingArrowHeadLength = 11.0;
  feedingArrowHeadWidth = 5.0;
  feedingArrowGap = 5.0;
  feedingArrowTextMargin = 4.0;
  parentNuclideToEnergyLevelsDistance = 30.0;
  highlightWidth = 5.0;
}

void SchemeVisualSettings::setStyle(const QFont &fontfamily, unsigned int sizePx)
{
  // prepare fonts and their metrics
  stdFont = fontfamily;
  stdFont.setPixelSize(sizePx);
  stdBoldFont = fontfamily;
  stdBoldFont.setPixelSize(sizePx);
  stdBoldFont.setBold(true);
  nucFont = fontfamily;
  nucFont.setPixelSize(sizePx * 20 / 10);
  nucFont.setBold(true);
  nucIndexFont = fontfamily;
  nucIndexFont.setPixelSize(sizePx * 12 / 10);
  nucIndexFont.setBold(true);
  parentHlFont = fontfamily;
  parentHlFont.setPixelSize(sizePx * 12 / 10);
  feedIntensityFont = fontfamily;
  feedIntensityFont.setPixelSize(sizePx);
  feedIntensityFont.setItalic(true);
  gammaFont = fontfamily;
  gammaFont.setPixelSize(sizePx);

  // prepare pens
  levelPen.setWidthF(1.0);
  levelPen.setCapStyle(Qt::FlatCap);
  stableLevelPen.setWidthF(2.0);
  stableLevelPen.setCapStyle(Qt::FlatCap);
  feedArrowPen.setWidthF(1.0);
  feedArrowPen.setCapStyle(Qt::FlatCap);
  intenseFeedArrowPen.setWidthF(2.0);
  intenseFeedArrowPen.setColor(QColor(232, 95, 92));
  intenseFeedArrowPen.setCapStyle(Qt::FlatCap);
  gammaPen.setWidthF(1.0);
  gammaPen.setCapStyle(Qt::FlatCap);
  intenseGammaPen.setWidthF(2.0);
  intenseGammaPen.setColor(QColor(232, 95, 92));
  intenseGammaPen.setCapStyle(Qt::FlatCap);
}
