#include "ensdf_records.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

bool match_record_type(const std::string& line,
                       const std::string& pattern)
{
  boost::regex filter(pattern);
  return (line.size() == 80) &&
      boost::regex_match(line, filter);
}

/**
 * @brief DaughterParser::extractContinuationRecords
 * @param adoptedblock block to search continuation records in
 * @param requestedRecords list of requested records
 * @param typeOfContinuedRecord type of record (default: L(evel))
 * @return list of found records (same size as requestedRecords, empty strings if no record was found)
 */
std::vector<std::string> extractContinuationRecords(const BlockIndices &adoptedblock,
                                                    const std::list<std::string> &requestedRecords,
                                                    const std::vector<std::string> &data,
                                                    std::string typeOfContinuedRecord)
{
  // fetch records
  boost::regex crecre("^[A-Z0-9\\s]{5,5}[A-RT-Z0-9] "
                      + typeOfContinuedRecord + " (.*)$");
  std::vector<std::string> crecs;
  for (size_t i = adoptedblock.first; i < adoptedblock.last; ++i)
  {
    boost::smatch what;
    if (boost::regex_search(data.at(i), what, crecre) &&
        (what.size() > 1))
      crecs.push_back(data.at(i));
  }
  std::vector<std::string> crecs2;
  // remove record id from beginning of string
  for (size_t i=0; i<crecs.size(); i++)
  {
    crecs[i].erase(0, 9);
    crecs2.push_back(crecs.at(i));
  }
  // join lines and then split records
  std::string tmp = join(crecs2, "$");
  boost::split(crecs2, tmp, boost::is_any_of("$"));
  for (size_t i=0; i<crecs2.size(); i++)
    crecs2[i] = boost::trim_copy(crecs2[i]);
  // search and parse requested fields
  std::vector<std::string> result;
  for ( auto &req : requestedRecords)
  {
    std::string rstr;
    for (size_t i=0; i<crecs2.size(); i++)
    {
      if ((crecs2.at(i).size() >= req.size()) &&
          (crecs2.at(i).substr(0, req.size()) == req))
      {
        rstr = boost::trim_copy(crecs2.at(i).substr(5, crecs2.at(i).size() - 5));
        break;
      }
    }
    if (!rstr.empty() && (rstr[0] == '='))
      rstr = rstr.substr(1, rstr.size()-1);
    result.push_back(rstr);
  }
  return result;
}

bool is_gamma_line(const std::string& line,
                   std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  G "))) ||
      (!nucid2.empty() && is_gamma_line(line, nucid2));
}

bool is_level_line(const std::string& line,
                   std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  L ")))||
      (!nucid2.empty() && is_level_line(line, nucid2));
}

bool is_intensity_line(const std::string& line,
                       std::string nucid,
                       std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  E ")))||
      (!nucid2.empty() && is_intensity_line(line, nucid2));
}

bool is_norm_line(const std::string& line,
                  std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  N ")))||
      (!nucid2.empty() && is_norm_line(line, nucid2));
}

bool is_p_norm_line(const std::string& line,
                    std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + " PN ")))||
      (!nucid2.empty() && is_p_norm_line(line, nucid2));
}

bool is_feed_a_line(const std::string& line,
                    std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  A ")))||
      (!nucid2.empty() && is_feed_a_line(line, nucid2));
}

bool is_feed_b_line(const std::string& line,
                    std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  B ")))||
      (!nucid2.empty() && is_feed_b_line(line, nucid2));
}

bool is_feed_line(const std::string& line,
                  std::string nucid, std::string nucid2)
{
  return is_feed_a_line(line, nucid, nucid2) ||
      is_feed_b_line(line, nucid, nucid2);
}

std::map<Energy, std::string> get_gamma_lines(const std::vector<std::string>& data,
                                              BlockIndices bounds,
                                              NuclideId nucid)
{
  std::map<Energy, std::string> e2g;
  std::string dNucid1 = nid_to_ensdf(nucid, true);
  std::string dNucid2 = nid_to_ensdf(nucid, false);
  for (size_t i = bounds.first; i < bounds.last; ++i)
    if (is_gamma_line(data[i], dNucid1, dNucid2))
    {
      Energy gk = parse_energy(data[i].substr(9, 10), data[i].substr(19, 2));
      if (gk.valid())
        e2g[gk] = data[i];
    }
  return e2g;
}
