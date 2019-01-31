#pragma once

namespace lightstep {
class AresLibraryHandle {
 public:
  bool initialized() const noexcept;

  // We use a global variable to initialize the ares library because
  // ares_library_init is not thread-safe and must be called before other
  // threads are started.
  //
  // See https://c-ares.haxx.se/ares_library_init.html
  static const AresLibraryHandle Instance;

 private:
  AresLibraryHandle() noexcept;

  ~AresLibraryHandle() noexcept;

  int status_;
};
}  // namespace lightstep
