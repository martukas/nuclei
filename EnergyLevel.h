#ifndef ENERGYLEVEL_H
#define ENERGYLEVEL_H

#include <stdint.h>
#include <limits>
#include <QString>
#include <QList>
#include "HalfLife.h"
#include "Energy.h"
#include "SpinParity.h"
#include "ClickableItem.h"
#include "UncertainDouble.h"

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
    EnergyLevel(Energy energy, SpinParity spin,
                HalfLife halfLife = HalfLife(std::numeric_limits<double>::infinity()),
                unsigned int isomerNum = 0,
                UncertainDouble Q = UncertainDouble(),
                UncertainDouble mu = UncertainDouble()
                );

    virtual ~EnergyLevel();

    Energy energy() const;
    SpinParity spin() const;
    HalfLife halfLife() const;
    unsigned int isomerNum() const;
    double normalizedFeedIntensity() const;
    UncertainDouble mu() const;
    UncertainDouble q() const;

    const QList<GammaTransition *> &depopulatingTransitions() const;

    void setFeedIntensity(double intensity);
    void setFeedingLevel(bool isfeeding);

    bool isFeedingLevel() const;

    friend class Decay;
    friend class GammaTransition;

private:
    static bool gammaSmallerThan(const GammaTransition * const g1, const GammaTransition * const g2);

    Energy m_e;
    SpinParity sp;
    HalfLife hl;
    unsigned int isonum; // >0 for isomeric levels (counted from low energy to high), 0 otherwise
    double feedintens; // says how often this level is directly fed per 100 parent decays
    bool feedinglevel; // true if this is belonging to a parent nuclide and is a starting point for decays

    UncertainDouble m_Q, m_mu; // quadrupole and magnetic moments

    mutable QList<GammaTransition*> m_populatingTransitions;
    mutable QList<GammaTransition*> m_depopulatingTransitions;

    QGraphicsLineItem *graline, *grafeedarrow;
    QGraphicsPolygonItem *graarrowhead;
    QGraphicsSimpleTextItem *graetext, *graspintext, *grafeedintens, *grahltext;
    QGraphicsRectItem *graclickarea;
    GraphicsHighlightItem *grahighlighthelper;
    double graYPos;
};

#endif // ENERGYLEVEL_H
