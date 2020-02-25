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

namespace logging {

  template <typename T>
  inline recorder& recorder::operator<< (T const& value) {
    m_buffer << value;
    return *this;
  }

  inline recorder& recorder::operator<< (const std::string& value) {
    return operator<<(value.c_str());
  }

  inline recorder::operator std::ostream& () {
    return m_buffer;
  }

  inline std::ostream& recorder::stream () {
    return m_buffer;
  }

  inline bool recorder::is_raw () const {
    return unescaped;
  }


} // namespace logging
