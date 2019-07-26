#pragma once

#include "common/platform/time.h"
#include "network/event.h"
#include "network/event_base.h"

struct event;

namespace lightstep {
/**
 * Manages a periodic timer event.
 */
class TimerEvent {
 public:
  TimerEvent() noexcept = default;

  template <class Rep, class Period>
  TimerEvent(const EventBase& event_base,
             std::chrono::duration<Rep, Period> interval,
             Event::Callback callback, void* context)
      : TimerEvent{
            event_base,
            std::chrono::duration_cast<std::chrono::microseconds>(interval),
            callback, context} {}

  TimerEvent(const EventBase& event_base, std::chrono::microseconds interval,
             Event::Callback callback, void* context);

  TimerEvent(const EventBase& event_base, Event::Callback callback,
             void* context);

  /**
   * Resets the timer to start anew from now.
   */
  void Reset();

  void Reset(const timeval* tv);

 private:
  Event event_;
  timeval tv_{};
};

/**
 * For a given method, creates a timer callback function that can be passed to
 * libevent.
 * @return a function pointer that can be passed to libevent.
 */
template <class T, void (T::*MemberFunction)()>
Event::Callback MakeTimerCallback() noexcept {
  return [](FileDescriptor /*socket*/, short /*what*/, void* context) noexcept {
    (static_cast<T*>(context)->*MemberFunction)();
  };
}
}  // namespace lightstep
