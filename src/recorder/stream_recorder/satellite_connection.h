#pragma once

namespace lightstep {
class SatelliteConnectionManager;

class SatelliteConnection {
 public:
  SatelliteConnection(SatelliteConnectionManager& connection_manager) noexcept;

  SatelliteConnection(const SatelliteConnection&) = delete;
  SatelliteConnection(SatelliteConnection&&) = delete;

  ~SatelliteConnection() noexcept = default;

  SatelliteConnection& operator=(const SatelliteConnection&) = delete;
  SatelliteConnection& operator=(SatelliteConnection&&) = delete;

 private:
  SatelliteConnectionManager& connection_manager_;
};
}  // namespace lightstep
