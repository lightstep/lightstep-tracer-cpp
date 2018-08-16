#include "stream_dummy_satellite.h"

#include "../src/packet_header.h"
#include "lightstep-tracer-common/collector.pb.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <stdexcept>
#include <array>
#include <iostream>

namespace lightstep {
//------------------------------------------------------------------------------
// ReadData
//------------------------------------------------------------------------------
static bool ReadData(int file_descriptor, char* data, size_t size) {
  size_t num_read = 0;
  while (num_read < size) {
    auto rcode = read(file_descriptor, static_cast<void*>(data + num_read),
                      size - num_read);
    if (rcode == 0) {
      return false;
    }
    if (rcode < 0) {
      std::cerr << "Read failed\n";
      std::terminate();
    }
  }
  return true;
}

//------------------------------------------------------------------------------
// ReadHeader
//------------------------------------------------------------------------------
static bool ReadHeader(int file_descriptor, PacketHeader& header) {
  std::array<char, PacketHeader::size> buffer;
  auto result = ReadData(file_descriptor, buffer.data(), PacketHeader::size);
  if (!result) {
    return false;
  }
  header = PacketHeader{buffer.data()};
  return true;
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamDummySatellite::StreamDummySatellite(const char* host, int port)
{
  sockaddr_in satellite_address = {};
  satellite_address.sin_family = AF_INET;
  satellite_address.sin_port = htons(port);

  auto rcode = inet_pton(AF_INET, host, &satellite_address.sin_addr);
  if (rcode <= 0) {
    throw std::runtime_error{"inet_pton failed"};
  }

  rcode = bind(listen_socket_.file_descriptor(),
               reinterpret_cast<sockaddr*>(&satellite_address),
               sizeof(satellite_address));
  if (rcode != 0) {
    throw std::runtime_error{"bind failed"};
  }

  listen_socket_.SetNonblocking();

  rcode = listen(listen_socket_.file_descriptor(), 10);
  if (rcode != 0) {
    throw std::runtime_error{"listen failed"};
  }

  thread_ = std::thread{&StreamDummySatellite::ProcessConnections, this};
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
StreamDummySatellite::~StreamDummySatellite() {
  running_ = false;
}

//------------------------------------------------------------------------------
// ProcessConnections
//------------------------------------------------------------------------------
void StreamDummySatellite::ProcessConnections() {
  while (running_) {
    auto file_descriptor =
        accept(listen_socket_.file_descriptor(), nullptr, nullptr);
    if (file_descriptor == EAGAIN || file_descriptor == EWOULDBLOCK) {
      std::this_thread::sleep_for(std::chrono::microseconds{1});
      continue;
    } else if (file_descriptor < 0) {
      throw std::runtime_error{"accept failed"};
    }

    Socket socket{file_descriptor};


    ProcessSession(socket);
  }
}

//------------------------------------------------------------------------------
// ProcessSession
//------------------------------------------------------------------------------
void StreamDummySatellite::ProcessSession(const Socket& socket) {
  while (1) {
    PacketHeader header;
    if (!ReadHeader(socket.file_descriptor(), header)) {
      break;
    }

    std::unique_ptr<char[]> buffer{new char[header.body_size()]};
    if (!ReadData(socket.file_descriptor(), buffer.get(), header.body_size())) {
      std::cerr << "Unexpected EOF\n";
      std::terminate();
    }

    collector::Span span;
    if (!span.ParseFromArray(static_cast<void*>(buffer.get()),
                             header.body_size())) {
      std::cerr << "Failed to Parse span\n";
      std::terminate();
    }
  }
}

//------------------------------------------------------------------------------
// span_ids
//------------------------------------------------------------------------------
std::vector<uint64_t> StreamDummySatellite::span_ids() const {
  return {};
}

//------------------------------------------------------------------------------
// Reserve
//------------------------------------------------------------------------------
void StreamDummySatellite::Reserve(size_t num_span_ids) {
}
}  // namespace lightstep
