// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_RECORDER_H__
#define __LIGHTSTEP_RECORDER_H__

#include <vector>

#include "lightstep_thrift/lightstep_types.h"

namespace lightstep {

class Recorder {
public:
  void ReportSpan(lightstep_thrift::SpanRecord&& span);

private:
  // Buffered spans for the next report.
  std::vector<lightstep_thrift::SpanRecord> pending_spans_;
};

// class UserDefinedTransport : public Recorder {
// public:
// private:
// };

}  // namespace lightstep

#endif // __LIGHTSTEP_RECORDER_H__
