#pragma once

#include "network/event.h"
#include "network/event_base.h"

#include <sys/time.h>

struct event;

namespace lightstep {
/**
 * Manages a periodic timer event.
 */
class TimerEvent {
 public:
  template <class Rep, class Period>
  TimerEvent(const EventBase& event_base,
             std::chrono::duration<Rep, Period> interval,
             Event::Callback callback, void* context)
      : TimerEvent{
            event_base,
            std::chrono::duration_cast<std::chrono::microseconds>(interval),
            callback, context} {}

  TimerEvent(const EventBase& event_base, std::chrono::microseconds interval,
             EventBase::EventCallback callback, void* context);

  /**
   * Resets the timer to start from now.
   */
  void Reset();

  /**
   * @return the underlying event associated with this timer.
   */
  const event* libevent_handle() const noexcept {
    return event_.libevent_handle();
  }

 private:
  Event event_;
  timeval tv_;
};
}  // namespace lightstep
