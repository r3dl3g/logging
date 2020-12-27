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

namespace logging {

  inline sink::sink (std::ostream* stream,
                     level lvl,
                     const record_formatter& formatter)
    : m_stream(stream)
    , m_level(lvl)
    , m_formatter(formatter)
  {}


} // namespace logging
