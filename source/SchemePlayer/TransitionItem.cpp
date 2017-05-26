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
#include "Level.h"
#include "GraphicsHighlightItem.h"
#include "custom_logger.h"

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

  m_pen = vis.gammaPen;

  // group origin is set to the start level!
  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(QColor(64, 166, 255, 180));

  arrowhead = new QGraphicsPolygonItem(initArrowHead(arrowHeadWidth));
  arrowhead->setBrush(QBrush(m_pen.color()));
  arrowhead->setPen(Qt::NoPen);

  arrowbase = new QGraphicsPolygonItem(initArrowBase(arrowBaseWidth));
  arrowbase->setBrush(QBrush(m_pen.color()));
  arrowbase->setPen(Qt::NoPen);
  arrowbase->setPos(0.0, 0.0);

  arrow = new QGraphicsLineItem;
  arrow->setPen(m_pen);

  QString intensstr = QString::fromStdString(transition.intensity_string());
  if (!intensstr.isEmpty())
    intensstr += " ";
  QString textstr = QString("<html><body bgcolor=\"white\">%1<b>%2</b> %3</body></html>")
      .arg(intensstr).arg(QString::fromStdString(transition.energy().to_string()))
      .arg(QString::fromStdString(transition.multipolarity()));
  text = new QGraphicsTextItem;
  text->setFont(vis.gammaFont);
  text->document()->setDocumentMargin(0.0);
  text->setHtml(textstr);
//  new QGraphicsRectItem(text->boundingRect(), text);
  double textheight = text->boundingRect().height();
  text->setPos(0.0, -textheight);
  text->setTransformOriginPoint(0.0, 0.5*textheight);
  text->setRotation(textAngle);
  mindist = std::abs(textheight / std::sin(-textAngle/180.0*M_PI));
  text->moveBy(0.5*textheight*std::sin(-textAngle/180.0*M_PI) - 0.5*mindist, 0.0);

  clickarea = new QGraphicsRectItem(-0.5*mindist, 0.0, mindist, 0.0);
  clickarea->setPen(Qt::NoPen);
  clickarea->setBrush(Qt::NoBrush);

  highlightHelper = new GraphicsHighlightItem(-0.5*highlightWidth, 0.0, highlightWidth, 0.0);

  item->addHighlightHelper(highlightHelper);
  item->addToGroup(arrowbase);
  item->addToGroup(text);
  item->addToGroup(arrow);
  item->addToGroup(arrowhead);
  item->addToGroup(clickarea);

  scene->addItem(item);

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

void TransitionItem::updateArrow(double arrowDestY, double max_intensity)
{
  item->removeFromGroup(clickarea);
  item->removeFromGroup(arrowhead);
  item->removeFromGroup(arrowbase);
  item->removeFromGroup(arrow);
  item->removeHighlightHelper(highlightHelper);
  arrowhead->setPos(0.0, arrowDestY - arrowHeadLength);
  arrow->setLine(0.0, 0.0, 0.0, arrowDestY - arrowHeadLength);
  clickarea->setRect(-0.5*mindist, 0.0, mindist, arrowDestY);
  highlightHelper->setRect(-0.5*highlightWidth, 0.0, highlightWidth, arrowDestY - arrowHeadLength);
  item->addHighlightHelper(highlightHelper);
  item->addToGroup(arrow);
  item->addToGroup(arrowbase);
  item->addToGroup(arrowhead);
  item->addToGroup(clickarea);

//  double width = std::ceil(maxwidth * transition_.intensity() / max_intensity);

//  auto linepen = m_pen;
//  linepen.setWidth(width);
//  arrow->setPen(linepen);

//  arrowhead->setPolygon(initArrowBase(width + arrowHeadWidth));
//  arrowbase->setPolygon(initArrowBase(width + arrowBaseWidth));
}

double TransitionItem::minimalXDistance() const
{
  return std::max(mindist, maxwidth);
}

double TransitionItem::widthFromOrigin() const
{
  return item->childrenBoundingRect().right();
}

QPen TransitionItem::pen() const
{
  return m_pen;
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
