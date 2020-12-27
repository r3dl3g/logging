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
* @brief     C++ API:logger
*
* @file
*/

// --------------------------------------------------------------------------
//
// Common includes
//
#include <iomanip>

// --------------------------------------------------------------------------
//
// Library includes
//

#include <logging/recorder.h>
#include <logging/core.h>


namespace logging {

  inline std::tm time_t2tm (const std::time_t now) {
    // --------------------------------------------------------------------------
    std::tm t;
#ifdef WIN32
    localtime_s(&t, &now);
#else
    localtime_r(&now, &t);
#endif
    return t;
  }

  std::ostream& operator << (std::ostream& out, std::chrono::system_clock::time_point const& tp) {
    std::time_t now = std::chrono::system_clock::to_time_t(tp);
    std::tm t = time_t2tm(now);

    const auto fill = out.fill();
    const auto width = out.width();

    out << std::setfill('0')
        << (t.tm_year + 1900) << '-'
        << std::setw(2) << (t.tm_mon + 1) << '-'
        << std::setw(2) << t.tm_mday << ' '
        << std::setw(2) << t.tm_hour << ':'
        << std::setw(2) << t.tm_min << ':'
        << std::setw(2) << t.tm_sec;

    auto t0 = std::chrono::time_point_cast<std::chrono::seconds>(tp);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(tp - t0);
    out << '.' << std::setfill('0') << std::setw(6) << micros.count();

    out.fill(fill);
    out.width(width);

    return out;
  }

  std::ostream& operator << (std::ostream& out, const std::exception& ex) {
#if !defined(NDEBUG) && defined(USE_BOOST)
    out << boost::current_exception_diagnostic_information();
#else
    out << ex.what();
#endif
    return out;
  }

  void escape_filter (std::ostream& out,
                      char ch) {
    switch (ch) {
      case '\0': out << "\\0"; break;
      case '\a': out << "\\a"; break;
      case '\b': out << "\\b"; break;
      case '\f': out << "\\f"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      case '\v': out << "\\v"; break;
      default:   out << ch; break;
    }
  }

  recorder::recorder (logging::level lvl)
    : m_time_point(std::chrono::system_clock::now())
    , m_level(lvl)
    , unescaped(false)
  {}

  recorder::~recorder () {
    core::instance().log(m_level, m_time_point, m_buffer.str());
  }

  recorder& recorder::operator<< (const char value) {
    if (unescaped) {
      m_buffer << value;
    } else {
      escape_filter(m_buffer, value);
    }
    return *this;
  }

  recorder& recorder::operator<< (const char* value) {
    if (value) {
      if (unescaped) {
        m_buffer << value;
      } else {
        while (*value) {
          escape_filter(m_buffer, *value);
          ++value;
        }
      }
    }
    return *this;
  }

  recorder& recorder::operator<< (const flush&) {
    core::instance().flush();
    return *this;
  }

  recorder& recorder::endl () {
    m_buffer << std::endl;
    return *this;
  }

  recorder& recorder::raw () {
    unescaped = true;
    return *this;
  }

  recorder& recorder::escaped () {
    unescaped = false;
    return *this;
  }

} // namespace logging

