#include "network/timer_event.h"

#include <cassert>
#include <stdexcept>

#include "common/utility.h"

#include <event2/event.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
TimerEvent::TimerEvent(const EventBase& event_base,
                       std::chrono::microseconds interval,
                       Event::Callback callback, void* context)
    : event_{event_base, -1, EV_PERSIST, callback, context},
      tv_{ToTimeval(interval)} {
  auto rcode = event_add(event_.libevent_handle(), &tv_);
  if (rcode != 0) {
    throw std::runtime_error{"event_add failed"};
  }
}

//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------
void TimerEvent::Reset() {
  assert(event_.libevent_handle() != nullptr);
  auto rcode = event_del(event_.libevent_handle());
  if (rcode != 0) {
    throw std::runtime_error{"event_del failed"};
  }
  rcode = event_add(event_.libevent_handle(), &tv_);
  if (rcode != 0) {
    throw std::runtime_error{"event_add failed"};
  }
}
}  // namespace lightstep
