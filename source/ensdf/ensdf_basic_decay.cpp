#include "ensdf_basic_decay.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

#include "xref_record.h"

DecayData::DecayData(const std::vector<std::string>& data,
                               BlockIndices idx)
{
  if (idx.first >= data.size())
    return;
  id = IdRecord(idx.first, data);
  block = idx;
  if (!id.valid())
  {
    DBG << "<DecayData> Bad ID " << data[idx.first];
    return;
  }

  idx.first++;

  read_hist(data, idx);
  read_prelims(data, idx);
  read_unplaced(data, idx);

  for (; idx.first < idx.last; ++idx.first)
  {
    try
    {
      if (CommentsRecord::match(data[idx.first], "L"))
      {
        auto com = CommentsRecord(idx.first, data);
        if (com.valid())
          comments.push_back(com);
        else
          DBG << "<LevelData> Invalid all-level comment " << com.debug()
              << "\nfrom " << idx.first << " = " << data[idx.first];
        //      DBG << "Added comment from" << idx.first << "=" << data[idx.first] << com.debug();
      }
      else if (LevelRecord::match(data[idx.first]))
      {
        auto i = idx.first;
        auto lev = LevelRecord(idx.first, data);
        if (lev.valid())
          levels.push_back(lev);
        else
        {
          //        DBG << "<LevelData> Invalid Level " << lev.debug()
          //            << " from " << i << "=" << data[i];
          auto val = data[i].substr(9,10);
          auto uncert = data[i].substr(19,2);
          auto vu = parse_val_uncert(val, uncert);
          DBG << "<LevelData> Invalid Level at " << i << "=" << data[i];
          DBG << "Val=" << val << " Unc=" << uncert << " VU=" << vu.to_string(true);
        }
        //      DBG << "Added level from" << idx.first << "=" << data[idx.first];
      }
      else
      {
        DBG << "<LevelData> Unidentified record "
            << idx.first << "=" << data[idx.first];
      }
    }
    catch (...)
    {
      DBG << "<LevelData::LevelData> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }


  if (!test(id.type) ||
      test(id.type & RecordType::Comments) ||
      test(id.type & RecordType::References) ||
      test(id.type & RecordType::MuonicAtom))
  {
    DBG << "Bad ID record type " << id.debug();
    //HACK (Tentative should propagate to NucData)
    return;
  }

  if (test(id.type & RecordType::Decay))
    decay_info_ = DecayInfo(id.extended_dsid);

  if (ReactionInfo::match(id.extended_dsid))
    reaction_info_ = ReactionInfo(id.extended_dsid, id.nuclide);

//  DBG << "Retrieved decay " << decay_info_.to_string()
//      << " with h=" << history.size()
//      << " & c=" << comments.size()
//      << " & p=" << parents.size();

  parents.clear();
}

std::string DecayData::to_string() const
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

void DecayData::read_hist(const std::vector<std::string>& data,
                          BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         HistoryRecord::match(data[idx.first]))
  {
    try
    {
      auto his = HistoryRecord(idx.first, data);
      if (his.valid())
        history.push_back(his);
      else
        DBG << "<DecayData> Invalid Hist " << his.debug()
            << " from " << idx.first << "=" << data[idx.first];
      idx.first++;
    }
    catch (...)
    {
      DBG << "<DecayData::read_hist> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }
}

void DecayData::read_prelims(const std::vector<std::string>& data,
                             BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         (ParentRecord::match(data[idx.first]) ||
          CommentsRecord::match(data[idx.first]) || //all comments
          NormalizationRecord::match(data[idx.first]) ||
          ProdNormalizationRecord::match(data[idx.first])
          )
         )
  {
    try
    {
      if (ProdNormalizationRecord::match(data[idx.first]))
      {
        auto pn = ProdNormalizationRecord(idx.first, data);
        if (pn.valid())
        {
          if (pnorm.valid())
            DBG << "<LevelData> More than one pnorm!!!";
          pnorm = pn;
        }
        else
          DBG << "<LevelData> Invalid Pnorm " << pn.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      else if (NormalizationRecord::match(data[idx.first]))
      {
        auto n = NormalizationRecord(idx.first, data);
        if (n.valid())
        {
          if (norm.valid())
            DBG << "<DecayData> More than one norm!!!";
          norm = n;
        }
        else
          DBG << "<DecayData> Invalid Norm " << n.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      else if (CommentsRecord::match(data[idx.first]))
      {
        auto com = CommentsRecord(idx.first, data);
        if (com.valid())
          comments.push_back(com);
        else
          DBG << "<DecayData> Invalid Comment " << com.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      else if (ParentRecord::match(data[idx.first]))
      {
        auto par = ParentRecord(idx.first, data);
        if (par.valid())
          parents.push_back(par);
        else
          DBG << "<DecayData> Invalid Parent " << par.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      idx.first++;
    }
    catch (...)
    {
      DBG << "<DecayData::read_prelims> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }
}


void DecayData::read_unplaced(const std::vector<std::string>& data,
                             BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         (
          GammaRecord::match(data[idx.first]) ||
          ParticleRecord::match(data[idx.first]) //|| //all comments
//          BetaRecord::match(data[idx.first]) ||
//          ECRecord::match(data[idx.first])
          )
         )
  {
    try
    {
      if (ParticleRecord::match(data[idx.first]))
      {
        auto p = ParticleRecord(idx.first, data);
        if (p.valid())
          particles.push_back(p);
        else
          DBG << "<DecayData> Invalid particle " << p.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      else if (GammaRecord::match(data[idx.first]))
      {
        auto g = GammaRecord(idx.first, data);
        if (g.valid())
          gammas.push_back(g);
        else
          DBG << "<DecayData> Invalid gamma " << g.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }

      idx.first++;
    }
    catch (...)
    {
      DBG << "<DecayData::read_prelims> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }
}





void LevelData::read_hist(const std::vector<std::string>& data,
                          BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         HistoryRecord::match(data[idx.first]))
  {
    try
    {
      auto his = HistoryRecord(idx.first, data);
      if (his.valid())
        history.push_back(his);
      else
        DBG << "<LevelData> Invalid Hist " << his.debug()
            << " from " << idx.first << "=" << data[idx.first];
      idx.first++;
    }
    catch (...)
    {
      DBG << "<LevelData::read_hist> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }
}

void LevelData::read_comments(const std::vector<std::string>& data,
                              BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         CommentsRecord::match(data[idx.first], ".")) //all comments
  {
    try
    {
      auto com = CommentsRecord(idx.first, data);
      if (com.valid())
        comments.push_back(com);
      else
        DBG << "<LevelData> Invalid Comment " << com.debug()
            << " from " << idx.first << "=" << data[idx.first];
      idx.first++;
    }
    catch (...)
    {
      DBG << "<LevelData::read_comments> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }
}

void LevelData::read_prelims(const std::vector<std::string>& data,
                             BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         (QValueRecord::match(data[idx.first]) ||
          CommentsRecord::match(data[idx.first], ".") || //all comments
          XRefRecord::match(data[idx.first]) ||
          ProdNormalizationRecord::match(data[idx.first])
          )
         )
  {
    try
    {
      if (ProdNormalizationRecord::match(data[idx.first]))
      {
        auto pn = ProdNormalizationRecord(idx.first, data);
        if (pn.valid())
        {
          if (pnorm.valid())
            DBG << "<LevelData> More than one pnorm!!!";
          pnorm = pn;
        }
        else
          DBG << "<LevelData> Invalid Pnorm " << pn.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      else if (QValueRecord::match(data[idx.first]))
      {
        auto qv = QValueRecord(idx.first, data);
        if (qv.valid())
          qvals.push_back(qv);
        else
          DBG << "<LevelData> Invalid Qval " << qv.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      else if (XRefRecord::match(data[idx.first]))
      {
        auto ref = XRefRecord(idx.first, data);
        if (ref.valid())
          xrefs[ref.dssym] = ref.dsid;
        else
          DBG << "<LevelData> Invalid Xref " << ref.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      else if (CommentsRecord::match(data[idx.first], "."))
      {
        auto com = CommentsRecord(idx.first, data);
        if (com.valid())
          comments.push_back(com);
        else
          DBG << "<LevelData> Invalid Comment " << com.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      idx.first++;
    }
    catch (...)
    {
      DBG << "<LevelData::read_prelims> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }
}

void LevelData::read_unplaced_gammas(const std::vector<std::string>& data,
                                     BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         GammaRecord::match(data[idx.first]))
  {
    try
    {
      if (GammaRecord::match(data[idx.first]))
      {
        auto gam = GammaRecord(idx.first, data);
        if (gam.valid())
          gammas.push_back(gam);
        else
          DBG << "<LevelData> Invalid unplaced gamma " << gam.debug()
              << " from " << idx.first << "=" << data[idx.first];
      }
      idx.first++;
    }
    catch (...)
    {
      DBG << "<LevelData::read_unplaced_gammas> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
    }
  }
}

LevelData::LevelData(const std::vector<std::string>& data,
                     BlockIndices idx)
{
  if (idx.first >= data.size())
    return;
  id = IdRecord(idx.first, data);
  if (!id.valid())
  {
    DBG << "<LevelData> Bad ID " << data[idx.first];
    return;
  }

  idx.first++;
  read_hist(data, idx);
  read_prelims(data, idx);

  read_unplaced_gammas(data, idx);
  read_comments(data, idx);

  for (; idx.first < idx.last; ++idx.first)
  {
    try
    {
      if (CommentsRecord::match(data[idx.first], "L"))
      {
        auto com = CommentsRecord(idx.first, data);
        if (com.valid())
          comments.push_back(com);
        else
          DBG << "<LevelData> Invalid all-level comment " << com.debug()
              << "\nfrom " << idx.first << " = " << data[idx.first];
        //      DBG << "Added comment from" << idx.first << "=" << data[idx.first] << com.debug();
      }
      else if (LevelRecord::match(data[idx.first]))
      {
        auto i = idx.first;
        auto lev = LevelRecord(idx.first, data);
        if (lev.valid())
          levels.push_back(lev);
        else
        {
          //        DBG << "<LevelData> Invalid Level " << lev.debug()
          //            << " from " << i << "=" << data[i];
          auto val = data[i].substr(9,10);
          auto uncert = data[i].substr(19,2);
          auto vu = parse_val_uncert(val, uncert);
          DBG << "<LevelData> Invalid Level at " << i << "=" << data[i];
          DBG << "Val=" << val << " Unc=" << uncert << " VU=" << vu.to_string(true);
        }
        //      DBG << "Added level from" << idx.first << "=" << data[idx.first];
      }
      else
      {
        DBG << "<LevelData> Unidentified record "
            << idx.first << "=" << data[idx.first];
      }
    }
    catch (...)
    {
      DBG << "<LevelData::LevelData> Shit hit the fan at "
          << idx.first << "=" << data[idx.first];
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
  if (!gammas.empty())
  {
    ret += "  Gammas:\n";
    for (auto c : gammas)
      ret += "    " + c.debug() + "\n";
  }
  return ret;
}

