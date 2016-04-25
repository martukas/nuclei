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
#include "Moment.h"

class ActiveGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;
class QGraphicsTextItem;
class QGraphicsPolygonItem;
class GraphicsHighlightItem;
class QGraphicsRectItem;
class GammaTransition;

class EnergyLevel : public ClickableItem
{
public:

  EnergyLevel()
    : ClickableItem(ClickableItem::EnergyLevelType)
    , isonum(0)
    , feedinglevel(false),
    graline(0), grafeedarrow(0), graarrowhead(0), graetext(0), graspintext(0), grahltext(0), grafeedintens(0),
    graclickarea(0), grahighlighthelper(0), graYPos(0.0)
  {}


    EnergyLevel(Energy energy, SpinParity spin,
                HalfLife halfLife = HalfLife(std::numeric_limits<double>::infinity()),
                unsigned int isomerNum = 0);

    virtual ~EnergyLevel();

    static EnergyLevel from_ensdf(std::string record);

    Energy energy() const;
    SpinParity spin() const;
    HalfLife halfLife() const;
    unsigned int isomerNum() const;
    UncertainDouble normalizedFeedIntensity() const;
    Moment mu() const;
    Moment q() const;

    void set_mu(const Moment &);
    void set_q(const Moment &);
    void set_halflife(const HalfLife&);
    void set_spin(const SpinParity&);

    const QList<GammaTransition *> &depopulatingTransitions() const;

    std::string to_string() const;

    void setFeedIntensity(UncertainDouble intensity);
    void setFeedingLevel(bool isfeeding);

    bool isFeedingLevel() const;

    friend class Decay;
    friend class GammaTransition;

private:
    static bool gammaSmallerThan(const GammaTransition * const g1, const GammaTransition * const g2);

    Energy m_e;
    SpinParity sp;
    HalfLife hl;
    Moment m_Q, m_mu; // quadrupole and magnetic moments
    unsigned int isonum; // >0 for isomeric levels (counted from low energy to high), 0 otherwise



    UncertainDouble feedintens; // says how often this level is directly fed per 100 parent decays
    bool feedinglevel; // true if this is belonging to a parent nuclide and is a starting point for decays


    mutable QList<GammaTransition*> m_populatingTransitions;
    mutable QList<GammaTransition*> m_depopulatingTransitions;

    QGraphicsLineItem *graline, *grafeedarrow;
    QGraphicsPolygonItem *graarrowhead;
    QGraphicsSimpleTextItem *graetext, *graspintext, *grahltext;
    QGraphicsTextItem *grafeedintens;
    QGraphicsRectItem *graclickarea;
    GraphicsHighlightItem *grahighlighthelper;
    double graYPos;
};

#endif // ENERGYLEVEL_H
