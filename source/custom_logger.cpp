/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      Utility based on boost log for output to file, console and gui.
 *
 ******************************************************************************/

#include "custom_logger.h"

#include <fstream>
#include <boost/core/null_deleter.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace date = boost::date_time;

typedef sinks::asynchronous_sink<sinks::text_ostream_backend, sinks::unbounded_fifo_queue> text_sink;
typedef sinks::synchronous_sink<sinks::text_file_backend> file_sink;

void CustomLogger::initLogger(std::ostream *gui_stream, std::string log_file_N)
{
  logging::add_common_attributes();

  logging::formatter format_basic =
      expr::stream << "["
                   << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S") << "] "
      //                   << severity << ": "
                   << expr::message;

  logging::formatter format_verbose =
      expr::stream << "["
                   << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y%m%d %H:%M:%S.%f")
                   << "] " << g_severity << ": "
                   << expr::message;
  
  boost::shared_ptr< logging::core > core = logging::core::get();

  // GUI
  if (gui_stream != nullptr) {
    boost::shared_ptr<sinks::text_ostream_backend> backend_gui = boost::make_shared<sinks::text_ostream_backend>();
    backend_gui->add_stream(boost::shared_ptr<std::ostream>(gui_stream, boost::null_deleter()));
    boost::shared_ptr<text_sink> sink_gui(new text_sink(backend_gui));
    sink_gui->set_formatter(format_basic);
    sink_gui->set_filter(expr::attr<SeverityLevel>("Severity") >= kInfo);
    core->add_sink(sink_gui);
  }

  // console
  boost::shared_ptr<text_sink> sink_console = boost::make_shared<text_sink>();
  boost::shared_ptr<std::ostream> console_stream(&std::cout, boost::null_deleter());
  sink_console->locked_backend()->add_stream(console_stream);
  sink_console->locked_backend()->auto_flush(true);
  sink_console->set_formatter(format_verbose);
  sink_console->set_filter(expr::attr<SeverityLevel>("Severity") >= kDebug);
  core->add_sink(sink_console);

  // file
  boost::shared_ptr<sinks::text_file_backend> backend_file = boost::make_shared<sinks::text_file_backend>(
      // file name pattern
      keywords::file_name = log_file_N,
      keywords::open_mode = (std::ios::out | std::ios::app),
      // rotate the file upon reaching 10 MiB size...
      keywords::rotation_size = 10 * 1024 * 1024
                                                                                                          );
  boost::shared_ptr<file_sink> sink_file(new file_sink(backend_file));
  sink_file->locked_backend()->auto_flush(true);
  sink_file->set_formatter(format_verbose);
  sink_file->set_filter(expr::attr<SeverityLevel >("Severity") >= kTrace);
  core->add_sink(sink_file);
}

void CustomLogger::closeLogger()
{
  DBG << "<CustomLogger> Closing logger sinks";
  logging::core::get()->remove_all_sinks();
}

