#include "dummy_satellite.h"

#include "../src/socket.h"

#include <string>
#include <thread>
#include <atomic>

namespace lightstep {
class StreamDummySatellite : public DummySatellite {
 public:
   StreamDummySatellite(const char* host, int port);

   ~StreamDummySatellite();

  std::vector<uint64_t> span_ids() const override;

  void Reserve(size_t num_span_ids) override;
 private:
  std::thread thread_;
  mutable std::mutex mutex_;
  std::vector<uint64_t> span_ids_;
  Socket listen_socket_;
  std::atomic<bool> running_{true};

  void ProcessConnections();

  void ProcessSession(const Socket& socket);
};
} // namespace lightstep
