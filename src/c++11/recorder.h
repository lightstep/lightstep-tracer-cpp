// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_RECORDER_H__
#define __LIGHTSTEP_RECORDER_H__

// TODO This class does not compile--fix. Presently this code can only
// be built with configure --disable-cpp-netlib.

// Defines the Recorder interface which supports encoding and
// transporting LightStep requests to the collector.

#include <mutex>
#include <utility>
#include <vector>

namespace lightstep_net {
class SpanRecord;
}  // namespace lightstep_net

namespace lightstep {

class TracerImpl;

// NewDefaultRecorder returns a Recorder implementation that writes to
// LightStep using a background thread that flushes periodically.
std::unique_ptr<Recorder> NewDefaultRecorder(const TracerImpl &impl);

}  // namespace lightstep

#endif // __LIGHTSTEP_RECORDER_H__
