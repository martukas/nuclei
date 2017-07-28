#include "ClickableItem.h"
#include "ActiveGraphicsItemGroup.h"

ClickableItem::ClickableItem(Type type)
  :t(type) {}

ClickableItem::Type ClickableItem::type() const
{
  return t;
}

ActiveGraphicsItemGroup *ClickableItem::graphicsItem() const
{
  return item;
}

void SchemeVisualSettings::setStyle(const QFont &fontfamily,
                                    unsigned int sizePx)
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
  gammaPen.setWidthF(1.0);
  gammaPen.setCapStyle(Qt::FlatCap);
}
