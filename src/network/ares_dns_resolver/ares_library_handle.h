#pragma once

#include <memory>

#include "common/noncopyable.h"

namespace lightstep {
/**
 * Manages initialization of the c-ares library.
 */
class AresLibraryHandle : private Noncopyable {
 public:
  ~AresLibraryHandle() noexcept;

  // We use a global variable to initialize the ares library because
  // ares_library_init is not thread-safe and must be called before other
  // threads are started.
  //
  // See https://c-ares.haxx.se/ares_library_init.html
  static std::shared_ptr<const AresLibraryHandle> Instance;

 private:
  AresLibraryHandle();
};
}  // namespace lightstep
