#include "stream_session.h"

#include "../../src/bipart_memory_stream.h"

#include <google/protobuf/io/coded_stream.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <array>
#include <cassert>
#include <exception>
#include <iostream>
#include <utility>

namespace lightstep {
//------------------------------------------------------------------------------
// ReadOrBlock
//------------------------------------------------------------------------------
static ssize_t ReadOrBlock(int file_descriptor, void* data, size_t count) {
  auto rcode = read(file_descriptor, data, count);
  if (rcode >= 0) {
    return rcode;
  }
  if (errno != EAGAIN && errno != EWOULDBLOCK) {
    std::cerr << "Unexpected error code " << errno << "\n";
    std::terminate();
  }
  timeval tv = {};
  fd_set readfds;
  tv.tv_sec = 60;

  FD_ZERO(&readfds);
  FD_SET(file_descriptor, &readfds);

  rcode = select(file_descriptor + 1, &readfds, nullptr, nullptr, &tv);
  if (rcode < 0) {
    std::cerr << "select failed: " << errno << "\n";
    std::terminate();
  }
  if (rcode == 0) {
    std::cerr << "select timed out\n";
    std::terminate();
  }
  rcode = read(file_descriptor, data, count);
  if (rcode < 0) {
    std::cerr << "read failure: " << errno << "\n";
    std::terminate();
  }
  return rcode;
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamSession::StreamSession(Socket&& socket) : socket_{std::move(socket)} {
  socket_.SetNonblocking();
}

//------------------------------------------------------------------------------
// ReadUntilNextMessage
//------------------------------------------------------------------------------
bool StreamSession::ReadUntilNextMessage() {
  while (!CheckForNextMessage()) {
    if (!DoRead()) {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------------------
// ConsumeMessage
//------------------------------------------------------------------------------
bool StreamSession::ConsumeMessage(google::protobuf::Message& message) {
  if (require_header_ || buffer_.size() < required_size_) {
    return false;
  }

  auto placement = buffer_.Peek(0, required_size_);
  BipartMemoryInputStream stream_buffer{placement.data1, placement.size1,
                                        placement.data2, placement.size2};
  if (!message.ParseFromBoundedZeroCopyStream(
          &stream_buffer, static_cast<int>(required_size_))) {
    std::cerr << "Failed to parse message\n";
    std::terminate();
  }
  buffer_.Consume(required_size_);
  require_header_ = true;
  required_size_ = PacketHeader::size;
  return true;
}

//------------------------------------------------------------------------------
// CheckForNextMessage
//------------------------------------------------------------------------------
bool StreamSession::CheckForNextMessage() {
  if (require_header_) {
    if (buffer_.size() < required_size_) {
      return false;
    }
    ConsumeHeader();
  }
  if (buffer_.size() < required_size_) {
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
// DoRead
//------------------------------------------------------------------------------
bool StreamSession::DoRead() {
  auto free_space = buffer_.free_space();
  auto contiguous_space = buffer_.capacity() - buffer_.head_index();
  auto max_bytes_to_read = std::min(free_space, contiguous_space);
  auto rcode =
      ReadOrBlock(socket_.file_descriptor(),
                  static_cast<void*>(buffer_.head_data()), max_bytes_to_read);
  if (rcode == 0) {
    return false;
  }
  buffer_.Reserve(rcode);
  return true;
}

//------------------------------------------------------------------------------
// ConsumeHeader
//------------------------------------------------------------------------------
void StreamSession::ConsumeHeader() {
  assert(require_header_ && buffer_.size() >= PacketHeader::size);

  auto placement = buffer_.Peek(0, PacketHeader::size);
  BipartMemoryInputStream stream_buffer{placement.data1, placement.size1,
                                        placement.data2, placement.size2};
  google::protobuf::io::CodedInputStream stream{&stream_buffer};
  std::array<char, PacketHeader::size> buffer;
  stream.ReadRaw(static_cast<void*>(buffer.data()),
                 static_cast<int>(PacketHeader::size));
  PacketHeader header{buffer.data()};

  buffer_.Consume(PacketHeader::size);
  require_header_ = false;
  packet_type_ = header.type();
  required_size_ = header.body_size();
  if (required_size_ > buffer_.capacity() - 1) {
    std::cerr << "Message is too big\n";
    std::terminate();
  }
}
}  // namespace lightstep
