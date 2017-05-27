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
class QGraphicsRectItem;
class GraphicsHighlightItem;

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
  GraphicsHighlightItem *highlight_helper_ {nullptr};
  QGraphicsSimpleTextItem *halflife_text_ {nullptr};
  QGraphicsLineItem *vertical_arrow_ {nullptr};
  QGraphicsRectItem *click_area_ {nullptr};

  NuclideId id_;
};
