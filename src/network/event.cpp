#include "network/event.h"

#include <event2/event.h>
#include <cassert>
#include <stdexcept>

#include "common/utility.h"
#include "network/event_base.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
Event::Event(const EventBase& event_base, FileDescriptor file_descriptor,
             short options, Callback callback, void* context) {
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
  assert(this != &other);
  FreeEvent();
  event_ = other.event_;
  other.event_ = nullptr;
  return *this;
}

//--------------------------------------------------------------------------------------------------
// Add
//--------------------------------------------------------------------------------------------------
void Event::Add(std::chrono::microseconds timeout) {
  assert(event_ != nullptr);
  auto tv = ToTimeval(timeout);
  Add(&tv);
}

void Event::Add(timeval* tv) {
  assert(event_ != nullptr);
  auto rcode = event_add(event_, tv);
  if (rcode != 0) {
    throw std::runtime_error{"event_add failed"};
  }
}

//--------------------------------------------------------------------------------------------------
// Remove
//--------------------------------------------------------------------------------------------------
void Event::Remove() {
  assert(event_ != nullptr);
  auto rcode = event_del(event_);
  if (rcode != 0) {
    throw std::runtime_error{"event_del failed"};
  }
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
