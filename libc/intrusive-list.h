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

  class iterator {
   public:
    explicit iterator(Node* node) : node_(node) {}

    iterator& operator++() {
      node_ = node_->next_;
      return *this;
    }

    Node& operator*() const { return *node_; }

   private:
    Node* node_ = nullptr;
  };

  explicit IntrusiveList() {
    node_.prev_ = &node_;
    node_.next_ = &node_;
  }

  iterator begin() { return iterator(node_.next_); }
  iterator end() { return iterator(&node_); }

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

bool operator==(const IntrusiveList::iterator& lhs,
                const IntrusiveList::iterator& rhs) {
  return &*lhs == &*rhs;
}

bool operator!=(const IntrusiveList::iterator& lhs,
                const IntrusiveList::iterator& rhs) {
  return &*lhs != &*rhs;
}
