#include "ensdf_records.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

#define RGX_NUCLIDE_ID "[\\s0-9A-Za-z]{5}"
#define RGX_CONTINUATION_ID "[0-9A-Za-z!@#\\$%\\^&\\*-\\+\"]"

bool match_record_type(const std::string& line,
                       const std::string& pattern)
{
  return (line.size() == 80) && boost::regex_match(line, boost::regex(pattern));
}

bool match_first(const std::string& line,
                 const std::string& sub_pattern)
{
  return match_record_type(line,
                           "^" RGX_NUCLIDE_ID "\\s"
                           + sub_pattern + ".*$");
}

bool match_cont(const std::string& line,
                 const std::string& sub_pattern)
{
  return match_record_type(line,
                           "^" RGX_NUCLIDE_ID RGX_CONTINUATION_ID
                           + sub_pattern + ".*$");
}

/**
 * @brief DaughterParser::extractContinuationRecords
 * @param adoptedblock block to search continuation records in
 * @param requestedRecords list of requested records
 * @param typeOfContinuedRecord type of record (default: L(evel))
 * @return list of found records (same size as requestedRecords, empty strings if no record was found)
 */
//std::vector<std::string> extractContinuationRecords(const BlockIndices &adoptedblock,
//                                                    const std::list<std::string> &requestedRecords,
//                                                    const std::vector<std::string> &data,
//                                                    std::string typeOfContinuedRecord)
//{
//  // fetch records
//  boost::regex crecre("^[A-Z0-9\\s]{5,5}[A-RT-Z0-9] "
//                      + typeOfContinuedRecord + " (.*)$");
//  std::vector<std::string> crecs;
//  for (size_t i = adoptedblock.first; i < adoptedblock.last; ++i)
//  {
//    boost::smatch what;
//    if (boost::regex_search(data.at(i), what, crecre) &&
//        (what.size() > 1))
//      crecs.push_back(data.at(i));
//  }
//  std::vector<std::string> crecs2;
//  // remove record id from beginning of string
//  for (size_t i=0; i<crecs.size(); i++)
//  {
//    crecs[i].erase(0, 9);
//    crecs2.push_back(crecs.at(i));
//  }
//  // join lines and then split records
//  std::string tmp = join(crecs2, "$");
//  boost::split(crecs2, tmp, boost::is_any_of("$"));
//  for (size_t i=0; i<crecs2.size(); i++)
//    crecs2[i] = boost::trim_copy(crecs2[i]);
//  // search and parse requested fields
//  std::vector<std::string> result;
//  for ( auto &req : requestedRecords)
//  {
//    std::string rstr;
//    for (size_t i=0; i<crecs2.size(); i++)
//    {
//      if ((crecs2.at(i).size() >= req.size()) &&
//          (crecs2.at(i).substr(0, req.size()) == req))
//      {
//        rstr = boost::trim_copy(crecs2.at(i).substr(5, crecs2.at(i).size() - 5));
//        break;
//      }
//    }
//    if (!rstr.empty() && (rstr[0] == '='))
//      rstr = rstr.substr(1, rstr.size()-1);
//    result.push_back(rstr);
//  }
//  return result;
//}

std::map<std::string, std::string>
parse_continuation(const std::string& crecs)
{
//  DBG << "CONT: " << crecs;
  std::map<std::string, std::string> ret;
  std::vector<std::string> crecs2;
  boost::split(crecs2, crecs, boost::is_any_of("$"));
  for (size_t i=0; i<crecs2.size(); i++)
  {
    boost::trim(crecs2[i]);
    if (crecs2[i].empty())
      continue;
    std::vector<std::string> kvp;
    boost::split(kvp, crecs2[i], boost::is_any_of("="));
    if (kvp.size() == 2)
      ret[boost::trim_copy(kvp[0])] = boost::trim_copy(kvp[1]);
//    else
//      DBG << "  bad crec: " << crecs2[i];
  }
//  for (auto r :ret)
//    DBG << "  crec: " << r.first << " = " << r.second;
  return ret;
}


