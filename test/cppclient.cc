#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "tracer.h"
#include "impl.h"

#include "dropbox_json/json11.hpp"
#include "zintinio_happyhttp/happyhttp.h"

namespace {

const char hostName[] = "localhost";
const int  hostPort = 8000;
const char controlPath[] = "/control";
const char resultPath[] = "/result";
const double nanosPerSecond = 1e9;

// logs_memory = None
// logs_size_max = 1<<20

using namespace json11;

lightstep::Tracer NewTracer() {
  lightstep::TracerOptions topts;
  topts.access_token = "ignored";
  topts.collector_host = hostName;
  topts.collector_port = hostPort;
  topts.collector_encryption = "none";
  return lightstep::NewTracer(topts);
}

double to_seconds(lightstep::TimeStamp::duration d) {
  using namespace std::chrono;
  return static_cast<double>(
      duration_cast<nanoseconds>(d).count()) / nanosPerSecond;
}

struct UD {
  std::string payload;
  bool complete = false;
};

class Test {
public:
  Test()
    : test_tracer_(NewTracer()),
      noop_tracer_(nullptr),
      conn_(hostName, hostPort) {
    conn_.setcallbacks([](const happyhttp::Response* r, void* ud) {},
		       [](const happyhttp::Response* r, void* ud, const unsigned char* data, int size) {
			 reinterpret_cast<UD*>(ud)->payload.append(std::string(reinterpret_cast<const char*>(data), size));
		       },
		       [](const happyhttp::Response* r, void* ud) {
			 reinterpret_cast<UD*>(ud)->complete = true;
		       },
		       &data_);
  }

  void run_benchmark();

private:
  Json get_control();

  uint64_t do_work(uint64_t work);

  void test_body(const lightstep::Tracer& tracer,
		 const Json& control,
		 std::string* sleep_nanos,
		 uint64_t* answer);

  std::string get(const std::string &path) {
    data_.complete = false;
    conn_.request("GET", path.c_str());
    while (!data_.complete) {
      conn_.pump();
    }
    return std::move(data_.payload);
  }
  
  lightstep::Tracer noop_tracer_;
  lightstep::Tracer test_tracer_;

  happyhttp::Connection conn_;
  UD data_;
};

Json Test::get_control() {
  auto c = get(controlPath);
  std::string errors;
  Json control = Json::parse(c, errors);
  if (!errors.empty()) {
    throw std::exception();
  }
  return control;
}

uint64_t Test::do_work(uint64_t work) {
  const uint64_t prime_work = 982451653;
  uint64_t x = prime_work;

  for (uint64_t i = 0; i < work; i++) {
    x *= prime_work;
  }

  return x;
}

void Test::test_body(const lightstep::Tracer& tracer,
		     const Json&   control,
		     std::string*  sleep_nanos,
		     uint64_t*     answer) {
  auto repeat    = static_cast<uint64_t>(control["Repeat"].number_value());
  auto sleepnano = static_cast<uint64_t>(control["Sleep"].number_value());
  auto sleepival = static_cast<uint64_t>(control["SleepInterval"].number_value());
  auto work      = static_cast<uint64_t>(control["Work"].number_value());
  auto logn      = static_cast<uint64_t>(control["NumLogs"].number_value());
  auto logsz     = static_cast<uint64_t>(control["BytesPerLog"].number_value());

  uint64_t sleep_debt = 0;

  for (uint64_t i = 0; i < repeat; i++) {
    lightstep::Span span = tracer.StartSpan("span/test");
    // TODO logging test
    *answer = do_work(work);
    span.Finish();

    if (sleepnano == 0) {
      continue;
    }

    sleep_debt += sleepnano;

    if (sleep_debt < sleepival) {
      continue;
    }

    using namespace std::chrono;
    lightstep::TimeStamp sleep = lightstep::Clock::now();
    std::this_thread::sleep_for(nanoseconds(sleep_debt));
    lightstep::TimeStamp awake = lightstep::Clock::now();
    uint64_t elapsed_nanos = duration_cast<nanoseconds>(awake - sleep).count();
    sleep_debt -= elapsed_nanos;
    if (!sleep_nanos->empty()) {
      sleep_nanos->append(",");
    }
    *sleep_nanos += std::to_string(elapsed_nanos);
  }
}

void Test::run_benchmark() {
  conn_.connect();
  while (true) {
    Json control = get_control();
    if (control["Exit"].bool_value()) {
      break;
    }
    lightstep::Tracer tracer(nullptr);
    if (control["Trace"].bool_value()) {
      tracer = test_tracer_;
    } else {
      tracer = noop_tracer_;
    }
    lightstep::TimeStamp start = lightstep::Clock::now();

    std::string sleep_nanos;
    uint64_t answer;
    // TODO concurrency test
    test_body(tracer, control, &sleep_nanos, &answer);

    lightstep::TimeStamp finish = lightstep::Clock::now();
    lightstep::TimeStamp flushed = finish;

    if (control["Trace"].bool_value()) {
      // TODO explicit Flush is not implemented, anyway
      tracer.impl()->Flush();
      flushed = lightstep::Clock::now();
    }

    std::stringstream rpath;
    rpath << resultPath << "?timing=" << to_seconds(finish - start)
	  << "&flush=" << to_seconds(flushed - finish)
	  << "&s=" << sleep_nanos
	  << "&a=" << answer;

      get(rpath.str());
  }
  test_tracer_.impl()->Stop();
}

} // namespace

int main(int, char**) {
  std::set_terminate([]() {
      std::cerr << "Top-level exception" << std::endl;
      abort();
    });
  Test test;
  try {
    test.run_benchmark();
  } catch (happyhttp::Wobbly& e) {
    std::cerr << "HTTP Exception: " << e.what() << std::endl;
    throw;
  }
  return 0;
}
