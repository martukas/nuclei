#pragma once

#include <QGraphicsScene>
#include <QPoint>

class GraphicsScene : public QGraphicsScene
{
  Q_OBJECT
public:
  explicit GraphicsScene(QObject *parent = 0);

signals:

  void clickedBackground();

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent) Q_DECL_OVERRIDE;

private:
  QPointF prevpos;
};
