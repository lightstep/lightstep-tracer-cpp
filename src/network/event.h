#pragma once

#include <chrono>

#include "common/platform/network.h"
#include "common/platform/time.h"

struct event;

namespace lightstep {
class EventBase;

/**
 * Wrapper for libevent's event struct.
 */
class Event {
 public:
  using Callback = void (*)(FileDescriptor socket, short what, void* context);

  Event() noexcept = default;

  Event(const EventBase& event_base, FileDescriptor file_descriptor,
        short options, Callback callback, void* context);

  Event(const Event&) = delete;

  Event(Event&& other) noexcept;

  ~Event() noexcept;

  Event& operator=(const Event&) = delete;
  Event& operator=(Event&& other) noexcept;

  /**
   * @return the underlying event.
   */
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

/**
 * For a given method, creates an event callback function that can be passed to
 * libevent.
 * @return a function pointer that can be passed to libevent.
 */
template <class T, void (T::*MemberFunction)(FileDescriptor, short)>
Event::Callback MakeEventCallback() {
  return [](FileDescriptor socket, short what, void* context) {
    (static_cast<T*>(context)->*MemberFunction)(socket, what);
  };
}
}  // namespace lightstep
