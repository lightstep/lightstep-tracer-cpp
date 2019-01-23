#pragma once

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
             EventBase::EventCallback callback, void* context)
      : TimerEvent{
            event_base,
            std::chrono::duration_cast<std::chrono::microseconds>(interval),
            callback, context} {}

  TimerEvent(const EventBase& event_base, std::chrono::microseconds interval,
             EventBase::EventCallback callback, void* context);

  ~TimerEvent() noexcept;

  TimerEvent(TimerEvent&& other) noexcept;
  TimerEvent(const TimerEvent&) = delete;

  TimerEvent& operator=(TimerEvent&& other) noexcept;
  TimerEvent& operator=(const TimerEvent&) = delete;

  /**
   * Resets the timer to start from now.
   */
  void Reset();

  /**
   * @return the underlying event associated with this timer.
   */
  const event* libevent_handle() const noexcept { return event_; }

 private:
  event* event_;
  timeval tv_;
  std::chrono::microseconds interval_;
};
}  // namespace lightstep
