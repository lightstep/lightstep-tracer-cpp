// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_ENVOY_H__
#define __LIGHTSTEP_ENVOY_H__

#include "lightstep/envoy_carrier.pb.h"
#include "lightstep/propagation.h"

namespace lightstep {
namespace envoy {

// ProtoReader reads an Envoy CarrierStruct for extracting Envoy-carried SpanContext.
class ProtoReader : public BasicCarrierReader {
public:
  explicit ProtoReader(const CarrierStruct& data) : data_(data) { }

  void ForeachKey(std::function<void(const std::string& key,
				     const std::string& value)> f) const override {
    for (const auto& c : data_.context()) {
      f(c.key(), c.value());
    }
  }

private:
  const CarrierStruct& data_;
};

// ProtoReader writes an Envoy CarrierStruct for injecting Envoy-carried SpanContext.
class ProtoWriter : public BasicCarrierWriter {
public:
  explicit ProtoWriter(CarrierStruct *output) : output_(output) { }

  void Set(const std::string &key, const std::string &value) const override {
    CarrierPair *kv = output_->add_context();
    kv->set_key(key);
    kv->set_value(value);
  }

private:
  CarrierStruct *const output_;
};

}  // namespace envoy
}  // namespace lightstep
 
#endif  // __LIGHTSTEP_ENVOY_H__
