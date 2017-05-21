#include "ActiveGraphicsItemGroup.h"

#include <QPen>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>
#include "GraphicsHighlightItem.h"
#include "GraphicsDropShadowEffect.h"
#include "ClickableItem.h"

const QColor ActiveGraphicsItemGroup::hoverColor(178, 223, 150, 180);
const double ActiveGraphicsItemGroup::animationDuration = 100.0;
const bool ActiveGraphicsItemGroup::animateShadow = false;
const bool ActiveGraphicsItemGroup::animate = false;

ActiveGraphicsItemGroup::ActiveGraphicsItemGroup(ClickableItem *associatedItem)
  : assocItem(associatedItem), shadow(new GraphicsDropShadowEffect), m_shape(0), m_helper(0),
    m_highlighted(false), m_hover(false), m_shadowenabled(true),
    aniHighlight(0), aniShadow(0), aniGroup(0)
{
  // prepare highlighting
  shadow->setBlurRadius(15.0);
  shadow->setOffset(QPointF(0.0, 0.0));

  setGraphicsEffect(shadow);
  setAcceptHoverEvents(true);

  if (animate) {
    shadow->setOpacity(0.0);
    aniHighlight = new QPropertyAnimation(m_helper, "opacity");
    aniHighlight->setDuration(animationDuration);
    aniHighlight->setStartValue(0.0);
    aniHighlight->setEndValue(1.0);
    aniShadow = new QPropertyAnimation(shadow, "opacity");
    aniShadow->setDuration(animationDuration);
    aniShadow->setStartValue(0.0);
    aniShadow->setEndValue(1.0);
    aniGroup = new QParallelAnimationGroup;
    aniGroup->addAnimation(aniHighlight);
    if (aniShadow)
      aniGroup->addAnimation(aniShadow);
  }
  else {
    shadow->setEnabled(false);
  }
}

ActiveGraphicsItemGroup::~ActiveGraphicsItemGroup()
{
  delete m_shape;
  delete aniGroup;
}

void ActiveGraphicsItemGroup::addToGroup(QGraphicsItem *item)
{
  delete m_shape;
  m_shape = 0;
  QGraphicsItemGroup::addToGroup(item);
}

void ActiveGraphicsItemGroup::addHighlightHelper(GraphicsHighlightItem *helperitem)
{
  delete m_helper;
  m_helper = helperitem;
  if (animate) {
    m_helper->setOpacity(0.0);
  }
  else {
    m_helper->setOpacity(1.0);
    m_helper->hide();
  }
  m_helper->setPen(Qt::NoPen);
  m_helper->setBrush(QBrush(hoverColor));
  addToGroup(m_helper);
}

void ActiveGraphicsItemGroup::removeHighlightHelper(GraphicsHighlightItem *helperitem)
{
  m_helper = 0;
  removeFromGroup(helperitem);
}


void ActiveGraphicsItemGroup::setPos(const QPointF &pos)
{
  QGraphicsItemGroup::setPos(pos);
  // force recreation of bounding rect (workaround)
  QGraphicsItem *tmp = childItems().last();
  removeFromGroup(tmp);
  addToGroup(tmp);
  delete m_shape;
  m_shape = 0;
}

void ActiveGraphicsItemGroup::setPos(qreal x, qreal y)
{
  setPos(QPointF(x, y));
}

QPainterPath ActiveGraphicsItemGroup::shape() const
{
  if (!m_shape) {
    m_shape = new QPainterPath;
    foreach (QGraphicsItem *child, childItems()) {
      m_shape->addPath(mapFromItem(child, child->shape()));
    }
  }
  return *m_shape;
}

void ActiveGraphicsItemGroup::setActiveColor(const QColor &color)
{
  activeColor = color;
}

void ActiveGraphicsItemGroup::setHighlighted(bool hl)
{
  m_highlighted = hl;
  if (!m_highlighted && !m_hover)
    hideHighlighting();
  if (m_highlighted && !m_hover)
    showHighlighting();

  if (hl || (!hl && m_hover))
    updateHighlightColor();
}

bool ActiveGraphicsItemGroup::isHighlighted() const
{
  return m_highlighted;
}

void ActiveGraphicsItemGroup::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  m_hover = true;
  updateHighlightColor();
  if (!m_highlighted)
    showHighlighting();
}

void ActiveGraphicsItemGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  m_hover = false;
  if (m_highlighted)
    updateHighlightColor();
  if (!m_highlighted)
    hideHighlighting();
}

void ActiveGraphicsItemGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->buttons() & (Qt::LeftButton | Qt::RightButton))
    emit clicked(assocItem);
}

void ActiveGraphicsItemGroup::showHighlighting()
{
  setZValue(1.0);
  if (animate) {
    aniGroup->stop();
    if (m_helper && !aniHighlight->targetObject())
      aniHighlight->setTargetObject(m_helper);
    aniGroup->setDirection(QAbstractAnimation::Forward);
    aniGroup->start();
    if (!aniShadow)
      shadow->setOpacity(1.0);
  }
  else {
    m_helper->show();
    shadow->setEnabled(true && m_shadowenabled);
  }
}

void ActiveGraphicsItemGroup::hideHighlighting()
{
  setZValue(0.0);
  if (animate) {
    aniGroup->stop();
    if (m_helper && !aniHighlight->targetObject())
      aniHighlight->setTargetObject(m_helper);
    aniGroup->setDirection(QAbstractAnimation::Backward);
    aniGroup->start();
    if (!aniShadow)
      shadow->setOpacity(0.0);
  }
  else {
    m_helper->hide();
    shadow->setEnabled(false);
  }
}

void ActiveGraphicsItemGroup::updateHighlightColor()
{
  QColor c;
  if (m_highlighted && m_hover)
    c = QColor::fromHsvF(
          0.5*(activeColor.hsvHueF()+hoverColor.hsvHueF()),
          0.5*(activeColor.hsvSaturationF()+hoverColor.hsvSaturationF()),
          0.5*(activeColor.valueF()+hoverColor.valueF()),
          0.5*(activeColor.alphaF()+hoverColor.alphaF())
          );
  else if (m_highlighted)
    c = activeColor;
  else if (m_hover)
    c = hoverColor;
  shadow->setColor(c);
  if (m_helper)
    m_helper->setBrush(c);
}

void ActiveGraphicsItemGroup::setShadowEnabled(bool enable)
{
  if (enable != m_shadowenabled) {
    m_shadowenabled = enable;
    shadow->setEnabled(enable && (m_highlighted || m_hover));
  }
}
