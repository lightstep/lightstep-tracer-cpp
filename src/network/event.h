#pragma once

struct event;

namespace lightstep {
struct EventBase;

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

 private:
  event* event_{nullptr};

  void FreeEvent() noexcept;
};
}  // namespace lightstep
