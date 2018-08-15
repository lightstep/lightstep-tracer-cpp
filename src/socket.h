#pragma once

namespace lightstep {
class Socket {
 public:
   Socket();

   explicit Socket(int file_descriptor) noexcept;

   Socket(const Socket&) = delete;

   Socket(Socket&& other) noexcept;

   ~Socket();

   Socket& operator=(Socket&& other) noexcept = delete;

   Socket& operator=(const Socket&) = delete;

   int file_descriptor() const noexcept { return file_descriptor_; }

   void SetNonblocking();

   void SetBlocking();
 private:
   int file_descriptor_{-1};
};
} // namespace lightstep
