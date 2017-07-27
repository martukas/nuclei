#include "Nuclide.h"
#include "qpx_util.h"
#include "custom_logger.h"

Nuclide::Nuclide()
{
}

Nuclide::Nuclide(NuclideId id)
  : id_(id)
{
}

void Nuclide::add_text(const std::string &heading, const json &j)
{
  json block;
  block["heading"] = heading;
  block["pars"] = j;
  text_.push_back(block);
}

json Nuclide::text() const
{
  return text_;
}

NuclideId Nuclide::id() const
{
  return id_;
}

bool Nuclide::empty() const
{
  return (!id_.valid() || levels_.empty() /*|| halflives_.empty()*/);
}

std::map<Energy, Level> Nuclide::levels() const
{
  return levels_;
}

std::map<Energy, Transition> Nuclide::transitions() const
{
  return transitions_;
}

void Nuclide::setHalflives(const std::vector<HalfLife>& hl)
{
  halflives_ = hl;
}

void Nuclide::addHalfLife(const HalfLife& hl)
{
  halflives_.push_back(hl);
}

std::vector<HalfLife> Nuclide::halfLifes() const
{
  return halflives_;
}

std::string Nuclide::halfLifeAsText() const
{
  std::vector<std::string> results;
  for (auto &h : halflives_)
    if (h.valid() && !h.stable())
      results.push_back(h.to_string());
  return join(results, ", ");
}

void Nuclide::add_level(const Level& level)
{
//  DBG << "<Nuclide::add_level> adding " << level.energy().to_string();
  if (level.energy().valid())
    levels_[level.energy()] = level;
}

Transition Nuclide::nearest_transition(double goal) const
{
  Energy best;
  //  if (transitions_.size())
  //    best = transitions_.begin()->first;
  for (const auto& tr : transitions_)
    if (tr.first.value().hasFiniteValue())
      best = tr.first;
  for (const auto& tr : transitions_)
  {
    if (std::isfinite(goal) &&
        tr.first.value().hasFiniteValue() &&
        (std::abs(goal - tr.first) <
         std::abs(goal - best)))
      best = tr.first;
  }
  if (transitions_.count(best))
    return transitions_.at(best);
  return Transition();
}

std::set<Energy> Nuclide::coincidences(std::set<Energy> transitions) const
{
  std::set<Energy> intersect;
  if (transitions.size())
  {
    intersect = coincidences(*transitions.begin());
    for (auto s : transitions)
    {
      auto s1 = coincidences(s);
      auto s2 = intersect;
      intersect.clear();
      set_intersection(s1.begin(),s1.end(),s2.begin(),s2.end(),
                        std::inserter(intersect,intersect.begin()));

    }
  }
  return intersect;
}

std::set<Energy> Nuclide::coincidences(Energy transition) const
{
  std::set<Energy> ret;
  auto u = upstream(transition);
  auto d = downstream(transition);
  ret.insert(u.begin(), u.end());
  ret.insert(d.begin(), d.end());
  return ret;
}

std::set<Energy> Nuclide::upstream(Energy transition) const
{
  std::set<Energy> ret;
  if (!transitions_.count(transition))
    return ret;
  Transition x = transitions_.at(transition);
  if (!levels_.count(x.from()))
    return ret;
  Level l = levels_.at(x.from());
  for (auto pop : l.populatingTransitions())
  {
    ret.insert(pop);
    auto u = upstream(pop);
    ret.insert(u.begin(), u.end());
  }
  return ret;
}

std::set<Energy> Nuclide::downstream(Energy transition) const
{
  std::set<Energy> ret;
  if (!transitions_.count(transition))
    return ret;
  Transition x = transitions_.at(transition);
  if (!levels_.count(x.to()))
    return ret;
  Level l = levels_.at(x.to());
  for (auto pop : l.depopulatingTransitions())
  {
    ret.insert(pop);
    auto d = downstream(pop);
    ret.insert(d.begin(), d.end());
  }
  return ret;
}

Energy Nuclide::nearest_level(const Energy& goal,
                              double max_dif,
                              double zero_thresh)
{
  double lowest = 0;
  for (const auto& lev : levels_)
    if (lev.first.value().hasFiniteValue() &&
        lev.first.value().value())
    {
      lowest = lev.first.value().value();
      break;
    }
  if (lowest)
    zero_thresh *= lowest;

  max_dif *= goal;
  Energy best;
//      = Energy(Uncert(kDoubleInf, 1, Uncert::SignMagnitudeDefined));
//  DBG << "<Nuclide::nearest_level> maxdif " << max_dif
//      << " for " << goal.to_string();
  for (const auto& lev : levels_)
  {
    if (std::isfinite(max_dif) &&
        ((goal > zero_thresh) || (lev.first.value().value() != 0)) &&
        (std::abs(goal - lev.first) > max_dif))
      continue;

    //    DBG << "     result " << std::abs(goal - lev.first)
    //        << " <? " << std::abs(goal - best)
    //        << " " << best.value().hasFiniteValue()
    //        << " " << (std::abs(goal - lev.first) <
    //                   std::abs(goal - best));

    if (!best.valid() ||
        (std::abs(goal - lev.first) <
         std::abs(goal - best)))
      best = lev.first;
  }

//  DBG << "<Nuclide::nearest_level> found best " << best.to_string()
//      << " " << levels_.count(best);
  return best;
}


void Nuclide::add_transition(const Transition& transition)
{
  transitions_[transition.energy()] = transition;
  register_transition(transition);
}

void Nuclide::add_transition_to(Transition trans,
                                double max_dif,
                                double zero_thresh)
{
  if (!levels_.count(trans.to()))
  {
    DBG << "<Nuclide::add_transition_to> no to level "
        << trans.to_string();
//    DBG << "  Levels " << levels_.size();
//    for (auto l : levels_)
//      DBG << "   " << l.first;
    return;
  }
  Energy from = trans.to() + trans.energy();
  if (std::isfinite(max_dif) && !levels_.count(from))
    from = nearest_level(from, max_dif, zero_thresh);
  if (from.valid() && levels_.count(from))
  {
    trans.set_from(from);
    add_transition(trans);
  }
  else
  {
//    DBG << "<Nuclide::add_transition_to> no valid from "
//        << trans.to_string()
//        << "    expected level "
//        << (trans.from() - trans.energy()).to_string();
  }
}

void Nuclide::add_transition_from(Transition trans,
                                  double max_dif,
                                  double zero_thresh)
{
  if (!levels_.count(trans.from()))
  {
    DBG << "<Nuclide::add_transition_from> no from level "
        << trans.to_string();
//    DBG << "  Levels " << levels_.size();
//    for (auto l : levels_)
//      DBG << "   " << l.first;
    return;
  }
  Energy to = trans.from() - trans.energy();
  if (std::isfinite(max_dif) && !levels_.count(to))
    to = nearest_level(to, max_dif, zero_thresh);
//  DBG << "found what? " << to.to_string() << " " <<  levels_.count(to);
  if (to.valid() && levels_.count(to))
  {
    trans.set_to(to);
    add_transition(trans);
  }
  else
  {
//    DBG << "<Nuclide::add_transition_from> no valid to "
//        << trans.to_string()
//        << "    expected level "
//        << (trans.from() - trans.energy()).to_string();
  }
}

void Nuclide::removeTransition(const Transition& t)
{
  if (transitions_.count(t.energy()))
    transitions_.erase(t.energy());
}

void Nuclide::cullLevels()
{
  for (auto l : levels_)
    if (!hasTransitions(l.first))
      levels_.erase(l.first);
}

bool Nuclide::hasTransitions(const Energy& level) const
{
  for (auto t : transitions_)
    if ((level == t.second.from()) || (level == t.second.to()))
      return true;
  return false;
}

void Nuclide::register_transition(const Transition& t)
{
  levels_[t.from()].addDepopulatingTransition(t.energy());
  levels_[t.to()].addPopulatingTransition(t.energy());
}

std::string Nuclide::to_string() const
{
  std::string ret = id_.symbolicName() + " ";
  for (auto h : halflives_)
    ret += h.to_string();
  ret += "\n";
  if (levels_.size())
  {
    ret += "Levels (" + boost::lexical_cast<std::string>(levels_.size()) +")\n";
    for (auto l : levels_)
      ret += l.second.to_string() +  "\n";
  }
  if (transitions_.size())
  {
    ret += "Transitions (" + boost::lexical_cast<std::string>(transitions_.size()) + ")\n";
    for (auto t : transitions_)
      ret += "  " + t.second.to_string() +  "\n";
  }
  return ret;
}
