#ifndef XNuclide_H
#define XNuclide_H

#include <vector>
#include <limits>
#include <memory>
#include "HalfLife.h"
#include "Energy.h"
#include "NuclideId.h"
#include "Level.h"

class Nuclide
{
public:
  Nuclide();
  Nuclide(NuclideId id, const HalfLife &halfLife = HalfLife(std::numeric_limits<double>::infinity()));
  Nuclide(NuclideId id, const std::vector<HalfLife> &halfLifes);

  NuclideId id() const;

  bool empty() const;

  void addLevels(const std::map<Energy, LevelPtr> &levels);
  std::map<Energy, LevelPtr> & levels();

  std::vector<HalfLife> halfLifes() const;
  std::string halfLifeAsText() const;

private:
  NuclideId   nid_;
  std::vector <HalfLife> hl;
  std::map<Energy, LevelPtr> m_levels;
};

typedef std::shared_ptr<Nuclide> NuclidePtr;

#endif // Nuclide_H
