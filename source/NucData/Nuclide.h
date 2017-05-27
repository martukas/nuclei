#pragma once

#include "nid.h"
#include "Level.h"
#include "Transition.h"
#include "qpx_util.h"

#include "json.h"
using namespace nlohmann;


class Nuclide
{
public:
  Nuclide();
  Nuclide(NuclideId id);

  NuclideId id() const;

  bool empty() const;

  void add_level(const Level& level);
  void finalize();

  void add_transition_to(Transition trans,
                         double max_dif = kDoubleNaN,
                         double zero_thresh = 0.25);
  void add_transition_from(Transition trans,
                           double max_dif = kDoubleNaN,
                           double zero_thresh = 0.25);

  void removeTransition(const Transition& t);

  void cullLevels();
  bool hasTransitions(const Energy& level) const;

  std::map<Energy, Level> levels() const;
  std::map<Energy, Transition> transitions() const;

  void setHalflives(const std::vector<HalfLife>& hl);
  void addHalfLife(const HalfLife& hl);

  std::vector<HalfLife> halfLifes() const;
  std::string halfLifeAsText() const;

  std::string to_string() const;

  json comments() const;
  void add_comments(const std::string &s, const json &j);

private:
  NuclideId   id_;
  std::vector <HalfLife> halflives_;
  std::map<Energy, Level> levels_;
  std::map<Energy, Transition> transitions_;

  json comments_;

  void add_transition(const Transition& transition);
  void register_transition(const Transition& t);

  Energy nearest_level(const Energy& goal,
                       double max_dif = kDoubleNaN,
                       double zero_thresh = 0.25);
};
