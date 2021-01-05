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
#include <fstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>



// Use c++ feature checking
#if defined __has_include

# if __has_include(<filesystem>)
#  define has_filesystem 1
# elif __has_include(<experimental/filesystem>)
#  define has_experimental_filesystem 1
# endif

// fallback to simple version checking
#elif (__cplusplus >= 201703L)

# define has_filesystem 1

#elif (_MSC_VER >= 1900) || (__GNUC__ > 5) || ((__GNUC__ == 5) && (__GNUC_MINOR__ > 3))

#  define has_experimental_filesystem 1

#endif


#if defined has_filesystem

# define USE_STD_FS
# include <filesystem>
namespace sys_fs = std::filesystem;

#elif defined has_experimental_filesystem

# define USE_STD_FS
# include <experimental/filesystem>
namespace sys_fs = std::experimental::filesystem;

#elif defined USE_BOOST

# define USE_STD_FS

# include <boost/exception/diagnostic_information.hpp>
# include <boost/filesystem.hpp>
namespace sys_fs = boost::filesystem;

#else

# include <unistd.h>

#endif

// --------------------------------------------------------------------------
//
// Library includes
//

#include <logging/core.h>
#include <logging/recorder.h>


#if !defined(LOGGING_BUILT_AS_STATIC_LIB)

DEFINE_LOGGING_CORE(LOGGING_EXPORT)

#else
namespace logging {

  extern core& get_logging_core();
}
// namespace logging
#endif

namespace logging {

#if (defined WIN32 || defined _WIN32 || defined WINCE || defined __CYGWIN__) && !defined(thread_local) && !defined(USE_MINGW)
# define thread_local __declspec(thread)
#endif

  thread_local const char* t_thread_name = "main";

  void core::logging_sink_call (core* core) {
    while (core->m_is_active) {
      record entry = core->m_messages.dequeue();
      do {
        core->log_to_sinks(std::move(entry));
      } while (core->m_messages.try_dequeue(entry));
    }
  }

  core::core ()
    : m_is_active(false)
    , m_line_id(0)
  {
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
      m_messages.wait_until_empty(std::chrono::milliseconds(
#ifdef NDEBUG
        500
#else
        2000
#endif
      ));
      m_is_active = false;
      m_messages.enqueue(record());
      m_sink_thread.join();
    }
  }

  void core::flush () {
    if (m_is_active) {
      m_messages.wait_until_empty(std::chrono::milliseconds(
#ifdef NDEBUG
        500
#else
        2000
#endif
      ));
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
                  std::string&& message) {
    log(lvl, std::chrono::system_clock::now(), std::move(message));
  }

  void core::log (level lvl,
                  std::chrono::system_clock::time_point time_point,
                  std::string&& message) {
    unsigned int id = ++m_line_id;
    record r(time_point, lvl, t_thread_name, line_id(id), std::move(message));
    if (m_is_active) {
      m_messages.enqueue(std::move(r));
      if (lvl >= level::error) {
        // wait up to 3 seconds until all messages are written!
        m_messages.wait_until_empty(std::chrono::milliseconds(1000));
      }
    } else {
      log_to_sinks(std::move(r));
    }
  }

  void core::log_to_sinks (record&& entry) {
    if (entry.level() != level::undefined) {
      std::lock_guard<std::mutex> lock(m_mutex);
      for (auto& s : m_sinks) {
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
    auto i = m_sinks.begin(), e = m_sinks.end();
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
    sys_fs::path next_path(next_name);
    if (sys_fs::exists(next_path)) {
      sys_fs::remove(next_path);
    }
#else
    if (access(next_name.c_str(), F_OK) != -1) {
      unlink(next_name.c_str());
    }
#endif

    if (num < maxnum) {
      std::string curr_name(name);

      if (num > 0) {
        numstr = ".";
        numstr += std::to_string(num);
        if (point_pos > 0)
          curr_name.insert(point_pos, numstr);
        else
          curr_name += numstr;
      }

#ifdef USE_STD_FS
      if (next_path.has_parent_path()) {
        sys_fs::create_directories(next_path.parent_path());
      }

      sys_fs::path curr_path(curr_name);
      if (sys_fs::exists(curr_path)) {
        sys_fs::rename(curr_path, next_path);
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
        std::ostringstream buf;
        buf << "Exception in core::rename_file_with_max_count:" << ex.what();
        instance().log(level::error, buf.str());
      }
    }
  }

  // ---------------------------------------------------------------------------
  std::string core::build_temp_log_file_name (const std::string& name) {
#ifdef USE_STD_FS
    sys_fs::path tmp = sys_fs::temp_directory_path();
    tmp /= name;
    return tmp.string();
#else
    return tempnam(P_tmpdir, name.c_str());
#endif
  }

} // namespace logging

