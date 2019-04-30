#pragma once

#include <chrono>

#include "network/event.h"

struct event_base;

namespace lightstep {
/**
 * Wrapper for libevent's event_base struct.
 */
class EventBase {
 public:
  EventBase();

  EventBase(EventBase&& other) noexcept;
  EventBase(const EventBase&) = delete;

  ~EventBase();

  EventBase& operator=(EventBase&& other) noexcept;
  EventBase& operator=(const EventBase&) = delete;

  /**
   * @return the underlying event_base.
   */
  event_base* libevent_handle() const noexcept { return event_base_; }

  /**
   * Run the dispatch event loop.
   */
  void Dispatch() const;

  /**
   * Break out of the dispatch event loop.
   */
  void LoopBreak() const;

  /**
   * Schedule a callback to be invoked.
   * @param timeout indicates when to invoke the callback.
   * @param callback supplies the callback to invoke.
   * @param context supplies an arbitrary argument to pass to the callback.
   */
  void OnTimeout(std::chrono::microseconds timeout, Event::Callback callback,
                 void* context);

  /**
   * Schedule a callback to be invoked.
   *
   * See other OnTimeout method.
   */
  template <class Rep, class Period>
  void OnTimeout(std::chrono::duration<Rep, Period> timeout,
                 Event::Callback callback, void* context) {
    OnTimeout(std::chrono::duration_cast<std::chrono::microseconds>(timeout),
              callback, context);
  }

 private:
  event_base* event_base_;
};
}  // namespace lightstep
