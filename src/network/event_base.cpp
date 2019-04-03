#include "event_base.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

#include "common/utility.h"

#include <event2/event.h>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
EventBase::EventBase() {
  event_base_ = event_base_new();
  if (event_base_ == nullptr) {
    throw std::runtime_error{"event_base_new failed"};
  }
}

EventBase::EventBase(EventBase&& other) noexcept {
  event_base_ = other.event_base_;
  other.event_base_ = nullptr;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
EventBase::~EventBase() {
  if (event_base_ != nullptr) {
    event_base_free(event_base_);
  }
}

//------------------------------------------------------------------------------
// operator=
//------------------------------------------------------------------------------
EventBase& EventBase::operator=(EventBase&& other) noexcept {
  assert(this != &other);
  if (event_base_ != nullptr) {
    event_base_free(event_base_);
  }
  event_base_ = other.event_base_;
  other.event_base_ = nullptr;
  return *this;
}

//------------------------------------------------------------------------------
// Dispatch
//------------------------------------------------------------------------------
void EventBase::Dispatch() const {
  assert(event_base_ != nullptr);
  auto rcode = event_base_dispatch(event_base_);
  if (rcode != 0) {
    std::ostringstream oss;
    oss << "event_base_dispatch faild with rcode = " << rcode;
    throw std::runtime_error{oss.str()};
  }
}

//------------------------------------------------------------------------------
// LoopBreak
//------------------------------------------------------------------------------
void EventBase::LoopBreak() const {
  assert(event_base_ != nullptr);
  auto rcode = event_base_loopbreak(event_base_);
  if (rcode != 0) {
    throw std::runtime_error{"event_base_loopbreak failed"};
  }
}

//------------------------------------------------------------------------------
// OnTimeout
//------------------------------------------------------------------------------
void EventBase::OnTimeout(std::chrono::microseconds timeout,
                          Event::Callback callback, void* context) {
  auto tv = ToTimeval(timeout);
  auto rcode =
      event_base_once(event_base_, -1, EV_TIMEOUT, callback, context, &tv);
  if (rcode != 0) {
    throw std::runtime_error{"OnTimeout failed"};
  }
}
}  // namespace lightstep
