/**
* @copyright (c) 2015-2021 Ing. Buero Rothfuss
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
* @brief     C++ API:log level
*
* @file
*/

#pragma once

// --------------------------------------------------------------------------
//
// Common includes
//
#include <iosfwd>

// --------------------------------------------------------------------------
//
// Library includes
//
#include <logging-export.h>


/**
* Provides an API for stream logging to multiple sinks.
*/
namespace logging {

  /**
  * logging levels. A level always includes subsequent levels.
  */
  enum class level {
    undefined, trace, debug, info, warning, error, fatal
  };

  /// convenience stream operator to print log level to ostream.
  LOGGING_EXPORT std::ostream& operator << (std::ostream& out, level const& lvl);


} // namespace logging
