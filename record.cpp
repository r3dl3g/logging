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

// --------------------------------------------------------------------------
//
// Common includes
//
#include <iomanip>
#include <ostream>
#include <utility>


// --------------------------------------------------------------------------
//
// Library includes
//

#include <logging/record.h>


namespace logging {

  extern thread_local const char* t_thread_name;

  std::ostream& operator << (std::ostream& out, line_id const& id) {
    const auto fill = out.fill();
    const auto width = out.width();
    out << std::setw(4) << std::setfill('0') << id.n;
    out.fill(fill);
    out.width(width);
    return out;
  }

  record::record (const std::chrono::system_clock::time_point& time_point,
                  logging::level lvl,
                  std::string thread_name,
                  line_id&& line,
                  std::string&& message)
    : m_time_point(time_point)
    , m_level(lvl)
    , m_thread_name(std::move(thread_name))
    , m_line(line)
    , m_message(std::move(message))
  {}

  record::record ()
    : m_time_point(std::chrono::system_clock::time_point())
    , m_level(logging::level::undefined)
    , m_thread_name(t_thread_name)
  {}

} // namespace logging

