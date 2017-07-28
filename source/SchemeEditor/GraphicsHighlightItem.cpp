#include "GraphicsHighlightItem.h"

GraphicsHighlightItem::GraphicsHighlightItem(QGraphicsItem *parent)
  : QGraphicsRectItem(parent)
{}

GraphicsHighlightItem::GraphicsHighlightItem(qreal x, qreal y, qreal width,
                                             qreal height, QGraphicsItem *parent)
  : QGraphicsRectItem(x, y, width, height, parent)
{}
