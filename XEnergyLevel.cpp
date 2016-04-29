#include <cmath>
#include "XEnergyLevel.h"
#include "XGammaTransition.h"
#include "custom_logger.h"
#include "qpx_util.h"

XEnergyLevel::XEnergyLevel(Energy energy, SpinParity spin, HalfLife halfLife,
                           unsigned int isomerNum)
  : XEnergyLevel()
{
  m_e = energy;
  sp = spin;
  hl = halfLife;
  isonum = isomerNum;
}

XEnergyLevel XEnergyLevel::from_ensdf(std::string record)
{
  if (record.size() != 80)
    return XEnergyLevel();

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

  XEnergyLevel ret(e, spin, hl, isonum);
//  DBG << record << " --> " << ret.to_string();
  return ret;
}

std::string XEnergyLevel::to_string() const
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

Energy XEnergyLevel::energy() const
{
  return m_e;
}

SpinParity XEnergyLevel::spin() const
{
  return sp;
}

HalfLife XEnergyLevel::halfLife() const
{
  return hl;
}

unsigned int XEnergyLevel::isomerNum() const
{
  return isonum;
}

UncertainDouble XEnergyLevel::normalizedFeedIntensity() const
{
  return feedintens;
}

Moment XEnergyLevel::mu() const
{
  return m_mu;
}

Moment XEnergyLevel::q() const
{
  return m_Q;
}

void XEnergyLevel::set_mu(const Moment &m)
{
  m_mu = m;
}

void XEnergyLevel::set_q(const Moment &m)
{
  m_Q = m;
}

void XEnergyLevel::set_halflife(const HalfLife& h)
{
  hl = h;
}

void XEnergyLevel::set_spin(const SpinParity& s)
{
  sp = s;
}



/**
  * \return Sorted list of transitions, lowest energy gamma first
  */
const std::list<std::shared_ptr<XGammaTransition>> & XEnergyLevel::depopulatingTransitions() const
{
  m_depopulatingTransitions.sort([](const XGammaTransitionPtr & a, const XGammaTransitionPtr & b)
                                   { return a->energy() < b->energy(); });
  return m_depopulatingTransitions;
}

const std::list<std::shared_ptr<XGammaTransition>> & XEnergyLevel::populatingTransitions() const
{
  m_populatingTransitions.sort([](const XGammaTransitionPtr & a, const XGammaTransitionPtr & b)
                                   { return a->energy() < b->energy(); });
  return m_populatingTransitions;
}

void XEnergyLevel::setFeedIntensity(UncertainDouble intensity)
{
  feedintens = intensity;
}

void XEnergyLevel::setFeedingLevel(bool isfeeding)
{
  feedinglevel = isfeeding;
}

bool XEnergyLevel::isFeedingLevel() const
{
  return feedinglevel;
}
