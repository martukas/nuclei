#include <util/logger.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <fstream>
#include <date/date.h>

namespace spdlog
{
namespace sinks
{

template<typename Mutex>
class ostream_sink final : public spdlog::sinks::base_sink<Mutex>
{
 public:
  ostream_sink(std::ostream* gui_stream)
      : outStream(gui_stream)
  {}

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override
  {
    fmt::memory_buffer formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    (*outStream) << fmt::to_string(formatted);
  }

  void flush_() override
  {
    outStream->flush();
  };

 private:
  std::ostream* outStream{nullptr};
};

using ostream_sink_mt = ostream_sink<std::mutex>;
using ostream_sink_st = ostream_sink<details::null_mutex>;

}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> ostream_mt(const std::string& logger_name,
                                          const spdlog::level::level_enum& LoggingLevel,
                                          const std::string& host,
                                          const int& port)
{
  return Factory::template create<sinks::ostream_sink_mt>
      (logger_name, LoggingLevel, host, port);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> ostream_st(const std::string& logger_name,
                                          const spdlog::level::level_enum& LoggingLevel,
                                          const std::string& host,
                                          const int& port)
{
  return Factory::template create<sinks::ostream_sink_st>
      (logger_name, LoggingLevel, host, port);
}

}

namespace CustomLogger
{

void initLogger(const spdlog::level::level_enum& LoggingLevel,
                const std::string& log_file_name,
                std::ostream* gui_stream)
{
  closeLogger();

  std::vector<spdlog::sink_ptr> sinks;

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [processID: %P]: %v");
  sinks.push_back(console_sink);

  if (!log_file_name.empty())
  {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [processID: %P]: %v");
    sinks.push_back(file_sink);
  }

  if (gui_stream)
  {
    auto file_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(gui_stream);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [processID: %P]: %v");
    sinks.push_back(file_sink);
  }

  auto combined_logger = std::make_shared<spdlog::logger>
      ("daquiri_logger", begin(sinks), end(sinks));
  combined_logger->set_level(LoggingLevel);
  combined_logger->flush_on(LoggingLevel);
  spdlog::flush_every(std::chrono::seconds(1));
  spdlog::set_default_logger(combined_logger);
}

void closeLogger()
{
  // Release all spdlog resources, and drop all loggers in the registry.
  // This is optional (only mandatory if using windows + async log).
  spdlog::shutdown();
}

}