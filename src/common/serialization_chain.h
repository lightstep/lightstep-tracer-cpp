#pragma once

#include <array>
#include <limits>
#include <memory>

#include "common/block_allocator.h"
#include "common/fragment_input_stream.h"
#include "common/noncopyable.h"
#include "common/serialization.h"
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
  static const int FirstBlockSize = 256;
  static const size_t ReportRequestSpansField = 3;

  explicit SerializationChain(BlockAllocator& allocator) noexcept;

  SerializationChain() noexcept;

  ~SerializationChain() noexcept;

  /**
   * Adds http/1.1 chunk framing and a message header so that the data can be
   * parsed as part of a protobuf ReportRequest.
   */
  void AddFraming() noexcept;

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
  BlockAllocator& allocator_;

  struct Block {
    Block* next;
    int max_size;
    int size;

    char* data() noexcept {
      return reinterpret_cast<char*>(&size) + sizeof(size);
    }

    const char* data() const noexcept {
      return const_cast<Block*>(this)->data();
    }
  };

  struct HeaderBlock : Block {
    std::array<char, FirstBlockSize> data;
  };

  int num_blocks_{1};
  int num_bytes_written_{0};
  int current_block_position_{0};
  int header_size_{0};
  Block* current_block_;

  int fragment_index_{0};
  int fragment_position_{0};

  HeaderBlock head_;
  static const size_t MaxHeaderSize =
      Num64BitHexDigits + 2 +
      StaticKeySerializationSize<ReportRequestSpansField,
                                 WireType::LengthDelimited>::value +
      google::protobuf::io::CodedOutputStream::StaticVarintSize32<
          std::numeric_limits<uint32_t>::max()>::value;
  std::array<char, MaxHeaderSize> header_;

  Block* AllocateNewBlock();

  void FreeBlocks(Block* block) noexcept;
};
}  // namespace lightstep
