#include "Nuclide.h"
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"
#include "qpx_util.h"

Nuclide::Nuclide()
{
}

Nuclide::Nuclide(NuclideId id, const HalfLife &halfLife)
  : nid_(id)
{
  hl.push_back(halfLife);
}

Nuclide::Nuclide(NuclideId id, const std::vector<HalfLife> &halfLifes)
  : nid_(id), hl(halfLifes)
{
}

NuclideId Nuclide::id() const
{
  return nid_;
}

bool Nuclide::empty() const
{
  return (!nid_.valid() || m_levels.empty() || hl.empty());
}

void Nuclide::addLevels(const std::map<Energy, LevelPtr> &levels)
{
  m_levels.insert(levels.begin(), levels.end());
}

std::map<Energy, LevelPtr> &Nuclide::levels()
{
  return m_levels;
}

std::vector<HalfLife> Nuclide::halfLifes() const
{
  return hl;
}

std::string Nuclide::halfLifeAsText() const
{
  std::vector<std::string> results;
  for (auto &h : hl)
    if (h.isValid() && !h.isStable())
      results.push_back(h.to_string());
  return join(results, ", ");
}

