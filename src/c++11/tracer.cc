#include <atomic>
#include <iostream>
#include <memory>

#include "impl.h"
#include "tracer.h"
#include "dropbox_json/json11.hpp"

namespace lightstep {
namespace {

using namespace json11;
using namespace lightstep_net;

std::atomic<ImplPtr*> global_tracer;

}  // namespace

Tracer Tracer::Global() {
  ImplPtr *ptr = global_tracer.load();
  if (ptr == nullptr) {
    return Tracer(nullptr);
  }
  // Note: there is an intentional race here.  InitGlobal releases the
  // shared pointer reference without synchronization.
  return Tracer(*ptr);
}

Tracer Tracer::InitGlobal(Tracer newt) {
  ImplPtr *newptr = new ImplPtr(newt.impl_);
  if (auto oldptr = global_tracer.exchange(newptr)) {
    delete oldptr;
  }
  return Tracer(*newptr);
}

Span Tracer::StartSpan(const std::string& operation_name) const {
  return StartSpanWithOptions(StartSpanOptions().
			      SetOperationName(operation_name));
}

Span Tracer::StartSpanWithOptions(const StartSpanOptions& options) const {
  if (!impl_) return Span();
  return Span(impl_->StartSpan(impl_, options));
}

JsonEncoder::JsonEncoder(const TracerImpl& tracer)
  : tracer_(tracer) {
  setJsonPrefix();
}

void JsonEncoder::recordSpan(lightstep_net::SpanRecord&& span) {
  if (assembled_ < 0) {
    assembled_ = 0;
    setJsonPrefix();
  }
  Json::object s;
  s["span_guid"] = Json(span.span_guid);
  s["trace_guid"] = Json(span.trace_guid);
  s["runtime_guid"] = Json(span.runtime_guid);
  s["span_name"] = Json(span.span_name);
  s["oldest_micros"] = Json(double(span.oldest_micros));
  s["youngest_micros"] = Json(double(span.youngest_micros));
  s["error_flag"] = Json(span.error_flag);
  if (span.join_ids.empty()) {
    s["join_ids"] = traceJoinArray(span.join_ids);
  }
  if (!span.attributes.empty()) {
    s["attributes"] = keyValueArray(span.attributes);
  }
  MutexLock lock(mutex_);
  if (assembled_++ != 0) {
    assembly_.append(", ");
  }
  Json(s).dump(assembly_);
}

std::string& JsonEncoder::jsonString() {
  if (assembled_ >= 0) {
    addJsonSuffix();
    assembled_ = -1;
  }
  return assembly_;
}
  
Json JsonEncoder::keyValueArray(const std::vector<KeyValue>& v) {
  Json::array a;
  for (const auto& x : v) {
    Json::object p;
    p["Key"] = Json(x.Key);
    p["Value"] = Json(x.Value);
    a.emplace_back(Json(p));
  }
  return Json(a);
}
Json JsonEncoder::keyValueArray(const std::unordered_map<std::string, std::string>& v) {
  Json::array a;
  for (const auto& x : v) {
    Json::object p;
    p["Key"] = Json(x.first);
    p["Value"] = Json(x.second);
    a.emplace_back(Json(p));
  }
  return Json(a);
}
Json JsonEncoder::traceJoinArray(const std::vector<TraceJoinId>& v) {
  Json::array a;
  for (const auto& x : v) {
    Json::object p;
    p["TraceKey"] = Json(x.TraceKey);
    p["Value"] = Json(x.Value);
    a.emplace_back(Json(p));
  }
  return Json(a);
}
  
void JsonEncoder::setJsonPrefix() {
  assembled_ = 0;
  assembly_.clear();
  assembly_.append("{ ");
  addReportFields();
  assembly_.append(", \"span_records\": [ ");
}

void JsonEncoder::addReportFields() {
  Json::object r;
  r["guid"] = Json(tracer_.runtime_guid());
  r["start_micros"] = Json(double(tracer_.runtime_start_micros()));
  r["group_name"] = Json(tracer_.component_name());
  if (!tracer_.runtime_attributes().empty()) {
    r["attrs"] = keyValueArray(tracer_.runtime_attributes());
  }

  assembly_.append("\"runtime\": ");
  Json(r).dump(assembly_);

  // TODO Add more "oldest_micros", "youngest_micros",
  // "timestamp_offset_micros", "log_records", "internal_logs",
  // "internal_metrics".
}

void JsonEncoder::addJsonSuffix() {
  // assembly_ contains { "runtime": ..., ..., "span_records": [ ...
  // now close the span_records array and report request object:
  assembly_.append("] }");
}

}  // namespace lightstep
