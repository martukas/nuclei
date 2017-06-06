#include "LevelsData.h"
#include "Fields.h"
#include "XRef.h"

#include "custom_logger.h"
#include "qpx_util.h"

LevelsData::LevelsData(ENSDFData& i)
{
  auto idx = i.i.first;
  id = IdRecord(i);
  if (!id.valid())
  {
    i.print("<LevelsData> Invalid ID", idx, id.debug());
    return;
  }

  read_hist(i);
  read_prelims(i);
  read_unplaced(i);
  read_comments(i);
  read_levels(i);

  //  if (valid())
  //    DBG << debug();

  decay_info_ = DecayInfo(id.extended_dsid);
  reaction_info_ = ReactionInfo(id.extended_dsid, id.nuclide);
  adopted = boost::contains(id.extended_dsid, "ADOPTED LEVELS");
  gammas = boost::contains(id.extended_dsid, "GAMMAS");
}

void LevelsData::read_comments(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first + 1;
    if (CommentsRecord::match(i.look_ahead(), "."))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        i.print("<LevelsData> Invalid comment", idx, com.debug());
    }
    else
      break;
  }
}

void LevelsData::read_hist(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first + 1;
    if (HistoryRecord::match(i.look_ahead()))
    {
      auto his = HistoryRecord(++i);
      if (his.valid())
        history.push_back(his);
      else
        i.print("<LevelsData> Invalid history", idx, his.debug());
    }
    else
      break;
  }
}

void LevelsData::read_prelims(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first + 1;
    auto line = i.look_ahead();
    if (ProdNormalizationRecord::match(line))
    {
      auto pn = ProdNormalizationRecord(++i);
      if (pn.valid())
      {
        if (pnorm.valid())
          i.print("<LevelsData> More than one pnorm", idx, pnorm.debug());
        pnorm = pn;
      }
      else
        i.print("<LevelsData> Invalid pnorm", idx, pn.debug());
    }
    else if (QValueRecord::match(line))
    {
      auto qv = QValueRecord(++i);
      if (qv.valid())
        qvals.push_back(qv);
      else
        i.print("<LevelsData> Invalid qval", idx, qv.debug());
    }
    else if (CommentsRecord::match(line, "."))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        i.print("<LevelsData> Invalid comment", idx, com.debug());
    }
    else if (XRefRecord::match(line))
    {
      auto ref = XRefRecord(++i);
      if (ref.valid())
        xrefs[ref.dsid] = ref.dssym;
      else
        i.print("<LevelsData> Invalid xref", idx, ref.debug());
    }
    else if (NormalizationRecord::match(line))
    {
      auto n = NormalizationRecord(++i);
      if (n.valid())
        norm.push_back(n);
      else
        i.print("<LevelsData> Invalid norm", idx, n.debug());
    }
    else if (ParentRecord::match(line))
    {
      auto par = ParentRecord(++i);
      if (par.valid())
        parents.push_back(par);
      else
        i.print("<LevelsData> Invalid parent", idx, par.debug());
    }
    else
      break;
  }
}

void LevelsData::read_unplaced(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first + 1;
    auto line = i.look_ahead();
    if (AlphaRecord::match(line))
    {
      auto a = AlphaRecord(++i);
      if (a.valid())
        unplaced.alpha.push_back(a);
      else
        i.print("<LevelsData> Invalid alpha", idx, a.debug());
    }
    else if (BetaRecord::match(line))
    {
      auto b = BetaRecord(++i);
      if (b.valid())
        unplaced.beta.push_back(b);
      else
        i.print("<LevelsData> Invalid beta", idx, b.debug());
    }
    else if (GammaRecord::match(line))
    {
      auto g = GammaRecord(++i);
      if (g.valid())
        unplaced.gamma.push_back(g);
      else
        i.print("<LevelsData> Invalid gamma", idx, g.debug());
    }
    else if (ParticleRecord::match(line))
    {
      auto p = ParticleRecord(++i);
      if (p.valid())
        unplaced.particle.push_back(p);
      else
        i.print("<LevelsData> Invalid particle", idx, p.debug());
    }
    else
      break;
  }
}

void LevelsData::read_levels(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first + 1;
    if (LevelRecord::match(i.look_ahead()))
    {
      auto lev = LevelRecord(++i);
      if (lev.valid())
        levels.push_back(lev);
      else
        i.print("<LevelsData> Invalid level", idx, lev.debug());
    }
    else
    {
      i.print("<LevelsData> Bad record in levels block ", idx);
      ++i;
    }
  }
}

std::list<LevelRecord> LevelsData::nearest_levels(const Energy &to,
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

std::string LevelsData::name() const
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
  else if (adopted)
    return id.nuclide.symbolicName() + " adopted levels"
        + (gammas ? std::string(" (gammas)") : std::string());
  return "UNPARSED '" + id.extended_dsid + "'";
}

std::string LevelsData::parent_string() const
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

std::string LevelsData::halflife_string() const
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

std::string LevelsData::debug() const
{
  std::string ret;
  ret += "===LEVEL DATA===\n" + id.debug() + "\n";

  if (decay_info_.valid())
    ret += "   decay=" + decay_info_.to_string();
  //  ret += " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
  if (reaction_info_.valid())
    ret += "   reaction=" + reaction_info_.to_string();
  ret += "   dsid=\"" + id.extended_dsid + "\"";

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



