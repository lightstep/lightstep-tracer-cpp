// -*- Mode: C++ -*-
//
// Note! Most LightStep client libraries support injecting/extracting
// LightStepCarrier to/from a string type and handle base64
// encoding/decoding as part of the library.
//
// This library DOES NOT PERFORM BASE64 encoding/decoding.
#ifndef __LIGHTSTEP_CARRIER_H__
#define __LIGHTSTEP_CARRIER_H__

#include "lightstep_carrier.pb.h"
#include "lightstep/propagation.h"

namespace lightstep {

// ProtoReader reads a LightStepCarrier.
class ProtoReader : public CarrierReader {
public:
  explicit ProtoReader(const BinaryCarrier& data) : data_(data) { }

private:
  friend class lightstep::TracerImpl;
  const BinaryCarrier& data_;
};

// ProtoReader writes a BinaryCarrier.
class ProtoWriter : public CarrierWriter {
public:
  explicit ProtoWriter(BinaryCarrier *output) : output_(output) { }

private:
  friend class lightstep::TracerImpl;
  BinaryCarrier *const output_;
};

// TextProtoReader/Writer handle reading and writing the opentracing
// text carrier representation.
class TextProtoReader : public BasicCarrierReader {
public:
  explicit TextProtoReader(const BinaryCarrier& data) : data_(data) { }

  void ForeachKey(std::function<void(const std::string& key,
  				     const std::string& value)> f) const override {
    for (const auto& c : data_.text_ctx()) {
      f(c.key(), c.value());
    }
  }

private:
  const BinaryCarrier& data_;
};

class TextProtoWriter : public BasicCarrierWriter {
public:
  explicit TextProtoWriter(BinaryCarrier *output) : output_(output) { }

  void Set(const std::string &key, const std::string &value) const override {
    TextCarrierPair *kv = output_->add_text_ctx();
    kv->set_key(key);
    kv->set_value(value);
  }

private:
  BinaryCarrier *const output_;
};

}  // namespace lightstep
 
#endif  // __LIGHTSTEP_CARRIER_H__
