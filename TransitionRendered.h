#ifndef TransitionRendered_H
#define TransitionRendered_H

#include <stdint.h>
#include <QFont>
#include <QPen>
#include "ClickableItem.h"
#include "Transition.h"

class QGraphicsItem;
class ActiveGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsTextItem;
class QGraphicsPolygonItem;
class QGraphicsRectItem;
class GraphicsHighlightItem;
class QGraphicsScene;

class TransitionRendered : public ClickableItem
{
public:

  TransitionRendered();
  TransitionRendered(TransitionPtr transition, SchemeVisualSettings vis, QGraphicsScene *scene);

  virtual ~TransitionRendered();

  void updateArrow(double arrowDestY);
  double minimalXDistance() const;
  /**
     * Distance between origin and right edge of the bounding rect
     */
  double widthFromOrigin() const;

  QPen pen() const;

  QGraphicsLineItem *arrow;
  QGraphicsTextItem *text;
  QGraphicsPolygonItem *arrowhead, *arrowbase;
  QGraphicsRectItem *clickarea;
  GraphicsHighlightItem *highlightHelper;
  double mindist;
  QPen m_pen;

  Energy from_, to_;

private:
  static const double textAngle;
  static const double arrowHeadLength;
  static const double arrowBaseLength;
  static const double arrowHeadWidth;
  static const double arrowBaseWidth;
  static const double highlightWidth;

  static const QPolygonF arrowHeadShape, arrowBaseShape;
  static QPolygonF initArrowHead();
  static QPolygonF initArrowBase();
};

#endif // TransitionRendered_H
