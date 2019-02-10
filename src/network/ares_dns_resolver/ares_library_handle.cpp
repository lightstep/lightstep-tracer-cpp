#include "network/ares_dns_resolver/ares_library_handle.h"

#include <ares.h>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
AresLibraryHandle::AresLibraryHandle() {
  auto status = ares_library_init(ARES_LIB_INIT_ALL);
  if (status != 0) {
    throw std::runtime_error{std::string{"ares_library_init failed: "} +
                             ares_strerror(status)};
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
AresLibraryHandle::~AresLibraryHandle() noexcept { ares_library_cleanup(); }

//--------------------------------------------------------------------------------------------------
// Instance
//--------------------------------------------------------------------------------------------------
std::shared_ptr<const AresLibraryHandle> AresLibraryHandle::Instance = [] {
  try {
    return std::shared_ptr<const AresLibraryHandle>{new AresLibraryHandle{}};
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Failed to initialize AresLibraryHandle: %s\n",
                 e.what());
    return std::shared_ptr<const AresLibraryHandle>{nullptr};
  }
}();
}  // namespace lightstep
