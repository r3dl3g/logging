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

// --------------------------------------------------------------------------
//
// Common includes
//
#include <sstream>
#ifdef WIN32
#include <windows.h>
#else
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

    ~odebugstream () override {
      core::instance().remove_sink(this);
    }

  };


} // namespace logging
