#include <NucData/SpinParity.h>
#include <boost/algorithm/string.hpp>
#include <ensdf/Fields.h>

void SpinParity::set_spin(Spin s)
{
  spin_ = s;
}

void SpinParity::set_parity(Parity p)
{
  parity_ = p;
}

void SpinParity::set_common_quality(DataQuality q)
{
  if (spin_.quality() != DataQuality::kUnknown)
    spin_.set_quality(q);
  if (parity_.quality() != DataQuality::kUnknown)
    parity_.set_quality(q);
}

void SpinParity::set_eq_type(Uncert::UncertaintyType t)
{
  eq_type_ = t;
}

Spin SpinParity::spin() const
{
  return spin_;
}

Parity SpinParity::parity() const
{
  return parity_;
}

bool SpinParity::valid() const
{
  return ( (parity_.quality() != DataQuality::kUnknown)
           || (spin_.quality() != DataQuality::kUnknown) );
}

std::string SpinParity::to_string(bool with_qualifiers) const
{
  std::string ret = uncert_to_ensdf(eq_type_);
  if (ret.size())
    ret += " ";
  if (with_qualifiers)
  {
    if (spin_.quality() != parity_.quality())
      return ret + spin_.to_qualified_string("") + parity_.to_qualified_string("");
    else
      return add_qualifiers(ret + spin_.to_string() + parity_.to_string(),
                                  spin_.quality());
  }
  return ret + spin_.to_string() + parity_.to_string();
}

//bool operator<(const SpinParity &left, const SpinParity &right)
//{
//  return (left.parity_ < right.parity_);
//}

//bool operator>(const SpinParity &left, const SpinParity &right)
//{

//}

//bool operator==(const SpinParity &left, const SpinParity &right)
//{

//}

void SpinSet::merge(const SpinSet& o)
{
  for (auto s : o.sps_)
    add(s);
  if (o.sps_.size())
    logical_ = o.logical_;
}

void SpinSet::add(SpinParity sp)
{
  if (!sp.valid())
    return;
  sps_.push_back(sp);
  spins_.insert(sp.spin());
  parities_.insert(sp.parity());
  qual_spins_.insert(sp.spin().quality());
  qual_parities_.insert(sp.parity().quality());
}

void SpinSet::set_common_quality(DataQuality q)
{
  qual_spins_.clear();
  qual_parities_.clear();
  qual_spins_.insert(q);
  qual_parities_.insert(q);
  for (auto& s : sps_)
    s.set_common_quality(q);
}

void SpinSet::set_common_parity(Parity p)
{
  parities_.clear();
  parities_.insert(p);
  qual_parities_.clear();
  qual_parities_.insert(p.quality());
  for (auto& s : sps_)
    s.set_parity(p);
}

bool SpinSet::valid() const
{
  return sps_.size();
}

std::string SpinSet::debug() const
{
  std::list<std::string> spinstrs;
  for (auto s : sps_)
    spinstrs.push_back(s.to_string(true));
  std::string ret = boost::join(spinstrs, logical_);
  return std::to_string(sps_.size()) + ">>  " + ret;
}

std::string SpinSet::logic() const
{
  return logical_;
}

void SpinSet::set_logic(std::string l)
{
  logical_ = l;
}

std::string SpinSet::to_pretty_string() const
{
  auto ret = to_string();
  boost::replace_all(ret, "LE ", "≤");
  boost::replace_all(ret, "LT ", "<");
  boost::replace_all(ret, "GE ", "≥");
  boost::replace_all(ret, "GT ", ">");
  boost::replace_all(ret, "AP ", "~");
  boost::replace_all(ret, "SY ", " (sys)");
  boost::replace_all(ret, "CA ", " (cal)");
  return ret;
}

std::string SpinSet::to_string() const
{
  bool one_parity = (parities_.size() == 1)
      && (qual_parities_.size() == 1);

  bool one_spin_quality = (qual_spins_.size() == 1);
  bool one_parity_quality = (qual_parities_.size() == 1);

  bool one_quality = one_spin_quality && one_parity_quality &&
      ((*qual_spins_.begin()) == (*qual_parities_.begin()));

  bool ommit_ind_qualities = one_quality && (sps_.size() > 1);

  bool ommit_ind_parities = (sps_.size() > 1) &&
      one_spin_quality && one_parity &&
      (*qual_spins_.begin() != DataQuality::kKnown) &&
      (*qual_spins_.begin() != DataQuality::kAbout) &&
      (*qual_parities_.begin() != DataQuality::kTentative) &&
      (*qual_parities_.begin() != DataQuality::kTheoretical);

  std::list<std::string> spinstrs;
  for (auto s : sps_)
  {
    if (ommit_ind_parities)
      spinstrs.push_back(s.spin().to_string());
    else
      spinstrs.push_back(s.to_string(!ommit_ind_qualities));
  }
  std::string ret = boost::join(spinstrs, logical_);
  if (ommit_ind_parities)
  {
    if ((*qual_spins_.begin()) == (*qual_parities_.begin()))
    {
      ret = add_qualifiers(ret, *qual_spins_.begin());
      ret += parities_.begin()->to_string();
    }
    else if (one_spin_quality)
    {
      ret = add_qualifiers(ret, *qual_spins_.begin());
      ret += parities_.begin()->to_qualified_string();
    }
  }
  else if (one_quality && (sps_.size() > 1))
  {
    ret = add_qualifiers(ret, *qual_spins_.begin());
  }
  if (ret == "?")
    ret = "";
  return ret;
}
