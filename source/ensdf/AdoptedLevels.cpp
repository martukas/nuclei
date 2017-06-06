#include "AdoptedLevels.h"
#include "custom_logger.h"

AdoptedLevels::AdoptedLevels(ENSDFData& i)
  : LevelsData(i)
{
}

bool AdoptedLevels::valid() const
{
  return id.valid();
}

std::string AdoptedLevels::debug() const
{
  std::string ret;
  ret += "===LEVEL DATA===\n" + id.debug() + "\n";

  if (!history.empty())
  {
    ret += "  History:\n";
    for (auto c : history)
      ret += "    " + c.debug() + "\n";
  }
  if (!comments.empty())
  {
    ret += "  Comments:\n";
    for (auto c : comments)
      ret += "    " + c.debug() + "\n";
  }
  if (!xrefs.empty())
  {
    ret += "  Xrefs:\n";
    for (auto c : xrefs)
      ret += "    " + c.first + "=" + c.second + "\n";
  }
  if (!qvals.empty())
  {
    ret += "  QValues:\n";
    for (auto c : qvals)
      ret += "    " + c.debug() + "\n";
  }
  if (!levels.empty())
  {
    ret += "  Levels:\n";
    for (auto c : levels)
      ret += "    " + c.debug() + "\n";
  }
  if (!unplaced.gamma.empty())
  {
    ret += "  Unplaced_gammas:\n";
    for (auto c : unplaced.gamma)
      ret += "    " + c.debug() + "\n";
  }
  return ret;
}

std::list<LevelRecord> AdoptedLevels::nearest_levels(const Energy &to,
                                                 std::string dsid,
                                                 double maxdif,
                                                 double zero_thresh) const
{
  maxdif *= to;

  std::string ssym;
  if (!dsid.empty() && xrefs.count(dsid))
    ssym = xrefs.at(dsid);

  Energy current;
  std::list<LevelRecord> ret;
  for (const auto& lev : levels)
  {
    if (!ssym.empty() && lev.continuations_.count("XREF")
        && xref_check(lev.continuations_.at("XREF").symbols, ssym))
      continue;

    if (std::isfinite(maxdif) &&
        ((to > zero_thresh) || (lev.energy.value().value() != 0)) &&
        (std::abs(to - lev.energy) > maxdif))
      continue;

    if (!current.valid() ||
        (std::abs(to - lev.energy) <
         std::abs(to - current)))
    {
      ret.clear();
      ret.push_back(lev);
      current = lev.energy;
    }
    else if (current.valid() &&
             (lev.energy == current))
    {
      ret.push_back(lev);
    }
  }
  return ret;
}
