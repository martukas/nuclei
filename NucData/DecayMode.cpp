#include "DecayScheme.h"
#include <boost/lexical_cast.hpp>

bool DecayMode::valid() const
{
  return (alpha() ||
          isomeric() ||
          spontaneous_fission() ||
          protons() ||
          neutrons() ||
          beta_minus() ||
          beta_plus() ||
          electron_capture());
}

bool DecayMode::alpha() const
{
  return alpha_;
}

bool DecayMode::isomeric() const
{
  return isomeric_;
}

bool DecayMode::spontaneous_fission() const
{
  return spontaneous_fission_;
}

uint16_t DecayMode::protons() const
{
  return protons_;
}

uint16_t DecayMode::neutrons() const
{
  return neutrons_;
}

uint16_t DecayMode::beta_plus() const
{
  return beta_plus_;
}

uint16_t DecayMode::beta_minus() const
{
  return beta_minus_;
}

uint16_t DecayMode::electron_capture() const
{
  return electron_capture_;
}

void DecayMode::set_alpha(bool a)
{
  alpha_ = a;
}

void DecayMode::set_isomeric(bool it)
{
  isomeric_ = it;
}

void DecayMode::set_spontaneous_fission(bool sf)
{
  spontaneous_fission_ = sf;
}

void DecayMode::set_protons(uint16_t p)
{
  protons_ = p;
}

void DecayMode::set_neutrons(uint16_t n)
{
  neutrons_ = n;
}

void DecayMode::set_beta_plus(uint16_t n)
{
  if (n < 3)
    beta_plus_ = n;
}

void DecayMode::set_beta_minus(uint16_t n)
{
  if (n < 3)
    beta_minus_ = n;
}

void DecayMode::set_electron_capture(uint16_t n)
{
  if (n < 3)
    electron_capture_ = n;
}


std::string DecayMode::to_string() const
{
  std::string ret;
  if (spontaneous_fission_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += "Spontaneous Fission";
  }
  if (isomeric_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += "Isomeric Transition";
  }
  if (beta_minus_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += (beta_minus_ > 1) ? boost::lexical_cast<std::string>(beta_minus_) : "";
    ret += "β-";
  }
  if (beta_plus_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += (beta_plus_ > 1) ? boost::lexical_cast<std::string>(beta_plus_) : "";
    ret += "β+";
  }
  if (electron_capture_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += (electron_capture_ > 1) ?
          (boost::lexical_cast<std::string>(electron_capture_) + "x "): "";
    ret += "Electron Capture";
  }
  if (protons_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += (protons_ > 1) ? boost::lexical_cast<std::string>(protons_) : "";
    ret += "p";
  }
  if (neutrons_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += (neutrons_ > 1) ? boost::lexical_cast<std::string>(neutrons_) : "";
    ret += "n";
  }
  if (alpha_)
  {
    ret += ret.empty() ? "" : ", ";
    ret += "α";
  }
  return ret;
}
