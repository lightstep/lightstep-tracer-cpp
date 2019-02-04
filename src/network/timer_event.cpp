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
  event_.Add(&tv_);
}

TimerEvent::TimerEvent(const EventBase& event_base, Event::Callback callback,
                       void* context)
    : event_{event_base, -1, EV_PERSIST, callback, context} {}

//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------
void TimerEvent::Reset() {
  event_.Remove();
  event_.Add(&tv_);
}

void TimerEvent::Reset(const timeval* tv) {
  event_.Remove();
  if (tv == nullptr) {
    return;
  }
  tv_ = *tv;
  event_.Add(&tv_);
}
}  // namespace lightstep
