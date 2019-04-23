#pragma once

#include <iostream>
#include <string>

#include <fmt/format.h>
#include <cstdint>
#include <libgen.h>

#include <spdlog/spdlog.h>

#pragma GCC system_header

namespace CustomLogger
{

void initLogger(const spdlog::level::level_enum& LoggingLevel,
                const std::string& log_file_name,
                std::ostream* gui_stream = nullptr);
void closeLogger();

}

#define LOG(Severity, Format, ...) spdlog::log(Severity, Format, ##__VA_ARGS__)

#define CRIT(Format, ...) LOG(spdlog::level::critical, Format, ##__VA_ARGS__)
#define ERR(Format, ...) LOG(spdlog::level::err, Format, ##__VA_ARGS__)
#define WARN(Format, ...) LOG(spdlog::level::warn, Format, ##__VA_ARGS__)
#define INFO(Format, ...) LOG(spdlog::level::info, Format, ##__VA_ARGS__)
#define DBG(Format, ...) LOG(spdlog::level::debug, Format, ##__VA_ARGS__)
#define TRC(Format, ...) LOG(spdlog::level::trace, Format, ##__VA_ARGS__)

//#define LOGL(Severity, Format, ...) Log::Msg(Severity, fmt::format(Format, ##__VA_ARGS__), {{"file", std::string(__FILE__)}, {"line", std::int64_t(__LINE__)}})
//
//#define CRITL(Format, ...) LOGL(Log::Severity::Critical, Format, ##__VA_ARGS__)
//#define ERRL(Format, ...) LOGL(Log::Severity::Error, Format, ##__VA_ARGS__)
//#define WARNL(Format, ...) LOGL(Log::Severity::Warning, Format, ##__VA_ARGS__)
//#define INFOL(Format, ...) LOGL(Log::Severity::Info, Format, ##__VA_ARGS__)
//#define DBGL(Format, ...) LOGL(Log::Severity::Debug, Format, ##__VA_ARGS__)
