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

template <typename T>
Energy findNearest(const std::map<Energy, T> &map,
                   const Energy &val)
{
  if (map.empty())
    return Energy();
  typename std::map<Energy, T>::const_iterator low, prev;
  low = map.lower_bound(val);
  if (low == map.end())
    low--;
  else if (low != map.begin()) {
    prev = low;
    --prev;
    if (std::abs(val - prev->first) < std::abs(low->first - val))
      low = prev;
  }
  return low->first;
}

bool match_record_type(const std::string& line,
                       const std::string& pattern);

std::vector<std::string> extractContinuationRecords(const BlockIndices &adoptedblock,
                                                    const std::list<std::string> &requestedRecords,
                                                    const std::vector<std::string>& data,
                                                    std::string typeOfContinuedRecord = "L");

std::map<Energy, std::string> get_gamma_lines(const std::vector<std::string>& data,
                                              BlockIndices bounds, NuclideId nucid);

bool is_gamma_line(const std::string& line,
                   std::string nucid,
                   std::string nucid2 = "");

bool is_level_line(const std::string& line,
                   std::string nucid,
                   std::string nucid2 = "");

bool is_intensity_line(const std::string& line,
                       std::string nucid,
                       std::string nucid2 = "");

bool is_norm_line(const std::string& line,
                  std::string nucid,
                  std::string nucid2 = "");

bool is_p_norm_line(const std::string& line,
                    std::string nucid,
                    std::string nucid2 = "");

bool is_feed_a_line(const std::string& line,
                    std::string nucid,
                    std::string nucid2 = "");

bool is_feed_b_line(const std::string& line,
                    std::string nucid,
                    std::string nucid2 = "");

bool is_feed_line(const std::string& line,
                  std::string nucid,
                  std::string nucid2 = "");

