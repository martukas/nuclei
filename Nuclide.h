#ifndef NUCLIDE_H
#define NUCLIDE_H

#include <QString>
#include <QMap>
#include <QFont>
#include <QPair>
#include <limits>
#include "HalfLife.h"
#include "Energy.h"

class QGraphicsItem;
class QGraphicsItemGroup;
class EnergyLevel;

class Nuclide
{
public:
    Nuclide();
    Nuclide(unsigned int A, unsigned int Z, const HalfLife &halfLife = HalfLife(std::numeric_limits<double>::infinity()));
    Nuclide(unsigned int A, unsigned int Z, const QList<HalfLife> &halfLifes);

    typedef QPair<unsigned int, unsigned int> Coordinates;

    unsigned int a() const;
    unsigned int z() const;
    Coordinates coordinates() const;

    static QString nameOf(Coordinates c);
    static QString nameOf(unsigned int Z);
    static unsigned int zOf(const QString &name);

    QString element() const;
    QString name() const;

    void addLevels(const QMap<Energy, EnergyLevel*> &levels);
    QMap<Energy, EnergyLevel*> & levels();

    QList<HalfLife> halfLifes() const;
    QString halfLifeAsText() const;

    QGraphicsItem * createNuclideGraphicsItem(const QFont &nucFont, const QFont &nucIndexFont);
    QGraphicsItem * nuclideGraphicsItem() const;

private:
    unsigned int m_A, m_Z;
    QList <HalfLife> hl;

    QMap<Energy, EnergyLevel*> m_levels;

    static const QMap<QString, unsigned int> elToZ;
    static QMap<QString, unsigned int> initElToZ();

    QGraphicsItemGroup * item;
};

#endif // NUCLIDE_H
