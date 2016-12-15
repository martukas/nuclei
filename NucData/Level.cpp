#include <cmath>
#include "Level.h"
#include "Transition.h"
#include "custom_logger.h"
#include "qpx_util.h"

Level::Level(Energy energy, SpinParity spin, HalfLife halfLife, uint16_t isomerNum)
  : Level()
{
  energy_ = energy;
  spin_ = spin;
  halflife_ = halfLife;
  isomeric_ = isomerNum;
}

Level Level::from_ensdf(std::string record)
{
  if (record.size() != 80)
    return Level();

  Energy        e = Energy::from_nsdf(record.substr(9,12));
  SpinParity spin = SpinParity::from_ensdf(record.substr(21, 18));
  HalfLife     halflife = HalfLife::from_ensdf(record.substr(39, 16));

  // determine isomer number
  uint16_t isomeric_ = 0;
  std::string isostr(record.substr(77,2));
  if (record[77] == 'M') {
    if (is_number(record.substr(78,1)))
      isomeric_ = boost::lexical_cast<uint16_t>(record.substr(78,1));
    else
      isomeric_ = 1;
  }

  Level ret(e, spin, halflife, isomeric_);
//  DBG << record << " --> " << ret.to_string();
  return ret;
}

std::string Level::to_string() const
{
  std::stringstream ss;
  ss << std::setw(16) << energy().to_string();
  if (spin_.valid())
    ss << std::setw(7) << spin().to_string();
  if (halflife_.isValid())
    ss << std::setw(13) << halflife_.to_string();
  if (q_.valid())
    ss << " Q=" + q_.to_string();
  if (mu_.valid())
    ss << " mu=" + mu_.to_string();
  if (isomeric_)
    ss << " M" + std::to_string(isomeric_);
  return ss.str();
}

Energy Level::energy() const
{
  return energy_;
}

SpinParity Level::spin() const
{
  return spin_;
}

HalfLife Level::halfLife() const
{
  return halflife_;
}

uint16_t Level::isomerNum() const
{
  return isomeric_;
}

UncertainDouble Level::normalizedFeedIntensity() const
{
  return feeding_intensity_;
}

Moment Level::mu() const
{
  return mu_;
}

Moment Level::q() const
{
  return q_;
}

void Level::set_mu(const Moment &m)
{
  mu_ = m;
}

void Level::set_q(const Moment &m)
{
  q_ = m;
}

void Level::set_halflife(const HalfLife& h)
{
  halflife_ = h;
}

void Level::set_spin(const SpinParity& s)
{
  spin_ = s;
}

void Level::addPopulatingTransition(const Energy& e)
{
  if (e.isValid())
    populating_transitions_.insert(e);
}

void Level::addDepopulatingTransition(const Energy& e)
{
  if (e.isValid())
    depopulating_transitions_.insert(e);
}

void Level::setFeedIntensity(UncertainDouble intensity)
{
  feeding_intensity_ = intensity;
}

void Level::setFeedingLevel(bool isfeeding)
{
  feeding_level_ = isfeeding;
}

bool Level::isFeedingLevel() const
{
  return feeding_level_;
}
