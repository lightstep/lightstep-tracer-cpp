// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_TEST_H__
#define __LIGHTSTEP_TEST_H__

#include <mutex>

#include "lightstep/tracer.h"

class error : public std::exception {
public:
  error(const char *what) : what_(what) { }
  error(const std::string& what) : what_(what) { }

  const char* what() const noexcept override { return what_.c_str(); }
private:
  const std::string what_;
};

class TestRecorder : public lightstep::Recorder {
public:
  explicit TestRecorder(const lightstep::TracerImpl& impl) : builder_(impl) { }

  virtual void RecordSpan(lightstep::collector::Span&& span) override {
    std::lock_guard<std::mutex> lock(mutex_);
    builder_.addSpan(std::move(span));
  }
  virtual bool FlushWithTimeout(lightstep::Duration timeout) override {
    request_ = builder_.pending();
    return true;
  }

  std::mutex mutex_;
  lightstep::ReportBuilder builder_;
  lightstep::collector::ReportRequest request_;
};

#endif // __LIGHTSTEP_TEST_H__
