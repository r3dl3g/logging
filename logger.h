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

namespace clog {

#if defined(LOGGING_ENABLE_TRACE)
  struct trace : public logging::recorder {
    inline trace ()
      : logging::recorder(logging::level::trace)
    {}
  };
#else
  struct trace : public  logging::null_recoder {};
#endif // LOGGING_ENABLE_TRACE

#if defined(NDEBUG)
  struct debug : public  logging::null_recoder {};
#else
  struct debug : public logging::recorder {
    inline debug ()
      : logging::recorder(logging::level::debug)
    {}
  };
#endif // NDEBUG

  struct info : public logging::recorder {
    inline info ()
      : logging::recorder(logging::level::info)
    {}
  };

  struct warn : public logging::recorder {
    inline warn ()
      : logging::recorder(logging::level::warning)
    {}
  };

  struct error : public logging::recorder {
    inline error ()
      : logging::recorder(logging::level::error)
    {}
  };

  struct fatal : public logging::recorder {
    inline fatal ()
      : logging::recorder(logging::level::fatal)
    {}
  };
}
