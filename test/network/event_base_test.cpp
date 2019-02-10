#include <chrono>

#include "network/event_base.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

TEST_CASE("EventBase") {
  EventBase event_base;

  SECTION("EventBase can be move constructed.") {
    auto libevent_handle = event_base.libevent_handle();
    REQUIRE(libevent_handle != nullptr);
    EventBase event_base2{std::move(event_base)};
    REQUIRE(event_base2.libevent_handle() == libevent_handle);
  }

  SECTION("EventBase can be move assigned.") {
    auto libevent_handle = event_base.libevent_handle();
    REQUIRE(libevent_handle != nullptr);
    EventBase event_base2;
    event_base2 = std::move(event_base);
    REQUIRE(event_base2.libevent_handle() == libevent_handle);
  }

  SECTION("OnTimeout can be used to schedule a 1-off event.") {
    bool was_called = false;
    auto callback = [](int /*socket*/, short /*what*/, void* context) {
      *static_cast<bool*>(context) = true;
    };
    auto exit_callback = [](int /*socket*/, short /*what*/, void* context) {
      static_cast<EventBase*>(context)->LoopBreak();
    };
    event_base.OnTimeout(std::chrono::milliseconds{5}, callback,
                         static_cast<void*>(&was_called));
    event_base.OnTimeout(std::chrono::milliseconds{10}, exit_callback,
                         static_cast<void*>(&event_base));
    event_base.Dispatch();
  }
}
