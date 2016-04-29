#ifndef LEVEL_RENDERED_H
#define LEVEL_RENDERED_H

#include "ClickableItem.h"
#include "XEnergyLevel.h"

class ActiveGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class QGraphicsTextItem;
class QGraphicsPolygonItem;
class GraphicsHighlightItem;
class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsItemGroup;

class LevelRendered : public ClickableItem
{
public:
  LevelRendered();
  LevelRendered(XEnergyLevelPtr level, ParentPosition parentpos, SchemeVisualSettings vis, QGraphicsScene *scene);

  //returns feeding arrow height, if any
  double align(double leftlinelength, double rightlinelength, double arrowleft, double arrowright,
             SchemeVisualSettings vis, ParentPosition parentpos);

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

#endif // ENERGYLEVEL_H
