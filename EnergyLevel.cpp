#include "EnergyLevel.h"
#include <QtAlgorithms>
#include <cmath>
#include "GammaTransition.h"

EnergyLevel::EnergyLevel(Energy energy, SpinParity spin, HalfLife halfLife, unsigned int isomerNum, UncertainDouble Q, UncertainDouble mu)
    : ClickableItem(ClickableItem::EnergyLevelType),
      m_e(energy), sp(spin), hl(halfLife), isonum(isomerNum),
      feedintens(std::numeric_limits<double>::quiet_NaN()), feedinglevel(false),
      m_Q(Q), m_mu(mu),
      graline(0), grafeedarrow(0), graarrowhead(0), graetext(0), graspintext(0), grafeedintens(0), grahltext(0),
      graclickarea(0), grahighlighthelper(0), graYPos(0.0)
{
}

EnergyLevel::~EnergyLevel()
{
    for (int i=0; i<m_depopulatingTransitions.size(); i++)
        delete m_depopulatingTransitions.at(i);
    m_depopulatingTransitions.clear();
    m_populatingTransitions.clear();
}

Energy EnergyLevel::energy() const
{
    return m_e;
}

SpinParity EnergyLevel::spin() const
{
    return sp;
}

HalfLife EnergyLevel::halfLife() const
{
    return hl;
}

unsigned int EnergyLevel::isomerNum() const
{
    return isonum;
}

double EnergyLevel::normalizedFeedIntensity() const
{
    return feedintens;
}

UncertainDouble EnergyLevel::mu() const
{
    return m_mu;
}

UncertainDouble EnergyLevel::q() const
{
    return m_Q;
}

/**
  * \return Sorted list of transitions, lowest energy gamma first
  */
const QList<GammaTransition *> & EnergyLevel::depopulatingTransitions() const
{
    qSort(m_depopulatingTransitions.begin(), m_depopulatingTransitions.end(), gammaSmallerThan);
    return m_depopulatingTransitions;
}

void EnergyLevel::setFeedIntensity(double intensity)
{
    feedintens = intensity;
}

void EnergyLevel::setFeedingLevel(bool isfeeding)
{
    feedinglevel = isfeeding;
}

bool EnergyLevel::isFeedingLevel() const
{
    return feedinglevel;
}

bool EnergyLevel::gammaSmallerThan(const GammaTransition *const g1, const GammaTransition *const g2)
{
    return g1->energy() < g2->energy();
}
