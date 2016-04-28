#include "recorder.h"

namespace lightstep {

void Recorder::ReportSpan(lightstep_thrift::SpanRecord&& span) {
  pending_spans_.emplace_back(std::move(span));
}

// TODO
  //Runtime runtime;
  //ReportRequest report;
  // runtime.__set_guid(tracer_->runtime_guid_);
  // runtime.__set_start_micros(tracer_->runtime_micros_);
  // runtime.__set_group_name(tracer_->component_name_);
  // runtime.__set_attrs(tracer_->runtime_attributes_);

  // report.__set_runtime(std::move(runtime));
  // report.__set_span_records({std::move(span)});

  // int n = report.write(&proto);

  // uint8_t *buffer;
  // uint32_t length;
  // memory->getBuffer(&buffer, &length);
  // if (length != n) abort();
  // tracer_->options_.binary_transport(buffer, length);


}  // namespace lightstep
