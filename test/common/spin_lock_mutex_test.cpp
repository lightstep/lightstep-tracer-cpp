#include "common/spin_lock_mutex.h"

#include <thread>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

static void AddNumbers(SpinLockMutex& mutex, int a, int b,
                       std::vector<int>& v) {
  for (int i = a; i < b; ++i) {
    SpinLockGuard lock_guard{mutex};
    v.push_back(i);
  }
}

TEST_CASE("SpinLockMutex") {
  std::vector<int> v;
  std::vector<std::thread> threads(4);
  SpinLockMutex mutex;
  int a = 0;
  int n = 5000;
  for (auto& thread : threads) {
    thread = std::thread{AddNumbers, std::ref(mutex), a, a + n, std::ref(v)};
    a += n;
  }
  for (auto& thread : threads) {
    thread.join();
  }
  std::sort(v.begin(), v.end());
  REQUIRE(v.size() == static_cast<size_t>(n * threads.size()));
  int index = 0;
  for (auto value : v) {
    REQUIRE(value == index++);
  }
}
