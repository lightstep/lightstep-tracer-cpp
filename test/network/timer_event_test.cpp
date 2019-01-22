#include <vector>

#include "3rd_party/catch2/catch.hpp"
#include "network/timer_event.h"

using namespace lightstep;

static const auto TestingCallbackInterval = std::chrono::milliseconds{600};
static const auto TestingDuration = std::chrono::milliseconds{2000};
static const auto TestingEpsilon = std::chrono::milliseconds{100};

namespace {
struct CallbackContext {
  EventBase* event_base{nullptr};
  std::vector<std::chrono::steady_clock::time_point> time_points;
};
}  // namespace

static void TimerCallback(int /*socket*/, short /*what*/, void* context) {
  auto& callback_context = *static_cast<CallbackContext*>(context);
  callback_context.time_points.push_back(std::chrono::steady_clock::now());
  auto duration = callback_context.time_points.back() -
                  callback_context.time_points.front();
  if (duration >= TestingDuration) {
    callback_context.event_base->LoopBreak();
  }
}

TEST_CASE("TimerEvent") {
  EventBase event_base;
  CallbackContext callback_context;
  callback_context.event_base = &event_base;
  TimerEvent timer_event{event_base, TestingCallbackInterval, TimerCallback,
                         static_cast<void*>(&callback_context)};

  SECTION("TimerEvent can be move constructed.") {
    TimerEvent timer_event2{event_base, TestingCallbackInterval, TimerCallback,
                            static_cast<void*>(&callback_context)};
    auto libevent_handle = timer_event.libevent_handle();
    REQUIRE(libevent_handle != nullptr);
    timer_event2 = std::move(timer_event);
    REQUIRE(timer_event.libevent_handle() == nullptr);
    REQUIRE(timer_event2.libevent_handle() == libevent_handle);
  }

  SECTION("TimerEvent cand be move assigned.") {
    auto libevent_handle = timer_event.libevent_handle();
    REQUIRE(libevent_handle != nullptr);
  }

  SECTION("TimerEvent schedules callbacks at a given interval.") {
    event_base.Dispatch();
    REQUIRE(callback_context.time_points.size() == 5);
    for (int i = 1; i < 5; ++i) {
      auto duration =
          callback_context.time_points[i] - callback_context.time_points[i - 1];
      REQUIRE(duration > TestingCallbackInterval - TestingEpsilon);
      REQUIRE(duration < TestingCallbackInterval + TestingEpsilon);
    }
  }

  SECTION(
      "Reset can be used to restart the timer callbacks from at the current "
      "time point.") {
    event_base.OnTimeout(5 * TestingCallbackInterval - TestingEpsilon,
                         [](int /*socket*/, short /*what*/, void* context) {
                           static_cast<TimerEvent*>(context)->Reset();
                         },
                         static_cast<void*>(&timer_event));
    event_base.Dispatch();
    auto duration =
        callback_context.time_points.at(4) - callback_context.time_points.at(3);
    auto expected_duration = 2 * TestingCallbackInterval - TestingEpsilon;
    REQUIRE(duration > expected_duration - TestingEpsilon);
    REQUIRE(duration < expected_duration + TestingEpsilon);
  }
}
