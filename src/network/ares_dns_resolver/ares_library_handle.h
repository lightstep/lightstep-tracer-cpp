#pragma once

namespace lightstep {
/**
 * Manages initialization of the c-ares library.
 */
class AresLibraryHandle {
 public:
  AresLibraryHandle(const AresLibraryHandle&) = delete;
  AresLibraryHandle(AresLibraryHandle&&) = delete;

  AresLibraryHandle& operator=(const AresLibraryHandle&) = delete;
  AresLibraryHandle& operator=(AresLibraryHandle&&) = delete;

  /**
   * @return if the libevent library initialization was successful.
   */
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
