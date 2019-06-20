#include <cstdlib>
#include <iostream>
#include <limits>

#include "test/http_connection.h"
#include "test/mock_satellite/mock_satellite.pb.h"

#include <gflags/gflags.h>

DEFINE_string(host, "127.0.0.1", "the host of the mock satellite");
DEFINE_int32(port, 0, "the port of the mock satellite");

static void QueryNumSpans(lightstep::HttpConnection& connection) {
  lightstep::mock_satellite::Spans spans;
  connection.Get("/spans", spans);
  std::cout << spans.spans().size();
}

int main(int argc, char* argv[]) try {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_port <= 0 || FLAGS_port > std::numeric_limits<uint16_t>::max()) {
    std::cerr << "must provide a valid port for the mock satellite\n";
    std::abort();
  }
  if (argc != 2) {
    std::cerr << "no command provided\n";
    std::abort();
  }
  std::string command{argv[1]};
  lightstep::HttpConnection connection{FLAGS_host.c_str(),
                                       static_cast<uint16_t>(FLAGS_port)};
  if (command == "num_spans") {
    QueryNumSpans(connection);
    return 0;
  }
  std::cerr << "Invalid command: " << command << "\n";
  std::abort();
} catch (const std::exception& e) {
  std::cerr << e.what() << "\n";
  std::abort();
}
