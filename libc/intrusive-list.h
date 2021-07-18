#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

class IntrusiveList {
 public:
  class Node {
    friend class IntrusiveList;
    void InsertAfter(Node& node) {
      node.prev_ = this;
      node.next_ = next_;
      next_->prev_ = &node;
      next_ = &node;
    }

    void InsertBefore(Node& node) {
      node.next_ = this;
      node.prev_ = prev_;
      prev_->next_ = &node;
      prev_ = &node;
    }

    Node* prev_;
    Node* next_;
  };

  using iterator = Node*;

  iterator begin() { return node_.next_; }

  iterator end() { return &node_; }

  explicit IntrusiveList() {
    node_.prev_ = &node_;
    node_.next_ = &node_;
  }

  bool empty() const {
    if (node_.prev_ == &node_) {
      assert(node_.next_ == &node_);
      return true;
    }

    assert(node_.next_ != &node_);
    return false;
  }

  void erase(Node& node) {
    node.next_->prev_ = node.prev_;
    node.prev_->next_ = node.next_;

#ifndef NDEBUG
    node.prev_ = nullptr;
    node.next_ = nullptr;
#endif
  }

  void push_back(Node& new_node) { node_.next_->InsertAfter(new_node); }

  void push_front(Node& new_node) { node_.prev_->InsertBefore(new_node); }

 private:
  Node node_;
};
