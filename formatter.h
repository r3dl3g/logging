/**
* @copyright (c) 2015-2021 Ing. Buero Rothfuss
*                          Riedlinger Str. 8
*                          70327 Stuttgart
*                          Germany
*                          http://www.rothfuss-web.de
*
* @author    <a href="mailto:armin@rothfuss-web.de">Armin Rothfuss</a>
*
* Project    logging lib
*
* @brief     C++ logger
*
* @license   MIT license. See accompanying file LICENSE.
*/

#pragma once

// --------------------------------------------------------------------------
//
// Common includes
//

// --------------------------------------------------------------------------
//
// Library includes
//
#include <logging/record.h>
#include <logging/recorder.h>
#include <logging/logging-export.h>


/**
* Provides an API for stream logging to multiple sinks.
*/
namespace logging {

  /**
    * Formatter functor definition.
    */
  typedef std::function<void(std::ostream& out, const record& entry)> record_formatter;

  namespace fmt {

    inline record_formatter constant (const std::string& str) {
      return [=] (std::ostream& out, const logging::record&) {
        out << str;
      };
    }

    template<char C = '/'>
    inline void character (std::ostream& out, const logging::record&) {
      out << C;
    }

    inline void line (std::ostream& out, const logging::record& e) {
      out << e.line();
    }

    inline void time_point (std::ostream& out, const logging::record& e) {
      out << e.time_point();
    }

    inline void date (std::ostream& out, const logging::record& e) {
      print_date(out, e.time_point());
    }

    inline void time (std::ostream& out, const logging::record& e) {
      print_time(out, e.time_point());
    }

    inline void level (std::ostream& out, const logging::record& e) {
      out << e.level();
    }

    inline void thread (std::ostream& out, const logging::record& e) {
      out << e.thread_name();
    }

    inline void message (std::ostream& out, const logging::record& e) {
      out << e.message();
    }

    inline void endl (std::ostream& out, const logging::record&) {
      out << std::endl;
    }

  } // namespace fmt

  inline void standard_formatter (std::ostream& out, const record& e) {
    out << e.line() << '|' << e.time_point() << '|' << e.level() << '|' << e.thread_name() << '|' << e.message() << std::endl;
  }

  inline void no_time_formatter (std::ostream& out, const record& e) {
    out << e.level() << '|' << e.thread_name() << '|' << e.message() << std::endl;
  }

  inline void console_formatter (std::ostream& out, const record& e) {
    out << e.message() << std::endl;
  }

  inline record_formatter custom_formatter (const std::vector<record_formatter>& fmts) {
    return [=] (std::ostream& out, const logging::record& e) {
      for(auto& f : fmts) {
        f(out, e);
      }
    };
  }

} // namespace logging
