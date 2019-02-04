#pragma once

#include <chrono>

#include <sys/time.h>

struct event;

namespace lightstep {
class EventBase;

class Event {
 public:
  using Callback = void (*)(int socket, short what, void* context);

  Event() noexcept = default;

  Event(const EventBase& event_base, int file_descriptor, short options,
        Callback callback, void* context);

  Event(const Event&) = delete;

  Event(Event&& other) noexcept;

  ~Event() noexcept;

  Event& operator=(const Event&) = delete;
  Event& operator=(Event&& other) noexcept;

  event* libevent_handle() const noexcept { return event_; }

  /**
   * Adds an event to dispatching.
   * @param timeout supplies the timeout for the event.
   */
  void Add(std::chrono::microseconds timeout);

  /**
   * Adds an event to dispatching.
   * @param timeout supplies the timeout for the event. Null can be used to set
   * no timeout.
   */
  void Add(timeval* tv);

  /**
   * Removes an event from dispatching.
   */
  void Remove();

 private:
  event* event_{nullptr};

  void FreeEvent() noexcept;
};

//--------------------------------------------------------------------------------------------------
// MakeEventCallback
//--------------------------------------------------------------------------------------------------
template <class T, void (T::*MemberFunction)(int, short)>
Event::Callback MakeEventCallback() {
  return [](int socket, short what, void* context) {
    (static_cast<T*>(context)->*MemberFunction)(socket, what);
  };
}
}  // namespace lightstep
