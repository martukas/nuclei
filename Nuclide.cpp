#include "Nuclide.h"
#include <QGraphicsItemGroup>
#include <QGraphicsSimpleTextItem>
#include <QFontMetrics>
#include <QBrush>
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"
#include "qpx_util.h"

Nuclide::Nuclide()
  : item(0)
{
}

Nuclide::Nuclide(NuclideId id, const HalfLife &halfLife)
  : nid_(id), item(0)
{
  hl.push_back(halfLife);
}

Nuclide::Nuclide(NuclideId id, const std::vector<HalfLife> &halfLifes)
  : nid_(id), hl(halfLifes), item(0)
{
}

NuclideId Nuclide::id() const
{
  return nid_;
}

void Nuclide::addLevels(const QMap<Energy, EnergyLevel *> &levels)
{
  m_levels.unite(levels);
}

QMap<Energy, EnergyLevel *> &Nuclide::levels()
{
  return m_levels;
}

std::vector<HalfLife> Nuclide::halfLifes() const
{
  return hl;
}

std::string Nuclide::halfLifeAsText() const
{
  std::vector<std::string> results;
  for (int i=0; i < hl.size(); ++i)
  foreach (HalfLife h, hl)
    if (h.isValid() && !h.isStable())
      results.push_back(h.to_string());
  std::string ret;
  for (int i=0; i < results.size(); ++i) {
    ret += results.at(i);
    if ((i > 0) && ((i+1) < results.size()))
        ret += ", ";
  }
  return ret;
}

QGraphicsItem *Nuclide::createNuclideGraphicsItem(const QFont &nucFont, const QFont &nucIndexFont)
{
  static const double numberToNameDistance = 4.0;

  QFontMetrics nucFontMetrics(nucFont);
  QFontMetrics nucIndexFontMetrics(nucIndexFont);

  item = new QGraphicsItemGroup;

  QGraphicsSimpleTextItem *granuc = new QGraphicsSimpleTextItem(QString::fromStdString(nid_.element()), item);
  granuc->setFont(nucFont);
  granuc->setBrush(QBrush(QColor(64, 166, 255)));

  QGraphicsSimpleTextItem *graA = new QGraphicsSimpleTextItem(QString::number(nid_.A()), item);
  graA->setFont(nucIndexFont);
  graA->setBrush(QBrush(QColor(64, 166, 255)));

  QGraphicsSimpleTextItem *graZ = new QGraphicsSimpleTextItem(QString::number(nid_.Z()), item);
  graZ->setFont(nucIndexFont);
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

  return item;
}

QGraphicsItem *Nuclide::nuclideGraphicsItem() const
{
  return item;
}
