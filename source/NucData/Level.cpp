#include "Level.h"
#include "qpx_util.h"

Level::Level(Energy energy, SpinSet spin, HalfLife halfLife, uint16_t isomerNum)
  : Level()
{
  energy_ = energy;
  spins_ = spin;
  halflife_ = halfLife;
  isomeric_ = isomerNum;
}

std::string Level::to_string() const
{
  std::stringstream ss;
  ss << std::setw(16) << energy().to_string();
  if (spins_.valid())
    ss << std::setw(10) << spins().to_string();
  if (halflife_.isValid())
    ss << std::setw(13) << halflife_.to_string();
  if (isomeric_)
    ss << " M" + std::to_string(isomeric_);
  return ss.str();
}

Energy Level::energy() const
{
  return energy_;
}

SpinSet Level::spins() const
{
  return spins_;
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

void Level::set_halflife(const HalfLife& h)
{
  halflife_ = h;
}

void Level::set_spins(const SpinSet& s)
{
  spins_ = s;
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

void Level::add_text(const std::string &heading, const json &j)
{
  json block;
  block["heading"] = heading;
  block["pars"] = j;
  text_.push_back(block);
}

json Level::text() const
{
  return text_;
}
