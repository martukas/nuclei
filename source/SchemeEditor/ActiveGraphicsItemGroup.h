#pragma once

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
    void setActiveColor(int, const QColor &color);
    void setHoverColor(const QColor &color);
    void setHighlighted(int active);
    int isHighlighted() const;

signals:
    void clicked(ClickableItem *item);

public slots:
    void setShadowEnabled(bool enable);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);

    void setTextColor(QColor c);

private:
    void showHighlighting();
    void hideHighlighting();
    void updateHighlightColor();
    ClickableItem *associated_item_;
    GraphicsDropShadowEffect *shadow_ {nullptr};
    mutable QPainterPath *shape_ {nullptr};
    mutable GraphicsHighlightItem *highlight_helper_ {nullptr};
    int highlighted_ {0};
    bool hovering_ {false};
    bool shadow_enabled_ {true};

    QPropertyAnimation *highlight_animation_ {nullptr};
    QPropertyAnimation *shadow_animation_ {nullptr};
    QParallelAnimationGroup *animation_group_ {nullptr};
    QMap<int,QColor> active_colors_;
    QColor hover_color_;

    static const double animation_duration_;
    static const bool animate_shadow_;
    static const bool animate_;
};
