#include "GraphicsScene.h"

#include <util/logger.h>
#include <QGraphicsSceneMouseEvent>

GraphicsScene::GraphicsScene(QObject *parent)
  : QGraphicsScene(parent)
{}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  prevpos = event->scenePos();
  QGraphicsScene::mousePressEvent(event);
}


void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem *gr = mouseGrabberItem();
  if (!gr && (event->scenePos() == prevpos))
  {
    DBG("simple empty click");
    emit clickedBackground();
  }
  QGraphicsScene::mouseReleaseEvent(event);
}

