#pragma once

#include <array>
#include <limits>
#include <memory>
#include <cassert>

#include "common/function_ref.h"
#include "common/fragment_input_stream.h"
#include "common/noncopyable.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
/**
 * Maintains a linked chain of blocks as they aree written
 */
class ChainedStream final
    : public google::protobuf::io::ZeroCopyOutputStream,
      public FragmentInputStream,
      private Noncopyable {
 public:
  static const int BlockSize = 256;

  ChainedStream() noexcept;

  // ZeroCopyOutputStream
  bool Next(void** data, int* size) override;

  void BackUp(int count) override;

  google::protobuf::int64 ByteCount() const override {
    return static_cast<google::protobuf::int64>(num_bytes_written_);
  }

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(Callback callback) const noexcept override;

  void Clear() noexcept override;

  void Seek(int relative_fragment_index, int position) noexcept override;

 private:
  struct Block {
    std::unique_ptr<Block> next;
    int size;
    std::array<char, BlockSize> data;
  };

  int num_blocks_{1};
  int num_bytes_written_{0};
  int num_bytes_after_framing_{0};
  int current_block_position_{0};
  Block* current_block_;

  int fragment_index_{0};
  int fragment_position_{0};

  Block head_;
};
} // namespace lightstep
