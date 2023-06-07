#ifndef NUCLIDE_H
#define NUCLIDE_H

#include <QString>
#include <QMap>
#include <QFont>
#include <QPair>
#include <limits>
#include "HalfLife.h"
#include "Energy.h"

#include <map>
#include <string>
#include <cinttypes>

class QGraphicsItem;
class QGraphicsItemGroup;
class EnergyLevel;

struct NuclideNomenclature {
  NuclideNomenclature() {}
  NuclideNomenclature(std::string s, std::string n) { symbol=s; name=n; }
  std::string symbol;
  std::string name;
};

class Nuclide
{
public:
  Nuclide();
  Nuclide(uint16_t A, uint16_t Z, const HalfLife &halfLife = HalfLife(std::numeric_limits<double>::infinity()));
  Nuclide(uint16_t A, uint16_t Z, const QList<HalfLife> &halfLifes);

  typedef QPair<int16_t, int16_t> Coordinates;

  uint16_t a() const;
  uint16_t z() const;
  Coordinates coordinates() const;

  static QString symbolicName(Coordinates c);
  static QString symbolOf(uint16_t Z);
  static int16_t zOfSymbol(const QString &name);

  QString element() const;
  QString name() const;

  void addLevels(const QMap<Energy, EnergyLevel*> &levels);
  QMap<Energy, EnergyLevel*> & levels();

  QList<HalfLife> halfLifes() const;
  QString halfLifeAsText() const;

  QGraphicsItem * createNuclideGraphicsItem(const QFont &nucFont, const QFont &nucIndexFont);
  QGraphicsItem * nuclideGraphicsItem() const;

private:
  uint16_t m_A, m_Z;
  QList <HalfLife> hl;

  QMap<Energy, EnergyLevel*> m_levels;

  static const std::map<uint16_t, NuclideNomenclature> names;
  static std::map<uint16_t, NuclideNomenclature> initNames();

  QGraphicsItemGroup * item;
};

#endif // NUCLIDE_H
