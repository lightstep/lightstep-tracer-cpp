#include <random>

#include "3rd_party/catch2/catch.hpp"
#include "common/composable_fragment_input_stream.h"
#include "common/fragment_array_input_stream.h"
#include "test/composable_fragment_input_stream_wrapper.h"
#include "test/utility.h"
using namespace lightstep;

TEST_CASE("ComposableFragmentInputStream") {
  ComposableFragmentInputStreamWrapper stream{
      std::unique_ptr<FragmentInputStream>{new FragmentArrayInputStream{
          MakeFragment("abc"), MakeFragment("123")}}};
  stream.Append(std::unique_ptr<ComposableFragmentInputStream>{
      new ComposableFragmentInputStreamWrapper{
          std::unique_ptr<FragmentInputStream>{new FragmentArrayInputStream{
              MakeFragment("xyz"), MakeFragment("456")}}}});
  REQUIRE(ToString(stream) == "abc123xyz456");
  REQUIRE(stream.num_fragments() == 4);

  stream.Append(std::unique_ptr<ComposableFragmentInputStream>{
      new ComposableFragmentInputStreamWrapper{
          std::unique_ptr<FragmentInputStream>{
              new FragmentArrayInputStream{MakeFragment("qrz")}}}});
  REQUIRE(ToString(stream) == "abc123xyz456qrz");
  REQUIRE(stream.num_fragments() == 5);

  SECTION("We can consume from ComposableFragmentInputStream") {
    auto contents = ToString(stream);
    for (size_t i = 0; i < contents.size(); ++i) {
      SECTION("Consume " + std::to_string(i)) {
        Consume({&stream}, i);
        REQUIRE(ToString(stream) == contents.substr(i));
      }
    }
  }
}
