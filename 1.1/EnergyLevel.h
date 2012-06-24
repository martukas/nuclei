#ifndef ENERGYLEVEL_H
#define ENERGYLEVEL_H

#include <stdint.h>
#include <limits>
#include <QString>
#include <QList>
#include "HalfLife.h"
#include "SpinParity.h"
#include "ClickableItem.h"

class ActiveGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class QGraphicsPolygonItem;
class GraphicsHighlightItem;
class QGraphicsRectItem;
class GammaTransition;

class EnergyLevel : public ClickableItem
{
public:
    EnergyLevel(int64_t energyEV, SpinParity spin,
                HalfLife halfLife = HalfLife(std::numeric_limits<double>::infinity()),
                unsigned int isomerNum = 0,
                double Q = std::numeric_limits<double>::quiet_NaN(),
                double mu = std::numeric_limits<double>::quiet_NaN()
                );

    ~EnergyLevel();

    int64_t energyEv() const;
    SpinParity spin() const;
    HalfLife halfLife() const;
    unsigned int isomerNum() const;
    double normalizedFeedIntensity() const;

    QList<GammaTransition*> depopulatingTransitions();

    QString energyAsText() const;
    QString muAsText() const;
    QString qAsText() const;
    QString momentaAsText() const;

    friend class Decay;
    friend class GammaTransition;

private:
    static bool gammaSmallerThan(const GammaTransition * const g1, const GammaTransition * const g2);

    int64_t e;
    SpinParity sp;
    HalfLife hl;
    unsigned int isonum; // >0 for isomeric levels (counted from low energy to high), 0 otherwise
    double feedintens; // says how often this level is directly fed per 100 parent decays

    double m_Q, m_mu; // quadrupole and magnetic moments

    QList<GammaTransition*> m_populatingTransitions;
    QList<GammaTransition*> m_depopulatingTransitions;

    QGraphicsLineItem *graline, *grafeedarrow;
    QGraphicsPolygonItem *graarrowhead;
    QGraphicsSimpleTextItem *graetext, *graspintext, *grafeedintens, *grahltext;
    QGraphicsRectItem *graclickarea;
    GraphicsHighlightItem *grahighlighthelper;
    double graYPos;
};

#endif // ENERGYLEVEL_H
