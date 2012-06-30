#ifndef NUCLIDE_H
#define NUCLIDE_H

#include <QString>
#include <QMap>
#include <QFont>
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
    Nuclide(unsigned int A, const QString &element, const HalfLife &halfLife = HalfLife(std::numeric_limits<double>::infinity()));
    Nuclide(unsigned int A, const QString &element, const QList<HalfLife> &halfLifes);

    unsigned int a() const;
    unsigned int z() const;
    QString element() const;
    QString name() const;

    void addLevels(const QMap<Energy, EnergyLevel*> &levels);
    QMap<Energy, EnergyLevel*> & levels();

    QString halfLifeAsText() const;

    QGraphicsItem * createNuclideGraphicsItem(const QFont &nucFont, const QFont &nucIndexFont);
    QGraphicsItem * nuclideGraphicsItem() const;

private:
    unsigned int m_A;
    QString el;
    QList <HalfLife> hl;

    QMap<Energy, EnergyLevel*> m_levels;

    static const QMap<QString, unsigned int> elToZ;
    static QMap<QString, unsigned int> initElToZ();

    QGraphicsItemGroup * item;
};

#endif // NUCLIDE_H
