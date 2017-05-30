#include "LevelsData.h"
#include "Fields.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

#include "XRef.h"

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
