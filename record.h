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
#include <chrono>
#include <string>

// --------------------------------------------------------------------------
//
// Library includes
//
#include "log_level.h"

#ifdef WIN32
#pragma warning (disable: 4251)
#endif

/**
* Provides an API for stream logging to multiple sinks.
*/
namespace logging {

  /**
    * Id for current logged line.
    */
  struct LOGGING_EXPORT line_id {
    line_id (unsigned int i);
    line_id ();

    unsigned int n;
  };

  /// convenience stream operator to print line id to ostream.
  LOGGING_EXPORT std::ostream& operator << (std::ostream& out, line_id const& id);

  /**
    * Logging record. Holds data for one record.
    */
  class LOGGING_EXPORT record {
  public:
    record (const std::chrono::system_clock::time_point& time_point,
            level lvl,
            const std::string& thread_name,
            const line_id& line,
            const std::string& message);

    record ();

    const line_id& line () const;
    const std::chrono::system_clock::time_point& time_point () const;
    const logging::level& level () const;
    const std::string& thread_name () const;
    const std::string& message () const;

  private:
    std::chrono::system_clock::time_point m_time_point;
    logging::level m_level;
    std::string m_thread_name;
    line_id m_line;
    std::string m_message;

  };

} // namespace logging

#include <logging/record.inl>
