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

// --------------------------------------------------------------------------
//
// Common includes
//


// --------------------------------------------------------------------------
//
// Library includes
//
#include "message_queue.h"


namespace logging {

  /// Enqueue an item and send signal to a waiting dequeuer.
  void message_queue::enqueue (record&& t) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_queue.push(std::move(t));
    }
    m_condition.notify_all();
  }

  /// Dequeue an item if available, else waits until a new item is enqueued.
  record message_queue::dequeue () {
    std::unique_lock<std::mutex> lock(m_mutex);

    wait_until_not_empty(lock);

    if (m_queue.empty()) {
      return record();
    }

    record item = m_queue.front();
    m_queue.pop();
    m_condition.notify_all();
    return item;
  }

  /// Dequeue an item if available and return true, else return false.
  bool message_queue::try_dequeue (record& t) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_queue.empty()) {
      return false;
    }

    t = m_queue.front();
    m_queue.pop();
    m_condition.notify_all();
    return true;
  }

  /// Waits until the queue is empty for maximum timeout time span.
  void message_queue::wait_until_empty (const std::chrono::milliseconds& timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_condition.wait_for(lock, timeout, [&]() {
      return m_queue.empty();
    });
  }

  /// Waits until the queue is no more empty.
  void message_queue::wait_until_not_empty (std::unique_lock<std::mutex> &lock) {
    m_condition.wait(lock, [&] () -> bool {
                       return !m_queue.empty();
                     });
  }

} // namespace logging

