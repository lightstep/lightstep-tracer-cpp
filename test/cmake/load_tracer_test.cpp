#include <iostream>

#include <opentracing/dynamic_load.h>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: <tracer_library>\n";
    return -1;
  }

  // Verify we can load the tracer library.
  std::string error_message;
  auto handle_maybe =
      opentracing::DynamicallyLoadTracingLibrary(argv[1], error_message);
  if (!handle_maybe) {
    std::cerr << "Failed to load tracer library " << error_message << "\n";
    return -1;
  }

  return 0;
}
