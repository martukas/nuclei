#include "XNuclide.h"
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"
#include "qpx_util.h"

XNuclide::XNuclide()
{
}

XNuclide::XNuclide(NuclideId id, const HalfLife &halfLife)
  : nid_(id)
{
  hl.push_back(halfLife);
}

XNuclide::XNuclide(NuclideId id, const std::vector<HalfLife> &halfLifes)
  : nid_(id), hl(halfLifes)
{
}

NuclideId XNuclide::id() const
{
  return nid_;
}

void XNuclide::addLevels(const std::map<Energy, XEnergyLevelPtr> &levels)
{
  m_levels.insert(levels.begin(), levels.end());
}

std::map<Energy, XEnergyLevelPtr> &XNuclide::levels()
{
  return m_levels;
}

std::vector<HalfLife> XNuclide::halfLifes() const
{
  return hl;
}

std::string XNuclide::halfLifeAsText() const
{
  std::vector<std::string> results;
  for (auto &h : hl)
    if (h.isValid() && !h.isStable())
      results.push_back(h.to_string());
  return join(results, ", ");
}

