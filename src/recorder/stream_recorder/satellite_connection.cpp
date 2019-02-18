#include "recorder/stream_recorder/satellite_connection.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteConnection::SatelliteConnection(
    SatelliteConnectionManager& connection_manager) noexcept
    : connection_manager_{connection_manager} {}
}  // namespace lightstep
