// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_ENVOY_H__
#define __LIGHTSTEP_ENVOY_H__

#include "envoy_carrier.pb.h"
#include "lightstep/propagation.h"

namespace lightstep {
namespace envoy {

// ProtoReader reads an EnvoyCarrier for extracting Envoy-carried
// SpanContext.  Note! Most client libraries support
// injecting/extracting EnvoyCarrier from a string and handle base64
// encoding/decoding themselves. This library does not, it is up to
// the caller to serialize / deserialize EnvoyCarrier then base64
// encode / decode manually.

class ProtoReader : public CarrierReader {
public:
  explicit ProtoReader(const EnvoyCarrier& data) : data_(data) { }

private:
  friend class lightstep::TracerImpl;
  const EnvoyCarrier& data_;
};

// ProtoReader writes an EnvoyCarrier for injecting Envoy-carried SpanContext.
class ProtoWriter : public CarrierWriter {
public:
  explicit ProtoWriter(EnvoyCarrier *output) : output_(output) { }

private:
  friend class lightstep::TracerImpl;
  EnvoyCarrier *const output_;
};

// Legacy Reader/Writer handle reading and writing the legacy format.
class LegacyProtoReader : public BasicCarrierReader {
public:
  explicit LegacyProtoReader(const EnvoyCarrier& data) : data_(data) { }

  void ForeachKey(std::function<void(const std::string& key,
  				     const std::string& value)> f) const override {
    for (const auto& c : data_.deprecated_context()) {
      f(c.key(), c.value());
    }
  }

private:
  const EnvoyCarrier& data_;
};

class LegacyProtoWriter : public BasicCarrierWriter {
public:
  explicit LegacyProtoWriter(EnvoyCarrier *output) : output_(output) { }

  void Set(const std::string &key, const std::string &value) const override {
    LegacyCarrierPair *kv = output_->add_deprecated_context();
    kv->set_key(key);
    kv->set_value(value);
  }

private:
  EnvoyCarrier *const output_;
};

}  // namespace envoy
}  // namespace lightstep
 
#endif  // __LIGHTSTEP_ENVOY_H__
