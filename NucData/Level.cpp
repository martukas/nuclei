#include <cmath>
#include "Level.h"
#include "Transition.h"
#include "custom_logger.h"
#include "qpx_util.h"

Level::Level(Energy energy, SpinParity spin, HalfLife halfLife,
                           unsigned int isomerNum)
  : Level()
{
  m_e = energy;
  sp = spin;
  hl = halfLife;
  isonum = isomerNum;
}

Level Level::from_ensdf(std::string record)
{
  if (record.size() != 80)
    return Level();

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

  Level ret(e, spin, hl, isonum);
//  DBG << record << " --> " << ret.to_string();
  return ret;
}

std::string Level::to_string() const
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

Energy Level::energy() const
{
  return m_e;
}

SpinParity Level::spin() const
{
  return sp;
}

HalfLife Level::halfLife() const
{
  return hl;
}

unsigned int Level::isomerNum() const
{
  return isonum;
}

UncertainDouble Level::normalizedFeedIntensity() const
{
  return feedintens;
}

Moment Level::mu() const
{
  return m_mu;
}

Moment Level::q() const
{
  return m_Q;
}

void Level::set_mu(const Moment &m)
{
  m_mu = m;
}

void Level::set_q(const Moment &m)
{
  m_Q = m;
}

void Level::set_halflife(const HalfLife& h)
{
  hl = h;
}

void Level::set_spin(const SpinParity& s)
{
  sp = s;
}



/**
  * \return Sorted list of transitions, lowest energy gamma first
  */
const std::list<std::shared_ptr<Transition>> & Level::depopulatingTransitions() const
{
  m_depopulatingTransitions.sort([](const TransitionPtr & a, const TransitionPtr & b)
                                   { return a->energy() < b->energy(); });
  return m_depopulatingTransitions;
}

const std::list<std::shared_ptr<Transition>> & Level::populatingTransitions() const
{
  m_populatingTransitions.sort([](const TransitionPtr & a, const TransitionPtr & b)
                                   { return a->energy() < b->energy(); });
  return m_populatingTransitions;
}

void Level::setFeedIntensity(UncertainDouble intensity)
{
  feedintens = intensity;
}

void Level::setFeedingLevel(bool isfeeding)
{
  feedinglevel = isfeeding;
}

bool Level::isFeedingLevel() const
{
  return feedinglevel;
}
