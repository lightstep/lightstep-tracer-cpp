#include "test/zero_copy_connection_input_stream.h"

#include <algorithm>
#include <random>

static thread_local std::mt19937 RandomNumberGenerator{std::random_device{}()};

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// CountBytes
//--------------------------------------------------------------------------------------------------
static int CountBytes(std::initializer_list<FragmentInputStream*> streams) {
  int result = 0;
  for (auto stream : streams) {
    stream->ForEachFragment([&result](void* /*data*/, int size) {
      result += size;
      return true;
    });
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// CopyN
//--------------------------------------------------------------------------------------------------
static void CopyN(std::initializer_list<FragmentInputStream*> streams, int n,
                  std::string& s) {
  for (auto stream : streams) {
    auto should_continue =
        stream->ForEachFragment([&n, &s](void* data, int size) {
          if (n <= size) {
            s.append(static_cast<char*>(data), n);
            return false;
          }
          s.append(static_cast<char*>(data), size);
          n -= size;
          return true;
        });
    if (!should_continue) {
      return;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ZeroCopyConnectionInputStream::ZeroCopyConnectionInputStream(
    ConnectionStream& stream)
    : stream_{stream} {}

//--------------------------------------------------------------------------------------------------
// Next
//--------------------------------------------------------------------------------------------------
bool ZeroCopyConnectionInputStream::Next(const void** data, int* size) {
  if (position_ == static_cast<int>(buffer_.size())) {
    SetBuffer();
  }
  if (stream_.completed() && buffer_.empty()) {
    return false;
  }
  *data = static_cast<const void*>(buffer_.data() + position_);
  *size = static_cast<int>(buffer_.size()) - position_;
  position_ = static_cast<int>(buffer_.size());
  byte_count_ += *size;
  return true;
}

//--------------------------------------------------------------------------------------------------
// BackUp
//--------------------------------------------------------------------------------------------------
void ZeroCopyConnectionInputStream::BackUp(int count) {
  assert(position_ >= count);
  byte_count_ -= count;
  position_ -= count;
}

//--------------------------------------------------------------------------------------------------
// Skip
//--------------------------------------------------------------------------------------------------
bool ZeroCopyConnectionInputStream::Skip(int count) {
  while (true) {
    if (stream_.completed() && buffer_.empty()) {
      return false;
    }
    if (count == 0) {
      return true;
    }
    if (position_ == static_cast<int>(buffer_.size())) {
      SetBuffer();
    }
    auto delta = static_cast<int>(buffer_.size()) - position_;
    auto n = std::min(count, delta);
    position_ += n;
    byte_count_ += n;
    count -= n;
  }
}

//--------------------------------------------------------------------------------------------------
// SetBuffer
//--------------------------------------------------------------------------------------------------
void ZeroCopyConnectionInputStream::SetBuffer() {
  buffer_.clear();
  position_ = 0;
  stream_.Flush([this](std::initializer_list<FragmentInputStream*> streams) {
    auto num_bytes = CountBytes(streams);
    if (num_bytes == 0) {
      return true;
    }
    std::uniform_int_distribution<int> distribution{0, num_bytes};
    auto n = distribution(RandomNumberGenerator);
    CopyN(streams, n, buffer_);
    return Consume(streams, n);
  });
}
}  // namespace lightstep
