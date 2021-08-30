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
// Library includes
//
#include <logging/recorder.h>

namespace logging {

#if defined(LOGGING_ENABLE_TRACE)
  struct trace : public recorder {
    inline trace ()
      : recorder(level::trace)
    {}
  };
#else
  struct trace : public  null_recoder {};
#endif // LOGGING_ENABLE_TRACE

#if defined(NDEBUG)
  struct debug : public  null_recoder {};
#else
  struct debug : public recorder {
    inline debug ()
      : recorder(level::debug)
    {}
  };
#endif // NDEBUG

  struct info : public recorder {
    inline info ()
      : recorder(level::info)
    {}
  };

  struct warn : public recorder {
    inline warn ()
      : recorder(level::warning)
    {}
  };

  struct error : public recorder {
    inline error ()
      : recorder(level::error)
    {}
  };

  struct fatal : public recorder {
    inline fatal ()
      : recorder(level::fatal)
    {}
  };

} // namespace logging
