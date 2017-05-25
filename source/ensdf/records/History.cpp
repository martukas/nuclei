#include "History.h"
#include "Fields.h"
#include <boost/algorithm/string.hpp>

#include "Translator.h"

bool HistoryRecord::match(const std::string& line)
{
  return match_first(line, "\\sH");
}

HistoryRecord::HistoryRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_check_nid(line.substr(0, 5));

  std::string hdata = line.substr(9, 71);
  while (i.has_more() && match_cont(i.look_ahead(), "\\sH"))
    hdata += i.read_pop().substr(9,71);

  std::vector<std::string> tokens;
  boost::split(tokens, hdata, boost::is_any_of("$"));
  for (auto t : tokens)
  {
    boost::trim(t);
    std::vector<std::string> tokens2;
    boost::split(tokens2, t, boost::is_any_of("="));
    if (tokens2.size() > 1)
      add_kvp(boost::trim_copy(tokens2[0]),
          boost::trim_copy(tokens2[1]));
  }
}

void HistoryRecord::add_kvp(const std::string& key,
                            const std::string& value)
{
  if (key.empty() || value.empty())
    return;

  std::string val = value;
  if (key == "TYP")
    val = Translator::instance().hist_eval_type(value);
  else if (key == "AUT")
    val = Translator::instance().auth_capitalize(value);
  else if ((key == "CIT") && (value == "ENSDF"))
    val = "included in ENSDF but not published";

  kvps[Translator::instance().hist_key(key)] = val;
}


std::string HistoryRecord::debug() const
{
  auto ret = nuclide.symbolicName() + " HIST  ";
  for (auto kvp : kvps)
    ret += "\n      " + kvp.first + "=" + kvp.second;
  return ret;
}

bool HistoryRecord::valid() const
{
  return nuclide.valid() && !kvps.empty();
}


