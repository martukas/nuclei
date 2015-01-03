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

#ifndef CUSTOM_LOGGER_H_
#define CUSTOM_LOGGER_H_

#include <iostream>
#include <string>
#include <boost/log/core.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

namespace CustomLogger {

  void initLogger(std::ostream *gui_stream, std::string log_file_N);
  void closeLogger();

  enum SeverityLevel
  {
    kTrace,
    kDebug,
    kInfo,
    kWarning,
    kError,
    kCritical
  };

  inline std::ostream& operator<< (std::ostream& strm, SeverityLevel level) {
    static const char* strings[] = {
      "TRACE",
      "DEBUG",
      "INFO",
      "WARN",
      "ERROR",
      "FATAL"
    };
    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
      strm << strings[level];
    else
      strm << static_cast<int>(level);
    return strm;
  }

}

BOOST_LOG_ATTRIBUTE_KEYWORD(g_severity, "Severity", CustomLogger::SeverityLevel)

      BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(g_custom_logger,
                                             boost::log::sources::severity_logger_mt<CustomLogger::SeverityLevel>)

#define TRC BOOST_LOG_SEV(g_custom_logger::get(), CustomLogger::kTrace)
#define DBG BOOST_LOG_SEV(g_custom_logger::get(), CustomLogger::kDebug)
#define INFO BOOST_LOG_SEV(g_custom_logger::get(), CustomLogger::kInfo)
#define WARN BOOST_LOG_SEV(g_custom_logger::get(), CustomLogger::kWarning)
#define ERR BOOST_LOG_SEV(g_custom_logger::get(), CustomLogger::kError)
#define CRIT BOOST_LOG_SEV(g_custom_logger::get(), CustomLogger::kCritical)

#endif // CUSTOM_LOGGER_H
