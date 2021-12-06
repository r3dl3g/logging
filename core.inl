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

namespace logging {

  inline sink::sink (std::ostream* stream,
                     level lvl,
                     const record_formatter& formatter)
    : m_stream(stream)
    , m_level(lvl)
    , m_formatter(formatter)
  {}


} // namespace logging
