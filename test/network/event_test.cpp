#include "network/event.h"
#include "network/event_base.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

TEST_CASE("Event") {
  auto callback = [](intptr_t /*file_descriptor*/, short /*what*/,
                     void* /*context*/) {};

  EventBase event_base;
  Event event(event_base, -1, 0, callback, nullptr);
  REQUIRE(event.libevent_handle() != nullptr);

  SECTION("Event can be default constructed.") {
    Event event;
    REQUIRE(event.libevent_handle() == nullptr);
  }

  SECTION("Event can be move constructed.") {
    auto handle = event.libevent_handle();
    Event event2{std::move(event)};
    REQUIRE(event2.libevent_handle() == handle);
    REQUIRE(event.libevent_handle() == nullptr);
  }

  SECTION("Event can be move assigned.") {
    Event event2 (event_base, -1, 0, (void (*)(intptr_t, short, void*)) (callback), nullptr);
    auto handle = event.libevent_handle();
    event2 = std::move(event);
    REQUIRE(event2.libevent_handle() == handle);
    REQUIRE(event.libevent_handle() == nullptr);
  }
}
