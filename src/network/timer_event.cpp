#include "network/timer_event.h"

#include <stdexcept>
#include <cassert>

#include "common/utility.h"

#include <event2/event.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
TimerEvent::TimerEvent(const EventBase& event_base,
                       std::chrono::microseconds interval,
                       EventBase::EventCallback callback, void* context)
    : tv_{ToTimeval(interval)} {
  event_ =
      event_new(event_base.libevent_handle(), -1, EV_PERSIST, callback, context);
  if (event_ == nullptr) {
    throw std::runtime_error{"event_new failed"};
  }
  auto rcode = event_add(event_, &tv_);
  if (rcode != 0) {
    event_free(event_);
    throw std::runtime_error{"event_add failed"};
  }
}

TimerEvent::TimerEvent(TimerEvent&& other) noexcept
    : event_{other.event_}, tv_{other.tv_} {
  other.event_ = nullptr;
}

//--------------------------------------------------------------------------------------------------
// operator=
//--------------------------------------------------------------------------------------------------
TimerEvent& TimerEvent::operator=(TimerEvent&& other) noexcept {
  if (event_ != nullptr) {
    event_free(event_);
  }
  event_ = other.event_;
  tv_ = other.tv_;
  other.event_ = nullptr;
  return *this;
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
TimerEvent::~TimerEvent() noexcept { 
  if (event_ != nullptr) {
    event_free(event_);
  }
}

//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------
void TimerEvent::Reset() {
  assert(event_ != nullptr);
  auto rcode = event_del(event_);
  if (rcode != 0) {
    throw std::runtime_error{"event_del failed"};
  }
  rcode = event_add(event_, &tv_);
  if (rcode != 0) {
    throw std::runtime_error{"event_add failed"};
  }
}
}  // namespace lightstep
