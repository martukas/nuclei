#include "ensdf_reaction_data.h"
#include "custom_logger.h"
#include "qpx_util.h"
#include <boost/regex.hpp>

#include "ensdf_types.h"

#define RGX_NUCLIDE "[\\w\\d]+"
#define RGX_REACTANT "[\\w\\d\\s\\+'-]+"

#define RGX_INOUT "\\(" RGX_REACTANT "," RGX_REACTANT "\\)"
#define RGX_INOUT_PARSE "^\\((" RGX_REACTANT "),(" RGX_REACTANT ")\\)$"

#define RGX_REACTION RGX_NUCLIDE "\\s*" RGX_INOUT "(?:," RGX_INOUT ")*"
#define RGX_REACTION_PARSE "^(" RGX_NUCLIDE ")\\s*(" RGX_INOUT "(?:," RGX_INOUT ")*)$"

#define RGX_EXTRAS "^(" RGX_REACTION "(?:\\s*,\\s*" RGX_REACTION "\\s*)*)(.*)$"
#define RGX_REACTIONS "^(" RGX_REACTION ")(?:\\s*,\\s*(" RGX_REACTION "))*$"


Reactants::Reactants(std::string s)
{
  boost::trim(s);
  boost::smatch what;
  if (boost::regex_match(s, what, boost::regex(RGX_INOUT_PARSE))
      && (what.size() > 2))
  {
    in = what[1];
    out = what[2];
  }
}

bool Reactants::valid() const
{
  return (!in.empty() && !out.empty());
}

std::string Reactants::to_string() const
{
  return "(" + in + "," + out + ")";
}




Reaction::Reaction(std::string s, NuclideId daughter)
{
  std::string ios;
  boost::smatch what;
  if (boost::regex_match(s, what,
                         boost::regex(RGX_REACTION_PARSE))
      && (what.size() > 2))
  {
    target = parse_nid(what[1]);
    if (!target.composition_known() && target.Z())
      target.set_A(daughter.A());

    ios = what[2];
  }

  //Target sometimes only letter!!! wtf?
  boost::trim(ios);
//  DBG << "target=" << target.symbolicName();
//  DBG << "pairs=" << ios;

  boost::sregex_token_iterator iter(ios.begin(), ios.end(),
                                    boost::regex(RGX_INOUT), 0);
  for( ; iter != boost::sregex_token_iterator(); ++iter )
  {
    Reactants reactants(*iter);
    if (reactants.valid())
      variants.push_back(reactants);
//    DBG << "   ants " << *iter
//        << " -> " << reactants.to_string();
  }
}

std::string Reaction::to_string() const
{
  std::string ret = boost::trim_copy(nid_to_ensdf(target, false));
  bool first = true;
  for (auto x : variants)
  {
    if (!first)
      ret += ",";
    if (first)
      first = false;
    ret += x.to_string();
  }
  return ret;
}

bool Reaction::valid() const
{
  return (target.valid() && !variants.empty());
}



bool ReactionInfo::match(std::string record)
{
  return boost::regex_match(boost::trim_copy(record),
                            boost::regex(RGX_EXTRAS));
}

ReactionInfo::ReactionInfo(std::string ext_dsid, NuclideId daughter)
{
  boost::trim(ext_dsid);

  std::string xtions, extras;
//  std::string xtras;

//  DBG << "Will parse " << record.debug();

  boost::smatch what;
  if (boost::regex_match(ext_dsid, what, boost::regex(RGX_EXTRAS))
      && (what.size() > 1))
  {
    xtions = what[1];
    if (what.size() > 2)
      extras = what[2];
    boost::trim(xtions);
    boost::trim(extras);
//    DBG << " good " << xtions << " AND " << extras;
  }

//  DBG << " Xions " << xtions;

  boost::smatch what2;
  boost::regex_match(xtions, what2, boost::regex(RGX_REACTIONS));
  for (size_t i=1; i < what2.size(); ++i)
  {
    std::string xtion = what2[i];
    boost::trim(xtion);
    if (xtion.empty())
      continue;
    Reaction reaction(xtion, daughter);
//    DBG << "Parsed reaction " << xtion << " as "
//        << reaction.to_string()
//        << " " << reaction.valid();
    if (reaction.valid())
      reactions.push_back(reaction);
  }

  if (find_remove(extras, "E=", "E=") ||
      find_remove(extras, "E =", "E =") ||
      find_remove(extras, "E<", "E") ||
      find_remove(extras, "E>", "E") ||
      find_remove(extras, "E AP", "E "))
  {
    boost::trim(energy);
    std::size_t found = energy.find("E=");
    if (found!=std::string::npos)
      energy.erase(found, 2);
    else
      energy.erase(0,1);
    boost::trim(energy);
  }

  boost::trim(extras);
  std::size_t found = energy.find(":");
  if (found!=std::string::npos)
    energy.erase(found, 1);
  boost::trim(extras);

  qualifier = extras;
}

bool ReactionInfo::find_remove(std::string& extras, std::string wanted, std::string trim_what)
{
  std::size_t found = extras.find(wanted);
  if (found!=std::string::npos)
  {
    energy = extras.substr(found, extras.size()-found);
    extras.erase(found, extras.size()-found);
    found = energy.find(trim_what);
    if (found!=std::string::npos)
      energy.erase(found, trim_what.size());
    boost::trim(energy);
    return true;
  }
  return false;
}

bool ReactionInfo::valid() const
{
  return !reactions.empty();
}

std::string ReactionInfo::to_string() const
{
  return name();
}

std::string ReactionInfo::name() const
{
  std::string ret;
  bool first = true;
  for (auto r : reactions)
  {
    if (!first)
      ret += ",";
    if (first)
      first = false;
    ret += r.to_string();
  }
  if (!qualifier.empty())
    ret += " " + qualifier;
  if (!energy.empty())
    ret += " E=" + energy;
  return ret;
}

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

DecayMode DecayInfo::parse_type(const std::string &tstring)
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


