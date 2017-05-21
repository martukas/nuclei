#pragma once

#include "DecayScheme.h"
#include <list>
#include <cmath>

struct BlockIndices
{
  BlockIndices() {}
  BlockIndices(size_t first, size_t last)
    : first(first), last(last) {}
  size_t first {0};
  size_t last {0};
};

struct ENSDFData
{
  ENSDFData(const std::vector<std::string>& l, BlockIndices ii);
  BlockIndices i;
  const std::vector<std::string>& lines;

  bool has_more() const;
  const std::string& look_ahead() const;
  const std::string& read_pop();
  const std::string& read();

  ENSDFData& operator++();   // prefix
  ENSDFData operator++(int); // postfix
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

void merge_continuations(std::map<std::string, std::string>& to,
                         const std::map<std::string, std::string>& from,
                         const std::string& debug_line);
