#define CATCH_CONFIG_RUNNER
#include <stdio.h>
#include "catch.hpp"
//#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
//#include <unistd.h>

#include "windows.h"

#define MAX_SYMBOLS 50 // because it's less than 63

// Taken from https://stackoverflow.com/a/77336/4447365
void handler(int sig) {
  void* symbol_pointers[MAX_SYMBOLS];

  // get void*'s for all entries on the stack
  // NULL so we don't compute a hash
  // https://
  // docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb204633(v=vs.85)
   size_t size = CaptureStackBackTrace(0, MAX_SYMBOLS, symbol_pointers, NULL);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);

  for (int i = 0; i < size; i++) {
    fprintf(stderr, "%s", symbol_pointers[i]);
  }

  exit(1);
}

int main(int argc, char* argv[]) {
  signal(SIGSEGV, handler);
  signal(SIGTERM, handler);

  int result = Catch::Session().run(argc, argv);

  return result;
}
