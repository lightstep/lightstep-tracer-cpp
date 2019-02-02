#include "network/event.h"

#include <event2/event.h>
#include <stdexcept>

#include "network/event_base.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
Event::Event(const EventBase& event_base, int file_descriptor, short options,
             Callback callback, void* context) {
  event_ = event_new(event_base.libevent_handle(), file_descriptor, options,
                     callback, context);
  if (event_ == nullptr) {
    throw std::runtime_error{"event_new failed"};
  }
}

Event::Event(Event&& other) noexcept : event_{other.event_} {
  other.event_ = nullptr;
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
Event::~Event() noexcept { FreeEvent(); }

//--------------------------------------------------------------------------------------------------
// operator=
//--------------------------------------------------------------------------------------------------
Event& Event::operator=(Event&& other) noexcept {
  FreeEvent();
  event_ = other.event_;
  other.event_ = nullptr;
  return *this;
}

//--------------------------------------------------------------------------------------------------
// FreeEvent
//--------------------------------------------------------------------------------------------------
void Event::FreeEvent() noexcept {
  if (event_ != nullptr) {
    event_free(event_);
  }
}
}  // namespace lightstep
