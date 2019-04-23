#pragma once

#include <string>
#include <sstream>

inline std::string itobin16 (uint16_t bin)
{
  std::stringstream ss;
  for (int k = 0; k < 16; ++k) {
    if (bin & 0x8000)
      ss << "1";
    else
      ss << "0";
    bin <<= 1;
  }
  return ss.str();
}

inline std::string itobin32 (uint32_t bin)
{
  uint16_t lo = bin & 0x0000FFFF;
  uint16_t hi = (bin >> 16) & 0x0000FFFF;

  return (itobin16(hi) + " " + itobin16(lo));
}

inline std::string itobin64 (uint64_t bin)
{
  uint32_t lo = bin & 0x00000000FFFFFFFF;
  uint32_t hi = (bin >> 32) & 0x00000000FFFFFFFF;

  return (itobin32(hi) + " " + itobin32(lo));
}


inline std::string itohex64 (uint64_t bin)
{
  std::stringstream stream;
  stream << std::uppercase << std::setfill ('0') << std::setw(sizeof(uint64_t)*2)
         << std::hex << bin;
  return stream.str();
}

inline std::string itohex32 (uint32_t bin)
{
  std::stringstream stream;
  stream << std::uppercase << std::setfill ('0') << std::setw(sizeof(uint32_t)*2)
         << std::hex << bin;
  return stream.str();
}

inline std::string itohex16 (uint16_t bin)
{
  std::stringstream stream;
  stream << std::uppercase << std::setfill ('0') << std::setw(sizeof(uint16_t)*2)
         << std::hex << bin;
  return stream.str();
}
