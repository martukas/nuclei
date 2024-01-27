#pragma once

#include <NucData/Uncert.h>
#include <NucData/Spin.h>
#include <vector>
#include <map>

struct Continuation
{
  Continuation() {}
  Continuation(std::string s);

  std::string key() const;
  std::string value() const;
  std::string value_refs() const;
  std::string debug() const;

  bool is_symbolic() const;
  bool is_uncert() const;
  bool is_spin() const;
  bool val_defined() const;

  bool valid() const;

  std::vector<std::string> quants;

  //should be one of
  std::vector<Uncert> values;
  std::string symbols;
  Spin spin;

  std::string units;
  std::string refs;
};

std::map<std::string, Continuation>
parse_continuation(const std::string&crecs);

void merge_continuations(std::map<std::string, Continuation>& to,
                         const std::map<std::string, Continuation>& from);
//const std::string& debug_line);
