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
* @brief     C++ API:dbgstream
*
* @file
*/

#pragma once

// --------------------------------------------------------------------------
//
// Common includes
//
#include <sstream>
#ifdef WIN32
#include <windows.h>
#endif // Win32
#ifdef UNIX
#include <iostream>
#endif

// --------------------------------------------------------------------------
//2
// Library includes
//
#include <logging/redirect_stream.h>
#include <logging/core.h>


/**
* Provides an API to stream into OutputDebugString.
*/
namespace logging {

  struct debug_log {
    debug_log ()
    {}

    void operator ()(const std::string& t) {
#ifdef WIN32
      ::OutputDebugString(t.c_str());
#else
      std::cerr << t << std::endl;
#endif // UNIX
    }
  };

  struct odebugstream : public oredirect_stream<debug_log> {

    odebugstream (level lvl, const record_formatter& fmt) {
      core::instance().add_sink(this, lvl, fmt);
    }

    ~odebugstream () {
      core::instance().remove_sink(this);
    }

  };


} // namespace logging
