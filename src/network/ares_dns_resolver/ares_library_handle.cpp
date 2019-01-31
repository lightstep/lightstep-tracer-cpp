#include "network/ares_dns_resolver/ares_library_handle.h"

#include <ares.h>
#include <cstdio>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
AresLibraryHandle::AresLibraryHandle() noexcept {
  status_ = ares_library_init(ARES_LIB_INIT_ALL);
  if (!initialized()) {
    printf("ares_library_init failed: %s", ares_strerror(status_));
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
AresLibraryHandle::~AresLibraryHandle() noexcept {
  if (initialized()) {
    ares_library_cleanup();
  }
}

//--------------------------------------------------------------------------------------------------
// initialized
//--------------------------------------------------------------------------------------------------
bool AresLibraryHandle::initialized() const noexcept {
  return status_ == ARES_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Instance
//--------------------------------------------------------------------------------------------------
const AresLibraryHandle AresLibraryHandle::Instance{};
}  // namespace lightstep
