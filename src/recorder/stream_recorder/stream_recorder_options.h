#pragma once

#include <chrono>
#include <cstddef>

#include "network/dns_resolver.h"

namespace lightstep {
/**
 * Options to tweak the behavior of StreamRecorder.
 *
 * Note: This class isn't exposed to users. It's only used to change the
 * behavior for testing.
 */
struct StreamRecorderOptions {
  // Instructs the stream recorder consume spans out of the buffer without
  // sending them.
  //
  // It's meant to be used as a mode for benchmarking only.
  bool throw_away_spans = false;

  // The amount of time between executions of StreamRecorder's polling callback.
  //
  // Note: This should be a short duration; otherwise, the recorder can hang the
  // process when exiting.
  std::chrono::microseconds polling_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{1});

  std::chrono::microseconds timestamp_delta_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{500});

  // If the span buffer fills past this fraction of its max size, then we do
  // flush the span buffer early.
  double early_flush_threshold = 0.5;

  // Options to use when resolving satellite host names.
  DnsResolverOptions dns_resolver_options;

  // Dns resolutions will be cached and refreshed at a random point of time
  // within the specified window from their last refresh.
  std::chrono::microseconds min_dns_resolution_refresh_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::minutes{5});
  std::chrono::microseconds max_dns_resolution_refresh_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::minutes{6});

  // The amount of time to wait until trying dns resolution again after a
  // failure.
  std::chrono::microseconds dns_failure_retry_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::seconds{5});

  // The number of connections to make to satellites for streaming.
  int num_satellite_connections = 8;

  // The amount of time to wait for a satellite connection to be writable before
  // reconnecting.
  std::chrono::microseconds satellite_write_timeout =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::seconds{5});

  // The amount of time to wait until attempting to reconnect to a satellite
  // after a failure.
  std::chrono::microseconds satellite_failure_retry_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::seconds{1});

  // Satellite connections will be reestablished at a random point of time
  // within the specified window from when the connection was last established.
  std::chrono::microseconds min_satellite_reconnect_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::seconds{5});
  std::chrono::microseconds max_satellite_reconnect_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::seconds{7});

  // When we try to close a satellite stream, first attempt to close it
  // gracefully by writing any pending data. If that can't be done within this
  // time window, then we'll close the socket so that we have an incomplete
  // stream.
  std::chrono::microseconds satellite_graceful_stream_shutdown_timeout =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::seconds{5});
};
}  // namespace lightstep
