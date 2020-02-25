#include <vector>

#include "network/timer_event.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

static const auto TestingCallbackInterval = std::chrono::milliseconds{900};
static const auto TestingDuration = std::chrono::milliseconds{3000};
static const auto TestingEpsilon = std::chrono::milliseconds{150};

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
    event_base.OnTimeout(
        5 * TestingCallbackInterval - TestingEpsilon,
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
