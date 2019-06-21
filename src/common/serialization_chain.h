#pragma once

#include <array>
#include <memory>

#include "common/fragment_input_stream.h"
#include "common/noncopyable.h"
#include "common/utility.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
/**
 * Maintains a linked chain of blocks for a serialization.
 */
class SerializationChain final
    : public google::protobuf::io::ZeroCopyOutputStream,
      public FragmentInputStream,
      private Noncopyable {
 public:
  static const int BlockSize = 256;

  SerializationChain() noexcept;

  /**
   * Adds http/1.1 chunk framing
   */
  void AddChunkFraming() noexcept;

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

  void Seek(int fragment_index, int position) noexcept override;

 private:
  struct Block {
    std::unique_ptr<Block> next;
    int size;
    std::array<char, BlockSize> data;
  };

  int num_blocks_{1};
  int num_bytes_written_{0};
  int current_block_position_{0};
  int chunk_header_size_{0};
  Block* current_block_;

  int fragment_index_{0};
  int fragment_position_{0};

  std::array<char, Num64BitHexDigits + 2> chunk_header_;
  Block head_;
};
}  // namespace lightstep
