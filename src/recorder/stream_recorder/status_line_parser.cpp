#include "recorder/stream_recorder/status_line_parser.h"

#include <algorithm>
#include <stdexcept>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------
void StatusLineParser::Reset() noexcept {
  line_.clear();
  complete_ = false;
}

//--------------------------------------------------------------------------------------------------
// Parse
//--------------------------------------------------------------------------------------------------
void StatusLineParser::Parse(opentracing::string_view s) {
  if (complete_) {
    return;
  }
  auto i = std::find(s.begin(), s.end(), '\r');
  line_.append(s.data(), static_cast<size_t>(std::distance(s.begin(), i)));
  complete_ = i != s.end();
  if (complete_) {
    ParseFields();
  }
}

//--------------------------------------------------------------------------------------------------
// ParseFields
//--------------------------------------------------------------------------------------------------
void StatusLineParser::ParseFields() try {
  auto status_code_first = std::find(line_.begin(), line_.end(), ' ');
  if (status_code_first == line_.end()) {
    throw std::runtime_error{"Invalid http status line \"" + line_ + "\""};
  }
  auto status_code_last =
      std::find(std::next(status_code_first), line_.end(), ' ');
  if (status_code_last == line_.end()) {
    throw std::runtime_error{"Invalid http status line \"" + line_ + "\""};
  }
  status_code_ = std::atoi(&*status_code_first);
  auto reason_first = std::next(status_code_last);
  reason_ = opentracing::string_view{
      &*reason_first,
      static_cast<size_t>(std::distance(reason_first, line_.end()))};
} catch (...) {
  Reset();
  throw;
}
}  // namespace lightstep
