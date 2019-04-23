#include <ensdf/Fields.h>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <util/lexical_extensions.h>
#include <util/UTF_extensions.h>
//#include "qpx_util.h"
#include <util/logger.h>

#define RGX_SPIN "\\d+(?:/\\d+)?"
#define RGX_QSPIN "[\\[\\(~]?" RGX_SPIN "[\\]\\)]?"
#define RGX_PARITY "[\\-\\+\\?]{1}"
#define RGX_QPARITY "[\\[\\(~]?" RGX_PARITY "[\\]\\)]?"
#define RGX_SP "^(" RGX_QSPIN ")?(" RGX_QPARITY  ")?$"

#define NUCLIDE "[0-9]+[A-Z]*"
#define MODE "\\([A-Z0-9]+,[A-Z0-9]+\\)"
#define HL ":[\\s0-9A-Z]+"
#define HLU "\\[\\+[0-9]+\\]"
#define PARENT "^(" NUCLIDE ")(" MODE ")?(" HL ")?(" HLU ")?$"



using dlim = std::numeric_limits<double>;
//using d_inf = std::numeric_limits<double>::infinity();
//using d_NaN = std::numeric_limits<double>::quiet_NaN();

bool is_uncertainty_id(const std::string& str)
{
  return (str == "LT" ||
          str == "GT" ||
          str == "LE" ||
          str == "GE" ||
          str == "AP" ||
          str == "CA" ||
          str == "SY");
}

bool has_uncertainty_id(const std::string& str)
{
  return boost::contains(str, "LT") ||
      boost::contains(str, "GT") ||
      boost::contains(str, "LE") ||
      boost::contains(str, "GE") ||
      boost::contains(str, "AP") ||
      boost::contains(str, "CA") ||
      boost::contains(str, "SY");
}

std::string uncert_to_ensdf(Uncert::UncertaintyType t)
{
  if (t == Uncert::LessThan)
    return "LT";
  else if (t == Uncert::GreaterThan)
    return "GT";
  else if (t == Uncert::LessEqual)
    return "LE";
  else if (t == Uncert::GreaterEqual)
    return "GE";
  else if (t == Uncert::Approximately)
    return "AP";
  else if (t == Uncert::Calculated)
    return "CA";
  else if (t == Uncert::Systematics)
    return "SY";
  return "";
}

std::string strip_uncert_type(const std::string& str)
{
  auto ret = str;
  boost::replace_all(ret, "LT", "");
  boost::replace_all(ret, "GT", "");
  boost::replace_all(ret, "LE", "");
  boost::replace_all(ret, "GE", "");
  boost::replace_all(ret, "AP", "");
  boost::replace_all(ret, "CA", "");
  boost::replace_all(ret, "SY", "");
  return boost::trim_copy(ret);
}


Uncert::UncertaintyType parse_uncert_type(const std::string& uncert)
{
  if (boost::contains(uncert, "LT"))
    return Uncert::LessThan;
  else if (boost::contains(uncert, "GT"))
    return Uncert::GreaterThan;
  else if (boost::contains(uncert, "LE"))
    return Uncert::LessEqual;
  else if (boost::contains(uncert, "GE"))
    return Uncert::GreaterEqual;
  else if (boost::contains(uncert, "AP")
           || uncert.empty()
           || !is_number(uncert)
           /*|| flag_tentative*/)
    return Uncert::Approximately;
  else if (boost::contains(uncert, "CA")
           /*|| flag_theoretical*/)
    return Uncert::Calculated;
  else if (boost::contains(uncert, "SY"))
    return Uncert::Systematics;
  return Uncert::UndefinedType;

//  SymmetricUncertainty  = 0x1,
//  AsymmetricUncertainty = 0x2,
}


Uncert parse_norm(std::string val, std::string uncert)
{
  auto ret = parse_val_uncert(val, uncert);
  if (ret.sign() & Uncert::MagnitudeDefined)
    ret.setSign(Uncert::SignMagnitudeDefined);
  return ret;
}

Energy parse_energy(std::string val, std::string uncert)
{
  return Energy(parse_val_uncert(val, uncert));
}

Uncert parse_val_uncert(std::string val, std::string uncert)
{
  boost::trim(val);
  boost::trim(uncert);

  bool flag_tentative = false;
  bool flag_theoretical = false;
  if (boost::contains(val, "(") || boost::contains(val, ")")) //what if sign only?
  {
    boost::replace_all(val, "(", "");
    boost::replace_all(val, ")", "");
    flag_tentative = true;
  }
  else if (boost::contains(val, "[") || boost::contains(val, "]")) //what if sign only?
  {
    boost::replace_all(val, "[", "");
    boost::replace_all(val, "]", "");
    flag_theoretical = true;
  }

  if (val.empty() || !is_number(val))
    return Uncert();

  Uncert result(std::stod(val), sig_digits(val), Uncert::UndefinedSign);
  double val_order = get_precision(val);

  if (boost::contains(val, "+") || boost::contains(val, "-"))
    result.setSign(Uncert::SignMagnitudeDefined);
  else
    result.setSign(Uncert::MagnitudeDefined);

  // parse uncertainty
  // symmetric or special case (consider symmetric if not + and - are both contained in string)
  if ( !( boost::contains(uncert,"+") && boost::contains(uncert, "-"))
       || flag_tentative )
  {
    if (uncert == "LT")
      result.setUncertainty(-dlim::infinity(), 0.0, Uncert::LessThan);
    else if (uncert == "GT")
      result.setUncertainty(0.0, dlim::infinity(), Uncert::GreaterThan);
    else if (uncert == "LE")
      result.setUncertainty(-dlim::infinity(), 0.0, Uncert::LessEqual);
    else if (uncert == "GE")
      result.setUncertainty(0.0, dlim::infinity(), Uncert::GreaterEqual);
    else if (uncert == "AP" || uncert.empty() || !is_number(uncert) || flag_tentative)
      result.setUncertainty(dlim::quiet_NaN(), dlim::quiet_NaN(), Uncert::Approximately);
    else if (uncert == "CA" || flag_theoretical)
      result.setUncertainty(0.0, 0.0, Uncert::Calculated);
    else if (uncert == "SY")
      result.setUncertainty(0.0, 0.0, Uncert::Systematics);
    else {
      // determine significant figure
      if (!uncert.empty() && is_number(uncert))
        result.setSymmetricUncertainty(val_order * std::stoi(uncert));
      else
        result.setUncertainty(dlim::quiet_NaN(),
                              dlim::quiet_NaN(),
                              Uncert::UndefinedType);
    }
  }
  // asymmetric case
  else
  {
    std::string uposstr, unegstr;
    boost::algorithm::replace_all(uncert, " ", "");
    boost::regex expr{"^\\+([^\\-]+)\\-(.*)$"};
    boost::regex inv_expr{"^\\-([^\\+]+)\\+(.*)$"};
    boost::smatch what;
    if (boost::regex_match(uncert, what, expr) &&
        (what.size() == 3))
    {
      uposstr = what[1];
      unegstr = what[2];
    }
    else if (boost::regex_match(uncert, what, inv_expr) &&
             (what.size() == 3))
    {
      unegstr = what[1];
      uposstr = what[2];
    }

    double upositive {0.0};
    double unegative {0.0};

    boost::trim(uposstr);
    boost::trim(unegstr);

    if (!uposstr.empty() && is_number(uposstr))
      upositive = std::stoi(uposstr);
    else if (uposstr == "|@")
      upositive = dlim::infinity();

    if (!unegstr.empty() && is_number(unegstr))
      unegative = std::stoi(unegstr);
    else if (unegstr == "|@")
      unegative = -1 * dlim::infinity();

    // work aournd bad entries with asymmetric uncertainty values of 0.
    if (upositive == 0.0 && unegative == 0.0)
    {
      result.setUncertainty(dlim::quiet_NaN(),
                            dlim::quiet_NaN(),
                            Uncert::Approximately);
      WARN("Found asymmetric error of -0+0 in '{}' parsed as -{} +{}."
           "Auto-changing to 'approximately'", uncert, unegstr, uposstr);
    }
    else
      result.setAsymmetricUncertainty(val_order * unegative,
                                      val_order * upositive);
  }

//  if (result.type_ == AsymmetricUncertainty)
//    DBG << std::setw(8) << val << std::setw(7) << uncert
//        << " finite=" << result.hasFiniteValue()
//        << " has " << result.sigfigs() << " sigfigs " << " order " << val_order
//        << " parsed as " << result.value_ << "+" << result.upper_sigma_ << "-" << result.lower_sigma_
//        << " renders " << result.to_string(false)
//           ;

  return result;
}

size_t occurrences(const std::string&s, const std::string&sub)
{
  size_t count = 0;
  std::string::size_type pos = 0;
  while ((pos = s.find(sub, pos )) != std::string::npos)
  {
    ++count;
    pos += sub.length();
  }
  return count;
}

bool common_brackets(const std::string&s, char open, char close)
{
  if ((s.size() < 2) || (s[0] != open) || (s[s.size()-1] != close))
    return false;
  int o {0};
  for (size_t i = 0; i < (s.size()-1); ++i)
  {
    if (s[i] == open)
      o++;
    else if (s[i] == close)
      o--;
    if (o < 1)
      return false;
  }
  return true;
}


DataQuality quality_of(const std::string& s)
{
  if (s.empty())
    return DataQuality::kUnknown;
  if (s[0] == '~')
    return DataQuality::kAbout;
  if ((s.size() > 1) && (s[0] == '(') && (s[s.size()-1] == ')'))
    return DataQuality::kTentative;
  if ((s.size() > 1) && (s[0] == '[') && (s[s.size()-1] == ']'))
    return DataQuality::kTheoretical;
  if (s[0] == '?')
    return DataQuality::kUnknown;
  return DataQuality::kKnown;
}

std::string strip_qualifiers(const std::string& s)
{
  if (s.size() < 1)
    return s;
  if (s[0] == '~')
    return s.substr(1, s.size()-1);
  if ((s.size() > 1) && (s[0] == '(') && (s[s.size()-1] == ')'))
    return s.substr(1, s.size()-1).substr(0, s.size()-2);
  if ((s.size() > 1) && (s[0] == '[') && (s[s.size()-1] == ']'))
    return s.substr(1, s.size()-1).substr(0, s.size()-2);
  if ((s.size() == 1) && (s[0] == '?'))
    return "";
  return s;
}

NuclideId parse_check_nid(std::string nucid)
{
  auto ret = parse_nid(nucid);
  if (!check_nid_parse(nucid, ret))
    ERR("Could not parse daughter NucID  \"{}\"", nucid);
  return ret;
}

NuclideId parse_nid(std::string id)
{
  boost::to_upper(id);
  boost::regex nid_expr("^(?:\\s)*([0-9]+)([A-Z]+)(?:\\s)*$");
  boost::smatch what;
  if (boost::regex_match(id, what, nid_expr) && (what.size() == 3))
  {
    std::string A = what[1];
    int16_t Z = NuclideId::zOfSymbol(what[2]);

//    DBG << "Parsed big nucID " << id << " -> "
//        << NuclideId::fromAZ(boost::lexical_cast<uint16_t>(A), Z).verboseName();

    if (!is_number(A))
    {
      DBG("<NuclideId> Bad A value from {}", id);
      return NuclideId();
    }
    return NuclideId::fromAZ(std::stoi(A), Z);
  }

  boost::trim(id);
  boost::regex dig_expr("^\\s*\\d+\\s*$");
  boost::regex w_expr("^\\s*\\d+\\s*$");
  if ((boost::regex_match(id, dig_expr)))
  {
    if (id.size() == 5)
    {
      std::string A = id.substr(0,3);
      std::string Z_str = id.substr(3,2);
      uint16_t Z = 0;
      if (!boost::trim_copy(Z_str).empty())
      {
        std::string zstring = "1" + id.substr(3,2);
        if (is_number(zstring))
          Z = std::stoi(zstring);
        else
          DBG("<NuclideId> Bad zstring from {}", id);
      }
      if (!is_number(A))
      {
        DBG("<NuclideId> Bad A value (2) from {}", id);
        return NuclideId();
      }
      return NuclideId::fromAZ(std::stoi(A), Z);
    }
    else
    {
      if (!is_number(id))
      {
        DBG("<NuclideId> Bad id value from {}", id);
        return NuclideId();
      }
      return NuclideId::fromAZ(std::stoi(id), 0, true);
    }
  }
  else if ((boost::regex_match(id, w_expr)))
  {
    int16_t Z = NuclideId::zOfSymbol(id);
    return NuclideId::fromAZ(Z, Z, true);
  }
  return NuclideId();

}

Spin parse_spin(const std::string& s)
{
  uint16_t numerator {0};
  uint16_t denominator {0};
  std::string st = strip_qualifiers(s);
  std::istringstream input; input.clear();
  if ( boost::contains(st, "/") )
  {
    // not an integer
    boost::replace_all(st, "/", " ");
    input.str(st);
    input >> numerator >> denominator;
  }
  else
  {
    input.str(st);
    input >> numerator;
    denominator = 1;
  }
  if ( input.fail() )
    return Spin(numerator, denominator, DataQuality::kUnknown);
  else
    return Spin(numerator, denominator, quality_of(s));
}

Parity parse_parity(const std::string& s)
{
  auto quality = quality_of(s);
  if ( boost::contains(s, "-") )
    return Parity(Parity::EnumParity::kMinus, quality);
  else if ( boost::contains(s, "+") )
    return Parity(Parity::EnumParity::kPlus, quality);
  else
    return Parity();
}

bool common_qualifiers(const std::string& s)
{
  if (common_brackets(s, '(', ')'))
    return true;
  if (common_brackets(s, '[', ']'))
    return true;
  if ((s.size() > 1)
      && (s[0] == '~')
      && !occurrences(s, "(")
      && !occurrences(s, ")")
      && !occurrences(s, "[")
      && !occurrences(s, "]"))
    return true;
  return false;
}

Parity get_common_parity(std::string&s)
{
  Parity ret;
  if ((s.size() > 5) &&
          ((s.substr(s.size()-3, 3) == "(+)") ||
           (s.substr(s.size()-3, 3) == "(-)")) &&
(common_brackets(s.substr(0, s.size()-3), '(', ')') ||
  common_brackets(s.substr(0, s.size()-3), '[', ']')))
  {
    ret = parse_parity(s.substr(s.size()-3, 3));
    s = s.substr(0, s.size()-3);
  }
  if ((s.size() > 3) &&
          ((s[s.size()-1] == '+') || (s[s.size()-1] == '-')) &&
(common_brackets(s.substr(0, s.size()-1), '(', ')') ||
  common_brackets(s.substr(0, s.size()-1), '[', ']')))
  {
    ret = parse_parity(s.substr(s.size()-1, 1));
    s = s.substr(0, s.size()-1);
  }

  boost::trim(s);
  return ret;
}

DataQuality scrape_quality(std::string& s)
{
  auto ret = quality_of(s);
  s = strip_qualifiers(s);
  return ret;
}

void simplify_logic(std::string& s)
{
  boost::replace_all(s, "OR", ",");
  boost::replace_all(s, "AND", "&");
  boost::replace_all(s, "TO", ":");
}

std::string split_spins_type(std::string& s)
{
  bool sor = boost::contains(s, ",");
  bool sand = boost::contains(s, "&");
  bool sto = boost::contains(s, ":");
  if (std::abs(int(sor) + int(sand) + int(sto)) > 1)
    DBG("Bad things {}", s);
  if (sor)
    return ",";
  if (sand)
    return "&";
  if (sto)
    return ":";
  return "";
}

std::pair<std::string, std::vector<std::string>>
spin_split(const std::string& data)
{
  std::pair<std::string, std::vector<std::string>> ret;
  int parens = 0;
  size_t start = 0;
  for (size_t i=0; i < data.size(); ++i)
  {
    if (data[i] == '(')
      parens++;
    else if (data[i] == ')')
      parens--;
    else if (!parens &&
             ((data[i] == ',') || (data[i] == ':') || (data[i] == '&')))
    {
      auto sep = std::string(1, data[i]);
      if (ret.first != sep)
        ret.first += sep;
      ret.second.push_back(data.substr(start, i-start));
      start = i+1;
    }
  }
  if (start < data.size())
    ret.second.push_back(data.substr(start, data.size()-start));
  return ret;
}


SpinSet parse_spins(std::string data)
{
  SpinSet ret;
  boost::to_upper(data);
  simplify_logic(data);
  boost::replace_all(data, " ", "");
//  boost::trim(data);

  auto data2 = data;

  Parity common_parity = get_common_parity(data2);
  if (common_parity.valid())
  {
    ret.merge(parse_spins(data2));
    ret.set_common_parity(common_parity);
  }
  else if (common_qualifiers(data2))
  {
    DataQuality comqual = quality_of(data2);
    ret.merge(parse_spins(strip_qualifiers(data2)));
    ret.set_common_quality(comqual);
  }
  else
  {
    auto split = spin_split(data2);
    if (split.first.empty())
    {
      SpinParity sp = parse_spin_parity(data);
      if (sp.valid())
        ret.add(sp);
    }
    else if (split.first.size() > 1)
    {
//      DBG << "  *  Something funky here " << data2;
    }
    else if (split.first.size() == 1)
    {
      for (auto &token : split.second)
        ret.merge(parse_spins(token));
      ret.set_logic(split.first);
    }
    else
      ret.merge(parse_spins(data2));
  }

//  auto result = boost::replace_all_copy(ret.to_string(), " ", "");

//  if (data.size() && (data != result))
//  {
//    DBG << "SpinParity \'" << data << "\' --> \'"
//        << ret.debug() << "\' --> \'"
//        << ret.to_string() << "\'";
//  }
  return ret;
}

SpinParity parse_spin_parity(std::string data)
{
  boost::trim(data);
  SpinParity ret;
  //what if tentative only parity or spin only?

//  DBG << " Data " << data;

  if (has_uncertainty_id(data))
  {
    ret.set_eq_type(parse_uncert_type(data));
    data = strip_uncert_type(data);
  }

  boost::smatch what;
  if (boost::regex_match(data, what, boost::regex(RGX_SP)))
  {
//    for (auto w : what)
//      DBG << "   " << w;
    if (what.size() > 1)
      ret.set_spin(parse_spin(what[1]));
    if (what.size() > 2)
      ret.set_parity(parse_parity(what[2]));
  }

  return ret;
}

HalfLife parse_halflife(std::string record_orig)
{
  record_orig = boost::regex_replace(record_orig, boost::regex("[\\s]{2,}"), " ");
  boost::trim(record_orig);

  auto record = record_orig;

  bool comqual = common_qualifiers(record);
  if (comqual)
    record = strip_qualifiers(record);

  std::string value_str;
  std::string units_str;
  std::string uncert_str = "0";

  std::vector<std::string> timeparts;
  boost::split(timeparts, record, boost::is_any_of(" \r\t\n\0"));
  if (timeparts.size() >= 1)
    value_str = timeparts[0];
  if (timeparts.size() >= 2)
    units_str = timeparts[1];
  if (timeparts.size() >= 3)
    uncert_str = timeparts[2];

//  DBG << "PARSED " << value_str
//      << " " << units_str
//      << " " << uncert_str;

  Uncert time = parse_val_uncert(value_str, uncert_str);
  if (boost::contains(boost::to_upper_copy(record), "STABLE"))
    time.setValue(dlim::infinity(),
                  Uncert::SignMagnitudeDefined);

//  if (comqual)
//    time.setUncertainty(0,0,Uncert::Approximately);

  HalfLife ret(time, comqual, units_str);

  std::string compst = record_orig;
  boost::replace_all(compst, ". ", " ");

  if (hl_to_ensdf(ret) != compst)
    DBG("HL parse mismatch '{}' != '{}' t={} v={} u1={} u2={} units={} tent={}",
        record_orig, hl_to_ensdf(ret), ret.time().to_string(false),
        ret.time().value(), ret.time().lowerUncertainty(),
        ret.time().upperUncertainty(), ret.units(),
        ret.tentative());


  return ret.preferred_units();
}

std::string hl_to_ensdf(HalfLife hl)
{
  if (!hl.valid())
    return "";
  std::string ret;
  if (hl.stable())
    ret = "STABLE";
  else
  {
    ret = hl.time().value_str();
    if (!hl.units().empty())
      ret += " " + hl.units();
    ret += uncert_to_ensdf(hl.time());
  }
  if (hl.tentative())
    ret = "(" + ret + ")";
  boost::replace_all(ret, "\u00B1", "");
  return ret;
}

std::string uncert_to_ensdf(Uncert u)
{
  std::string ret;
  if ((u.uncertaintyType() == Uncert::SymmetricUncertainty)
      || (u.uncertaintyType() == Uncert::AsymmetricUncertainty))
  {
    if (u.symmetric() &&
        std::isfinite(u.lowerUncertainty())
        && (0 != u.lowerUncertainty()))
    {
      ret = " " + u.sym_uncert_str();
      boost::replace_all(ret, "\u00B1", "");
      boost::replace_all(ret, ".", "");
    }
    else if (std::isfinite(u.lowerUncertainty()) &&
             std::isfinite(u.upperUncertainty())
             && (0 != u.lowerUncertainty())
             && (0 != u.upperUncertainty()))
    {
      ret = " " + u.asym_uncert_str();
      boost::replace_all(ret, "\u207A", "+");
      boost::replace_all(ret, "\u208B", "-");
      boost::replace_all(ret, "\u00B7", "");
      boost::replace_all(ret, ".", "");
      for (size_t i=0; i < k_UTF_subscripts.size(); ++i)
        boost::replace_all(ret, k_UTF_subscripts[i], std::to_string(i));
      for (size_t i=0; i < k_UTF_superscripts.size(); ++i)
        boost::replace_all(ret, k_UTF_superscripts[i], std::to_string(i));
    }
  }
  else if (u.uncertaintyType() == Uncert::Approximately)
    return " AP";
  else if (u.uncertaintyType() == Uncert::GreaterEqual)
    return " GE";
  else if (u.uncertaintyType() == Uncert::LessEqual)
    return " LE";
  else if (u.uncertaintyType() == Uncert::GreaterThan)
    return " GT";
  else if (u.uncertaintyType() == Uncert::LessThan)
    return " LT";
  return ret;
}

DecayMode parse_decay_mode(std::string record)
{
  DecayMode ret;
  std::string type = boost::to_upper_copy(record);
  if (boost::contains(type, "SF"))
  {
    ret.set_spontaneous_fission(true);
    boost::replace_all(type, "SF", "");
  }
  if (boost::contains(type, "2EC"))
  {
    ret.set_electron_capture(2);
    boost::replace_all(type, "2EC", "");
  }
  if (boost::contains(type, "EC"))
  {
    ret.set_electron_capture(1);
    boost::replace_all(type, "EC", "");
  }
  if (boost::contains(type, "2B+"))
  {
    ret.set_beta_plus(2);
    boost::replace_all(type, "2B+", "");
  }
  if (boost::contains(type, "B+"))
  {
    ret.set_beta_plus(1);
    boost::replace_all(type, "B+", "");
  }
  if (boost::contains(type, "2B-"))
  {
    ret.set_beta_minus(2);
    boost::replace_all(type, "2B-", "");
  }
  if (boost::contains(type, "B-"))
  {
    ret.set_beta_minus(1);
    boost::replace_all(type, "B-", "");
  }
  if (boost::contains(type, "IT"))
  {
    ret.set_isomeric(true);
    boost::replace_all(type, "IT", "");
  }
  if (boost::contains(type, "A"))
  {
    ret.set_alpha(true);
    boost::replace_all(type, "A", "");
  }
  if (boost::contains(type, "N"))
  {
    boost::replace_all(type, "N", "");
    boost::trim(type);
    if (!type.empty() && is_number(type))
      ret.set_neutrons(std::stoi(type));
    else
      ret.set_neutrons(1);
  }
  if (boost::contains(type, "P"))
  {
    boost::replace_all(type, "P", "");
    boost::trim(type);
    if (!type.empty() && is_number(type))
      ret.set_protons(std::stoi(type));
    else
      ret.set_protons(1);
  }
  return ret;
}

std::string mode_to_ensdf(DecayMode mode)
{
  std::string ret;
  if (mode.spontaneous_fission())
    ret += "SF";
  if (mode.isomeric())
    ret += "IT";
  if (mode.beta_minus())
  {
    ret += (mode.beta_minus() > 1) ? std::to_string(mode.beta_minus()) : "";
    ret += "B-";
  }
  if (mode.beta_plus())
  {
    ret += (mode.beta_plus() > 1) ? std::to_string(mode.beta_plus()) : "";
    ret += "B+";
  }
  if (mode.electron_capture())
  {
    ret += (mode.electron_capture() > 1) ? std::to_string(mode.electron_capture()) : "";
    ret += "EC";
  }
  if (mode.protons())
  {
    ret += (mode.protons() > 1) ? std::to_string(mode.protons()) : "";
    ret += "P";
  }
  if (mode.neutrons())
  {
    ret += (mode.neutrons() > 1) ? std::to_string(mode.neutrons()) : "";
    ret += "N";
  }
  if (mode.alpha())
  {
    ret += "A";
  }
  return ret;
}

DecayInfo parse_decay_info(std::string dsid_orig)
{
  DecayInfo ret;

  boost::trim(dsid_orig);
  auto dsid = dsid_orig;
  if (!boost::contains(dsid, "DECAY"))
    return ret;
  boost::replace_all(dsid, "DECAY", "");
  boost::replace_all(dsid, ":", "");
  boost::regex_replace(dsid, boost::regex("[\\s]{2,}"), " ");

  std::vector<std::string> ptokens;
  int good = 0;
  boost::split(ptokens, dsid, boost::is_any_of(","));
  for (auto& t : ptokens)
  {
    boost::trim(t);
    if (t.empty())
      continue;

    good++;

    std::vector<std::string> tokens;
    boost::split(tokens, t, boost::is_any_of(" "));
    if (tokens.size() < 2)
    {
      DBG("Not enough tokens {}", dsid_orig);
      continue;
    }

    ret.parent = parse_nid(tokens[0]);
    if (!check_nid_parse(tokens[0], ret.parent))
    {
      ERR("<BasicDecay> Could not parse parent NucID   \"{}\" from \"{}\"   in   \"{}\"",
          tokens[0], t, dsid_orig);
//              << boost::trim_copy(nid_to_ensdf(parent)) << "\"  !=  \""
  //    return DecayInfo();
    }

    auto m = parse_decay_mode(tokens[1]);
    if (mode_to_ensdf(m) == tokens[1])
      ret.mode = m;
    else
    {
      ERR("Decay mode parse failed '{}' != {} or {} in \"{}\"",
             tokens[1], mode_to_ensdf(m), m.to_string(), dsid);
    }

    if (tokens.size() > 2)
    {
      std::vector<std::string> t2(tokens.begin() + 2, tokens.begin() + tokens.size());
      auto tt = boost::trim_copy(boost::join(t2, " "));
      ret.hl = parse_halflife(tt);
    }

//    if (boost::contains(dsid_orig, "EV"))
    if (good > 1)
      DBG(" **MULTIPLE** DSID={} --> {}", dsid_orig, ret.to_string());
//    else
//      DBG << "DSID=" << dsid_orig << " --> " << ret.to_string();

  }

//  if (tokens.size() > 3)
//  {
//    DBG << "Decay mode DSID=" << dsid << " has extra tokens "
//        << boost::join(tokens, ",");
//  }
  return ret;
}

std::string nid_to_ensdf(NuclideId id, bool alt)
{
  std::string nucid = std::to_string(id.A());
  while (nucid.size() < 3)
    nucid = " " + nucid;
  if (id.composition_known())
  {
    if ((id.Z() > 109) && alt)
      nucid += std::to_string(id.Z() - 100);
    else
      nucid += boost::to_upper_copy(NuclideId::symbolOf(id.Z()));
  }
  while (nucid.size() < 5)
    nucid += " ";
  return nucid;
}

bool check_nid_parse(const std::string& s, const NuclideId& n)
{
  std::string s1 = boost::trim_copy(nid_to_ensdf(n, false));
  std::string s2 = boost::trim_copy(nid_to_ensdf(n, true));
  std::string ss = boost::trim_copy(s);
  return (ss == s1) || (ss == s2);
}

Uncert eval_mixing_ratio(Uncert vu,
                                  const std::string& mpol)
{
  if (!vu.hasFiniteValue())
  {
    std::string tmp(mpol);
    boost::replace_all(tmp, "(", "");
    boost::replace_all(tmp, ")", "");
    if (tmp.size() == 2)
      return Uncert(0.0, 1, Uncert::SignMagnitudeDefined);
    else
      return Uncert();
  }
  return vu;
}


