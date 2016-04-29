#include "TransitionRendered.h"

#include <QGraphicsTextItem>
#include <QFontMetrics>
#include <QList>
#include <QTextDocument>
#include <QGraphicsScene>

#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#if !defined(M_PI)
#define M_PI        3.14159265358979323846264338327950288
#endif

#include "ActiveGraphicsItemGroup.h"
#include "Level.h"
#include "GraphicsHighlightItem.h"
#include "custom_logger.h"

const double TransitionRendered::textAngle = -60.0;
const double TransitionRendered::arrowHeadLength = 11.0;
const double TransitionRendered::arrowBaseLength = 7.0;
const double TransitionRendered::arrowHeadWidth = 5.0;
const double TransitionRendered::arrowBaseWidth = 5.0;
const double TransitionRendered::highlightWidth = 5.0;
const QPolygonF TransitionRendered::arrowHeadShape = initArrowHead();
const QPolygonF TransitionRendered::arrowBaseShape = initArrowBase();


TransitionRendered::TransitionRendered()
  : ClickableItem(ClickableItem::GammaTransitionType),
    arrow(0), text(0), arrowhead(0), arrowbase(0), clickarea(0), highlightHelper(0), mindist(0.0)
{

}

TransitionRendered::TransitionRendered(TransitionPtr transition, SchemeVisualSettings vis, QGraphicsScene *scene)
  : TransitionRendered()
{
  if (!transition)
    return;

  from_ = transition->depopulatedLevel()->energy();
  to_   = transition->populatedLevel()->energy();

  m_pen = vis.gammaPen;
  if (transition->intensity() >= 5.0)
    m_pen = vis.intenseGammaPen;

  // group origin is set to the start level!
  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(QColor(64, 166, 255, 180));

  arrowhead = new QGraphicsPolygonItem(arrowHeadShape);
  arrowhead->setBrush(QBrush(m_pen.color()));
  arrowhead->setPen(Qt::NoPen);

  arrowbase = new QGraphicsPolygonItem(arrowBaseShape);
  arrowbase->setBrush(QBrush(m_pen.color()));
  arrowbase->setPen(Qt::NoPen);
  arrowbase->setPos(0.0, 0.0);

  arrow = new QGraphicsLineItem;
  arrow->setPen(m_pen);

  QString intensstr = QString::fromStdString(transition->intensityAsText());
  if (!intensstr.isEmpty())
    intensstr += " ";
  QString textstr = QString("<html><body bgcolor=\"white\">%1<b>%2</b> %3</body></html>")
      .arg(intensstr).arg(QString::fromStdString(transition->energy().to_string()))
      .arg(QString::fromStdString(transition->multipolarityAsText()));
  text = new QGraphicsTextItem;
  text->setFont(vis.gammaFont);
  text->document()->setDocumentMargin(0.0);
  text->setHtml(textstr);
  //new QGraphicsRectItem(text->boundingRect(), text);
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

TransitionRendered::~TransitionRendered()
{
}

void TransitionRendered::updateArrow(double arrowDestY)
{
  item->removeFromGroup(clickarea);
  item->removeFromGroup(arrowhead);
  item->removeFromGroup(arrowbase);
  item->removeFromGroup(arrow);
  item->removeHighlightHelper(highlightHelper);
  arrowhead->setPos(0.0, arrowDestY);
  arrow->setLine(0.0, 0.0, 0.0, arrowDestY - arrowHeadLength);
  clickarea->setRect(-0.5*mindist, 0.0, mindist, arrowDestY);
  highlightHelper->setRect(-0.5*highlightWidth, 0.0, highlightWidth, arrowDestY - arrowHeadLength);
  item->addHighlightHelper(highlightHelper);
  item->addToGroup(arrow);
  item->addToGroup(arrowbase);
  item->addToGroup(arrowhead);
  item->addToGroup(clickarea);
}

double TransitionRendered::minimalXDistance() const
{
  return mindist;
}

double TransitionRendered::widthFromOrigin() const
{
  return item->childrenBoundingRect().right();
}

QPen TransitionRendered::pen() const
{
  return m_pen;
}

QPolygonF TransitionRendered::initArrowHead()
{
  QPolygonF arrowheadpol;
  arrowheadpol << QPointF(0.0, 0.0);
  arrowheadpol << QPointF(0.5*arrowHeadWidth, -arrowHeadLength);
  arrowheadpol << QPointF(-0.5*arrowHeadWidth, -arrowHeadLength);
  return arrowheadpol;
}

QPolygonF TransitionRendered::initArrowBase()
{
  QPolygonF arrowbasepol;
  arrowbasepol << QPointF(0.0, arrowBaseLength);
  arrowbasepol << QPointF(0.5*arrowBaseWidth, 0.0);
  arrowbasepol << QPointF(-0.5*arrowBaseWidth, 0.0);
  return arrowbasepol;
}
