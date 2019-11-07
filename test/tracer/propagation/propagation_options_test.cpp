#include "tracer/propagation/propagation_options.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SetInjectExtractPropagationModes") {
  std::vector<PropagationMode> inject_propagation_modes;
  std::vector<PropagationMode> extract_propagation_modes;

  SECTION(
      "SetInjectExtractPropagationModes determines the inject-extract "
      "propagation modes from a list"
      " of propagation modes") {
    std::vector<PropagationMode> propagation_modes = {
        PropagationMode::b3, PropagationMode::lightstep};
    SetInjectExtractPropagationModes(
        propagation_modes, inject_propagation_modes, extract_propagation_modes);
    REQUIRE(inject_propagation_modes == propagation_modes);
    REQUIRE(extract_propagation_modes == propagation_modes);
  }

  SECTION(
      "When envoy is used as a propagation mode, extraction always falls back "
      "to lightstep's propagator") {
    std::vector<PropagationMode> propagation_modes = {PropagationMode::envoy};
    SetInjectExtractPropagationModes(
        propagation_modes, inject_propagation_modes, extract_propagation_modes);
    REQUIRE(inject_propagation_modes == propagation_modes);
    REQUIRE(extract_propagation_modes ==
            std::vector<PropagationMode>{PropagationMode::envoy,
                                         PropagationMode::lightstep});
  }

  SECTION(
      "SetInjectExtractPropagationModes correctly handles lists with duplicate "
      "propagation modes") {
    std::vector<PropagationMode> propagation_modes = {
        PropagationMode::b3, PropagationMode::lightstep, PropagationMode::b3,
        PropagationMode::lightstep};
    SetInjectExtractPropagationModes(
        propagation_modes, inject_propagation_modes, extract_propagation_modes);
    REQUIRE(inject_propagation_modes ==
            std::vector<PropagationMode>{PropagationMode::b3,
                                         PropagationMode::lightstep});
    REQUIRE(extract_propagation_modes ==
            std::vector<PropagationMode>{PropagationMode::b3,
                                         PropagationMode::lightstep});
  }

  SECTION("SetInjectExtractPropagationModes preserves ordering") {
    std::vector<PropagationMode> propagation_modes = {
        PropagationMode::b3, PropagationMode::lightstep};
    SetInjectExtractPropagationModes(
        propagation_modes, inject_propagation_modes, extract_propagation_modes);
    REQUIRE(inject_propagation_modes == propagation_modes);
    REQUIRE(extract_propagation_modes == propagation_modes);

    std::vector<PropagationMode> propagation_modes_reverse = {
        PropagationMode::lightstep, PropagationMode::b3};
    SetInjectExtractPropagationModes(propagation_modes_reverse,
                                     inject_propagation_modes,
                                     extract_propagation_modes);
    REQUIRE(inject_propagation_modes == propagation_modes_reverse);
    REQUIRE(extract_propagation_modes == propagation_modes_reverse);
  }
}
