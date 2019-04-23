#pragma once

#include <nlohmann/json.hpp>
#include <fstream>

inline nlohmann::json from_json_file(std::string fname)
{
  std::ifstream ifs(fname, std::ofstream::in);
  nlohmann::json j;
  if (ifs.good())
    ifs >> j;
  return j;
}

inline void to_json_file(const nlohmann::json& j, std::string fname)
{
  std::ofstream(fname, std::ofstream::trunc) << j.dump(1);
}
