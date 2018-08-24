#pragma once

#include "../src/circular_buffer.h"
#include "../src/packet_header.h"
#include "../src/socket.h"

#include <google/protobuf/message.h>

namespace lightstep {
class StreamSession {
 public:
  explicit StreamSession(Socket&& socket);

  bool ReadUntilNextMessage();

  bool ConsumeMessage(google::protobuf::Message& message);

 private:
  bool require_header_{true};
  size_t required_size_{PacketHeader::size};

  Socket socket_;
  CircularBuffer buffer_{1025};

  void ConsumeHeader();
  bool CheckForNextMessage();
  bool DoRead();
};
}  // namespace lightstep
