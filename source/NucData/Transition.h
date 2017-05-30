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

  Transition(Energy energy, Uncert intensity);

  Energy energy() const;
  Energy from() const;
  Energy to() const;

  Uncert intensity() const;
  std::string intensity_string() const;
  std::string multipolarity() const;
  Uncert delta() const;

  std::string to_string() const;

  void set_to(const Energy& e);
  void set_from(const Energy& e);
  void set_multipol(const std::string& s);
  void set_delta(const Uncert& u);

//  std::map<std::string, std::string> kvps;

  json text() const;
  void add_text(const std::string& heading, const json &j);

private:
  Energy energy_;
  Uncert intensity_;
  std::string multipolarity_;
  Uncert delta_;
  Energy from_, to_;

  json text_;
};
