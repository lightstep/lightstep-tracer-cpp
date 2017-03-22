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

}  // namespace lightstep
 
#endif  // __LIGHTSTEP_CARRIER_H__
