#include "DecayInfo.h"
#include "custom_logger.h"
#include "qpx_util.h"
#include <boost/regex.hpp>

#include "ensdf_types.h"

DecayInfo::DecayInfo(std::string dsid)
{
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
    return;

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
        parent = parse_nid(what2[1]);
        if (!check_nid_parse(what2[1], parent))
        {
          ERR << "<BasicDecay> Could not parse parent NucID   \""
//              << boost::trim_copy(nid_to_ensdf(parent)) << "\"  !=  \""
              << what2[1] << "\" from \"" << parent_str << "\""
              << "   in   \"" << dsid << "\"";
      //    return DecayInfo();
        }


      }
      else
        DBG << "Failed parent " << parent_str;

    }
  }
//  else
//    DBG << "Failed parents " << parents;

  mode = parse_decay_mode(tokens.at(1));
  if (mode_to_ensdf(mode) != tokens.at(1))
  {
//    ERR << "Could not parse decay mode in \"" << record.extended_dsid << "\"";
    return;
  }

  if (tokens.size() > 2)
  {
    hl = parse_halflife(tokens.at(2));
  }
}

bool DecayInfo::valid() const
{
  return parent.valid() && mode.valid();
}

std::string DecayInfo::to_string() const
{
  std::string ret;
  ret += "   mode=" + mode.to_string();
//  ret += " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
  if (hl.isValid())
    ret += "   hl=" + hl.to_string(true);
  return ret;
}

std::string DecayInfo::name() const
{
  return mode.to_string();
}


