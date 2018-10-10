#include "../src/satellite_connection.h"
#include "dummy_satellite/dummy_satellite.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>

using namespace lightstep;

TEST_CASE("SatelliteConnection") {
  configuration_proto::TracerConfiguration configuration;
  configuration.set_collector_host("localhost");
  configuration.set_collector_port(9001);
  auto satellite = MakeDummySatellite(configuration);
  Logger logger;

  SECTION("We can successfully establish a satellite connection.") {
    SatelliteConnection connection{logger, "localhost", 9001};
    CHECK(connection.is_connected());
  }

  SECTION(
      "If SatelliteConnection fails to establish a connection, is_connected "
      "returns false") {
    SatelliteConnection connection{logger, "localhost", 9002};
  }
}
