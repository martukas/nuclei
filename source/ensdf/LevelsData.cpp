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
    DBG << "<LevelsData> Bad ID " << i.lines[idx];
    return;
  }

  read_hist(i);
  read_prelims(i);
  read_unplaced(i);
  read_comments(i);
  read_levels(i);

  //  if (valid())
  //    DBG << debug();
}

void LevelsData::read_hist(ENSDFData& i)
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
        DBG << "<LevelsData> Invalid Hist " << his.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

void LevelsData::read_comments(ENSDFData& i)
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
        DBG << "<LevelsData> Invalid Comment " << com.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

void LevelsData::read_prelims(ENSDFData& i)
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
          DBG << "<LevelsData> More than one norm " << pnorm.debug()
              << " from " << idx << "=" << line;

        pnorm = pn;
      }
      else
        DBG << "<LevelsData> Invalid Pnorm " << pn.debug()
            << " from " << idx << "=" << line;
    }
    else if (QValueRecord::match(line))
    {
      auto qv = QValueRecord(++i);
      if (qv.valid())
        qvals.push_back(qv);
      else
        DBG << "<LevelsData> Invalid Qval " << qv.debug()
            << " from " << idx << "=" << line;
    }
    else if (CommentsRecord::match(line, "."))
    {
      auto com = CommentsRecord(++i);
      if (com.valid())
        comments.push_back(com);
      else
        DBG << "<LevelsData> Invalid Comment " << com.debug()
            << " from " << idx << "=" << line;
    }
    else if (XRefRecord::match(line))
    {
      auto ref = XRefRecord(++i);
      if (ref.valid())
        xrefs[ref.dsid] = ref.dssym;
      else
        DBG << "<LevelsData> Invalid Xref " << ref.debug()
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

void LevelsData::read_unplaced(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (AlphaRecord::match(line))
    {
      auto a = AlphaRecord(++i);
      if (a.valid())
        unplaced.alpha.push_back(a);
      else
        DBG << "<DecayData> Invalid alpha " << a.debug()
            << " from " << idx << "=" << line;
    }
    else if (BetaRecord::match(line))
    {
      auto b = BetaRecord(++i);
      if (b.valid())
        unplaced.beta.push_back(b);
      else
        DBG << "<DecayData> Invalid beta " << b.debug()
            << " from " << idx << "=" << line;
    }
    else if (GammaRecord::match(line))
    {
      auto g = GammaRecord(++i);
      if (g.valid())
        unplaced.gamma.push_back(g);
      else
        DBG << "<DecayData> Invalid gamma " << g.debug()
            << " from " << idx << "=" << line;
    }
    else if (ParticleRecord::match(line))
    {
      auto p = ParticleRecord(++i);
      if (p.valid())
        unplaced.particle.push_back(p);
      else
        DBG << "<DecayData> Invalid particle " << p.debug()
            << " from " << idx << "=" << line;
    }
    else
      break;
  }
}

void LevelsData::read_levels(ENSDFData& i)
{
  while (i.has_more())
  {
    auto idx = i.i.first;
    auto line = i.look_ahead();
    if (LevelRecord::match(line))
    {
      auto lev = LevelRecord(++i);
      if (lev.valid())
        levels.push_back(lev);
      else
        i.print("<LevelsData> Invalid level", idx, lev.debug());
    }
    else
    {
      i.print("<LevelsData> Unidentified record", idx);
      ++i;
    }
  }

}
