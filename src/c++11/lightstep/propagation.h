// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_PROPAGATION_H__
#define __LIGHTSTEP_PROPAGATION_H__

#include <functional>
#include <string>

#include "options.h"

namespace lightstep {

class ContextImpl;

// SpanContext is an immutable span context.
class SpanContext {
public:
  // Returns a !valid() span context.
  SpanContext() { }

  uint64_t trace_id() const;
  uint64_t span_id() const;

  // ForeachBaggageItem calls a function for each baggage item in the
  // context.  If the function returns false, it will not be called
  // again and ForeachBaggageItem will return.
  void ForeachBaggageItem(std::function<bool(const std::string& key,
					     const std::string& value)> f) const;

  // Note: opentracing to possibly add std::string BaggageItem(const std::string& key);
  // See https://github.com/opentracing/opentracing.github.io/issues/106

  // Returns true if the context is initialized.  A span context may not be valid
  // because Extract() encounters an error.
  bool valid() const { return ctx() != nullptr; }

private:
  friend class Span;
  friend class SpanImpl;
  friend class TracerImpl;

  const ContextImpl* ctx() const;

  explicit SpanContext(std::shared_ptr<SpanImpl> span) : span_ctx_(span) { }
  explicit SpanContext(std::shared_ptr<ContextImpl> ctx) : impl_ctx_(ctx) { }

  std::shared_ptr<SpanImpl>    span_ctx_;
  std::shared_ptr<ContextImpl> impl_ctx_;
};

enum SpanReferenceType {
  // NoRef is the type of an uninitialized reference.
  NoRef = 0,

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
  SpanReference(const SpanReference& o)
    : type_(o.type_), referenced_(o.referenced_) { }
  SpanReference() : type_(NoRef) { }

  // Returns true iff this is a valid reference.
  bool valid() const { return type_ != NoRef && referenced_.valid(); }

  SpanContext referenced() const { return referenced_; }

  // This is a no-op if the referenced context is not valid.
  virtual void Apply(SpanImpl *span) const override;

private:
  SpanReferenceType type_;
  SpanContext referenced_;
};

// Create a ChildOf-referencing StartSpan option.
SpanReference ChildOf(const SpanContext& ctx);

// Create a FollowsFrom-referencing StartSpan option.
SpanReference FollowsFrom(const SpanContext& ctx);

// Carrier format values.
class CarrierFormat {
public:
  enum FormatType {
    // Binary encodes the SpanContext for propagation as opaque binary data.
    Binary = 1,  // RESERVED, NOT IMPLEMENTED
    
    // HTTPHeaders represents SpanContexts as HTTP header string pairs.
    //
    // The HTTPHeaders format requires that the keys and values be valid
    // as HTTP headers as-is (i.e., character casing may be unstable and
    // special characters are disallowed in keys, values should be
    // URL-escaped, etc).
    //
    // For Tracer::Inject(): the carrier must be a `TextMapReader`.
    //
    // For Tracer::Extract(): the carrier must be a `TextMapWriter`.
    //
    // For example, Inject():
    //
    //   std::vector<std::pair<std::string, std::string>> *headers = ...;
    //   if (!span.tracer().Inject(span, CarrierFormat::HTTPHeadersCarrier,
    // 	  			   make_ordered_string_pairs_writer(headers))) {
    //     throw error("inject failed");
    //   }
    //
    // Or Extract():
    //
    //   SpanContext extracted;
    //   extracted = Tracer::Global().Extract(CarrierFormat::HTTPHeadersCarrier,
    //                                        make_ordered_string_pairs_reader(*headers));
    //   auto span = Tracer::Global().StartSpan("op", { ChildOf(extracted) });
    //
    HTTPHeaders = 2,

    // TextMap encodes the SpanContext as key:value pairs.
    //
    // The TextMap format is similar to the HTTPHeaderes format,
    // without restrictions on the character set.
    //
    // For Tracer::Inject(): the carrier must be a `TextMapReader`.
    //
    // For Tracer::Extract(): the carrier must be a `TextMapWriter`.
    //
    // See the HTTPHeaders examples.
    TextMap = 3,

    // EnvoyProto carriers use a serialized protocol message.
    EnvoyProto = 4,
  };

  static CarrierFormat TextMapCarrier;
  static CarrierFormat HTTPHeadersCarrier;
  static CarrierFormat EnvoyProtoCarrier;

  CarrierFormat(FormatType type) : type_(type) { }

  FormatType type() const { return type_; }

private:
  const FormatType type_;
};

// Base class for implementation-dependent Tracer::Inject carrier-type adapter.
class CarrierReader {
public:
  virtual ~CarrierReader() { }
};

// Basic foundation for OpenTracing basictracer-compatible carrier readers.
class BasicCarrierReader : public CarrierReader {
public:
  virtual void ForeachKey(std::function<void(const std::string& key,
					     const std::string& value)> f) const = 0;
};

// Base class for implementation-dependent Tracer::Extract carrier-type adapter.
class CarrierWriter {
public:
  virtual ~CarrierWriter() { }
};

// Basic foundation for OpenTracing basictracer-compatible carrier writers.
class BasicCarrierWriter : public CarrierWriter {
public:
  virtual void Set(const std::string& key, const std::string& value) const = 0;
};

// Base class for injecting into TextMap and HTTPHeaders carriers.
class TextMapReader : public BasicCarrierReader {
  // TODO distinguish TextMap and HTTPHeaders behavior.
};

// Base class for extracting from TextMap and HTTPHeaders carriers.
class TextMapWriter : public BasicCarrierWriter {
  // TODO distinguish TextMap and HTTPHeaders behavior.
};

// OrderedStringPairsReader is a TextMapReader for any container
// similar to a std::vector of std::pair<std::string, std::string>.
template <typename T>
class OrderedStringPairsReader : public TextMapReader {
public:
  explicit OrderedStringPairsReader(const T& data) : data_(data) { }

  virtual void ForeachKey(std::function<void(const std::string& key,
					     const std::string& value)> f) const override {
    for (const auto& e : data_) {
      f(e.first, e.second);
    }
  }

private:
  const T& data_;
};

template <typename T>
OrderedStringPairsReader<T> make_ordered_string_pairs_reader(const T& arg) {
  return OrderedStringPairsReader<T>(arg);
}

// OrderedStringPairsReader is the Inject() carrier for the HTTPHeaders builtin format.
//
// Headers should be similar to a std::vector of std::pairs.
template <typename T>
class OrderedStringPairsWriter : public TextMapWriter {
public:
  explicit OrderedStringPairsWriter(T* data) : data_(data) { }

  virtual void Set(const std::string& key, const std::string& value) const override {
    data_->push_back(typename T::value_type(key, value));
  }

private:
  T* const data_;
};

template <typename T>
OrderedStringPairsWriter<T> make_ordered_string_pairs_writer(T* arg) {
  return OrderedStringPairsWriter<T>(arg);
}

} // namespace lightstep

#endif // __LIGHTSTEP_PROPAGATION_H__
