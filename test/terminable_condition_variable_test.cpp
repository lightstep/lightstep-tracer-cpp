#include <lightstep/terminable_condition_variable.h>

#include <chrono>
#include <thread>

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>

using namespace lightstep;

TEST_CASE("TerminableConditionVariable") {
  TerminableConditionVariable condition_variable;

  auto duration = std::chrono::microseconds{100};

  SECTION(
      "TerminableConditionVariable supports waiting for a given amount of "
      "time.") {
    auto t1 = std::chrono::system_clock::now();
    condition_variable.WaitFor(duration);
    auto t2 = std::chrono::system_clock::now();
    CHECK(t2 - t1 >= duration);

    t1 = std::chrono::system_clock::now();
    condition_variable.WaitFor(duration, [] { return false; });
    t2 = std::chrono::system_clock::now();
    CHECK(t2 - t1 >= duration);
  }

  SECTION("WaitFor can be cancelled with a predicate that returns true.") {
    auto t1 = std::chrono::system_clock::now();
    condition_variable.WaitFor(duration, [] { return true; });
    auto t2 = std::chrono::system_clock::now();
    CHECK(t2 - t1 < duration);
  }

  SECTION(
      "is_active returns the correct value when TerminableConditionVariable is "
      "terminated.") {
    CHECK(condition_variable.is_active());
    condition_variable.Terminate();
    CHECK(!condition_variable.is_active());
  }

  SECTION("Waits never start if TerminableConditionVariable is terminated.") {
    condition_variable.Terminate();

    auto t1 = std::chrono::system_clock::now();
    condition_variable.WaitFor(duration);
    auto t2 = std::chrono::system_clock::now();
    CHECK(t2 - t1 < duration);

    t1 = std::chrono::system_clock::now();
    condition_variable.WaitFor(duration, [] { return false; });
    t2 = std::chrono::system_clock::now();
    CHECK(t2 - t1 < duration);
  }

  SECTION(
      "Waits are interrupted if TerminableConditionVariable is terminated.") {
    std::chrono::system_clock::time_point t1, t2;
    std::thread thread{[&] {
      t1 = std::chrono::system_clock::now();
      condition_variable.WaitFor(duration);
      t2 = std::chrono::system_clock::now();
    }};
    condition_variable.Terminate();
    thread.join();
    CHECK(t2 - t1 < duration);

    thread = std::thread{[&] {
      t1 = std::chrono::system_clock::now();
      condition_variable.WaitFor(duration, [] { return false; });
      t2 = std::chrono::system_clock::now();
    }};
    condition_variable.Terminate();
    thread.join();
    CHECK(t2 - t1 < duration);
  }
}
