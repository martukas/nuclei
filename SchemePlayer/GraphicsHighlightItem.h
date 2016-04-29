#ifndef GRAPHICSHIGHLIGHTITEM_H
#define GRAPHICSHIGHLIGHTITEM_H

#include <QObject>
#include <QGraphicsRectItem>

class GraphicsHighlightItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit GraphicsHighlightItem(QGraphicsItem *parent = 0);
    GraphicsHighlightItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = 0);
    
signals:
    
public slots:
    
};

#endif // GRAPHICSHIGHLIGHTITEM_H
