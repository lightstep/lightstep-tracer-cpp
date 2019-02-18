#include "recorder/stream_recorder/satellite_connection.h"

#include "recorder/stream_recorder/satellite_streamer.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteConnection::SatelliteConnection(SatelliteStreamer& streamer) noexcept
    : streamer_{streamer} {}
}  // namespace lightstep
