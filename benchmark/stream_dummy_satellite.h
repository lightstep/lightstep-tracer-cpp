#include "dummy_satellite.h"

namespace lightstep {
class StreamDummySatellite : public DummySatellite {
 public:
   StreamDummySatellite(const char* host, int port);

  void Run();

  std::vector<uint64_t> span_ids() const override;

  void Reserve(size_t num_span_ids) override;
 private:
};
} // namespace lightstep
