#include "LevelItem.h"
#include "ActiveGraphicsItemGroup.h"
#include "GraphicsHighlightItem.h"
#include <QFontMetrics>
#include <QTextItem>
#include <QGraphicsScene>
#include <QTextDocument>
#include <util/logger.h>


FeedingArrow::FeedingArrow(Level level, ParentPosition parentpos,
                           SchemeVisualSettings vis,
                           QGraphicsScene *scene)
{
  if (level.normalizedFeedIntensity().uncertaintyType()
      == Uncert::UndefinedType)
    return;

  t = FeedingType;
  energy_ = level.energy();

  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(0, vis.inactive_color());
  item->setActiveColor(1, vis.selected_color());
  item->setActiveColor(2, vis.implicated_color());
  item->setHoverColor(vis.hover_color());

  // create line
  arrow_ = new QGraphicsLineItem;
  arrow_->setPen(vis.feedArrowPen);
  item->addToGroup(arrow_);

  // create arrow head
  QPolygonF arrowpol;
  arrowpol << QPointF(0.0, 0.0);
  arrowpol << QPointF((parentpos == RightParent ? 1.0 : -1.0) * vis.feedingArrowHeadLength, 0.5*vis.feedingArrowHeadWidth);
  arrowpol << QPointF((parentpos == RightParent ? 1.0 : -1.0) * vis.feedingArrowHeadLength, -0.5*vis.feedingArrowHeadWidth);
  arrowhead_ = new QGraphicsPolygonItem(arrowpol);
  arrowhead_->setBrush(QColor(arrow_->pen().color()));
  arrowhead_->setPen(Qt::NoPen);
  item->addToGroup(arrowhead_);

  // create intensity label
  intensity_ = new QGraphicsTextItem;
  intensity_->setHtml(QString("%1 %").arg(QString::fromStdString(level.normalizedFeedIntensity().to_markup())));
  intensity_->document()->setDocumentMargin(0);
  intensity_->setFont(vis.feedIntensityFont());
  item->addToGroup(intensity_);

  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont());

  click_area_
      = new QGraphicsRectItem(-vis.outerGammaMargin,
                              -0.5*stdBoldFontMetrics.height(),
                              2.0*vis.outerGammaMargin,
                              stdBoldFontMetrics.height());
  click_area_->setPen(Qt::NoPen);
  click_area_->setBrush(Qt::NoBrush);
  item->addToGroup(click_area_);


  highlight_helper_
      = new GraphicsHighlightItem(-vis.outerGammaMargin,
                                  -0.5*vis.highlightWidth,
                                  2.0*vis.outerGammaMargin,
                                  vis.highlightWidth);
  highlight_helper_->setOpacity(0.0);
  item->addHighlightHelper(highlight_helper_);

  scene->addItem(item);
}

void FeedingArrow::align(double arrowY,
                           double leftlinelength,
                           double rightlinelength,
                           double arrowleft,
                           double arrowright,
                           ParentPosition parentpos,
                           SchemeVisualSettings vis)
{
  if (!arrow_)
    return;

  double leftend = (parentpos == RightParent) ? rightlinelength + vis.feedingArrowGap + vis.feedingArrowHeadLength : arrowleft;
  double rightend = (parentpos == RightParent) ? arrowright : -leftlinelength - vis.feedingArrowGap - vis.feedingArrowHeadLength;
  arrow_->setLine(leftend, arrowY, rightend, arrowY);
  arrowhead_->setPos((parentpos == RightParent) ? rightlinelength + vis.feedingArrowGap : -leftlinelength - vis.feedingArrowGap, arrowY);
  intensity_->setPos(leftend + 15.0, arrowY - intensity_->boundingRect().height());

  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont());

  item->removeFromGroup(click_area_);
  item->removeHighlightHelper(highlight_helper_);
  highlight_helper_->setRect(leftend,
                             arrowY - 0.5*vis.highlightWidth,
                             rightend - leftend,
                             vis.highlightWidth);
  click_area_->setRect(leftend,
                       arrowY - 0.5*stdBoldFontMetrics.height(),
                       rightend - leftend,
                       stdBoldFontMetrics.height());
  item->addHighlightHelper(highlight_helper_);
  item->addToGroup(click_area_);
}

Energy FeedingArrow::energy() const
{
  return energy_;
}

double FeedingArrow::intensity_width() const
{
  if (intensity_)
    return intensity_->boundingRect().width();
  return 0;
}



Energy LevelItem::energy() const
{
  return energy_;
}

int LevelItem::energy_width() const
{
  if (etext_)
    return etext_->boundingRect().width();
  return 0;
}

int LevelItem::spin_width() const
{
  if (spintext_)
    return spintext_->boundingRect().width();
  return 0;
}

double LevelItem::ypos() const
{
  return ypos_;
}

double LevelItem::bottom_ypos() const
{
  return ypos_ + 0.5 * line_->pen().widthF();
}

double LevelItem::nuc_line_width() const
{
  return etext_->boundingRect().width()
      + spintext_->boundingRect().width();
}

void LevelItem::set_funky_position(double left, double right,
                                   double y,
                                   SchemeVisualSettings vis)
{
  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont());
  item->removeFromGroup(click_area_);
  item->removeFromGroup(line_);
  item->removeHighlightHelper(highlight_helper_);
  line_->setLine(left, y, right, y);
  highlight_helper_->setRect(left,
                             y - 0.5*vis.highlightWidth,
                             right - left,
                             vis.highlightWidth);
  click_area_->setRect(left,
                       y - 0.5*stdBoldFontMetrics.height(),
                       right - left,
                       stdBoldFontMetrics.height());
  item->addHighlightHelper(highlight_helper_);
  item->addToGroup(line_);
  item->addToGroup(click_area_);
}

void LevelItem::set_funky2_position(double xe,
                                    double xspin,
                                    double y)
{
  double yy = y - etext_->boundingRect().height();
  etext_->setPos(xe - etext_->boundingRect().width(), yy);
  spintext_->setPos(xspin, yy);
}

double LevelItem::max_y_height() const
{
  return
      qMax(etext_->boundingRect().height(),
           spintext_->boundingRect().height());
}

void LevelItem::set_ypos(double new_ypos)
{
  // add 0.5*pen-width to avoid antialiasing artifacts
  ypos_ = new_ypos + 0.5 * line_->pen().widthF();
}

double LevelItem::above_ypos(double offset)
{
  return std::floor(ypos_ - offset);
}

LevelItem::LevelItem(Level level, Type type, ParentPosition parentpos,
                     SchemeVisualSettings vis,
                     QGraphicsScene *scene)
  : LevelItem()
{
  if (!level.energy().valid())
    return;

  t = type;
  energy_ = level.energy();

  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont());

  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(0, vis.inactive_color());
  item->setActiveColor(1, vis.selected_color());
  item->setActiveColor(2, vis.implicated_color());
  item->setHoverColor(vis.hover_color());

  line_ = new QGraphicsLineItem(-vis.outerGammaMargin, 0.0, vis.outerGammaMargin, 0.0, item);
  line_->setPen(vis.levelPen);
  // thick line for stable/isomeric levels
  if (level.halfLife().stable() || level.isomerNum() > 0)
    line_->setPen(vis.stableLevelPen);

  click_area_
      = new QGraphicsRectItem(-vis.outerGammaMargin,
                              -0.5*stdBoldFontMetrics.height(),
                              2.0*vis.outerGammaMargin,
                              stdBoldFontMetrics.height());
  click_area_->setPen(Qt::NoPen);
  click_area_->setBrush(Qt::NoBrush);

  highlight_helper_
      = new GraphicsHighlightItem(-vis.outerGammaMargin,
                                  -0.5*vis.highlightWidth,
                                  2.0*vis.outerGammaMargin,
                                  vis.highlightWidth);
  highlight_helper_->setOpacity(0.0);

  QString etext = QString::fromStdString(energy_.to_string());
  etext_ = new QGraphicsSimpleTextItem(etext, item);
  etext_->setFont(vis.stdBoldFont());
  etext_->setPos(0.0, -stdBoldFontMetrics.height());

  QString spintext = QString::fromStdString(level.spins().to_pretty_string());
  spintext_ = new QGraphicsSimpleTextItem(spintext, item);
  spintext_->setFont(vis.stdBoldFont());
  spintext_->setPos(0.0, -stdBoldFontMetrics.height());

  if (parentpos != NoParent)
  {
    QString hltext
        = QString::fromStdString(level.halfLife().preferred_units().to_string());
    hltext_ = new QGraphicsSimpleTextItem(hltext, item);
    hltext_->setFont(vis.stdFont());
    hltext_->setPos(0.0, -0.5*stdBoldFontMetrics.height());
    item->addToGroup(hltext_);
  }

  item->addHighlightHelper(highlight_helper_);
  item->addToGroup(line_);
  item->addToGroup(click_area_);
  item->addToGroup(etext_);
  item->addToGroup(spintext_);
  scene->addItem(item);
}

void LevelItem::align(double leftlinelength, double rightlinelength,
                      ParentPosition parentpos, SchemeVisualSettings vis)
{
  QFontMetrics stdFontMetrics(vis.stdFont());
  QFontMetrics stdBoldFontMetrics(vis.stdBoldFont());

  set_funky_position(-leftlinelength, rightlinelength, 0, vis);

  item->removeFromGroup(spintext_);
  item->removeFromGroup(etext_);
  spintext_->setPos(-leftlinelength + vis.outerLevelTextMargin, -stdBoldFontMetrics.height());
  etext_->setPos(rightlinelength - vis.outerLevelTextMargin - stdBoldFontMetrics.horizontalAdvance(etext_->text()), -etext_->boundingRect().height());
  item->addToGroup(etext_);
  item->addToGroup(spintext_);

  if (parentpos != NoParent)
    item->removeFromGroup(hltext_);
  double levelHlPos = 0.0;
  if (parentpos == RightParent && hltext_)
    levelHlPos = -leftlinelength
        - vis.levelToHalfLifeDistance
        - stdFontMetrics.horizontalAdvance(hltext_->text());
  else
    levelHlPos = rightlinelength + vis.levelToHalfLifeDistance;
  if (parentpos != NoParent)
    hltext_->setPos(levelHlPos, -0.5*stdBoldFontMetrics.height());
  item->addToGroup(hltext_);

  item->setPos(0.0, ypos_);
}

