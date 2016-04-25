#include "EnergyLevel.h"
#include <QtAlgorithms>
#include <cmath>
#include "GammaTransition.h"
#include "custom_logger.h"
#include "qpx_util.h"

EnergyLevel::EnergyLevel(Energy energy, SpinParity spin, HalfLife halfLife,
                         unsigned int isomerNum)
    : EnergyLevel()
{
  m_e = energy;
  sp = spin;
  hl = halfLife;
  isonum = isomerNum;
}

EnergyLevel::~EnergyLevel()
{
    for (int i=0; i<m_depopulatingTransitions.size(); i++)
        delete m_depopulatingTransitions.at(i);
    m_depopulatingTransitions.clear();
    m_populatingTransitions.clear();
}

EnergyLevel EnergyLevel::from_ensdf(std::string record)
{
  if (record.size() != 80)
    return EnergyLevel();

  Energy        e = Energy::from_nsdf(record.substr(9,12));
  SpinParity spin = SpinParity::from_ensdf(record.substr(21, 18));
  HalfLife     hl = HalfLife::from_ensdf(record.substr(39, 16));

  // determine isomer number
  unsigned int isonum = 0;
  std::string isostr(record.substr(77,2));
  if (record[77] == 'M') {
    if (is_number(record.substr(78,1)))
      isonum = boost::lexical_cast<uint16_t>(record.substr(78,1));
    else
      isonum = 1;
  }

  EnergyLevel ret(e, spin, hl, isonum);
//  DBG << record << " --> " << ret.to_string();
  return ret;
}

std::string EnergyLevel::to_string() const
{
  std::string ret = energy().to_string();
  if (sp.valid())
    ret += " " + spin().to_string();
  if (hl.isValid())
    ret += " " + hl.to_string();
  if (m_Q.valid())
    ret += " Q=" + m_Q.to_string();
  if (m_mu.valid())
    ret += " mu=" + m_mu.to_string();
  if (isonum)
    ret += " M" + std::to_string(isonum);
  return ret;
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

UncertainDouble EnergyLevel::normalizedFeedIntensity() const
{
    return feedintens;
}

Moment EnergyLevel::mu() const
{
    return m_mu;
}

Moment EnergyLevel::q() const
{
    return m_Q;
}

void EnergyLevel::set_mu(const Moment &m)
{
  m_mu = m;
}

void EnergyLevel::set_q(const Moment &m)
{
  m_Q = m;
}

void EnergyLevel::set_halflife(const HalfLife& h)
{
  hl = h;
}

void EnergyLevel::set_spin(const SpinParity& s)
{
  sp = s;
}



/**
  * \return Sorted list of transitions, lowest energy gamma first
  */
const QList<GammaTransition *> & EnergyLevel::depopulatingTransitions() const
{
    qSort(m_depopulatingTransitions.begin(), m_depopulatingTransitions.end(), gammaSmallerThan);
    return m_depopulatingTransitions;
}

void EnergyLevel::setFeedIntensity(UncertainDouble intensity)
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
