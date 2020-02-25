/**
* @copyright (c) 2015-2019 Ing. Buero Rothfuss
*                          Riedlinger Str. 8
*                          70327 Stuttgart
*                          Germany
*                          http://www.rothfuss-web.de
*
* @author    <a href="mailto:armin@rothfuss-web.de">Armin Rothfuss</a>
*
* Project    standard lib
*
* Customer   -
*
* @brief     C++ API:logger
*
* @file
*/

#pragma once

// --------------------------------------------------------------------------
//
// Common includes
//
#include <exception>
#include <string>
#include <chrono>
#include <sstream>

// --------------------------------------------------------------------------
//
// Library includes
//
#include <logging/log_level.h>

#ifdef WIN32
#pragma warning (disable: 4251)
#endif

/**
* Provides an API for stream logging to multiple sinks.
*/
namespace logging {

  /// convenience stream operator to print system_clock to ostream.
  LOGGING_EXPORT std::ostream& operator << (std::ostream& out, std::chrono::system_clock::time_point const& tp);

  /// convenience stream operator to print exception infos to ostream.
  LOGGING_EXPORT std::ostream& operator << (std::ostream& out, const std::exception& ex);

  struct flush {};

  /**
    * Logging recorder. Capture data for one record.
    */
  class LOGGING_EXPORT recorder {
  public:
    recorder (level lvl);

    ~recorder ();

    template <typename T>
    recorder& operator<< (T const& value);

    recorder& operator<< (const char value);
    recorder& operator<< (const char* value);
    recorder& operator<< (const std::string& value);
    recorder& operator<< (const flush&);

    recorder& endl ();
    recorder& raw ();
    recorder& escaped ();
    bool is_raw () const;

    operator std::ostream& ();
    std::ostream& stream ();

  private:
    std::chrono::system_clock::time_point m_time_point;
    level m_level;
    bool unescaped;
    std::ostringstream m_buffer;
  };

  class null_recoder {
  public:
    template <typename T>
    inline null_recoder& operator<< (T const&) {
      return *this;
    }
  };

} // namespace logging

#include <logging/recorder.inl>

