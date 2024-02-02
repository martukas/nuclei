#include "TransitionItem.h"

#include <QGraphicsTextItem>
#include <QFontMetrics>
#include <QList>
#include <QTextDocument>
#include <QGraphicsScene>

#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
//#if !defined(M_PI)
//#define M_PI        3.14159265358979323846264338327950288
//#endif

#include "ActiveGraphicsItemGroup.h"
#include <NucData/Level.h>
#include "GraphicsHighlightItem.h"
#include <util/logger.h>

const double TransitionItem::textAngle = -60.0;
const double TransitionItem::arrowHeadLength = 11.0;
const double TransitionItem::arrowBaseLength = 7.0;
const double TransitionItem::arrowHeadWidth = 7.0;
const double TransitionItem::arrowBaseWidth = 5.0;
const double TransitionItem::highlightWidth = 5.0;
const double TransitionItem::maxwidth = 0;//50;

TransitionItem::TransitionItem()
  : ClickableItem(ClickableItem::GammaTransitionType)
{}

TransitionItem::TransitionItem(Transition transition,
                               SchemeVisualSettings vis,
                               QGraphicsScene *scene)
  : TransitionItem()
{
  if (!transition.energy().valid())
    return;

  transition_ = transition;

  pen_ = vis.gammaPen;

  // group origin is set to the start level!
  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(0, vis.inactive_color());
  item->setActiveColor(1, vis.selected_color());
  item->setActiveColor(2, vis.implicated_color());
  item->setHoverColor(vis.hover_color());
  scene->addItem(item);

  arrow_head_ = new QGraphicsPolygonItem(initArrowHead(arrowHeadWidth));
  arrow_head_->setBrush(QBrush(pen_.color()));
  arrow_head_->setPen(Qt::NoPen);
  item->addToGroup(arrow_head_);

  arrow_base_ = new QGraphicsPolygonItem(initArrowBase(arrowBaseWidth));
  arrow_base_->setBrush(QBrush(pen_.color()));
  arrow_base_->setPen(Qt::NoPen);
  arrow_base_->setPos(0.0, 0.0);
  item->addToGroup(arrow_base_);

  arrow_ = new QGraphicsLineItem;
  arrow_->setPen(pen_);
  item->addToGroup(arrow_);

  QString intensstr;
  if (transition.intensity().defined())
    intensstr = QString::fromStdString(transition.intensity_string()) + " ";
  QString textstr
      = QString("<html><body bgcolor=\"white\">%1<b>%2</b> %3</body></html>")
      .arg(intensstr).arg(QString::fromStdString(transition.energy().to_string()))
      .arg(QString::fromStdString(transition.multipolarity()));
  text_ = new QGraphicsTextItem;
  text_->setFont(vis.gammaFont());
  text_->document()->setDocumentMargin(0.0);
  text_->setHtml(textstr);
//  new QGraphicsRectItem(text_->boundingRect(), text_);
  double textheight = text_->boundingRect().height();
  text_->setPos(0.0, -textheight);
  text_->setTransformOriginPoint(0.0, 0.5*textheight);
  text_->setRotation(textAngle);
  min_x_distance_ = std::abs(textheight / std::sin(-textAngle/180.0*M_PI));
  text_->moveBy(0.5*textheight*std::sin(-textAngle/180.0*M_PI)
                - 0.5*min_x_distance_, 0.0);
  item->addToGroup(text_);

  click_area_ = new QGraphicsRectItem(-0.5*min_x_distance_, 0.0,
                                      min_x_distance_, 0.0);
  click_area_->setPen(Qt::NoPen);
  click_area_->setBrush(Qt::NoBrush);
  item->addToGroup(click_area_);

  highlight_helper_
      = new GraphicsHighlightItem(
        -0.5*highlightWidth, 0.0,
        highlightWidth, 0.0);
  item->addHighlightHelper(highlight_helper_);

//  updateArrow();
}

TransitionItem::~TransitionItem()
{
}

double TransitionItem::intensity() const
{
  if (transition_.intensity().hasFiniteValue())
    return transition_.intensity().value();
  return 0;
}

Energy TransitionItem::energy() const
{
  return transition_.energy();
}

Energy TransitionItem::from() const
{
  return transition_.from();
}

Energy TransitionItem::to() const
{
  return transition_.to();
}

void TransitionItem::updateArrow(double arrowDestY, [[maybe_unused]] double max_intensity)
{
  item->removeFromGroup(click_area_);
  item->removeFromGroup(arrow_head_);
  item->removeFromGroup(arrow_base_);
  item->removeFromGroup(arrow_);
  item->removeHighlightHelper(highlight_helper_);
  arrow_head_->setPos(0.0, arrowDestY - arrowHeadLength);
  arrow_->setLine(0.0, 0.0, 0.0, arrowDestY - arrowHeadLength);
  click_area_->setRect(-0.5*min_x_distance_, 0.0,
                       min_x_distance_, arrowDestY);
  highlight_helper_->setRect(
        -0.5*highlightWidth, 0.0,
        highlightWidth, arrowDestY - arrowHeadLength);
  item->addHighlightHelper(highlight_helper_);
  item->addToGroup(arrow_);
  item->addToGroup(arrow_base_);
  item->addToGroup(arrow_head_);
  item->addToGroup(click_area_);

//  double width = std::ceil(maxwidth * transition_.intensity() / max_intensity);

//  auto linepen = pen_;
//  linepen.setWidth(width);
//  arrow_->setPen(linepen);

//  arrow_head_->setPolygon(initArrowBase(width + arrowHeadWidth));
//  arrow_base_->setPolygon(initArrowBase(width + arrowBaseWidth));
}

double TransitionItem::minimalXDistance() const
{
  return std::max(min_x_distance_, maxwidth);
}

double TransitionItem::widthFromOrigin() const
{
  return item->childrenBoundingRect().right();
}

QPen TransitionItem::pen() const
{
  return pen_;
}

QPolygonF TransitionItem::initArrowHead(double width)
{
  QPolygonF arrowheadpol;
  arrowheadpol << QPointF( 0.0, arrowHeadLength);
  arrowheadpol << QPointF( 0.5*width, 0);
  arrowheadpol << QPointF(-0.5*width, 0);
  return arrowheadpol;
}

QPolygonF TransitionItem::initArrowBase(double width)
{
  QPolygonF arrowbasepol;
  arrowbasepol << QPointF( 0.0, arrowBaseLength);
  arrowbasepol << QPointF( 0.5*width, 0.0);
  arrowbasepol << QPointF(-0.5*width, 0.0);
  return arrowbasepol;
}
