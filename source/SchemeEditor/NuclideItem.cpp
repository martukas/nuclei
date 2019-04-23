#include "NuclideItem.h"
#include <QGraphicsScene>
#include "ActiveGraphicsItemGroup.h"
#include <QGraphicsSimpleTextItem>
#include <QFontMetrics>
#include <QBrush>
#include <boost/algorithm/string.hpp>
#include "GraphicsHighlightItem.h"
#include <util/logger.h>
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
//  if (nuc.empty())
//  {
//    t = ClickableItem::InvalidType;
//    return;
//  }

  if ((tp != ParentNuclideType) && (tp != DaughterNuclideType))
    return;

  id_ = nuc.id();
  item = new ActiveGraphicsItemGroup(this);
  item->setActiveColor(0, vis.nuclide_color());
  item->setActiveColor(1, vis.selected_color());
  item->setActiveColor(2, vis.implicated_color());
  item->setHoverColor(vis.hover_color());
  scene->addItem(item);

  double numberToNameDistance = 4.0;

  QFontMetrics nucFontMetrics(vis.nucFont());
  QFontMetrics nucIndexFontMetrics(vis.nucIndexFont());

  auto symbol = new QGraphicsSimpleTextItem(QString::fromStdString(nuc.id().element()), item);
  symbol->setFont(vis.nucFont());
  symbol->setBrush(QBrush(vis.nuclide_color()));
  item->addToGroup(symbol);

  auto A_text = new QGraphicsSimpleTextItem(QString::number(nuc.id().A()), item);
  A_text->setFont(vis.nucIndexFont());
  A_text->setBrush(QBrush(vis.nuclide_color()));
  item->addToGroup(A_text);

  auto Z_text = new QGraphicsSimpleTextItem(QString::number(nuc.id().Z()), item);
  Z_text->setFont(vis.nucIndexFont());
  Z_text->setBrush(QBrush(vis.nuclide_color()));
  item->addToGroup(Z_text);

  double numberwidth
      = qMax(A_text->boundingRect().width(),
             Z_text->boundingRect().width());

  symbol->setPos(numberwidth + numberToNameDistance,
                 0.2*nucIndexFontMetrics.ascent());
  A_text->setPos(numberwidth - A_text->boundingRect().width(), 0.0);
  Z_text->setPos(numberwidth - Z_text->boundingRect().width(), 1.2*nucIndexFontMetrics.ascent());

  highlight_helper_
      = new GraphicsHighlightItem(numberwidth - A_text->boundingRect().width(),
                                  0.2*nucIndexFontMetrics.ascent(),
                                  numberwidth + numberToNameDistance + symbol->boundingRect().width(),
                                  nucFontMetrics.height());
  highlight_helper_->setOpacity(0.0);
  item->addHighlightHelper(highlight_helper_);


  click_area_
      = new QGraphicsRectItem(numberwidth - A_text->boundingRect().width(),
                              0.2*nucIndexFontMetrics.ascent(),
                              numberwidth + numberToNameDistance + symbol->boundingRect().width(),
                              nucFontMetrics.height());
  click_area_->setPen(Qt::NoPen);
  click_area_->setBrush(Qt::NoBrush);
  item->addToGroup(click_area_);

  // added in the end to work around a bug in QGraphicsItemGroup:
  //   it does not update boundingRect if contents are moved after adding them

  if (tp == ParentNuclideType)
  {
    // create half-life label
    halflife_text_ = new QGraphicsSimpleTextItem(QString::fromStdString(nuc.halfLifeAsText()));
    halflife_text_->setFont(vis.parentHlFont());
    scene->addItem(halflife_text_);

    // create vertical arrow component
    vertical_arrow_ = new QGraphicsLineItem;
    vertical_arrow_->setPen(vis.feedArrowPen);
    scene->addItem(vertical_arrow_);
  }
}
