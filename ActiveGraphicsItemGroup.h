#ifndef ACTIVEGRAPHICSITEMGROUP_H
#define ACTIVEGRAPHICSITEMGROUP_H

#include <QGraphicsItemGroup>
#include <QColor>

class GraphicsDropShadowEffect;
class GraphicsHighlightItem;
class QPropertyAnimation;
class QParallelAnimationGroup;
class ClickableItem;

class ActiveGraphicsItemGroup : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    ActiveGraphicsItemGroup(ClickableItem *associatedItem);
    ~ActiveGraphicsItemGroup();
    void addToGroup(QGraphicsItem * item);
    void addHighlightHelper(GraphicsHighlightItem *helperitem);
    void removeHighlightHelper(GraphicsHighlightItem *helperitem);
    void setPos(const QPointF & pos);
    void setPos(qreal x, qreal y);
    virtual QPainterPath shape() const;
    void setActiveColor(const QColor &color);
    void setHighlighted(bool active);
    bool isHighlighted() const;

signals:
    void clicked(ClickableItem *item);

public slots:
    void setShadowEnabled(bool enable);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);

private:
    void showHighlighting();
    void hideHighlighting();
    void updateHighlightColor();
    ClickableItem *assocItem;
    GraphicsDropShadowEffect *shadow;
    mutable QPainterPath *m_shape;
    mutable GraphicsHighlightItem *m_helper;
    bool m_highlighted, m_hover, m_shadowenabled;

    QPropertyAnimation *aniHighlight, *aniShadow;
    QParallelAnimationGroup *aniGroup;
    QColor activeColor;

    static const QColor hoverColor;
    static const double animationDuration;
    static const bool animateShadow;
    static const bool animate;
};

#endif // ACTIVEGRAPHICSITEMGROUP_H
