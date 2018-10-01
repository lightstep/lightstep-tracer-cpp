#include "dummy_satellite.h"

#include "../src/socket.h"
#include "stream_session.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace lightstep {
class StreamDummySatellite : public DummySatellite {
 public:
  StreamDummySatellite(const char* host, int port);

  ~StreamDummySatellite() override;

  std::vector<uint64_t> span_ids() const override;

  size_t num_dropped_spans() const noexcept override;

  void Reserve(size_t num_span_ids) override;

  void Close() override;

 private:
  std::thread thread_;
  mutable std::mutex mutex_;
  std::vector<uint64_t> span_ids_;
  size_t num_dropped_spans_{0};
  Socket listen_socket_;
  std::atomic<bool> running_{true};

  void ProcessConnections();

  void ProcessSession(StreamSession& session);

  void ConsumeSpanMessage(StreamSession& session);
  void ConsumeMetricsMessage(StreamSession& session);
};
}  // namespace lightstep
