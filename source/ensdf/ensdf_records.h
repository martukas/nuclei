#pragma once

#include "DecayScheme.h"
#include <list>
#include <cmath>

struct BlockIndices
{
  BlockIndices() {}
  BlockIndices(size_t first, size_t last)
    : first(first), last(last) {}
  void clear() { first = last = 0; }
  size_t first {0};
  size_t last {0};
};

bool match_first(const std::string& line,
                 const std::string& sub_pattern);

bool match_cont(const std::string& line,
                 const std::string& sub_pattern);

bool match_record_type(const std::string& line,
                       const std::string& pattern);

std::map<std::string, std::string> parse_continuation(const std::string&crecs);

bool xref_check(const std::string& xref,
                const std::string& dssym);
