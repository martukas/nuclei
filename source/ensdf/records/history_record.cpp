#include "history_record.h"
#include "Fields.h"
#include <boost/algorithm/string.hpp>

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
      kvps[boost::trim_copy(tokens2[0])] =
          boost::trim_copy(tokens2[1]);
  }
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


