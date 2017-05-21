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

bool xref_check(const std::string& xref,
                const std::string& dssym)
{
  if (xref == "+")
    return true;
  else if (boost::regex_match(xref, boost::regex("^[A-Za-z]+$")))
  {
    if (boost::contains(xref, dssym))
      return true;
  }
  else if (boost::regex_match(xref, boost::regex("^-\\([A-Za-z]+\\)$")))
  {
    if (!boost::contains(xref, dssym))
      return true;
  }
  else
  {
//    DBG << "  xref unprocessed " << xref << "  cf  " << dssym;
//    boost::sregex_token_iterator iter(xref.begin(), xref.end(),
//                                      boost::regex("[A-Z]+\\([A-Z]+(?:,[A-Z]+)\\)"), 0);
//    for( ; iter != boost::sregex_token_iterator(); ++iter )
//    {
//      DBG << "      element = " << *iter;
//    }
  }

  return false;
}

void merge_continuations(std::map<std::string, std::string>& to,
                         const std::map<std::string, std::string>& from,
                         const std::string &debug_line)
{
  for (const auto& cont : from)
  {
    if (cont.first == "XREF")
      continue;
    if (to.count(cont.first) &&
        (to[cont.first] != cont.second))
    {
      DBG << debug_line
          << " adopted continuation mismatch "
          << cont.first
          << "  " << to[cont.first]
          << "!=" << cont.second;
    }
    to[cont.first] = cont.second;
  }
}


ENSDFData::ENSDFData(const std::vector<std::string>& l, BlockIndices ii)
  : i(ii)
  , lines(l)
{}

bool ENSDFData::has_more() const
{
  return ((i.first+1) < i.last);
}
const std::string& ENSDFData::look_ahead() const
{
  return lines[i.first + 1];
}

const std::string& ENSDFData::read_pop()
{
  return lines[++i.first];
}

const std::string& ENSDFData::read()
{
  return lines[i.first];
}

// prefix
ENSDFData& ENSDFData::operator++()
{
  i.first++;
  return *this;
}

// postfix
ENSDFData ENSDFData::operator++(int)
{
  ENSDFData tmp(*this);
  operator++();
  return tmp;
}
