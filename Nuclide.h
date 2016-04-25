#ifndef NUCLIDE_H
#define NUCLIDE_H

#include <QString>
#include <QMap>
#include <QFont>
#include <QPair>
#include <limits>
#include "HalfLife.h"
#include "Energy.h"
#include "NuclideId.h"

class QGraphicsItem;
class QGraphicsItemGroup;
class EnergyLevel;

class Nuclide
{
public:
  Nuclide();
  Nuclide(NuclideId id, const HalfLife &halfLife = HalfLife(std::numeric_limits<double>::infinity()));
  Nuclide(NuclideId id, const std::vector<HalfLife> &halfLifes);

  NuclideId id() const;

  void addLevels(const QMap<Energy, EnergyLevel*> &levels);
  QMap<Energy, EnergyLevel*> & levels();

  std::vector<HalfLife> halfLifes() const;
  std::string halfLifeAsText() const;

  QGraphicsItem * createNuclideGraphicsItem(const QFont &nucFont, const QFont &nucIndexFont);
  QGraphicsItem * nuclideGraphicsItem() const;

private:
  NuclideId   nid_;
  std::vector <HalfLife> hl;
  QMap<Energy, EnergyLevel*> m_levels;
  QGraphicsItemGroup * item;
};

#endif // NUCLIDE_H
