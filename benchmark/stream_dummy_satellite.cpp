#include "stream_dummy_satellite.h"

#include "../src/packet_header.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <stdexcept>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamDummySatellite::StreamDummySatellite(const char* host, int port) {
  sockaddr_in satellite_address = {};
  satellite_address.sin_family = AF_INET;
  satellite_address.sin_port = htons(port);

  auto rcode = inet_pton(AF_INET, host, &satellite_address.sin_addr);
  if (rcode <= 0) {
    std::cerr << "inet_pton failed\n";
    std::terminate();
  }

  rcode = bind(listen_socket_.file_descriptor(),
               reinterpret_cast<sockaddr*>(&satellite_address),
               sizeof(satellite_address));
  if (rcode != 0) {
    std::cerr << "bind failed\n";
    std::terminate();
  }

  listen_socket_.SetNonblocking();

  rcode = listen(listen_socket_.file_descriptor(), 10);
  if (rcode != 0) {
    std::cerr << "listen failed\n";
    std::terminate();
  }

  thread_ = std::thread{&StreamDummySatellite::ProcessConnections, this};
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
StreamDummySatellite::~StreamDummySatellite() {
  if (!running_.exchange(false)) {
    return;
  }
  thread_.join();
}

//------------------------------------------------------------------------------
// ProcessConnections
//------------------------------------------------------------------------------
void StreamDummySatellite::ProcessConnections() {
  while (running_) {
    auto file_descriptor =
        accept(listen_socket_.file_descriptor(), nullptr, nullptr);
    if (file_descriptor < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        std::this_thread::sleep_for(std::chrono::microseconds{1});
        continue;
      } else {
        std::cerr << "accept failed\n";
        std::terminate();
      }
    }

    Socket socket{file_descriptor};
    StreamSession session{std::move(socket)};
    ProcessSession(session);
  }
}

//------------------------------------------------------------------------------
// ProcessSession
//------------------------------------------------------------------------------
void StreamDummySatellite::ProcessSession(StreamSession& session) {
  if (!session.ReadUntilNextMessage()) {
    std::cerr << "Expected initialization message\n";
    std::terminate();
  }
  collector::StreamInitialization initialization;
  if (!session.ConsumeMessage(initialization)) {
    std::cerr << "Failed to consume initialization message\n";
    std::terminate();
  }
  while (session.ReadUntilNextMessage()) {
    collector::Span span;
    if (!session.ConsumeMessage(span)) {
      std::cerr << "Faild to consume span\n";
      std::terminate();
    }
    std::unique_lock<std::mutex> lock{mutex_};
    span_ids_.push_back(span.span_context().span_id());
  }
}

//------------------------------------------------------------------------------
// span_ids
//------------------------------------------------------------------------------
std::vector<uint64_t> StreamDummySatellite::span_ids() const {
  std::unique_lock<std::mutex> lock{mutex_};
  return span_ids_;
}

//------------------------------------------------------------------------------
// Reserve
//------------------------------------------------------------------------------
void StreamDummySatellite::Reserve(size_t num_span_ids) {
  std::unique_lock<std::mutex> lock{mutex_};
  span_ids_.reserve(num_span_ids);
}

//------------------------------------------------------------------------------
// Close
//------------------------------------------------------------------------------
void StreamDummySatellite::Close() {
  if (!running_.exchange(false)) {
    return;
  }
  thread_.join();
}
}  // namespace lightstep
