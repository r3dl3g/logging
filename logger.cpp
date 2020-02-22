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
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>


#if ((_MSC_VER >= 1900) && defined(EXPERIMENTAL)) || (__GNUC__ > 5) || ((__GNUC__ == 5) && (__GNUC_MINOR__ >3)) || ((__clang__) && !__APPLE__)
# include <experimental/filesystem>
  using namespace std::experimental;
# define USE_STD_FS
#elif !defined(NO_BOOST)
# define USE_BOOST
# include <boost/exception/diagnostic_information.hpp>
# include <boost/filesystem.hpp>
  using namespace boost;
# define USE_STD_FS
#else
# include <unistd.h>
#endif

// --------------------------------------------------------------------------
//
// Library includes
//
#include <util/time_util.h>
#include <util/ostream_resetter.h>

#include "logger.h"


#if !defined(LOGGING_BUILT_AS_STATIC_LIB)

DEFINE_LOGGING_CORE(LOGGING_EXPORT)

#else
namespace logging {

  extern core& get_logging_core();
}
// namespace logging
#endif


namespace logging {

#if (defined WIN32 || defined _WIN32 || defined WINCE || defined __CYGWIN__) && !defined(thread_local)
# define thread_local __declspec(thread)
#endif

  static thread_local const char* t_thread_name = "main";

  static const char* s_log_level_strings[] = {
    "undef", "trace", "debug", "info ", "warn ", "error", "fatal"
  };

  std::ostream& operator << (std::ostream& out, level const& lvl) {
    out << s_log_level_strings[static_cast<int>(lvl)];
    return out;
  }

  std::ostream& operator << (std::ostream& out, std::chrono::system_clock::time_point const& tp) {
    return util::time::format_datetime(out, tp, "-", " ", ":", true);
  }

  std::ostream& operator << (std::ostream& out, line_id const& id) {
    util::ostream_resetter r(out);
    out << std::setw(4) << std::setfill('0') << id.n;
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

  /// Enqueue an item and send signal to a waiting dequeuer.
  void message_queue::enqueue (record&& t) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_queue.push(std::move(t));
    }
    m_condition.notify_all();
  }

  /// Dequeue an item if available, else waits until a new item is enqueued.
  record message_queue::dequeue () {
    std::unique_lock<std::mutex> lock(m_mutex);

    wait_until_not_empty(lock);

    if (m_queue.empty()) {
      return record();
    }

    record item = m_queue.front();
    m_queue.pop();
    m_condition.notify_all();
    return item;
  }

  /// Dequeue an item if available and return true, else return false.
  bool message_queue::try_dequeue (record& t) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_queue.empty()) {
      return false;
    }

    t = m_queue.front();
    m_queue.pop();
    m_condition.notify_all();
    return true;
  }

  /// Waits until the queue is empty for maximum timeout time span.
  void message_queue::wait_until_empty (const std::chrono::milliseconds& timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_condition.wait_for(lock, timeout, [&]() {
      return m_queue.empty();
    });
  }

  /// Waits until the queue is no more empty.
  void message_queue::wait_until_not_empty (std::unique_lock<std::mutex> &lock) {
    m_condition.wait(lock, [&] () -> bool {
                       return !m_queue.empty();
                     });
  }

  void core::logging_sink_call (core* core) {
    while (core->m_is_active) {
      record entry = core->m_messages.dequeue();
      do {
        core->log_to_sinks(entry);
      } while (core->m_messages.try_dequeue(entry));
    }
  }

  record::record (const std::chrono::system_clock::time_point& time_point,
                  logging::level lvl,
                  const std::string& thread_name,
                  const line_id& line,
                  const std::string& message)
    : m_time_point(time_point)
    , m_level(lvl)
    , m_thread_name(thread_name)
    , m_line(line)
    , m_message(message)
  {}

  record::record ()
    : m_time_point(std::chrono::system_clock::time_point())
    , m_level(logging::level::undefined)
    , m_thread_name(t_thread_name)
  {}

  core::core ()
    : m_is_active(false)
  {
    m_line_id = 0;
    start();
  }

  core::~core () {
    finish();
  }

  void core::start () {
    if (!m_is_active) {
      m_is_active = true;
      add_sink(&std::clog,
         #ifdef NDEBUG
               level::info
         #else
               level::trace
         #endif
               , get_standard_formatter());

      m_sink_thread = std::thread(core::logging_sink_call, this);
    }
  }

  void core::finish () {
    if (m_is_active) {
      m_messages.wait_until_empty(std::chrono::milliseconds(500));
      m_is_active = false;
      m_messages.enqueue(record());
      m_sink_thread.join();
    }
  }

  void core::flush () {
    if (m_is_active) {
      m_messages.wait_until_empty(std::chrono::milliseconds(500));
    }
  }

  record_formatter core::get_standard_formatter () {
    return [](std::ostream& out, const record& e) {
      out << e.line() << '|' << e.time_point() << '|' << e.level() << '|' << e.thread_name() << '|' << e.message() << std::endl;
    };
  }

  record_formatter core::get_console_formatter () {
    return [](std::ostream& out, const record& e) {
      out << e.message() << std::endl;
    };
  }

  void core::log (level lvl,
                  const std::string& message) {
    log(lvl, std::chrono::system_clock::now(), message);
  }

  void core::log (level lvl,
                  const std::chrono::system_clock::time_point& time_point,
                  const std::string& message) {
    unsigned int id = ++m_line_id;
    record r(time_point, lvl, t_thread_name, id, message);
    if (m_is_active) {
      m_messages.enqueue(std::move(r));
      if (lvl >= level::error) {
        // wait up to 3 seconds until all messages are written!
        m_messages.wait_until_empty(std::chrono::milliseconds(1000));
      }
    } else {
      log_to_sinks(r);
    }
  }

  void core::log_to_sinks (const record& entry) {
    if (entry.level() != level::undefined) {
      std::lock_guard<std::mutex> lock(m_mutex);
      for (sink_list::iterator i = m_sinks.begin(), e = m_sinks.end(); i != e; ++i) {
        sink& s = *i;
        if (entry.level() >= s.m_level) {
          try {
            s.m_formatter(*s.m_stream, entry);
            s.m_stream->flush();
          } catch (const std::exception& ex) {
            std::cerr << "core::log_to_sinks:" << ex.what();
          }
        }
      }
    }
  }

  void core::add_sink (std::ostream* stream,
                       level lvl,
                       const record_formatter& formatter) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sinks.push_back(sink(stream, lvl, formatter));
  }

  void core::remove_sink (std::ostream* stream) {
    std::lock_guard<std::mutex> lock(m_mutex);
    sink_list::iterator i = m_sinks.begin(), e = m_sinks.end();
    i = std::find_if(i, e, [=](sink& s) { return s.m_stream == stream; });
    if (i != e) {
      m_sinks.erase(i);
    }
  }

  void core::remove_all_sinks () {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sinks.clear();
  }

  void core::set_thread_name (const char* name) {
    t_thread_name = name;
  }

  core& core::instance() {
    return get_logging_core();
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

  // ---------------------------------------------------------------------------
  void rename_file_with_max_count (const std::string& name,
                                   int num,
                                   int maxnum) {
    std::string numstr(".");
    numstr += std::to_string(num + 1);

    std::string next_name = name;

    std::string::size_type point_pos = next_name.rfind('.');
    if (point_pos > 0) {
      next_name.insert(point_pos, numstr);
    } else {
      next_name += numstr;
    }

#ifdef USE_STD_FS
    filesystem::path next_path(next_name);
    if (filesystem::exists(next_path)) {
      filesystem::remove(next_path);
    }
#else
    if (access(next_name.c_str(), F_OK) != -1) {
      unlink(next_name.c_str());
    }
#endif

    if (num < maxnum) {
      std::string curr_name(name);

      if (num > 0) {
        std::string numstr(".");
        numstr += std::to_string(num);
        if (point_pos > 0)
          curr_name.insert(point_pos, numstr);
        else
          curr_name += numstr;
      }

#ifdef USE_STD_FS
      if (next_path.has_parent_path()) {
        filesystem::create_directories(next_path.parent_path());
      }

      filesystem::path curr_path(curr_name);
      if (filesystem::exists(curr_path)) {
        filesystem::rename(curr_path, next_path);
      }
#else
      if (access(curr_name.c_str(), F_OK) != -1) {
        rename(curr_name.c_str(), next_name.c_str());
      }
#endif

    }
  }

  // ---------------------------------------------------------------------------
  void core::rename_file_with_max_count (const std::string& name,
                                         int maxnum) {
    for (int i = maxnum - 1; i >= 0; --i) {
      try {
        logging::rename_file_with_max_count(name, i, maxnum);
      } catch (const std::exception& ex) {
        LogError << "Exception in core::rename_file_with_max_count:" << ex.what();
      }
    }
  }

  // ---------------------------------------------------------------------------
  std::string core::build_temp_log_file_name (const std::string& name) {
#ifdef USE_STD_FS
    filesystem::path tmp = filesystem::temp_directory_path();
    tmp /= name;
    return tmp.string();
#else
    return tempnam(P_tmpdir, name.c_str());
#endif
  }

} // namespace logging

