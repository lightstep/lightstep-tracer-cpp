#pragma once

#include <opentracing/util.h>
#include <opentracing/string_view.h>
#include <google/protobuf/message.h>


namespace lightstep {
class AsyncTransporter {
 public:
  virtual ~AsyncTransporter() = default;

  virtual void Send(const google::protobuf::Message& request,
                    google::protobuf::Message& response,
                    void (*on_success)(void* context),
                    void (*on_failure)(std::error_code error, void* context));
};

class LightStepAsyncTransporter : public AsyncTransporter {
 public:
   int file_descriptor() const;

   void OnRead();
   void OnWrite();
   void OnTimeout();
};

std::unique_ptr<LightStepAsyncTransporter> MakeLightStepAsyncTransporter();
}  // namespace lightstep
