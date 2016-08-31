// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_RECORDER_H__
#define __LIGHTSTEP_RECORDER_H__

// Defines the Recorder interface which supports encoding and
// transporting LightStep requests to the collector.

#include "tracer.h"

namespace lightstep {

// To setup user-defined transport, configure BasicRecorderOptions.
class BasicRecorderOptions {
public:
  BasicRecorderOptions();

  Duration   time_limit; // Default is 1s
  size_t     span_limit; // Default is 1000 spans
};

Tracer NewLightStepTracer(const TracerOptions& topts,
			  const BasicRecorderOptions& bopts);

}  // namespace lightstep

#endif // __LIGHTSTEP_RECORDER_H__
