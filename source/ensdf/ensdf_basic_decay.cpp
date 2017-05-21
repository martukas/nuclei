#include "ensdf_basic_decay.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

#include "xref_record.h"

DecayData::DecayData(ENSDFData& i)
{
  auto idx = i.i.first;
  id = IdRecord(i);
  if (!id.valid())
  {
    DBG << "<DecayData> Bad ID " << i.lines[idx];
    return;
  }

  read_hist(i);
  read_prelims(i);
  read_unplaced(i);

  while (i.has_more())
  {
    idx = i.i.first;
    auto line = i.look_ahead();
    if (CommentsRecord::match(line, "L"))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        DBG << "<DecayData> Invalid all-level comment " << com.debug()
            << "\nfrom " << idx << " = " << line;
      //      DBG << "Added comment from" << idx << "=" << line << com.debug();
    }
    else if (LevelRecord::match(line))
    {
      auto lev = LevelRecord(++i);
      if (lev.valid())
        levels.push_back(lev);
      else
      {
        DBG << "<DecayData>(" << id.nuclide.symbolicName() << ":" << name() << ")"
            << " Invalid Level at " << idx << "=" << line
               << "\n" << lev.debug();
      }
      //      DBG << "Added level from" << idx << "=" << line;
    }
    else
    {
      DBG << "<DecayData> Unidentified record "
          << idx << "=" << line;
    }
  }


  if (!test(id.type) ||
      test(id.type & RecordType::Comments) ||
      test(id.type & RecordType::References))
  {
    DBG << "<DecayData::DecayData> Bad ID record type " << id.debug();
    //HACK (Tentative should propagate to NucData)
    return;
  }

  //  DBG << "dsid=" << id.extended_dsid;

  if (test(id.type & RecordType::Decay))
    decay_info_ = DecayInfo(id.extended_dsid);

  if (ReactionInfo::match(id.extended_dsid))
  {
    reaction_info_ = ReactionInfo(id.extended_dsid, id.nuclide);
    //    if (!reaction_info_.valid())
    //      DBG << "INVALID REACTION " << id.extended_dsid;
    //    DBG << "  ----> " << reaction_info_.to_string()
    //        << (reaction_info_.valid() ? " valid" : " INVALID") ;
  }

  //  DBG << "Retrieved decay " << decay_info_.to_string()
  //      << " with h=" << history.size()
  //      << " & c=" << comments.size()
  //      << " & p=" << parents.size();
}

std::list<LevelRecord> LevelData::nearest_levels(const Energy &to,
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
        && xref_check(lev.continuations_.at("XREF"), ssym))
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

void DecayData::read_hist(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (HistoryRecord::match(line))
    {
      auto his = HistoryRecord(++i);
      if (his.valid())
        history.push_back(his);
      else
        DBG << "<DecayData> Invalid Hist " << his.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

void DecayData::read_prelims(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (ProdNormalizationRecord::match(line))
    {
      auto pn = ProdNormalizationRecord(++i);
      if (pn.valid())
      {
        if (pnorm.valid())
          DBG << "<DecayData> More than one pnorm " << pnorm.debug()
              << " from " << idx << "=" << line;
        pnorm = pn;
      }
      else
        DBG << "<LevelData> Invalid Pnorm " << pn.debug()
            << " from " << idx << "=" << line;
    }
    else if (NormalizationRecord::match(line))
    {
      auto n = NormalizationRecord(++i);
      if (n.valid())
        norm.push_back(n);
      else
        DBG << "<DecayData> Invalid Norm " << n.debug()
            << " from " << idx << "=" << line;
    }
    else if (QValueRecord::match(line))
    {
      auto qv = QValueRecord(++i);
      if (qv.valid())
        qvals.push_back(qv);
      else
        DBG << "<DecayData> Invalid Qval " << qv.debug()
            << " from " << idx << "=" << line;
    }
    else if (CommentsRecord::match(line))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        DBG << "<DecayData> Invalid Comment " << com.debug()
            << " from " << idx << "=" << line;
    }
    else if (ParentRecord::match(line))
    {
      auto par = ParentRecord(++i);
      if (par.valid())
        parents.push_back(par);
      else
        DBG << "<DecayData> Invalid Parent " << par.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}


void DecayData::read_unplaced(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (AlphaRecord::match(line))
    {
      auto a = AlphaRecord(++i);
      if (a.valid())
        unplaced_alphas.push_back(a);
      else
        DBG << "<DecayData> Invalid alpha " << a.debug()
            << " from " << idx << "=" << line;
    }
    else if (BetaRecord::match(line))
    {
      auto b = BetaRecord(++i);
      if (b.valid())
        unplaced_betas.push_back(b);
      else
        DBG << "<DecayData> Invalid beta " << b.debug()
            << " from " << idx << "=" << line;
    }
    else if (GammaRecord::match(line))
    {
      auto g = GammaRecord(++i);
      if (g.valid())
        unplaced_gammas.push_back(g);
      else
        DBG << "<DecayData> Invalid gamma " << g.debug()
            << " from " << idx << "=" << line;
    }
    else if (ParticleRecord::match(line))
    {
      auto p = ParticleRecord(++i);
      if (p.valid())
        unplaced_particles.push_back(p);
      else
        DBG << "<DecayData> Invalid particle " << p.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}





void LevelData::read_hist(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (HistoryRecord::match(line))
    {
      auto his = HistoryRecord(++i);
      if (his.valid())
        history.push_back(his);
      else
        DBG << "<LevelData> Invalid Hist " << his.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

void LevelData::read_comments(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (CommentsRecord::match(line, "."))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        DBG << "<LevelData> Invalid Comment " << com.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

void LevelData::read_prelims(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (ProdNormalizationRecord::match(line))
    {
      auto pn = ProdNormalizationRecord(++i);
      if (pn.valid())
      {
        if (pnorm.valid())
          DBG << "<LevelData> More than one norm " << pnorm.debug()
              << " from " << idx << "=" << line;

        pnorm = pn;
      }
      else
        DBG << "<LevelData> Invalid Pnorm " << pn.debug()
            << " from " << idx << "=" << line;
    }
    else if (QValueRecord::match(line))
    {
      auto qv = QValueRecord(++i);
      if (qv.valid())
        qvals.push_back(qv);
      else
        DBG << "<LevelData> Invalid Qval " << qv.debug()
            << " from " << idx << "=" << line;
    }
    else if (XRefRecord::match(line))
    {
      auto ref = XRefRecord(++i);
      if (ref.valid())
        xrefs[ref.dsid] = ref.dssym;
      else
        DBG << "<LevelData> Invalid Xref " << ref.debug()
            << " from " << idx << "=" << line;
    }
    else if (CommentsRecord::match(line, "."))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        DBG << "<LevelData> Invalid Comment " << com.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

void LevelData::read_unplaced(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (GammaRecord::match(line))
    {
      auto gam = GammaRecord(++i);
      if (gam.valid())
        unplaced_gammas.push_back(gam);
      else
        DBG << "<LevelData> Invalid unplaced gamma " << gam.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

LevelData::LevelData(ENSDFData& i)
{
  auto idx = i.i.first;
  id = IdRecord(i);
  if (!id.valid())
  {
    DBG << "<LevelData> Bad ID " << i.lines[idx];
    return;
  }

  idx++;
  read_hist(i);
  read_prelims(i);

  read_unplaced(i);
  read_comments(i);

  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (CommentsRecord::match(line, "L"))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        DBG << "<LevelData> Invalid all-level comment " << com.debug()
            << "\nfrom " << idx << " = " << line;
      //      DBG << "Added comment from" << idx << "=" << line << com.debug();
    }
    else if (LevelRecord::match(line))
    {
      auto lev = LevelRecord(++i);
      if (lev.valid())
        levels.push_back(lev);
      else
      {
        DBG << "<LevelData>(" << id.nuclide.symbolicName() << ")"
            << " Invalid Level at " << idx << "=" << line
               << "\n" << lev.debug();
      }
      //      DBG << "Added level from" << idx << "=" << line;
    }
    else
    {
      DBG << "<LevelData> Unidentified record "
          << idx << "=" << line;
    }
  }
  //  if (valid())
  //    DBG << debug();
}

bool LevelData::valid() const
{
  return id.valid();
}

std::string LevelData::debug() const
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
  if (!unplaced_gammas.empty())
  {
    ret += "  Unplaced_gammas:\n";
    for (auto c : unplaced_gammas)
      ret += "    " + c.debug() + "\n";
  }
  return ret;
}

void NuclideData::merge_adopted(DecayData& decaydata,
                                double max_level_dif,
                                double max_gamma_dif) const
{
  for (LevelRecord& lev : decaydata.levels)
  {
    std::list<LevelRecord> relevant_levels
        = adopted_levels.nearest_levels(lev.energy, decaydata.id.dsid,
                                        max_level_dif);

    if (!relevant_levels.empty())
    {

//    DBG << " B E F O R E\n"
//        << "=======================================\n"
//        << lev.debug() << "\n"
//        << "=======================================\n"
//           ;

    for (const LevelRecord& l
         : adopted_levels.nearest_levels(lev.energy,
                                         decaydata.id.dsid,
                                         max_level_dif))
    {
//      DBG << "+++++FoundE = " << l.debug();
      lev.merge_adopted(l, max_gamma_dif);
    }

//    DBG << "                              A F T E R\n"
//        << "---------------------------------------\n"
//        << lev.debug() << "\n"
//        << "---------------------------------------\n"
//           ;
    }

//    for (const LevelRecord& l : adopted_levels.levels)
//      if (nearest_levels(l.energy, ).empty())
//        gammas.push_back(g);


  }
}


std::string NuclideData::add_decay(const DecayData& dec)
{
  auto base_name = dec.name();
  if (!dec.valid())
    base_name = "INVALID PARSE of " + dec.id.extended_dsid;

  // insert into decay map
  int count {0};
  auto disambiguated = base_name;
  while (decays.count(disambiguated))
  {
    count++;
    if (count > 1)
      disambiguated = base_name + " (alt."
          + boost::lexical_cast<std::string>(count)
          + ")";
    else
      disambiguated = base_name + " (alt.)";
  }

  //  if (!dec.valid())
  //    DBG << "<NuclideData> Adding decay: " << disambiguated;
  decays[disambiguated] = dec;

  return disambiguated;
}
