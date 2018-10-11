#include <lightstep/terminable_condition_variable.h>

#include <cassert>

namespace lightstep {
//------------------------------------------------------------------------------
// Terminate
//------------------------------------------------------------------------------
void TerminableConditionVariable::Terminate() noexcept {
  {
    std::unique_lock<std::mutex> lock{mutex_};
    is_active_ = false;
  }
  condition_variable_.notify_all();
}
}  // namespace lightstep
