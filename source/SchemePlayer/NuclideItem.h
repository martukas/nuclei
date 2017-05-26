#pragma once

#include <QString>
#include <QMap>
#include <QFont>
#include <QPair>
#include "ClickableItem.h"
#include "Nuclide.h"

class QGraphicsItem;
class QGraphicsItemGroup;
class QGraphicsScene;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;

class NuclideItem : public ClickableItem
{
public:
  NuclideItem();
  NuclideItem(const Nuclide& nuc, Type tp,
              SchemeVisualSettings vis,
              QGraphicsScene *scene);

  void position_arrow(double x, double start, double end);
  void position_text(double parent_center_x,
                     double ypos);

private:
  QGraphicsSimpleTextItem *pNucHl {nullptr};
  QGraphicsLineItem *pNucVerticalArrow {nullptr};

  NuclideId id_;
};
