#include "common/circular_buffer2.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("CircularBuffer") {
  CircularBuffer2<int> buffer{10};

  SECTION("We can add values into a circular buffer") {
    std::unique_ptr<int> x{new int{11}};
    REQUIRE(buffer.Add(x));
    REQUIRE(x == nullptr);
    auto range = buffer.Peek();
    REQUIRE(range.size() == 1);
    range.ForEach([] (const AtomicUniquePtr<int>& y) {
        REQUIRE(*y == 11);
        return true;
    });
  }

  SECTION("If CircularBuffer is full, Add fails") {
    for (int i = 0; i < static_cast<int>(buffer.max_size()); ++i) {
      std::unique_ptr<int> x{new int{i}};
      REQUIRE(buffer.Add(x));
    }
    std::unique_ptr<int> x{new int{33}};
    REQUIRE(!buffer.Add(x));
    REQUIRE(x != nullptr);
    REQUIRE(*x == 33);
  }

  SECTION("We can consume elements out of CircularBuffer") {
    for (int i = 0; i < static_cast<int>(buffer.max_size()); ++i) {
      std::unique_ptr<int> x{new int{i}};
      REQUIRE(buffer.Add(x));
    }
    int count = 0;
    buffer.Consume(5, [&] (CircularBufferRange<AtomicUniquePtr<int>> range) noexcept {
      range.ForEach([&] (AtomicUniquePtr<int>& ptr) {
          REQUIRE(*ptr == count++);
          ptr.Reset();
          return true;
      });
    });
    REQUIRE(count == 5);
  }
}
