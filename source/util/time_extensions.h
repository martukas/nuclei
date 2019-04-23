#pragma once

#include <string>
#include <chrono>

using hr_time_t = std::chrono::system_clock::time_point;
using hr_duration_t = std::chrono::duration<double>;

std::string to_simple(const hr_time_t& time);
std::string to_iso_extended(const hr_time_t& time);
hr_time_t from_iso_extended(const std::string& timestr);

std::string very_simple(const hr_duration_t &duration);
std::string to_simple(const hr_duration_t &duration);
hr_duration_t duration_from_string(const std::string& durstring);
