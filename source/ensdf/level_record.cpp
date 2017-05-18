#include "level_record.h"
#include "ensdf_types.h"
#include "qpx_util.h"
#include <boost/algorithm/string/trim_all.hpp>
#include "custom_logger.h"

#include <boost/regex.hpp>

#define RE_NUMBER "([\\+-]?[0-9]+\\.?[0-9]*E?[0-9]*)"
#define RE_OFFSET "([A-Z]{1,2})"

bool LevelRecord::match(const std::string& line)
{
  return match_first(line, "\\sL");
}

LevelRecord::LevelRecord(size_t& idx,
                         const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !match(data[idx]))
    return;
  const auto& line = data[idx];

  nuclide = parse_nid(line.substr(0,5));

  parse_energy_offset(line.substr(9,10), line.substr(19,2));

  spin_parity = parse_spin_parity(line.substr(21, 18));
  halflife = parse_halflife(line.substr(39, 16));
  L = boost::trim_copy(line.substr(55,9));

//  if (hasoffset)
//  {
//    DBG << nuclide.symbolicName()
//        << ":" << idx << " has offset "
//        << offset << " + " << energy.to_string()
//        << " from " << line;
//  }

  try
  {
    S = parse_norm_value(line.substr(64,10), line.substr(74,2));
  }
  catch (...)
  {
    auto sstr = line.substr(64, 12);
    //      DBG << "<LevelRecord::parse> failed to parse Svalue=\'" << sstr << "\'";
  }

  comment_flag = boost::trim_copy(line.substr(76,1));
  if (line[77] == 'M')
  {
    if (is_number(line.substr(78,1)))
      isomeric = boost::lexical_cast<uint16_t>(line.substr(78,1));
    else
      isomeric = 1;
  }

  quality = boost::trim_copy(line.substr(79,1));

  while ((idx+1 < data.size()) &&
         (match_cont(data[idx+1], "\\sL") ||
          CommentsRecord::match(data[idx+1], "L") ||
          GammaRecord::match(data[idx+1]) ||
          ECRecord::match(data[idx+1]) ||
          ParticleRecord::match(data[idx+1]) ||
          BetaRecord::match(data[idx+1])))
  {
    ++idx;
    if (CommentsRecord::match(data[idx], "L"))
      comments.push_back(CommentsRecord(idx, data));
    if (GammaRecord::match(data[idx]))
      gammas.push_back(GammaRecord(idx, data));
    if (ECRecord::match(data[idx]))
      ECs.push_back(ECRecord(idx, data));
    if (ParticleRecord::match(data[idx]))
      particles.push_back(ParticleRecord(idx, data));
    if (BetaRecord::match(data[idx]))
      betas.push_back(BetaRecord(idx, data));
    else
      continuation += "$" + boost::trim_copy(data[idx].substr(9,71));
  }
}

void LevelRecord::parse_energy_offset(std::string val,
                                      std::string uncert)
{
  boost::replace_all(val, " ", "");

  boost::smatch what1, what2, what3;
  if (boost::regex_match(val, boost::regex("^" RE_NUMBER "$")))
  {

  }
  if ((boost::regex_search(val, what1,
                           boost::regex("^" RE_OFFSET "\\+?$")))
    && (what1.size() == 2))
  {
    offset = what1[1];
    val = "0";
  }
  else if ((boost::regex_search(val, what2,
                                boost::regex("^" RE_OFFSET "\\+" RE_NUMBER "$")))
    && (what2.size() == 3))
  {
    offset = what2[1];
    val = what2[2];
  }
  else if ((boost::regex_search(val, what3,
                                boost::regex("^" RE_NUMBER "\\+" RE_OFFSET "$")))
    && (what3.size() == 3))
  {
    val = what3[1];
    offset = what3[2];
  }

  energy = Energy(parse_val_uncert(val, uncert));
}


std::string LevelRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + " LEVEL ";
  if (energy.value().defined())
    ret += " Energy=" + energy.to_string();
  if (!offset.empty())
    ret += "+" + offset + "(offset)";
  if (isomeric)
    ret += " M" + std::to_string(isomeric);
  if (spin_parity.valid())
    ret += " SpinParity=" + spin_parity.to_string();
  if (halflife.isValid())
    ret += " Halflife=" + halflife.to_string(true);
  if (S.hasFiniteValue())
    ret += " S=" + S.to_string(true);
  if (!comment_flag.empty())
    ret += " comment=" + comment_flag;
  if (!quality.empty())
    ret += " quality=" + quality;
  if (!L.empty())
    ret += " L=" + L;
  if (!continuation.empty())
    ret += "\n      Continuation:" + continuation;
  for (auto c : comments)
    ret += "\n      Comment: " + c.debug();
  for (auto c : gammas)
    ret += "\n      Gamma: " + c.debug();
  for (auto c : ECs)
    ret += "\n      EC: " + c.debug();
  for (auto c : particles)
    ret += "\n      Particle: " + c.debug();
  for (auto c : betas)
    ret += "\n      Beta: " + c.debug();
  return ret;
}

bool LevelRecord::valid() const
{
  return nuclide.valid() &&
      (energy.value().defined() || !offset.empty());
}

