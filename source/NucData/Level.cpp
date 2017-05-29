#include "Level.h"
#include "qpx_util.h"

Level::Level(Energy energy, SpinParity spin, HalfLife halfLife, uint16_t isomerNum)
  : Level()
{
  energy_ = energy;
  spin_ = spin;
  halflife_ = halfLife;
  isomeric_ = isomerNum;
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

Uncert Level::normalizedFeedIntensity() const
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
  if (e.valid())
    populating_transitions_.insert(e);
}

void Level::addDepopulatingTransition(const Energy& e)
{
  if (e.valid())
    depopulating_transitions_.insert(e);
}

void Level::setFeedIntensity(Uncert intensity)
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

void Level::add_comments(const std::string &s, const json &j)
{
  comments_[s] = j;
}

json Level::comments() const
{
  return comments_;
}

