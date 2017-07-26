#include "ActiveGraphicsItemGroup.h"

#include <QPen>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>
#include "GraphicsHighlightItem.h"
#include "GraphicsDropShadowEffect.h"
#include "ClickableItem.h"

const double ActiveGraphicsItemGroup::animation_duration_ = 100.0;
const bool ActiveGraphicsItemGroup::animate_shadow_ = false;
const bool ActiveGraphicsItemGroup::animate_ = false;

ActiveGraphicsItemGroup::ActiveGraphicsItemGroup(ClickableItem *associatedItem)
  : associated_item_(associatedItem)
  , shadow_(new GraphicsDropShadowEffect)
{
  // prepare highlighting
  shadow_->setBlurRadius(15.0);
  shadow_->setOffset(QPointF(0.0, 0.0));

  hover_color_ = QColor(178, 223, 150, 180);

  setGraphicsEffect(shadow_);
  setAcceptHoverEvents(true);

  if (animate_) {
    shadow_->setOpacity(0.0);
    highlight_animation_ = new QPropertyAnimation(highlight_helper_, "opacity");
    highlight_animation_->setDuration(animation_duration_);
    highlight_animation_->setStartValue(0.0);
    highlight_animation_->setEndValue(1.0);
    shadow_animation_ = new QPropertyAnimation(shadow_, "opacity");
    shadow_animation_->setDuration(animation_duration_);
    shadow_animation_->setStartValue(0.0);
    shadow_animation_->setEndValue(1.0);
    animation_group_ = new QParallelAnimationGroup;
    animation_group_->addAnimation(highlight_animation_);
    if (shadow_animation_)
      animation_group_->addAnimation(shadow_animation_);
  }
  else {
    shadow_->setEnabled(false);
  }
}

ActiveGraphicsItemGroup::~ActiveGraphicsItemGroup()
{
  delete shape_;
  delete animation_group_;
}

void ActiveGraphicsItemGroup::addToGroup(QGraphicsItem *item)
{
  delete shape_;
  shape_ = 0;
  QGraphicsItemGroup::addToGroup(item);
}

void ActiveGraphicsItemGroup::addHighlightHelper(GraphicsHighlightItem *helperitem)
{
  delete highlight_helper_;
  highlight_helper_ = helperitem;
  if (animate_) {
    highlight_helper_->setOpacity(0.0);
  }
  else {
    highlight_helper_->setOpacity(1.0);
    highlight_helper_->hide();
  }
  highlight_helper_->setPen(Qt::NoPen);
  highlight_helper_->setBrush(QBrush(hover_color_));
  addToGroup(highlight_helper_);
}

void ActiveGraphicsItemGroup::removeHighlightHelper(GraphicsHighlightItem *helperitem)
{
  highlight_helper_ = 0;
  removeFromGroup(helperitem);
}


void ActiveGraphicsItemGroup::setPos(const QPointF &pos)
{
  QGraphicsItemGroup::setPos(pos);
  // force recreation of bounding rect (workaround)
  QGraphicsItem *tmp = childItems().last();
  removeFromGroup(tmp);
  addToGroup(tmp);
  delete shape_;
  shape_ = 0;
}

void ActiveGraphicsItemGroup::setPos(qreal x, qreal y)
{
  setPos(QPointF(x, y));
}

QPainterPath ActiveGraphicsItemGroup::shape() const
{
  if (!shape_) {
    shape_ = new QPainterPath;
    foreach (QGraphicsItem *child, childItems()) {
      shape_->addPath(mapFromItem(child, child->shape()));
    }
  }
  return *shape_;
}

void ActiveGraphicsItemGroup::setActiveColor(int i, const QColor &color)
{
  active_colors_[i] = color;
}

void ActiveGraphicsItemGroup::setHoverColor(const QColor &color)
{
  hover_color_ = color;
}

void ActiveGraphicsItemGroup::setHighlighted(int hl)
{
  highlighted_ = hl;
  if (!highlighted_ && !hovering_)
    hideHighlighting();
  if (highlighted_ && !hovering_)
    showHighlighting();

  if (hl || (!hl && hovering_))
    updateHighlightColor();
}

int ActiveGraphicsItemGroup::isHighlighted() const
{
  return highlighted_;
}

void ActiveGraphicsItemGroup::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  hovering_ = true;
  updateHighlightColor();
  if (!highlighted_)
    showHighlighting();
}

void ActiveGraphicsItemGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  hovering_ = false;
  if (highlighted_)
    updateHighlightColor();
  if (!highlighted_)
    hideHighlighting();
}

void ActiveGraphicsItemGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->buttons() & (Qt::LeftButton | Qt::RightButton))
    emit clicked(associated_item_);
}

void ActiveGraphicsItemGroup::showHighlighting()
{
  setZValue(1.0);
  if (animate_)
  {
    animation_group_->stop();
    if (highlight_helper_ && !highlight_animation_->targetObject())
      highlight_animation_->setTargetObject(highlight_helper_);
    animation_group_->setDirection(QAbstractAnimation::Forward);
    animation_group_->start();
    if (!shadow_animation_)
      shadow_->setOpacity(1.0);
  }
  else if (highlight_helper_ && shadow_)
  {
    highlight_helper_->show();
    shadow_->setEnabled(true && shadow_enabled_);
  }
}

void ActiveGraphicsItemGroup::hideHighlighting()
{
  setZValue(0.0);
  if (animate_) {
    animation_group_->stop();
    if (highlight_helper_ && !highlight_animation_->targetObject())
      highlight_animation_->setTargetObject(highlight_helper_);
    animation_group_->setDirection(QAbstractAnimation::Backward);
    animation_group_->start();
    if (!shadow_animation_)
      shadow_->setOpacity(0.0);
  }
  else if (highlight_helper_ && shadow_)
  {
    highlight_helper_->hide();
    shadow_->setEnabled(false);
  }
}

void ActiveGraphicsItemGroup::updateHighlightColor()
{
  QColor c;
  if (highlighted_ && active_colors_.count(highlighted_) && hovering_)
    c = QColor::fromHsvF(
          0.5*(active_colors_[highlighted_].hsvHueF()+hover_color_.hsvHueF()),
          0.5*(active_colors_[highlighted_].hsvSaturationF()+hover_color_.hsvSaturationF()),
          0.5*(active_colors_[highlighted_].valueF()+hover_color_.valueF()),
          0.5*(active_colors_[highlighted_].alphaF()+hover_color_.alphaF())
          );
  else if (highlighted_ && active_colors_.count(highlighted_))
    c = active_colors_[highlighted_];
  else if (hovering_)
    c = hover_color_;
  shadow_->setColor(c);
  if (highlight_helper_)
    highlight_helper_->setBrush(c);
}

void ActiveGraphicsItemGroup::setShadowEnabled(bool enable)
{
  if (enable != shadow_enabled_) {
    shadow_enabled_ = enable;
    shadow_->setEnabled(enable && (highlighted_ || hovering_));
  }
}
