#include <memory>

#include "recorder/fork_aware_recorder.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

class DummyRecorder final : public ForkAwareRecorder {
 public:
  void RecordSpan(const collector::Span& /*span*/) noexcept override {}
};

TEST_CASE("ForkAwareRecorder") {
  REQUIRE(ForkAwareRecorder::GetActiveRecordersForTesting().empty());

  SECTION("Active recorders are tracked in a global variable.") {
    std::unique_ptr<DummyRecorder> recorder1{new DummyRecorder{}};
    REQUIRE(ForkAwareRecorder::GetActiveRecordersForTesting().size() == 1);
    recorder1.reset(nullptr);
    REQUIRE(ForkAwareRecorder::GetActiveRecordersForTesting().empty());
  }

  SECTION(
      "The active recorder list is properly updated when there are more than 1 "
      "recorders.") {
    std::unique_ptr<DummyRecorder> recorder1{new DummyRecorder{}};
    std::unique_ptr<DummyRecorder> recorder2{new DummyRecorder{}};
    std::unique_ptr<DummyRecorder> recorder3{new DummyRecorder{}};
    recorder2.reset(nullptr);
    REQUIRE(ForkAwareRecorder::GetActiveRecordersForTesting() ==
            std::vector<const ForkAwareRecorder*>{recorder3.get(),
                                                  recorder1.get()});
  }
}
