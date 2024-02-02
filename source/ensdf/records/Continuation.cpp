#include "Continuation.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <util/logger.h>
#include <ensdf/Fields.h>
#include <qpx_util.h>

#define RGX_KEYNUM "\\d{4}\\w{2}[\\w\\d]{2}"
#define RGX_REF "\\((" RGX_KEYNUM "(?:," RGX_KEYNUM ")*)\\)"
#define RGX_CONT "^(.*?)(?:" RGX_REF  ")?$"

Continuation::Continuation(std::string s)
{
  boost::trim(s);
  boost::smatch what;
  if (boost::regex_match(s, what, boost::regex(RGX_CONT)))
  {
    std::string data;
    if (what.size() > 1)
      data = what[1];
    if (what.size() > 2)
      refs = what[2];
    boost::trim(data);
    boost::trim(refs);

    std::size_t found = std::string::npos;
    std::size_t found_space = data.find(" ");
    std::size_t found_equals = data.find("=");
    std::size_t found_lt = data.find("<");
    std::size_t found_gt = data.find(">");
    if (found_space!=std::string::npos)
      found = found_space;
    if ((found_equals!=std::string::npos) &&
        ((found_equals < found) || (found_equals == std::string::npos)))
      found = found_equals;
    if ((found_lt!=std::string::npos) &&
        ((found_equals < found) || (found_equals == std::string::npos)))
      found = found_lt;
    if ((found_gt!=std::string::npos) &&
        ((found_equals < found) || (found_equals == std::string::npos)))
      found = found_gt;

    std::string qs, vs;
    if (found != std::string::npos)
    {
      qs = boost::trim_copy(data.substr(0, found));
      vs = boost::trim_copy(data.substr(found, data.size() - found));
    }
    else
      DBG("{} FAILED QV", s);

    boost::split(quants, qs, boost::is_any_of(":"));
    if (((qs == "XREF") || (qs == "FLAG"))
        && (vs.size() > 1))
      symbols = vs.substr(1, vs.size()-1);
    else if (boost::contains(qs, "SPIN") && boost::contains(vs, "/"))
      spin = parse_spin(vs.substr(1, vs.size()-1));
    else
    {
      std::list<std::string> vals;
      boost::replace_all(vs, "= ", "=");
      boost::replace_all(vs, "< ", "<");
      boost::replace_all(vs, "> ", ">");
      boost::split(vals, vs, boost::is_any_of(" "));
//      for (auto vvv : vals)
//        DBG << "    " << vvv;

      while (vals.size())
      {
        std::string val = vals.front();
        vals.pop_front();
        std::string uncert;

        if (is_uncertainty_id(val) && vals.size())
        {
          uncert = val;
          val = vals.front();
          vals.pop_front();
        }
        else if ((val.size() > 1) && (val.substr(0,1) == "="))
          val = val.substr(1, val.size()-1);
        else if ((val.size() > 2) && (val.substr(0,2) == "<="))
        {
          val = val.substr(2, val.size()-2);
          uncert = "LE ";
        }
        else if ((val.size() > 2) && (val.substr(0,2) == ">="))
        {
          val = val.substr(2, val.size()-2);
          uncert = "GE ";
        }
        else if ((val.size() > 1) && (val.substr(0,1) == "<"))
        {
          val = val.substr(1, val.size()-1);
          uncert = "LT ";
        }
        else if ((val.size() > 1) && (val.substr(0,1) == ">"))
        {
          val = val.substr(1, val.size()-1);
          uncert = "GT ";
        }

        if (vals.size() && (boost::contains(vals.front(), "EV")))
        {
          units = vals.front();
          vals.pop_front();
        }

        if (vals.size() && uncert.empty() && is_number(vals.front()))
        {
          uncert = vals.front();
          vals.pop_front();
        }

//        DBG << "Extracted from " << vs << " val=" << val
//            << " uncert=" << uncert;

        Uncert u = parse_val_uncert(val, uncert);
        values.push_back(u);
      }

    }

  }
  else
    DBG("{} FAILED REF", s);
}

std::string Continuation::key() const
{
  return boost::join(quants, ":");
}

std::string Continuation::value() const
{
  std::vector<std::string> vals;
  for (Uncert v : values)
    vals.push_back(v.to_string(false));
  std::string ret = boost::join(vals, ":");

  if (units.size())
    ret += " " + units;
  if (symbols.size())
    ret += "=" + symbols;
  if (symbols.empty() && values.empty())
    ret += "=" + spin.to_qualified_string();

  return ret;
}

std::string Continuation::value_refs() const
{
  auto ret = value();
  if (refs.size())
    ret += " (" + refs + ")";
  return ret;
}

std::string Continuation::debug() const
{
  return key() + " " + value_refs();
}


bool Continuation::is_symbolic() const
{
  return symbols.size();
}

bool Continuation::is_uncert() const
{
  return values.size();
}

bool Continuation::is_spin() const
{
  return spin.numerator();
}

bool Continuation::val_defined() const
{
  int i = 0;
  if (is_symbolic())
    i++;
  if (is_spin())
    i++;
  if (is_uncert())
    i++;
  return (i == 1);
}

bool Continuation::valid() const
{
  return quants.size() && val_defined();
}


std::map<std::string, Continuation>
parse_continuation(const std::string& crecs)
{
//  DBG << "CONT: " << crecs;
  std::map<std::string, Continuation> ret;
  std::vector<std::string> crecs2;
  boost::split(crecs2, crecs, boost::is_any_of("$"));
  for (size_t i=0; i<crecs2.size(); i++)
  {
    boost::trim(crecs2[i]);
    if (crecs2[i].empty())
      continue;

    Continuation ccc(crecs2[i]);

//    if ((ccc.quants.size() > 1) || !ccc.valid())
//      DBG << crecs2[i] << "  --->  " << ccc.debug();

    if (ccc.key().size())
      ret[ccc.key()] = ccc;
  }
//  for (auto r :ret)
//    DBG << "  crec: " << r.first << " = " << r.second;
  return ret;
}

void merge_continuations(std::map<std::string, Continuation> &to,
                         const std::map<std::string, Continuation>& from,
                         [[maybe_unused]] const std::string &debug_line)
{
  for (const auto& cont : from)
  {
    if (cont.first == "XREF")
      continue;
    if (!to.count(cont.first))
      to[cont.first] = cont.second;
    else
    {
      //TRC("{} adopted continuation mismatch {} {}!={}", debug_line, cont.first, to[cont.first], cont.second);
    }
  }
}
