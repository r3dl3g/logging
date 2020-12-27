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
* @brief     C++ API:log level
*
* @file
*/

#pragma once

#include <logging/core.h>
#include <fstream>

/**
* Provides an API for stream logging to multiple sinks.
*/
namespace logging {

  /**
  * log to a file.
  * Automatic adds and remove itself to the logging core.
  */
  class file_logger {
  public:
    file_logger (const std::string& name, level lvl, const record_formatter& fmt)
      : file(name, std::ios_base::out|std::ios_base::ate)
    {
      core::instance().add_sink(&file, lvl, fmt);
    }

    ~file_logger () {
      core::instance().remove_sink(&file);
      file.close();
    }

  private:
    std::ofstream file;
  };

} // namespace logging
