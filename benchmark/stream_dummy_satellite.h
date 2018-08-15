#include "dummy_satellite.h"

#include <string>

namespace lightstep {
class StreamDummySatellite : public DummySatellite {
 public:
   StreamDummySatellite(const char* host, int port);

  void Run();

  std::vector<uint64_t> span_ids() const override;

  void Reserve(size_t num_span_ids) override;
 private:
  std::string host_;
  int port_;

  void ProcessConnections();
};
} // namespace lightstep
