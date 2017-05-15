#include "history_record.h"
#include "ensdf_types.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

bool HistoryRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9A-Z]{5}[\\s02-9A-Za-z]\\sH.*$");
}

HistoryRecord HistoryRecord::parse(size_t& idx,
                                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return HistoryRecord();
  auto line = data[idx];

  HistoryRecord ret;
  ret.nuc_id = parse_check_nid(line.substr(0, 5));

  boost::regex filter("^[\\s0-9A-Z]{5}[02-9A-Za-z]\\sH.*$");
  std::string hdata = line.substr(9, 71);
  while ((idx+1 < data.size()) &&
         boost::regex_match(data[idx+1], filter))
    hdata += data[++idx].substr(9,71);

  std::vector<std::string> tokens;
  boost::split(tokens, hdata, boost::is_any_of("$"));
  for (auto t : tokens)
  {
    boost::trim(t);
    std::vector<std::string> tokens2;
    boost::split(tokens2, t, boost::is_any_of("="));
    if (tokens2.size() > 1)
      ret.kvps[boost::trim_copy(tokens2[0])] =
          boost::trim_copy(tokens2[1]);
  }
  return ret;
}

std::string HistoryRecord::debug() const
{
  auto ret = nuc_id.symbolicName() + " history ";
  for (auto kvp : kvps)
    ret += "\n  " + kvp.first + "=" + kvp.second;
  return ret;
}

