#pragma once

#include <atomic>

namespace lightstep {
template <class Node>
void ListPushFront(std::atomic<Node*>& head, Node& node) noexcept {
  Node* expected_head = head;
  while (true) {
    node.next = expected_head;
    if (head.compare_exchange_weak(expected_head, &node,
                                   std::memory_order_release,
                                   std::memory_order_relaxed)) {
      return;
    }
  }
}

template <class Node>
Node* ListPopFront(std::atomic<Node*>& head) noexcept {
  Node* expected_head = head;
  while (true) {
    if (expected_head == nullptr) {
      return nullptr;
    }
    if (head.compare_exchange_weak(expected_head, expected_head->next,
                                   std::memory_order_release,
                                   std::memory_order_relaxed)) {
      expected_head->next = nullptr;
      return expected_head;
    }
  }
}
}  // namespace lightstep
