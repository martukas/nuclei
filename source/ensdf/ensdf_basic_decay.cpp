#include "ensdf_basic_decay.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

#include "xref_record.h"

BasicDecayData BasicDecayData::from_id(const IdRecord &record,
                                       BlockIndices block)
{
  if (((record.type & RecordType::Decay) == RecordType::Invalid) &&
      ((record.type & RecordType::InelasticScattering) == RecordType::Invalid))
  {
    DBG << "Bad ID record type " << record.debug();
    //HACK (Tentative should propagate to NucData)
    return BasicDecayData();
  }

  BasicDecayData ret;
  ret.dsid = record.dsid; // saved for comparison with xref records
  ret.block = block;
  ret.daughter = record.nuc_id;

  auto dsid = record.extended_dsid;
  std::string parents; bool valid = false; bool space = false;
  for (size_t i = 0; i < dsid.size(); ++i)
  {
    if (dsid.at(i) != ' ')
      valid = true;
    if (valid && (dsid.at(i) == ' '))
      space = true;
    if (valid && space && (dsid.at(i) != ' '))
      break;
    parents += std::string(1, dsid.at(i));
  }

  boost::replace_all(dsid, "DECAY", "");
  std::vector<std::string> tokens;
  boost::split(tokens, dsid, boost::is_any_of(" "));
  if (tokens.size() < 2)
    return BasicDecayData();

  std::string parent_xx = "([0-9]+[A-Z]*(?:\\([A-Z0-9]+,[A-Z0-9]+\\))?(?::[\\s0-9A-Z]+)?(?:\\[\\+[0-9]+\\])?)";

  boost::regex parents_expr("^(?:\\s)*" + parent_xx + "(?:,[\\s]*" + parent_xx + ")*(?:\\s)*$");
  boost::smatch what;
  if (boost::regex_match(parents, what, parents_expr) && (what.size() > 1))
  {
    for (size_t i=1; i < what.size(); ++i)
    {
      std::string parent_str = what[1];
//      DBG << "Parent string " << parent_str;
      boost::regex parent_expr("^([0-9]+[A-Z]*)(\\([A-Z0-9]+,[A-Z0-9]+\\))?(:[\\s0-9A-Z]+)?(\\[\\+[0-9]+\\])?$");
      boost::smatch what2;
      if (boost::regex_match(parent_str, what2, parent_expr) && (what2.size() > 1))
      {
        ret.parent = parse_nid(what2[1]);
        if (!check_nid_parse(what2[1], ret.parent))
        {
          ERR << "<BasicDecay> Could not parse parent NucID   \""
//              << boost::trim_copy(nid_to_ensdf(ret.parent)) << "\"  !=  \""
              << what2[1] << "\" from \"" << parent_str << "\""
              << "   in   \"" << record.extended_dsid << "\"";
      //    return BasicDecayData();
        }


      }
      else
        DBG << "Failed parent " << parent_str;

    }
  }
//  else
//    DBG << "Failed parents " << parents;

  ret.mode = parse_decay_mode(tokens.at(1));
  if (mode_to_ensdf(ret.mode) != tokens.at(1))
  {
//    ERR << "Could not parse decay mode in \"" << record.extended_dsid << "\"";
    return BasicDecayData();
  }

  if (tokens.size() > 2)
  {
    ret.hl = parse_halflife(tokens.at(2));

  }

  return ret;
}

//BasicDecayData BasicDecayData::from_ensdf(const std::string &header, BlockIndices block)
//{
//  BasicDecayData decaydata;

//  boost::regex decay_expr("^([\\sA-Z0-9]{5,5})\\s{4,4}[0-9]{1,3}[A-Z]{1,2}\\s+((?:B-|B\\+|EC|IT|A)\\sDECAY).*$");
//  boost::smatch what;
//  if (!boost::regex_match(header, what, decay_expr) || (what.size() < 2))
//    return decaydata;

//  decaydata.daughter = parse_nid(what[1]);
//  decaydata.mode = parseDecayType(what[2]);
//  decaydata.block = block;
//  decaydata.dsid = boost::trim_copy(substr(9,30)); // saved for comparison with xref records
//  return decaydata;
//}

DecayMode BasicDecayData::parseDecayType(const std::string &tstring)
{
  DecayMode mode;
  if (tstring == "EC DECAY")
    mode.set_electron_capture(1);
  if (tstring == "B+ DECAY")
    mode.set_beta_plus(1);
  if (tstring == "B- DECAY")
    mode.set_beta_minus(1);
  if (tstring == "IT DECAY")
    mode.set_isomeric(true);
  if (tstring == "A DECAY")
    mode.set_alpha(true);
  return mode;
}

std::string BasicDecayData::to_string() const
{
  std::string ret;
  ret = daughter.verboseName();
  ret += "   mode=" + mode.to_string();
//  ret += " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
  if (hl.isValid())
    ret += "   hl=" + hl.to_string(true);
  ret += "   dsid=\"" + dsid + "\"";
  return ret;
}






void LevelData::read_hist(const std::vector<std::string>& data,
                          BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         HistoryRecord::match(data[idx.first]))
  {
    auto his = HistoryRecord(idx.first, data);
    if (his.valid())
      history.push_back(his);
    else
      DBG << "<LevelData> Invalid Hist " << his.debug()
          << " from " << idx.first << "=" << data[idx.first];
    idx.first++;
  }
}

void LevelData::read_comments(const std::vector<std::string>& data,
                              BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         CommentsRecord::match(data[idx.first], ".")) //all comments
  {
    auto com = CommentsRecord(idx.first, data);
    if (com.valid())
      comments.push_back(com);
    else
      DBG << "<LevelData> Invalid Comment " << com.debug()
          << " from " << idx.first << "=" << data[idx.first];
    idx.first++;
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
}

void LevelData::read_unplaced_gammas(const std::vector<std::string>& data,
                                     BlockIndices& idx)
{
  while ((idx.first < idx.last) &&
         (GammaRecord::match(data[idx.first])
          /*|| CommentsRecord::match(data[idx.first], "G")*/))
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
//    else if (CommentsRecord::match(data[idx.first], "G"))
//    {
//      auto com = CommentsRecord(idx.first, data);
//      if (com.valid())
//        comments.push_back(com);
//      else
//        DBG << "<LevelData> Invalid all-level gamma comment " << com.debug()
//            << " from " << idx.first << "=" << data[idx.first];
//    }
    idx.first++;
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
        DBG << "<LevelData> Invalid Level at " << i << "  ";
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

