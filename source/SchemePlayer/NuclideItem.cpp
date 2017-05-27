#include "NuclideItem.h"
#include <QGraphicsScene>
#include "ActiveGraphicsItemGroup.h"
#include <QGraphicsSimpleTextItem>
#include <QFontMetrics>
#include <QBrush>
#include <boost/algorithm/string.hpp>
#include "GraphicsHighlightItem.h"
#include "custom_logger.h"
#include "qpx_util.h"

NuclideItem::NuclideItem()
  : ClickableItem(ClickableItem::InvalidType)
{
}

void NuclideItem::position_arrow(double x, double start, double end)
{
  if (std::isfinite(start) && std::isfinite(end))
    vertical_arrow_->setLine(x, start, x, end);
}

void NuclideItem::position_text(double parent_center_x,
                                double ypos)
{
  halflife_text_->setPos(parent_center_x - 0.5*halflife_text_->boundingRect().width(),
                         ypos - 12.0);
}


NuclideItem::NuclideItem(const Nuclide &nuc, Type tp,
                         SchemeVisualSettings vis,
                         QGraphicsScene *scene)
  : ClickableItem(tp)
{
  if (nuc.empty())
  {
    t = ClickableItem::InvalidType;
    return;
  }

  if ((tp != ParentNuclideType) && (tp != DaughterNuclideType))
    return;

  id_ = nuc.id();
  item = new ActiveGraphicsItemGroup(this);

  double numberToNameDistance = 4.0;

  QFontMetrics nucFontMetrics(vis.nucFont);
  QFontMetrics nucIndexFontMetrics(vis.nucIndexFont);

  QGraphicsSimpleTextItem *granuc = new QGraphicsSimpleTextItem(QString::fromStdString(nuc.id().element()), item);
  granuc->setFont(vis.nucFont);
  granuc->setBrush(QBrush(QColor(64, 166, 255)));

  QGraphicsSimpleTextItem *graA = new QGraphicsSimpleTextItem(QString::number(nuc.id().A()), item);
  graA->setFont(vis.nucIndexFont);
  graA->setBrush(QBrush(QColor(64, 166, 255)));

  QGraphicsSimpleTextItem *graZ = new QGraphicsSimpleTextItem(QString::number(nuc.id().Z()), item);
  graZ->setFont(vis.nucIndexFont);
  graZ->setBrush(QBrush(QColor(64, 166, 255)));

  double numberwidth
      = qMax(graA->boundingRect().width(),
             graZ->boundingRect().width());

  granuc->setPos(numberwidth + numberToNameDistance,
                 0.2*nucIndexFontMetrics.ascent());
  graA->setPos(numberwidth - graA->boundingRect().width(), 0.0);
  graZ->setPos(numberwidth - graZ->boundingRect().width(), 1.2*nucIndexFontMetrics.ascent());

  click_area_
      = new QGraphicsRectItem(numberwidth - graA->boundingRect().width(),
                              0.2*nucIndexFontMetrics.ascent(),
                              numberwidth + numberToNameDistance + granuc->boundingRect().width(),
                              nucFontMetrics.height());
  click_area_->setPen(Qt::NoPen);
  click_area_->setBrush(Qt::NoBrush);

  highlight_helper_
      = new GraphicsHighlightItem(numberwidth - graA->boundingRect().width(),
                                  0.2*nucIndexFontMetrics.ascent(),
                                  numberwidth + numberToNameDistance + granuc->boundingRect().width(),
                                  nucFontMetrics.height());
  item->setHoverColor(QColor(64, 166, 255, 100));
  highlight_helper_->setOpacity(0.0);


  // added in the end to work around a bug in QGraphicsItemGroup:
  //   it does not update boundingRect if contents are moved after adding them
  item->addHighlightHelper(highlight_helper_);
  item->addToGroup(granuc);
  item->addToGroup(graA);
  item->addToGroup(graZ);
  item->addToGroup(click_area_);

  if (tp == ParentNuclideType)
  {
    // create half-life label
    halflife_text_ = new QGraphicsSimpleTextItem(QString::fromStdString(nuc.halfLifeAsText()));
    halflife_text_->setFont(vis.parentHlFont);
    scene->addItem(halflife_text_);

    // create vertical arrow component
    vertical_arrow_ = new QGraphicsLineItem;
    vertical_arrow_->setPen(vis.feedArrowPen);
    scene->addItem(vertical_arrow_);
  }

  scene->addItem(item);
}
