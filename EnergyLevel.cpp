#include "EnergyLevel.h"
#include <QtAlgorithms>
#include <cmath>
#include "GammaTransition.h"

EnergyLevel::EnergyLevel(double energyKeV, SpinParity spin, HalfLife halfLife, unsigned int isomerNum, double Q, double mu)
    : ClickableItem(ClickableItem::EnergyLevelType),
      m_e(energyKeV), sp(spin), hl(halfLife), isonum(isomerNum), feedintens(std::numeric_limits<double>::quiet_NaN()),
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

double EnergyLevel::energyKeV() const
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

/**
  * \return Sorted list of transitions, lowest energy gamma first
  */
QList<GammaTransition *> EnergyLevel::depopulatingTransitions()
{
    qSort(m_depopulatingTransitions.begin(), m_depopulatingTransitions.end(), gammaSmallerThan);
    return m_depopulatingTransitions;
}

QString EnergyLevel::energyAsText() const
{
    if (m_e >= 10000.0)
        return QString::number(m_e / 1.E3) + " MeV";
    return QString::number(m_e) + " keV";
}

QString EnergyLevel::muAsText() const
{
    if (std::isfinite(m_mu))
        return QString::number(m_mu);
    return "?";
}

QString EnergyLevel::qAsText() const
{
    if (std::isfinite(m_Q))
        return QString::number(m_Q);
    return "?";
}

QString EnergyLevel::momentaAsText() const
{
    QStringList r;
    if (std::isfinite(m_mu))
        r.append(QString::fromUtf8("µ=%1").arg(m_mu));
    if (std::isfinite(m_Q))
        r.append(QString("Q=%1").arg(m_Q));
    return r.join(", ");
}

bool EnergyLevel::gammaSmallerThan(const GammaTransition *const g1, const GammaTransition *const g2)
{
    return g1->energyKeV() < g2->energyKeV();
}
