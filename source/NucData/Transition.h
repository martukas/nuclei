#pragma once

#include "Energy.h"
#include <map>
#include <vector>

#include "json.h"
using namespace nlohmann;

class Transition
{
public:
  Transition() {}

  Transition(Energy energy, UncertainDouble intensity);

  Energy energy() const;
  Energy from() const;
  Energy to() const;

  UncertainDouble intensity() const;
  std::string intensity_string() const;
  std::string multipolarity() const;
  UncertainDouble delta() const;

  std::string to_string() const;

  void set_to(const Energy& e);
  void set_from(const Energy& e);
  void set_multipol(const std::string& s);
  void set_delta(const UncertainDouble& u);

//  std::map<std::string, std::string> kvps;

  json comments() const;
  void add_comments(const std::string &s, const json &j);

private:
  Energy energy_;
  UncertainDouble intensity_;
  std::string multipolarity_;
  UncertainDouble delta_;
  Energy from_, to_;

  json comments_;
};
