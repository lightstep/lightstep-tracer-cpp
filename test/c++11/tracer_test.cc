#include <iostream>
#include <chrono>
#include <thread>
#include <exception>

#include "lightstep/impl.h"
#include "lightstep/recorder.h"
#include "lightstep/tracer.h"

using namespace lightstep;

class error : public std::exception {
public:
  error(const char *what) : what_(what) { }

  const char* what() const noexcept override { return what_; }
private:
  const char *what_;
};

int main() {
  try {
    Value val1 = 0.54321;
    Value val2 = "pooh";
    Values val3 = { val1, val2 };
    Dictionary val4 = { std::make_pair("hello", val3),
			std::make_pair("whatever", val2) };

    auto span = Tracer::Global().StartSpan("span/untraced", {
	SetTag("hello", std::move(val3)),
	SetTag("whatever", std::move(val2)),
      });
    span.Finish();

    TracerOptions topts;

    topts.access_token = "3e5170e56cc4f4b2c94695a13ddf23d1";
    topts.collector_host = "localhost";
    topts.collector_port = 9997;
    topts.collector_encryption = "none";

    BasicRecorderOptions bopts;

    Tracer::InitGlobal(NewLightStepTracer(topts, bopts));

    for (int i = 0; i < 10; i++) {
      span = Tracer::Global().StartSpan("span/test");
      span.Finish();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    span = Tracer::Global().StartSpan("span/parent");
    auto parent = span.context();
    span.Finish();

    auto cspan = Tracer::Global().StartSpan("span/child", { ChildOf(parent) });
    cspan.Finish();

    Tracer::Global().impl()->Flush();
    std::cerr << "Success!" << std::endl;
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
