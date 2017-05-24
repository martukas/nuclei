#pragma once

#include "ClickableItem.h"
#include "Level.h"

class ActiveGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class QGraphicsTextItem;
class QGraphicsPolygonItem;
class GraphicsHighlightItem;
class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsItemGroup;

class LevelItem : public ClickableItem
{
public:
  LevelItem();
  LevelItem(Level level, SchemeVisualSettings vis, QGraphicsScene *scene);

  //returns feeding arrow height, if any
  double align(double leftlinelength, double rightlinelength,
               double arrowleft, double arrowright,
               SchemeVisualSettings vis);

//private:
  QGraphicsLineItem *graline, *grafeedarrow;
  QGraphicsPolygonItem *graarrowhead;
  QGraphicsSimpleTextItem *graetext, *graspintext, *grahltext;
  QGraphicsTextItem *grafeedintens;
  QGraphicsRectItem *graclickarea;
  GraphicsHighlightItem *grahighlighthelper;
  double graYPos;

  Energy energy_;
};
