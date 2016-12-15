#include "LevelItem.h"
#include "ActiveGraphicsItemGroup.h"
#include "GraphicsHighlightItem.h"
#include <QFontMetrics>
#include <QTextItem>
#include <QGraphicsScene>
#include <QTextDocument>
#include "custom_logger.h"

LevelItem::LevelItem()
  : ClickableItem(ClickableItem::EnergyLevelType),
  graline(0), grafeedarrow(0), graarrowhead(0), graetext(0), graspintext(0), grahltext(0), grafeedintens(0),
  graclickarea(0), grahighlighthelper(0), graYPos(0.0)
{
}

LevelItem::LevelItem(Level level, SchemeVisualSettings vis, QGraphicsScene *scene)
  : LevelItem()
{
  if (!level.energy().isValid())
    return;

  energy_ = level.energy();

  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont);

  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(QColor(224, 186, 100, 180));

  graline = new QGraphicsLineItem(-vis.outerGammaMargin, 0.0, vis.outerGammaMargin, 0.0, item);
  graline->setPen(vis.levelPen);
  // thick line for stable/isomeric levels
  if (level.halfLife().isStable() || level.isomerNum() > 0)
    graline->setPen(vis.stableLevelPen);

  graclickarea = new QGraphicsRectItem(-vis.outerGammaMargin, -0.5*stdBoldFontMetrics.height(),
                                       2.0*vis.outerGammaMargin, stdBoldFontMetrics.height());
  graclickarea->setPen(Qt::NoPen);
  graclickarea->setBrush(Qt::NoBrush);

  grahighlighthelper = new GraphicsHighlightItem(-vis.outerGammaMargin, -0.5*vis.highlightWidth,
                                                 2.0*vis.outerGammaMargin, vis.highlightWidth);
  grahighlighthelper->setOpacity(0.0);

  QString etext = QString::fromStdString(energy_.to_string());
  graetext = new QGraphicsSimpleTextItem(etext, item);
  graetext->setFont(vis.stdBoldFont);
  graetext->setPos(0.0, -stdBoldFontMetrics.height());

  QString spintext = QString::fromStdString(level.spin().to_string());
  graspintext = new QGraphicsSimpleTextItem(spintext, item);
  graspintext->setFont(vis.stdBoldFont);
  graspintext->setPos(0.0, -stdBoldFontMetrics.height());

  if (vis.parentpos != NoParent) {
    QString hltext = QString::fromStdString(level.halfLife().to_string());
    grahltext = new QGraphicsSimpleTextItem(hltext, item);
    grahltext->setFont(vis.stdFont);
    grahltext->setPos(0.0, -0.5*stdBoldFontMetrics.height());
    item->addToGroup(grahltext);
  }

  item->addHighlightHelper(grahighlighthelper);
  item->addToGroup(graline);
  item->addToGroup(graclickarea);
  item->addToGroup(graetext);
  item->addToGroup(graspintext);
  scene->addItem(item);

  // plot level feeding arrow if necessary
  if (level.normalizedFeedIntensity().uncertaintyType() != UncertainDouble::UndefinedType) {
    // create line
    grafeedarrow = new QGraphicsLineItem;
    grafeedarrow->setPen(vis.feedArrowPen);
    scene->addItem(grafeedarrow);
    // create arrow head
    QPolygonF arrowpol;
    arrowpol << QPointF(0.0, 0.0);
    arrowpol << QPointF((vis.parentpos == RightParent ? 1.0 : -1.0) * vis.feedingArrowHeadLength, 0.5*vis.feedingArrowHeadWidth);
    arrowpol << QPointF((vis.parentpos == RightParent ? 1.0 : -1.0) * vis.feedingArrowHeadLength, -0.5*vis.feedingArrowHeadWidth);
    graarrowhead = new QGraphicsPolygonItem(arrowpol);
    graarrowhead->setBrush(QColor(grafeedarrow->pen().color()));
    graarrowhead->setPen(Qt::NoPen);
    scene->addItem(graarrowhead);
    // create intensity label
    grafeedintens = new QGraphicsTextItem;
    grafeedintens->setHtml(QString("%1 %").arg(QString::fromStdString(level.normalizedFeedIntensity().to_markup())));
    grafeedintens->document()->setDocumentMargin(0);
    grafeedintens->setFont(vis.feedIntensityFont);
    scene->addItem(grafeedintens);
  }
}

double LevelItem::align(double leftlinelength, double rightlinelength, double arrowleft, double arrowright,
                          SchemeVisualSettings vis)
{
  QFontMetrics stdFontMetrics(vis.stdFont);
  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont);

  // temporarily remove items from group (workaround)
  item->removeFromGroup(graspintext);
  item->removeFromGroup(graetext);
  item->removeFromGroup(graclickarea);
  item->removeFromGroup(graline);
  item->removeHighlightHelper(grahighlighthelper);
  if (vis.parentpos != NoParent)
    item->removeFromGroup(grahltext);

  // rescale
  grahighlighthelper->setRect(-leftlinelength, -0.5*vis.highlightWidth, leftlinelength+rightlinelength, vis.highlightWidth);
  graline->setLine(-leftlinelength, 0.0, rightlinelength, 0.0);
  graclickarea->setRect(-leftlinelength, -0.5*stdBoldFontMetrics.height(), leftlinelength+rightlinelength, stdBoldFontMetrics.height());
  graspintext->setPos(-leftlinelength + vis.outerLevelTextMargin, -stdBoldFontMetrics.height());
  graetext->setPos(rightlinelength - vis.outerLevelTextMargin - stdBoldFontMetrics.width(graetext->text()), -graetext->boundingRect().height());
  double levelHlPos = 0.0;
  if (vis.parentpos == RightParent && grahltext)
    levelHlPos = -leftlinelength - vis.levelToHalfLifeDistance - stdFontMetrics.width(grahltext->text());
  else
    levelHlPos = rightlinelength + vis.levelToHalfLifeDistance;

  if (vis.parentpos != NoParent)
    grahltext->setPos(levelHlPos, -0.5*stdBoldFontMetrics.height());

  // re-add items to group
  item->addHighlightHelper(grahighlighthelper);
  item->addToGroup(graline);
  item->addToGroup(graclickarea);
  item->addToGroup(graetext);
  item->addToGroup(graspintext);
  item->addToGroup(grahltext);

  item->setPos(0.0, graYPos); // add 0.5*pen-width to avoid antialiasing artifacts

  if (grafeedarrow)
  {
    double leftend = (vis.parentpos == RightParent) ? rightlinelength + vis.feedingArrowGap + vis.feedingArrowHeadLength : arrowleft;
    double rightend = (vis.parentpos == RightParent) ? arrowright : -leftlinelength - vis.feedingArrowGap - vis.feedingArrowHeadLength;
    double arrowY = graYPos;
    grafeedarrow->setLine(leftend, arrowY, rightend, arrowY);
    graarrowhead->setPos((vis.parentpos == RightParent) ? rightlinelength + vis.feedingArrowGap : -leftlinelength - vis.feedingArrowGap, arrowY);
    grafeedintens->setPos(leftend + 15.0, arrowY - grafeedintens->boundingRect().height());
    return arrowY + 0.5*grafeedarrow->pen().widthF();
  }
  return std::numeric_limits<double>::quiet_NaN();
}

