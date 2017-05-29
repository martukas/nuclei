#include "Fields.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "qpx_util.h"
#include "custom_logger.h"

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

  Uncert result(boost::lexical_cast<double>(val), sig_digits(val), Uncert::UndefinedSign);
  double val_order = get_precision(val);

  if (boost::contains(val, "+") || boost::contains(val, "-"))
    result.setSign(Uncert::SignMagnitudeDefined);
  else
    result.setSign(Uncert::MagnitudeDefined);

  // parse uncertainty
  // symmetric or special case (consider symmetric if not + and - are both contained in string)
  if ( !( boost::contains(uncert,"+") && boost::contains(uncert, "-"))
       || flag_tentative ) {
    if (uncert == "LT")
      result.setUncertainty(-std::numeric_limits<double>::infinity(), 0.0, Uncert::LessThan);
    else if (uncert == "GT")
      result.setUncertainty(0.0, std::numeric_limits<double>::infinity(), Uncert::GreaterThan);
    else if (uncert == "LE")
      result.setUncertainty(-std::numeric_limits<double>::infinity(), 0.0, Uncert::LessEqual);
    else if (uncert == "GE")
      result.setUncertainty(0.0, std::numeric_limits<double>::infinity(), Uncert::GreaterEqual);
    else if (uncert == "AP" || uncert.empty() || !is_number(uncert) || flag_tentative)
      result.setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), Uncert::Approximately);
    else if (uncert == "CA" || flag_theoretical)
      result.setUncertainty(0.0, 0.0, Uncert::Calculated);
    else if (uncert == "SY")
      result.setUncertainty(0.0, 0.0, Uncert::Systematics);
    else {
      // determine significant figure
      if (!uncert.empty() && is_number(uncert))
        result.setSymmetricUncertainty(val_order * boost::lexical_cast<int16_t>(uncert));
      else
        result.setUncertainty(std::numeric_limits<double>::quiet_NaN(),
                              std::numeric_limits<double>::quiet_NaN(),
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
      upositive = boost::lexical_cast<int16_t>(uposstr);
    else if (uposstr == "|@")
      upositive = std::numeric_limits<double>::infinity();

    if (!unegstr.empty() && is_number(unegstr))
      unegative = boost::lexical_cast<int16_t>(unegstr);
    else if (unegstr == "|@")
      unegative = -1 * std::numeric_limits<double>::infinity();

    // work aournd bad entries with asymmetric uncertainty values of 0.
    if (upositive == 0.0 && unegative == 0.0)
    {
      result.setUncertainty(std::numeric_limits<double>::quiet_NaN(),
                            std::numeric_limits<double>::quiet_NaN(),
                            Uncert::Approximately);
      WARN << "Found asymmetric error of -0+0 in '"
           << uncert
           << "' parsed as -" << unegstr << " +" << uposstr
           << ". Auto-changing to 'approximately'";
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


DataQuality quality_of(const std::string& s)
{
  std::string st = s;
  int16_t nb, nb_unkown, nb_theo_left, nb_theo_right, nb_tenta_left,
      nb_tenta_right, nb_about;
  nb = nb_unkown = nb_theo_left = nb_theo_right = nb_tenta_left =
      nb_tenta_right = nb_about = 0;

  // reads the string and counts the specfics characters
  size_t end = st.length();
  for(size_t i = 0 ; i < end; i++)
  {
    if ( st[i] == '?' )
    {
      nb_unkown++;
      nb++;
    }
    if ( st[i] == '(' )
    {
      nb_tenta_left++;
      nb++;
    }
    if ( st[i] == ')' )
    {
      nb_tenta_right++;
      nb++;
    }
    if ( st[i] == '[' )
    {
      nb_theo_left++; nb++;
    }
    if ( st[i] == ']' )
    {
      nb_theo_right++;
      nb++;
    }
    if ( st[i] == '~' )
    {
      nb_about++;
      nb++;
    }
  }
  // set the informations
  if ( nb  > 2 ) return DataQuality::kUnknown;
  if ( nb == 0 ) return DataQuality::kKnown;

  if ( nb == 1 ) {
    if ( nb_about  == 1 ) return DataQuality::kAbout;
    else return DataQuality::kUnknown;
  }
  else { // nb = 2
    if ( nb_tenta_left == 1 && nb_tenta_right == 1 )
      return DataQuality::kTentative;
    if (  nb_theo_left == 1 &&  nb_theo_right == 1  )
      return DataQuality::kTheoretical;
  }
  return DataQuality::kUnknown;
}

std::string strip_qualifiers(const std::string& original)
{
  std::string ret = original;
  boost::replace_all(ret, "~","");
  boost::replace_all(ret, "(","");
  boost::replace_all(ret, ")","");
  boost::replace_all(ret, "[","");
  boost::replace_all(ret, "]","");
  boost::trim(ret);
  return ret;
}


Moment parse_moment(const std::string& record)
{
  std::string value_str;
  std::string uncert_str;
  std::string references_str;

  std::vector<std::string> momentparts;
  std::string rec_copy = trim_all(record);
  boost::trim(rec_copy);
  boost::split(momentparts, rec_copy, boost::is_any_of(" \r\t\n\0"));
  if (momentparts.size() >= 1)
    value_str = momentparts[0];
  if (momentparts.size() >= 2)
    uncert_str = momentparts[1];
  if (momentparts.size() >= 3)
    references_str = momentparts[2];

  if (is_uncertainty_id(value_str))
  {
    value_str = uncert_str;
    uncert_str = momentparts[0];
  }

  Moment ret(parse_val_uncert(value_str, uncert_str));

  if (!references_str.empty())
  {
    std::vector<std::string> refs;
    boost::replace_all(references_str, "(", "");
    boost::replace_all(references_str, ")", "");
    boost::split(refs, references_str, boost::is_any_of(","));
    ret.set_references(refs);
  }

  return ret;
}

NuclideId parse_check_nid(std::string nucid)
{
  auto ret = parse_nid(nucid);
  if (!check_nid_parse(nucid, ret))
    ERR << "Could not parse daughter NucID  \"" << nucid << "\"";
  return ret;
}

NuclideId parse_nid(std::string id)
{
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
      DBG << "<NuclideId> Bad A value from " << id;
      return NuclideId();
    }
    return NuclideId::fromAZ(boost::lexical_cast<uint16_t>(A), Z);
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
          Z = boost::lexical_cast<uint16_t>(zstring);
        else
          DBG << "<NuclideId> Bad zstring from " << id;
      }
      if (!is_number(A))
      {
        DBG << "<NuclideId> Bad A value (2) from " << id;
        return NuclideId();
      }
      return NuclideId::fromAZ(boost::lexical_cast<uint16_t>(A), Z);
    }
    else
    {
      if (!is_number(id))
      {
        DBG << "<NuclideId> Bad id value from " << id;
        return NuclideId();
      }
      return NuclideId::fromAZ(boost::lexical_cast<uint16_t>(id), 0, true);
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
  uint16_t denominator {1};
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
  else
    return Parity(Parity::EnumParity::kPlus, quality);
}


SpinParity parse_spin_parity(std::string data)
{
  SpinParity ret;

  //what if tentative only parity or spin only?
  boost::trim_copy(data);
  ret.set_parity(parse_parity(data));
  boost::replace_all(data, "(", "");
  boost::replace_all(data, ")", "");
  std::vector<std::string> spin_strs;
  boost::split(spin_strs, data, boost::is_any_of(","));
  for (auto &token : spin_strs) {
    Spin spin = parse_spin(token);
    spin.set_quality(quality_of(data));
    ret.add_spin(spin);
  }
//  if (!ret.parity_.has_quality(DataQuality::kKnown))
//    DBG << "SpinParity " << data << " --> " << ret.to_string();
  return ret;
}

HalfLife parse_halflife(const std::string& record)
{
  std::string value_str;
  std::string units_str;
  std::string uncert_str;

  std::vector<std::string> timeparts;
  std::string rec_copy = trim_all(record);
  boost::split(timeparts, rec_copy, boost::is_any_of(" \r\t\n\0"));
  if (timeparts.size() >= 1)
    value_str = timeparts[0];
  if (timeparts.size() >= 2)
    units_str = timeparts[1];
  if (timeparts.size() >= 3)
    uncert_str = timeparts[2];

  Uncert time = parse_val_uncert(value_str, uncert_str);
  if (boost::contains(boost::to_upper_copy(record), "STABLE"))
    time.setValue(std::numeric_limits<double>::infinity(),
                  Uncert::SignMagnitudeDefined);
  else if (boost::contains(record, "EV"))
    time.setValue(std::numeric_limits<double>::quiet_NaN(),
                  Uncert::SignMagnitudeDefined);

  return HalfLife(time, units_str).preferred_units();
}

DecayMode parse_decay_mode(std::string record)
{
  DecayMode ret;
  std::string type = boost::to_upper_copy(record);
  if (boost::contains(type, "INELASTIC SCATTERING"))
  {
    ret.set_inelastic_scattering(true);
    boost::replace_all(type, "INELASTIC SCATTERING", "");
  }
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
      ret.set_neutrons(boost::lexical_cast<uint16_t>(type));
    else
      ret.set_neutrons(1);
  }
  if (boost::contains(type, "P"))
  {
    boost::replace_all(type, "P", "");
    boost::trim(type);
    if (!type.empty() && is_number(type))
      ret.set_protons(boost::lexical_cast<uint16_t>(type));
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
    ret += (mode.beta_minus() > 1) ? boost::lexical_cast<std::string>(mode.beta_minus()) : "";
    ret += "B-";
  }
  if (mode.beta_plus())
  {
    ret += (mode.beta_plus() > 1) ? boost::lexical_cast<std::string>(mode.beta_plus()) : "";
    ret += "B+";
  }
  if (mode.electron_capture())
  {
    ret += (mode.electron_capture() > 1) ? boost::lexical_cast<std::string>(mode.electron_capture()) : "";
    ret += "EC";
  }
  if (mode.protons())
  {
    ret += (mode.protons() > 1) ? boost::lexical_cast<std::string>(mode.protons()) : "";
    ret += "P";
  }
  if (mode.neutrons())
  {
    ret += (mode.neutrons() > 1) ? boost::lexical_cast<std::string>(mode.neutrons()) : "";
    ret += "N";
  }
  if (mode.alpha())
  {
    ret += "A";
  }
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
      nucid += boost::lexical_cast<std::string>(id.Z() - 100);
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


