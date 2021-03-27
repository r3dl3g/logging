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

#pragma once

// --------------------------------------------------------------------------
//
// Common includes
//
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#ifdef USE_MINGW
#include <mingw/mingw.thread.h>
#endif

// --------------------------------------------------------------------------
//
// Library includes
//
#include <logging/message_queue.h>

#ifdef WIN32
#pragma warning (disable: 4251)
#endif

/**
* Provides an API for stream logging to multiple sinks.
*/
namespace logging {

  /**
    * Formatter functor definition.
    */
  typedef std::function<void(std::ostream& out, const record& entry)> record_formatter;

  /**
    * Sink description with target ostream, level to log and log record formatter
    */
  struct LOGGING_EXPORT sink {
    sink (std::ostream* stream,
          level lvl,
          const record_formatter& formatter);

    std::ostream* m_stream;
    level m_level;
    record_formatter m_formatter;
  };

  /**
    * Logging core. Manage sinks and thread safe logging.
    */
  class LOGGING_EXPORT core {
  public:
    core ();
    ~core ();

    /// start the logging core
    void start ();

    /// finish the logging core
    void finish ();

    /// flush cashed entries to the sinks
    void flush ();

    /// add a log entry with current time point to the cache
    void log (level lvl, std::string&& message);

    /// add a log entry with specific time point to the cache
    void log (level lvl, std::chrono::system_clock::time_point time_point, std::string&& message);

    /// add a sink with a formatter
    void add_sink (std::ostream* stream, level lvl, const record_formatter& formatter);

    /// remove a sink
    void remove_sink (std::ostream* stream);

    /// remove all sinks
    void remove_all_sinks ();

    /// get a standard formatter
    static record_formatter get_standard_formatter ();

    /// get a standard formatter without time point prefix
    static record_formatter get_no_time_formatter ();

    /// get a simplified console formatter
    static record_formatter get_console_formatter ();

    /// get the singleton core instance.
    static core& instance ();

    /// helper to rename files with a number and a given maximum number
    static void rename_file_with_max_count (const std::string& name, int maxnum);

    /// helper to build a temporary file name
    static std::string build_temp_log_file_name (const std::string& name);

    /// Thread name. Thread specific variable to hold the name of the thread.
    static void set_thread_name (const char* name);

    core (const core&) = delete;
    void operator= (const core&) = delete;

  protected:
    friend class record;

  private:
    static void logging_sink_call (core* core);

    void log_to_sinks (record&& entry);

    volatile bool m_is_active;
    std::atomic_uint m_line_id{};

    std::mutex m_mutex;
    typedef std::vector<sink> sink_list;

    sink_list m_sinks;

    message_queue m_messages;
    std::thread m_sink_thread;
  };

} // namespace logging

#include <logging/core.inl>

/**
* Since statics are not shared over DLL boundaries, we have
* to handle this in a special manner.
* If you build logging as static library, we have to decide in
* which module the core of logging resists.
*/
#if !defined(LOGGING_BUILT_AS_STATIC_LIB)
namespace logging {
  LOGGING_EXPORT core& get_logging_core();
} // namespace logging
#endif

/**
* Macro to define the login core singleton.
*
* Since statics are not shared over DLL boundaries, implement this somewhere
* where you can ensure, that it will only be instantiated once.
* Usualy this will be in the main module, where your gui_main resist.
*
*
* in C++11 statics are "magic statics" and their initialization _is_ thread safe.
* @see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2660.htm
*
* Unfortunately Visual Studio 2013 doesn't support them yet.
* @see https://msdn.microsoft.com/library/hh567368(v=vs.130).aspx
*/
#if (_MSC_VER >= 1900) || !defined(_MSC_VER)

#define DEFINE_LOGGING_CORE(EXP) \
namespace logging {\
  EXP logging::core& get_logging_core() {\
    static logging::core s_logging_core; \
    return s_logging_core; \
  }\
} // namespace logging

#else // _MSC_VER < 1900

/**
* Macro to define the login core singleton with the "double-check, single-lock" pattern.
* No longer needed for modern C++11 compilers, except Visual Studio 2013.
*/
#define DEFINE_LOGGING_CORE(EXP) \
namespace logging {\
  static std::atomic<core*> s_logging_core;\
  static std::mutex s_logging_core_mutex;\
  EXP core& get_logging_core() {\
  core* temp_core = s_logging_core.load();\
  if (nullptr == temp_core) {\
    std::lock_guard<std::mutex> lock(s_logging_core_mutex);\
    temp_core = s_logging_core.load();\
      if (nullptr == temp_core) {\
        temp_core = new core();\
        s_logging_core.store(temp_core);\
      }\
    }\
    return *temp_core;\
  }\
} // namespace logging

#endif
