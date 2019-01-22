#pragma once

#include <chrono>

struct event_base;

namespace lightstep {
class EventBase {
 public:
  using EventCallback = void (*)(int socket, short what, void* context);

  EventBase();

  EventBase(EventBase&& other) noexcept;
  EventBase(const EventBase&) = delete;

  ~EventBase();

  EventBase& operator=(EventBase&& other) noexcept;
  EventBase& operator=(const EventBase&) = delete;

  event_base* libevent_handle() const noexcept { return event_base_; }

  void Dispatch() const;

  void LoopBreak() const;

  void OnTimeout(std::chrono::microseconds timeout, EventCallback callback,
                 void* context);

  template <class Rep, class Period>
  void OnTimeout(std::chrono::duration<Rep, Period> timeout,
                 EventCallback callback, void* context) {
    OnTimeout(std::chrono::duration_cast<std::chrono::microseconds>(timeout),
              callback, context);
  }

 private:
  event_base* event_base_;
};
}  // namespace lightstep
