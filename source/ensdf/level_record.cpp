#include "level_record.h"
#include "ensdf_types.h"
#include "qpx_util.h"
#include <boost/algorithm/string/trim_all.hpp>
#include "custom_logger.h"

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
  auto val = line.substr(9,10);
  auto uncert = line.substr(19,2);
  bool hasoffset = false;
  for (size_t i=0; i < val.size(); ++i)
  {
    if (std::isupper(val[i]) && hasoffset)
      offset += val.substr(i,1);
    else if (val[i] == '+')
      hasoffset = true;
  }
  if (hasoffset && offset.size())
  {
    offset = "+" + offset;
    boost::replace_all(val, "+X", "  ");
    boost::replace_all(val, "+Y", "  ");
  }
  else if (boost::trim_copy(val) == "X")
    offset = "X";
  else if (boost::trim_copy(val) == "Y")
    offset = "Y";

  energy = Energy(parse_val_uncert(val, uncert));
  spin_parity = parse_spin_parity(line.substr(21, 18));
  halflife = parse_halflife(line.substr(39, 16));
  L = boost::trim_copy(line.substr(55,9));

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
          GammaRecord::match(data[idx+1])))
  {
    ++idx;
    if (CommentsRecord::match(data[idx]))
      comments.push_back(CommentsRecord(idx, data));
    if (GammaRecord::match(data[idx]))
      gammas.push_back(GammaRecord(idx, data));
    else
      continuation += "$" + boost::trim_copy(data[idx].substr(9,71));
  }
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
  return ret;
}

bool LevelRecord::valid() const
{
  return nuclide.valid() &&
      (energy.value().defined() || !offset.empty());
}

