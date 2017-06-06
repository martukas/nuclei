#include "DecayData.h"
#include "custom_logger.h"

DecayData::DecayData(ENSDFData& i)
  : LevelsData(i)
{
  if (!test(id.type) ||
      test(id.type & RecordType::Comments) ||
      test(id.type & RecordType::References))
  {
    DBG << "<DecayData::DecayData> Bad ID record type " << id.debug();
    //HACK (Tentative should propagate to NucData)
    return;
  }

  //  DBG << "dsid=" << id.extended_dsid;

  decay_info_ = DecayInfo(id.extended_dsid);
  reaction_info_ = ReactionInfo(id.extended_dsid, id.nuclide);
}

std::string DecayData::name() const
{
  if (decay_info_.valid())
  {
    auto ret = parent_string() + " → " + decay_info_.name();
    auto hl = halflife_string();
    if (!hl.empty())
      ret += " " + hl;
    return ret;
  }
  else if (reaction_info_.valid())
    return reaction_info_.name();
  return "INVALID";
}

std::string DecayData::parent_string() const
{
  if (!parents.empty())
  {
    const ParentRecord &prec(parents[0]);
    std::string decayname  = prec.nuclide.symbolicName();
    if (prec.energy > 0.0)
      decayname += "m";
    return decayname;
  }
  return "NOPARENTS";
}

std::string DecayData::halflife_string() const
{
  std::vector<std::string> hlstrings;
  for (const ParentRecord &prec : parents)
  {
    // check "same parent/different half-life" scheme
    if (prec.nuclide == parents.at(0).nuclide)
      hlstrings.push_back(prec.hl.to_string(false));

    //different parents??
  }
  if (!hlstrings.empty())
    return join(hlstrings, " + ");
  return "NOHALFLIFE";
}

bool DecayData::valid() const
{
  return id.valid() &&
      (decay_info_.valid() || reaction_info_.valid());
}

std::string DecayData::debug() const
{
  std::string ret;
  ret = id.nuclide.verboseName();
  if (decay_info_.valid())
    ret += "   decay=" + decay_info_.to_string();
  //  ret += " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
  if (reaction_info_.valid())
    ret += "   reaction=" + reaction_info_.to_string();
  ret += "   dsid=\"" + id.dsid + "\"";
  return ret;
}
