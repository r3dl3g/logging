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
#include <condition_variable>
#include <mutex>
#include <queue>
#ifdef USE_MINGW
#include <mingw/mingw.condition_variable.h>
#include <mingw/mingw.mutex.h>
#endif

// --------------------------------------------------------------------------
//
// Library includes
//
#include <logging/record.h>


#ifdef WIN32
#pragma warning (disable: 4251)
#endif

/**
* Provides an API for stream logging to multiple sinks.
*/
namespace logging {

  /**
    * Blocking (waiting) thread safe fifo queue.
    */
  struct LOGGING_EXPORT message_queue {

    /// Enqueue an item and send signal to a waiting dequeuer.
    void enqueue (record&& t);

    /// Dequeue an item if available, else waits until a new item is enqueued.
    record dequeue ();

    /// Dequeue an item if available and return true, else return false.
    bool try_dequeue (record& t);

    /// Waits until the queue is empty for maximum timeout time span.
    void wait_until_empty (const std::chrono::milliseconds& timeout);

    /// Waits until the queue is no more empty.
    void wait_until_not_empty (std::unique_lock<std::mutex> &lock);

  private:
    /// The queue to store the items in.
    std::queue<record> m_queue;

    /// Condition to signal new item to dequeuer.
    std::condition_variable m_condition;

    /// Mutex for thread safe access to the queue.
    mutable std::mutex m_mutex;

  };

} // namespace logging
