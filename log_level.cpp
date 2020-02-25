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

// --------------------------------------------------------------------------
//
// Common includes
//
#include <ostream>

// --------------------------------------------------------------------------
//
// Library includes
//

#include <logging/log_level.h>


namespace logging {

  static const char* s_log_level_strings[] = {
    "undef", "trace", "debug", "info ", "warn ", "error", "fatal"
  };

  std::ostream& operator << (std::ostream& out, level const& lvl) {
    out << s_log_level_strings[static_cast<int>(lvl)];
    return out;
  }

} // namespace logging

