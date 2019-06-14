#pragma once

#include <cstdint>

namespace lightstep {
enum class PortAssignments : uint16_t {
  AresDnsResolverTest = 9000,
  SatelliteDnsResolutionManagerTest,
  SatelliteEndpointManagerTest,
  StreamRecorderTest,
  StreamRecorderForkTest,
  MockSatelliteTest,
  VectorWriteTestHttp,
  VectorWriteTestTcp,
  DynamicLoadTest
};
}  // namespace lightstep
