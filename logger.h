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

// --------------------------------------------------------------------------
//
// Library includes
//
#include <logging/recorder.h>

#define NOT_LOGGING_ENABLE_TRACE

/// Log macro trace
#if defined(LOGGING_ENABLE_TRACE)
# define log_trace logging::recorder (logging::level::trace)
#else
# define log_trace logging::null_recoder ()
#endif // LOGGING_ENABLE_TRACE

/// Log macro debug
#if defined(NDEBUG)
# define log_debug logging::null_recoder ()
#else
# define log_debug logging::recorder (logging::level::debug)
#endif // NDEBUG

/// Log macro info
#define log_info  logging::recorder (logging::level::info)
/// Log macro warning
#define log_warn logging::recorder (logging::level::warning)
/// Log macro error
#define log_error logging::recorder (logging::level::error)
/// Log macro fatal
#define log_fatal logging::recorder (logging::level::fatal)

