#include <util/time_extensions.h>

#include <sstream>
#include <date/date.h>

std::string very_simple(const hr_duration_t &duration)
{
  std::stringstream ss;
  ss << date::format("%T", date::round<std::chrono::seconds>(duration));
  return ss.str();
}

std::string to_simple(const hr_duration_t &duration)
{
  std::stringstream ss;
  ss << date::format("%T", date::round<std::chrono::microseconds>(duration));
  return ss.str();
}

hr_duration_t duration_from_string(const std::string& durstring)
{
  std::istringstream in{durstring};
  std::chrono::microseconds us;
  in >> date::parse("%6H:%M:%S", us);
  return us;
}

std::string to_simple(const hr_time_t& time)
{
  std::stringstream ss;
  ss << date::format("%F %T", date::floor<std::chrono::seconds>(time));
  return ss.str();
}

std::string to_iso_extended(const hr_time_t& time)
{
  std::stringstream ss;
  ss << date::format("%FT%T", date::floor<std::chrono::microseconds>(time));
  return ss.str();
}

hr_time_t from_iso_extended(const std::string& timestr)
{
  std::istringstream in{timestr};
  hr_time_t ret;
  in >> date::parse("%FT%T", ret);
  return ret;
}
