#pragma once

#include <google/protobuf/message.h>
#include <lightstep/buffer_chain.h>
#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
// Transporter is the abstract base class for SyncTransporter and
// LegacyAsyncTransporter.
class Transporter {
 public:
  Transporter() noexcept = default;

  Transporter(const Transporter&) noexcept = default;
  Transporter(Transporter&&) noexcept = default;

  virtual ~Transporter() = default;

  Transporter& operator=(const Transporter&) noexcept = default;
  Transporter& operator=(Transporter&&) noexcept = default;

  // Generates a valid artificial collector response.
  //
  // Can be used by testing code to to simulate the interaction with a LightStep
  // collector.
  static std::unique_ptr<google::protobuf::Message> MakeCollectorResponse();
};

// SyncTransporter customizes how synchronous tracing reports are sent.
class SyncTransporter : public Transporter {
 public:
  // Synchronously sends `request` to a collector and sets `response` to the
  // collector's response.
  virtual opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) = 0;
};

// LegacyAsyncTransporter customizes how asynchronous tracing reports are sent.
//
// Deprecated: Use AsyncTransporter
class LegacyAsyncTransporter : public Transporter {
 public:
  // Callback interface used by Send.
  class Callback {
   public:
    Callback() noexcept = default;
    Callback(const Callback&) noexcept = default;
    Callback(Callback&&) noexcept = default;

    virtual ~Callback() = default;

    Callback& operator=(const Callback&) noexcept = default;
    Callback& operator=(Callback&&) noexcept = default;

    virtual void OnSuccess() noexcept = 0;

    virtual void OnFailure(std::error_code error) noexcept = 0;
  };

  // Asynchronously sends `request` to a collector.
  //
  // On success, `response` is set to the collector's response and
  // `callback.OnSuccess()` should be called.
  //
  // On failure, `callback.OnFailure(error)` should be called.
  virtual void Send(const google::protobuf::Message& request,
                    google::protobuf::Message& response,
                    Callback& callback) = 0;
};

/**
 * Provides a hook to customize how ReportRequests are transported.
 */
class AsyncTransporter : public Transporter {
 public:
  /**
   * A Callback to be invoked after transporting a ReportRequest
   */
  class Callback {
   public:
    Callback() noexcept = default;
    Callback(const Callback&) noexcept = default;
    Callback(Callback&&) noexcept = default;

    virtual ~Callback() = default;

    Callback& operator=(const Callback&) noexcept = default;
    Callback& operator=(Callback&&) noexcept = default;

    /**
     * Call when a ReportRequest was successfully transported.
     * @param message the ReportRequest
     */
    virtual void OnSuccess(BufferChain& message) noexcept = 0;

    /**
     * Call when a ReportRequest not successfully transported.
     * @param message the ReportRequest
     */
    virtual void OnFailure(BufferChain& message) noexcept = 0;
  };

  /**
   * Called when the tracer's span buffer is full. Implementors can either flush
   * the pending spans early (so that we don't drop anything) or do nothing and
   * let subsequent finished spans drop.
   */
  virtual void OnSpanBufferFull() noexcept {}

  /**
   * Send a ReportRequest
   * @param message a BufferChain representing the ReportRequest's serialization
   * @param callback a Callback to be called after message is transported
   */
  virtual void Send(std::unique_ptr<BufferChain>&& message,
                    Callback& callback) noexcept = 0;
};
}  // namespace lightstep
