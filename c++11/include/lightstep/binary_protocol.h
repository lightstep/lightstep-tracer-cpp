#ifndef LIGHTSTEP_BINARY_PROTOCOL_H
#define LIGHTSTEP_BINARY_PROTOCOL_H

#include <lightstep_carrier.pb.h>
#include <opentracing/propagation.h>

namespace lightstep {
class LightStepBinaryReader : public opentracing::CustomCarrierReader {
 public:
  LightStepBinaryReader(const BinaryCarrier* carrier) noexcept
      : carrier_(carrier) {}

  opentracing::Expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::Tracer& tracer) const override;

 private:
  const BinaryCarrier* carrier_;
};

class LightStepBinaryWriter : public opentracing::CustomCarrierWriter {
 public:
  LightStepBinaryWriter(BinaryCarrier& carrier) noexcept : carrier_(carrier) {}

  opentracing::Expected<void> Inject(
      const opentracing::Tracer& tracer,
      const opentracing::SpanContext& sc) const override;

 private:
  BinaryCarrier& carrier_;
};
}  // namespace lightstep

#endif  // LIGHTSTEP_BINARY_PROTOCOL_H
