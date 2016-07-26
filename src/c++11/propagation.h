// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_PROPAGATION_H__
#define __LIGHTSTEP_PROPAGATION_H__

#include <functional>
#include <string>

#include "options.h"

namespace lightstep {
  
enum SpanReferenceType {
  // ChildOfRef refers to a parent Span that caused *and* somehow depends
  // upon the new child Span. Often (but not always), the parent Span cannot
  // finish unitl the child Span does.
  //
  // An timing diagram for a ChildOfRef that's blocked on the new Span:
  //
  //     [-Parent Span---------]
  //          [-Child Span----]
  //
  // See http://opentracing.io/spec/
  //
  // See opentracing.ChildOf()
  ChildOfRef = 1,

  // FollowsFromRef refers to a parent Span that does not depend in any way
  // on the result of the new child Span. For instance, one might use
  // FollowsFromRefs to describe pipeline stages separated by queues,
  // or a fire-and-forget cache insert at the tail end of a web request.
  //
  // A FollowsFromRef Span is part of the same logical trace as the new Span:
  // i.e., the new Span is somehow caused by the work of its FollowsFromRef.
  //
  // All of the following could be valid timing diagrams for children that
  // "FollowFrom" a parent.
  //
  //     [-Parent Span-]  [-Child Span-]
  //
  //
  //     [-Parent Span--]
  //      [-Child Span-]
  //
  //
  //     [-Parent Span-]
  //                 [-Child Span-]
  //
  // See http://opentracing.io/spec/
  //
  // See opentracing.FollowsFrom()
  FollowsFromRef = 2
};

class SpanReference : public SpanStartOption {
public:
  SpanReference(SpanReferenceType type, const SpanContext &referenced)
    : type_(type),
      referenced_(referenced) { }

  virtual void Apply(SpanImpl *span) const;

private:
  SpanReferenceType type_;
  SpanContext referenced_;
};

// Create a ChildOf-referencing StartSpan option.
SpanReference ChildOf(const SpanContext& ctx);

// Create a FollowsFrom-referencing StartSpan option.
SpanReference FollowsFrom(const SpanContext& ctx);

enum BuiltinFormat {
  // Binary encodes the SpanContext for propagation as opaque binary data.
  Binary = 1,  // NOT IMPLEMENTED

  // TextMap encodes the SpanContext as key:value pairs.
  TextMap = 2,  // NOT IMPLEMENTED

  // HTTPHeaders represents SpanContexts as HTTP header string pairs.
  //
  // The HTTPHeaders format requires that the keys and values be valid
  // as HTTP headers as-is (i.e., character casing may be unstable and
  // special characters are disallowed in keys, values should be
  // URL-escaped, etc).
  //
  // For Tracer::Inject(): the carrier must be a `HTTPHeadersWriter`.
  //
  // For Tracer::Extract(): the carrier must be a `HTTPHeadersReader`.
  //
  // For example, Inject():
  //
  //    std::vector<std::pair> headers;
  //    lightstep::HTTPHeadersWriter writer(&headers);
  //    span.tracer().Inject(span, lightstep::HTTPHeaders, &writer);
  //    ... apply headers to HTTP request ...
  //
  // Or Extract():
  //
  //    lightstep::HTTPHeadersReader writer(request->headers());
  //    SpanContext parent = span.tracer().Extract(lightstep::HTTPHeaders, &writer);
  //    Span child = StartSpan("childOp", { ChildOf(parent) });
  //
  HTTPHeaders = 3,
};

// Base class for implementation-provided or builtin carrier formats.
class CarrierFormat { };

// Builtin carrier format values.
class BuiltinCarrierFormat : public CarrierFormat {
public:
  explicit BuiltinCarrierFormat(BuiltinFormat format) : format_(format) { }

  static BuiltinCarrierFormat Binary;
  static BuiltinCarrierFormat TextMap;
  static BuiltinCarrierFormat HTTPHeaders;

private:
  BuiltinFormat format_;
};

// Base class for implementation-dependent Tracer::Inject argument.
class CarrierReader { };

// Base class for implementation-dependent Tracer::Extract argument.
class CarrierWriter { };

// Helper methods for the HTTP carriers.
std::string uriQueryEscape(const std::string& str);
std::string uriQueryUnescape(const std::string& str);

// HTTPHeadersReader is the Extract() carrier for the HTTPHeaders builtin format.
//
// Headers should be similar to a std::vector of std::pairs.
template <typename T>
class HTTPHeadersReader : public CarrierReader {
public:
  explicit HTTPHeadersReader(const T& data) : data_(data) { }
  virtual ~HTTPHeadersReader() { }

  // TODO throws?
  virtual void ForeachKey(std::function<void(const std::string& key, const std::string& value)> f) const {
    for (const auto& e : data_) {
      f(e.first, uriQueryEscape(e.second));
    }
  }

private:
  const T& data_;
};

// HTTPHeadersReader is the Inject() carrier for the HTTPHeaders builtin format.
//
// Headers should be similar to a std::vector of std::pairs.
template <typename T>
class HTTPHeadersWriter : public CarrierWriter {
public:
  explicit HTTPHeadersWriter(const T* data) : data_(data) { }
  virtual ~HTTPHeadersWriter() { }

  virtual void Set(const std::string& key, const std::string& value) {
    data_->emplace(typename T::value_type(key, uriQueryUnescape(value)));
  }

private:
  const T* data_;
};

} // namespace lightstep

#endif // __LIGHTSTEP_PROPAGATION_H__
