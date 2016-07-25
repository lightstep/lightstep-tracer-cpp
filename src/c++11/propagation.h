// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_PROPAGATION_H__
#define __LIGHTSTEP_PROPAGATION_H__

#include <functional>
#include <string>

namespace lightstep {
  
enum BuiltinFormat {
  // Binary encodes the SpanContext for propagation as opaque binary data.
  Binary = 1,  // NOT IMPLEMENTED

  // TextMap encodes the SpanContext as key:value pairs.
  TextMap = 2,  // NOT IMPLEMENTED

  // HttpHeaders encodes the SpanContext as HTTP headers.
  HttpHeaders = 3,
};

// Base class for implementation-provided or builtin carrier formats.
class CarrierFormat {
public:
};

// Builtin carrier format values.
class BuiltinCarrierFormat : public CarrierFormat {
public:
  explicit BuiltinCarrierFormat(BuiltinFormat format) : format_(format) { }

  static BuiltinCarrierFormat Binary;
  static BuiltinCarrierFormat TextMap;
  static BuiltinCarrierFormat HttpHeaders;

private:
  BuiltinFormat format_;
};

// Base class for implementation-dependent Tracer::Inject argument.
class CarrierReader {
public:
};

// Base class for implementation-dependent Tracer::Extract argument.
class CarrierWriter {
public:
};

// HttpHeadersReader is the Extract() carrier for the HttpHeaders builtin format.
//
// Headers should be similar to a std::vector of std::pairs.
template <typename T>
class HttpHeadersReader : public CarrierReader {
public:
  explicit HttpHeadersReader(const T& data) : data_(data) { }
  virtual ~HttpHeadersReader() { }

  // TODO throws?
  virtual void ForeachKey(std::function<void(const std::string& key, const std::string& value)> f) const {
    for (const auto& e : data_) {
      f(e.first, e.second);
    }
  }

private:
  const T& data_;
};

// HttpHeadersReader is the Inject() carrier for the HttpHeaders builtin format.
//
// Headers should be similar to a std::vector of std::pairs.
template <typename T>
class HttpHeadersWriter : public CarrierWriter {
public:
  explicit HttpHeadersWriter(const T* data) : data_(data) { }
  virtual ~HttpHeadersWriter() { }

  virtual void Set(const std::string& key, const std::string& value) {
    // TODO unescaping
    data_->emplace(typename T::value_type(key, value));
  }

private:
  const T* data_;
};

// For escaping, unescaping carried values.
std::string uriQueryEscape(const std::string& str);
std::string uriQueryUnescape(const std::string& str);

} // namespace lightstep

#endif // __LIGHTSTEP_PROPAGATION_H__
