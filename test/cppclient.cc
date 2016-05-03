#include <iostream>

#include "tracer.h"
#include "impl.h"

#include "dropbox_json/json11.hpp"
#include "zintinio_happyhttp/happyhttp.h"

// test_tracer = lightstep.tracer.init_tracer(
//     access_token='ignored',
//     secure=False,
//     service_host='localhost',
//     service_port=8000,
// )

// noop_tracer = opentracing.tracer
// base_url = 'http://localhost:8000'
// prime_work = 982451653
// logs_memory = None
// logs_size_max = 1<<20
// nanos_per_second = 1e9

namespace {

const char hostName[] = "localhost";
const int  hostPort = 8000;
const char controlPath[] = "/control";
const char resultPath[] = "/result";

lightstep::Tracer real_tracer(nullptr);
lightstep::Tracer test_tracer(nullptr);

using namespace json11;

lightstep::Tracer NewTracer() {
  lightstep::TracerOptions topts;
  topts.access_token = "ignored";
  topts.collector_host = hostName;
  topts.collector_port = hostPort;
  topts.collector_encryption = "none";
  return lightstep::NewTracer(topts);
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
    conn_.connect();
  }

  void run_benchmark();

private:
  Json get_control();

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
  try {
    auto c = get(controlPath);
    std::string errors;
    Json control = Json::parse(c, errors);
    if (!errors.empty()) {
      throw std::exception();
    }
    return control;
  } catch (happyhttp::Wobbly& e) {
    std::cerr << "HTTP Exception: " << e.what() << std::endl;
    throw;
  }
}

void Test::run_benchmark() {
  while (true) {
    Json control = get_control();
    Json::object cobject = control.object_items();
    if (cobject["Exit"].bool_value()) {
      break;
    }

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
  test.run_benchmark();
  return 0;
}
