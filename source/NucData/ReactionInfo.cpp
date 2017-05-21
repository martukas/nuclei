#include "ReactionInfo.h"
#include "custom_logger.h"
#include "qpx_util.h"
#include <boost/regex.hpp>

#include "Fields.h"

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
