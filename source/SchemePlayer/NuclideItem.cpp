#include "NuclideItem.h"
#include <QGraphicsScene>
#include "ActiveGraphicsItemGroup.h"
#include <QGraphicsSimpleTextItem>
#include <QFontMetrics>
#include <QBrush>
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"
#include "qpx_util.h"

NuclideItem::NuclideItem()
  : ClickableItem(ClickableItem::InvalidType)
{
}

void NuclideItem::position_arrow(double x, double start, double end)
{
  if (std::isfinite(start) && std::isfinite(end))
    pNucVerticalArrow->setLine(x, start, x, end);
}

void NuclideItem::position_text(double parent_center_x,
                                double ypos)
{
  pNucHl->setPos(parent_center_x - 0.5*pNucHl->boundingRect().width(),
                 ypos - 12.0);
}


NuclideItem::NuclideItem(const Nuclide &nuc, Type tp,
                         SchemeVisualSettings vis,
                         QGraphicsScene *scene)
  : NuclideItem()
{
  if (nuc.empty())
    return;

  if ((tp != ParentNuclideType) && (tp != DaughterNuclideType))
    return;

  t = tp;
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

  double numberwidth = qMax(graA->boundingRect().width(), graZ->boundingRect().width());

  granuc->setPos(numberwidth + numberToNameDistance, 0.2*nucIndexFontMetrics.ascent());
  graA->setPos(numberwidth - graA->boundingRect().width(), 0.0);
  graZ->setPos(numberwidth - graZ->boundingRect().width(), 1.2*nucIndexFontMetrics.ascent());

  // added in the end to work around a bug in QGraphicsItemGroup:
  //   it does not update boundingRect if contents are moved after adding them
  item->addToGroup(granuc);
  item->addToGroup(graA);
  item->addToGroup(graZ);

  if (tp == ParentNuclideType)
  {
    // create half-life label
    pNucHl = new QGraphicsSimpleTextItem(QString::fromStdString(nuc.halfLifeAsText()));
    pNucHl->setFont(vis.parentHlFont);
    scene->addItem(pNucHl);

    // create vertical arrow component
    pNucVerticalArrow = new QGraphicsLineItem;
    pNucVerticalArrow->setPen(vis.feedArrowPen);
    scene->addItem(pNucVerticalArrow);
  }


  scene->addItem(item);
}
