#include "ensdf_records.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

IdentificationRecord IdentificationRecord::parse(const std::string& line)
{
  if (line.size() != 80)
  {
    ERR << "Bad ID Record length " << line.size();
    return IdentificationRecord();
  }

  IdentificationRecord ret;
  std::string nid = line.substr(0, 5);

  ret.nuc_id = parse_nid(nid);
  if (!check_nid_parse(nid, ret.nuc_id))
    ERR << "<IdentificationRecord> Could not parse daughter NucID   \""
//        << nid_to_ensdf(ret.nuc_id) << "\"  !=  \""
        << nid << "\"";

  ret.dsid = line.substr(9, 30);
  ret.dsref = line.substr(39, 15);
  ret.pub = line.substr(65, 8);
  std::string year_str = line.substr(74, 4);
  if (!boost::trim_copy(year_str).empty())
    ret.year = boost::lexical_cast<uint16_t>(year_str);
  std::string month_str = line.substr(78, 2);
  if (!boost::trim_copy(month_str).empty())
    ret.month = boost::lexical_cast<uint16_t>(month_str);

  ret.extended_dsid = ret.dsid;
  ret.continued = (ret.dsid.at(ret.dsid.size()-1) == ',');
  ret.type = is_type(ret.extended_dsid);
  return ret;
}

void IdentificationRecord::merge_continued(IdentificationRecord other)
{
  extended_dsid += other.extended_dsid;
  numlines += other.numlines;
  type = is_type(extended_dsid);
}

RecordType IdentificationRecord::is_type(std::string s)
{
  std::string str = boost::to_upper_copy(s);
  if (boost::contains(str, "COMMENTS"))
    return RecordType::Comments;
  else if (boost::contains(str, "REFERENCES"))
    return RecordType::References;
  else if (boost::contains(str, "MUONIC ATOM"))
    return RecordType::MuonicAtom;
  else if (boost::contains(str, "(HI,"))
    return RecordType::HiXng;

  RecordType ret = RecordType::Invalid;
  if (boost::contains(str, "COULOMB EXCITATION"))
    ret |= RecordType::CoulombExcitation;
  if (boost::contains(str, "ADOPTED LEVELS"))
    ret |= RecordType::AdoptedLevels;
  if (boost::contains(str, "TENTATIVE"))
    ret |= RecordType::Tentative;
  if (boost::contains(str, "GAMMAS"))
    ret |= RecordType::Gammas;
  if (boost::contains(str, "DECAY"))
    ret |= RecordType::Decay;

  return ret;
}

std::string IdentificationRecord::type_to_str(RecordType t)
{
  if (test(t & RecordType::Comments))
    return "COMMENTS";
  else if (test(t & RecordType::References))
    return "REFERENCES";
  else if (test(t & RecordType::CoulombExcitation))
    return "COULOMB EXCITATION";
  else if (test(t & RecordType::MuonicAtom))
    return "MUONIC ATOM";
  else if (test(t & RecordType::HiXng))
    return "(HI,XNG)";

  std::string ret;

  if (test(t & RecordType::AdoptedLevels))
    ret += "ADOPTED LEVELS";
  if (test(t & RecordType::Decay))
    ret += "DECAY";
  if (test(t & RecordType::Gammas))
    ret += ", GAMMAS";
  if (test(t & RecordType::Tentative))
    ret += ": TENTATIVE";

  return ret;
}

std::string IdentificationRecord::debug() const
{
  std::stringstream ss;
  ss << type_to_str(type);
  ss << " " << nuc_id.verboseName();
  ss << " \"" << boost::trim_copy(extended_dsid) << "\"";
  ss << " dsref=\"" << dsref << "\"";
  ss << " pub=\"" << pub << "\"  ";
  ss << year << "/" << month;
//  ss << " dsid=\"" << boost::trim_copy(dsid) << "\"";
//  if (continued)
//    ss << " CONTINUED ";
//  if (numlines)
//    ss << " lines=" << numlines;
  return ss.str();
}


CommentsRecord CommentsRecord::from_id(const IdentificationRecord &record, BlockIndices block)
{
  if ((record.type & RecordType::Comments) == RecordType::Invalid)
  {
    DBG << "Bad ID record type " << record.debug();
    return CommentsRecord();
  }

  CommentsRecord ret;
  ret.block = block;
  ret.nuclide = record.nuc_id;

  return ret;
}


ParentRecord ParentRecord::from_ensdf(const std::string &record)
{
  ParentRecord prec;
  if (record.size() < 50)
    return prec;
  prec.nuclide = parse_nid(record.substr(0,5));
  prec.energy = parse_energy(record.substr(9, 12));
  prec.hl = parse_halflife(record.substr(39, 16));
  prec.spin = parse_spin_parity(record.substr(21, 18));
  return prec;
}

std::string ParentRecord::to_string() const
{
  std::string ret;
  ret = nuclide.verboseName()
      + " " + energy.to_string()
      + " " + spin.to_string()
      + " " + hl.to_string();
  return ret;
}

BasicDecayData BasicDecayData::from_id(const IdentificationRecord &record, BlockIndices block)
{
  if ((record.type & RecordType::Decay) == RecordType::Invalid)
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
    for (int i=1; i < what.size(); ++i)
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
  else
    DBG << "Failed parents " << parents;

  ret.mode = parse_decay_mode(tokens.at(1));
  if (mode_to_ensdf(ret.mode) != tokens.at(1))
  {
    ERR << "Could not parse decay mode in \"" << record.extended_dsid << "\"";
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
//  decaydata.dsid = boost::trim_copy(header.substr(9,30)); // saved for comparison with xref records
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

ReactionData ReactionData::from_id(const IdentificationRecord &record, BlockIndices block)
{
  ReactionData ret;
  ret.dsid = record.dsid;
  ret.block = block;
  ret.daughter = record.nuc_id;

//  std::list<std::string> tokens;

//  boost::regex decay_expr("^[([\\sA-Z0-9]+[\\([\\sA-Z0-9]+\\),*\\s+]+)[, ]*]+([E=[\\sA-Z0-9]+]*)");

//  boost::regex decay_expr("^[([\\sA-Z0-9]+[\\([\\sA-Z0-9]+\\),*\\s+]+)[, ]*]");

//  boost::smatch what;
//  if (boost::regex_match(boost::trim_copy(record.extended_dsid),
//                         what, decay_expr) && (what.size() > 0))
//    for (auto w : what)
//      DBG << "   " << w;

  return ret;
}

std::string ReactionData::to_string() const
{
  std::string ret;
  return ret;
}
