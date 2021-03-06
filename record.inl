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

namespace logging {

  inline line_id::line_id (unsigned int i)
    : n(i)
  {}

  inline line_id::line_id ()
    : n(0)
  {}

  inline const line_id& record::line () const {
    return m_line;
  }

  inline const std::chrono::system_clock::time_point& record::time_point () const {
    return m_time_point;
  }

  inline const level& record::level () const {
    return m_level;
  }

  inline const std::string& record::thread_name () const {
    return m_thread_name;
  }

  inline const std::string& record::message () const {
    return m_message;
  }

} // namespace logging
