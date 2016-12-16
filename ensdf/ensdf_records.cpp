#include "ensdf_records.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

IdentificationRecord IdentificationRecord::parse(const std::string& line)
{
  IdentificationRecord ret;
  if (line.size() != 80)
  {
    ERR << "Bad ID Record length " << line.size();
    return ret;
  }

  ret.numlines = 1;
  ret.nuc_id = as_nucid(line.substr(0, 5));
  ret.dsid = boost::algorithm::trim_copy(line.substr(9, 30));
  ret.extended_dsid = ret.dsid;

  if (!ret.dsid.empty())
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
  if (nuc_id.valid())
    ss << " " << nuc_id.verboseName();
  ss << " dsid=\"" << boost::trim_copy(dsid) << "\"";
  ss << " full=\"" << boost::trim_copy(extended_dsid) << "\"";
  if (continued)
    ss << " CONTINUED ";
  if (numlines)
    ss << " lines=" << numlines;
  return ss.str();
}



ParentRecord ParentRecord::from_ensdf(const std::string &record)
{
  ParentRecord prec;
  if (record.size() < 50)
    return prec;
  prec.nuclide = as_nucid(record.substr(0,5));
  prec.energy = as_energy(record.substr(9, 12));
  prec.hl = as_halflife(record.substr(39, 16));
  prec.spin = as_spin_parity(record.substr(21, 18));
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
  ret.daughter = record.nuc_id;
  ret.block = block;

  auto dsid = record.extended_dsid;
  boost::replace_all(dsid, "DECAY", "");

  std::vector<std::string> tokens;
  boost::split(tokens, dsid, boost::is_any_of(" "));
  if (tokens.size() < 2)
    return BasicDecayData();

  ret.parent = as_nucid(tokens.at(0)); //hack
  ret.mode = as_mode(tokens.at(1));

  if (tokens.size() > 2)
    ret.hl = as_halflife(tokens.at(2));

  return ret;
}

BasicDecayData BasicDecayData::from_ensdf(const std::string &header, BlockIndices block)
{
  BasicDecayData decaydata;

  boost::regex decay_expr("^([\\sA-Z0-9]{5,5})\\s{4,4}[0-9]{1,3}[A-Z]{1,2}\\s+((?:B-|B\\+|EC|IT|A)\\sDECAY).*$");
  boost::smatch what;
  if (!boost::regex_match(header, what, decay_expr) || (what.size() < 2))
    return decaydata;

  decaydata.daughter = as_nucid(what[1]);
  decaydata.mode.types_.push_back(parseDecayType(what[2]));
  decaydata.block = block;
  decaydata.dsid = boost::trim_copy(header.substr(9,30)); // saved for comparison with xref records
  return decaydata;
}

DecayMode::DecayType BasicDecayData::parseDecayType(const std::string &tstring)
{
  if (tstring == "EC DECAY")
    return DecayMode::DecayType::ElectronCapture;
  if (tstring == "B+ DECAY")
    return DecayMode::DecayType::BetaPlus;
  if (tstring == "B- DECAY")
    return DecayMode::DecayType::BetaMinus;
  if (tstring == "IT DECAY")
    return DecayMode::DecayType::IsomericTransition;
  if (tstring == "A DECAY")
    return DecayMode::DecayType::Alpha;
  return DecayMode::DecayType::Undefined;
}

std::string BasicDecayData::to_string() const
{
  std::string ret;
  ret = daughter.verboseName();
  if (mode.types_.size())
  ret += "   mode=" + mode.to_string();
//  ret += " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
  if (hl.isValid())
    ret += "   hl=" + hl.to_string(true);
  ret += "   dsid=\"" + dsid + "\"";
  return ret;
}
