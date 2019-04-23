#pragma once

#include <exception>
#include <system_error>
#include <future>
#include <iostream>
#include <ios>

template <typename T>
void processCodeException(const T& e)
{
  auto c = e.code();
  std::cERR( "- category:         " << c.category().name() << std::endl;
  std::cERR( "- value:            " << c.value() << std::endl;
  std::cERR( "- message:          " << c.message() << std::endl;
  std::cERR( "- def category:     " << c.default_error_condition().category().name() << std::endl;
  std::cERR( "- def value:        " << c.default_error_condition().value() << std::endl;
  std::cERR( "- def message:      " << c.default_error_condition().message() << std::endl;
}

inline void printException()
{
  try
  {
    throw;
  }
  catch (const std::ios_base::failure& e)
  {
    std::cERR( "I/O EXCEPTION: " << e.what() << std::endl;
//    processCodeException(e);
  }
  catch (const std::system_error& e)
  {
    std::cERR( "SYSTEM EXCEPTION: " << e.what() << std::endl;
    processCodeException(e);
  }
  catch (const std::future_error& e)
  {
    std::cERR( "FUTURE EXCEPTION: " << e.what() << std::endl;
    processCodeException(e);
  }
  catch (const std::bad_alloc& e)
  {
    std::cERR( "BAD ALLOC EXCEPTION: " << e.what() << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cERR( "EXCEPTION: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cERR( "EXCEPTION (unknown)" << std::endl;
  }
}
