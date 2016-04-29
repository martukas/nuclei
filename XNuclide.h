#ifndef XXNuclide_H
#define XXNuclide_H

#include <vector>
#include <limits>
#include <memory>
#include "HalfLife.h"
#include "Energy.h"
#include "NuclideId.h"
#include "XEnergyLevel.h"

class XNuclide
{
public:
  XNuclide();
  XNuclide(NuclideId id, const HalfLife &halfLife = HalfLife(std::numeric_limits<double>::infinity()));
  XNuclide(NuclideId id, const std::vector<HalfLife> &halfLifes);

  NuclideId id() const;

  bool empty() const;

  void addLevels(const std::map<Energy, XEnergyLevelPtr> &levels);
  std::map<Energy, XEnergyLevelPtr> & levels();

  std::vector<HalfLife> halfLifes() const;
  std::string halfLifeAsText() const;

private:
  NuclideId   nid_;
  std::vector <HalfLife> hl;
  std::map<Energy, XEnergyLevelPtr> m_levels;
};

typedef std::shared_ptr<XNuclide> XNuclidePtr;

#endif // XNuclide_H
