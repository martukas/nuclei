#include "ensdf_reaction_data.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

ReactionData ReactionData::from_id(const IdRecord &record,
                                   BlockIndices block)
{
  ReactionData ret;
  ret.dsid = record.dsid;
  ret.block = block;
  ret.daughter = record.nuclide;

  std::string nuclide = "[\\w\\d]+";
  std::string reactant = "[\\s\\w\\d'\\+-]+";
  std::string in_out = "\\(" + reactant + "," + reactant + "\\)";
  std::string target_reactions = nuclide + "\\s*" + in_out + "(?:\\s*,\\s*" + in_out + ")*";
  std::string splitxtras = "^(" + target_reactions + "(?:\\s*,\\s*" + target_reactions + "\\s*)*)(.*)$";

  std::string xtions, extras;
//  std::string xtras;

//  DBG << "Will parse " << record.debug();

  boost::regex xtions_xtras(splitxtras);
  boost::smatch what;
  if (boost::regex_match(boost::trim_copy(record.extended_dsid), what, xtions_xtras) && (what.size() > 1))
  {
//    DBG << " good";
    xtions = what[1];
    if (what.size() > 2)
      extras = what[2];
  }

//  DBG << " Xions " << xtions;

  std::string split_xxtions = "^\\s*(" + target_reactions + ")(?:\\s*,\\s(" + target_reactions + "))*.*$";
  boost::regex xtions_exp(split_xxtions);
  boost::smatch what2;
  boost::regex_match(xtions, what2, xtions_exp);
  for (size_t i=1; i < what2.size(); ++i)
  {
    std::string parse_xtions = "^(" + nuclide + ")\\s*(" + in_out + ")(?:\\s*,\\s*(" + in_out + "))*$";
    boost::regex xtion_exp(parse_xtions);
    boost::smatch what3;

//    DBG << "  xoion \"" << what2[i] << "\"";
    std::string layout = what2[i];
    std::string layout2;
    for (size_t z =0; z < layout.size(); ++z)
      layout2 += "\"" + std::string(1,layout.at(z)) + "\" ";

//    DBG << "        " << layout2;

    if (boost::regex_match(std::string(what2[i]), what3, xtion_exp) && (what3.size() > 2))
    {
//      for (auto xx =0; xx < what3.size(); ++xx)
//        DBG << "    " << xx << "=" << what3[xx];

      Reaction reaction;
//      DBG << "   nucid=" << what3[1];
      reaction.target = parse_nid(what3[1]);
      if (!reaction.target.composition_known() && reaction.target.Z())
        reaction.target.set_A(ret.daughter.A());

      for (size_t j=2; j < what3.size(); ++j)
      {
//        DBG << "   ants " << what3[j];

        std::string variant = "^\\((" + reactant + "),(" + reactant + ")\\)$";
        boost::regex variant_exp(variant);
        boost::smatch what4;
        if (boost::regex_match(std::string(what3[j]), what4, variant_exp) && (what4.size() > 2))
        {
          Reactants reactants;
          reactants.in = what4[1];
          reactants.out = what4[2];
          reaction.variants.push_back(reactants);
        }
      }
      ret.reactions.push_back(reaction);
    }
  }

  if (ret.find_remove(extras, "E=", "E=") ||
      ret.find_remove(extras, "E =", "E =") ||
      ret.find_remove(extras, "E<", "E") ||
      ret.find_remove(extras, "E>", "E") ||
      ret.find_remove(extras, "E AP", "E "))
  {
    boost::trim(ret.energy);
    std::size_t found = ret.energy.find("E=");
    if (found!=std::string::npos)
      ret.energy.erase(found, 2);
    else
      ret.energy.erase(0,1);
    boost::trim(ret.energy);
  }

  boost::trim(extras);
  std::size_t found = ret.energy.find(":");
  if (found!=std::string::npos)
    ret.energy.erase(found, 1);
  boost::trim(extras);

  ret.qualifier = extras;

  return ret;
}

bool ReactionData::find_remove(std::string& extras, std::string wanted, std::string trim_what)
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


std::string ReactionData::Reactants::to_string() const
{
  return "(" + in + "," + out + ")";
}

std::string ReactionData::Reaction::to_string() const
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

std::string ReactionData::to_string() const
{
  std::string ret;
  ret = daughter.verboseName();
  ret += "   dsid=\"" + dsid + "\"  ";
  bool first = true;
  for (auto r : reactions)
  {
    if (!first)
      ret += ",";
    if (first)
      first = false;
    ret += r.to_string();
  }
  ret += "  q=\"" + qualifier + "\"  ";
//  if (!energy.empty())
  ret += energy;
  return ret;
}

bool ReactionData::match(std::string record)
{
  std::string nuclide = "[\\w\\d]+";
  std::string reactant = "[\\s\\w\\d'\\+-]+";
  std::string in_out = "\\(" + reactant + "," + reactant + "\\)";
  std::string target_reactions = nuclide + "\\s*" + in_out + "(?:\\s*,\\s*" + in_out + ")*";
  std::string splitxtras = "^(" + target_reactions + "(?:\\s*,\\s*" + target_reactions + "\\s*)*)(.*)$";

  boost::smatch what;
  boost::regex filter(splitxtras);
  return boost::regex_match(boost::trim_copy(record), what, filter);
}
