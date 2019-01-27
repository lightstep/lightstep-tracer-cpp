#pragma once

#include <atomic>
#include <iostream>
#include <vector>

#include "common/condition_variable_wrapper.h"

namespace lightstep {

// TestingConditionVariableWrapper provides an interface to a condition variable
// that supports pausing, stepping, and advancing without waiting so as to
// support testing.
class TestingConditionVariableWrapper : public ConditionVariableWrapper {
 public:
  TestingConditionVariableWrapper() = default;

  TestingConditionVariableWrapper(TestingConditionVariableWrapper&&) = delete;
  TestingConditionVariableWrapper(const TestingConditionVariableWrapper&) =
      delete;

  ~TestingConditionVariableWrapper() override;

  TestingConditionVariableWrapper& operator=(
      TestingConditionVariableWrapper&&) = delete;
  TestingConditionVariableWrapper& operator=(
      const TestingConditionVariableWrapper&) = delete;

  std::chrono::steady_clock::time_point Now() const override;

  void set_now(const std::chrono::steady_clock::time_point& time_point);

  bool WaitFor(std::unique_lock<std::mutex>& lock,
               const std::chrono::steady_clock::duration& duration,
               std::function<bool()> predicate) override;

  bool WaitUntil(std::unique_lock<std::mutex>& lock,
                 const std::chrono::steady_clock::time_point& timeout,
                 std::function<bool()> predicate) override;

  void NotifyAll() override;

  void WaitTillNextEvent();

  void Step();

  void set_block_notify_all(bool value) { block_notify_all_ = value; }

  class Event {
   public:
    explicit Event(const std::chrono::steady_clock::time_point& timeout)
        : timeout_{timeout} {}

    Event(Event&&) = delete;
    Event(const Event&) = delete;

    virtual ~Event() = default;

    Event& operator=(Event&&) = delete;
    Event& operator=(const Event&) = delete;

    virtual void Process(
        TestingConditionVariableWrapper& condition_variable) = 0;

    virtual void Notify() {}

    const std::chrono::steady_clock::time_point timeout() const {
      return timeout_;
    }

   private:
    std::chrono::steady_clock::time_point timeout_;
  };

  class NotifyAllEvent : public Event {
   public:
    using Event::Event;
    void Process(TestingConditionVariableWrapper& condition_variable) override;
    void Notify() override;
  };

  class WaitEvent : public Event {
   public:
    using Event::Event;
    void Process(
        TestingConditionVariableWrapper& /*condition_variable*/) override {
      Notify();
    }
    void Notify() override { ++ticker_; }

    bool DecrementTicker() {
      // Only a single thread can call DecrementTicker, so there's no chance
      // of decrementing it below zero.
      if (ticker_ == 0) {
        return false;
      }
      --ticker_;
      return true;
    }

   private:
    std::atomic<int> ticker_{0};
  };

  const Event* next_event() const;

 private:
  void AddEvent(Event* event);

  mutable std::mutex mutex_;
  std::chrono::steady_clock::time_point now_ = std::chrono::steady_clock::now();
  std::atomic<bool> block_notify_all_{false};
  std::vector<Event*> events_;
};
}  // namespace lightstep
