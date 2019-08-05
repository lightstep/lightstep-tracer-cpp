#pragma once

#include <cassert>
#include <cstddef>
#include <atomic>
#include <memory>

namespace lightstep {
class BlockAllocator {
 public:
  BlockAllocator(size_t block_size, size_t max_blocks);

  ~BlockAllocator() noexcept;
 
  void* allocate() noexcept;

  void deallocate(void* ptr) noexcept;

  size_t block_size() const noexcept { return block_size_; }

  size_t max_blocks() const noexcept { return max_blocks_; }
 private:
  size_t block_size_;
  size_t max_blocks_;

  struct Node {
    Node* next;
  };
  std::atomic<Node*> free_list_{nullptr};
  std::atomic<int64_t> reserve_index_{0};
  char* data_;
};

template <class T>
std::shared_ptr<BlockAllocator>& GetBlockAllocator(size_t max_blocks = 10000) {
  static_assert(sizeof(T) >= sizeof(void*),
                "T must be at least as large as a pointer");
  static auto result = std::make_shared<BlockAllocator>(sizeof(T), max_blocks);
  return result;
}
} // namespace lightstep
