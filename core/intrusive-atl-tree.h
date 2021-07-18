#pragma once

#include <assert.h>
#include <stddef.h>

#include <algorithm>

#include "libc/macros.h"

struct AvlNode {
  AvlNode* left = nullptr;
  AvlNode* right = nullptr;
  int height = 1;

  static int Height(const AvlNode* node) {
    return node == nullptr ? 0 : node->height;
  }

  void CalcHeight() { height = std::max(Height(left), Height(right)) + 1; }

  int BalanceFactor() const { return Height(left) - Height(right); }
};

// - Returns -1 if l < r
// - Returns  1 if l > r
// - Returns  0 if l == r
using AvlNodeCmp = int (*)(AvlNode* l, AvlNode* r);

namespace internal {

inline void RotateRight(AvlNode*& node) {
  AvlNode* new_root = node->left;
  AvlNode* rotated = new_root->right;

  new_root->right = node;
  node->left = rotated;

  node->CalcHeight();
  new_root->CalcHeight();

  node = new_root;
}

inline void RotateLeft(AvlNode*& node) {
  AvlNode* new_root = node->right;
  AvlNode* rotated = new_root->left;

  new_root->left = node;
  node->right = rotated;

  node->CalcHeight();
  new_root->CalcHeight();

  node = new_root;
}

template <AvlNodeCmp compare>
AvlNode* Insert(AvlNode*& t, AvlNode* node) {
  if (t != nullptr) {
    assert(t->BalanceFactor() <= 1);
    assert(t->BalanceFactor() >= -1);
  }

  if (t == nullptr) {
    node->left = nullptr;
    node->right = nullptr;
    node->height = 1;
    t = node;
    return nullptr;
  }

  int cmp = compare(node, t);
  if (cmp == 0) {
    return t;
  }

  AvlNode* duplicate = nullptr;
  if (cmp < 0) {
    duplicate = Insert<compare>(t->left, node);
  } else {
    assert(cmp > 0);
    duplicate = Insert<compare>(t->right, node);
  }
  if (duplicate != nullptr) {
    return duplicate;
  }

  t->CalcHeight();

  int balance = t->BalanceFactor();
  if (balance > 1) {
    if (compare(node, t->left) < 0) {  // Left-Left
      RotateRight(t);
    } else {  // Left-Right
      RotateLeft(t->left);
      RotateRight(t);
    }
  } else if (balance < -1) {
    if (compare(node, t->right) > 0) {  // Right-Right
      RotateLeft(t);
    } else {  // Right-Left
      RotateRight(t->right);
      RotateLeft(t);
    }
  }

  return nullptr;
}

template <AvlNodeCmp compare>
AvlNode* Erase(AvlNode*& t, AvlNode* key) {
  if (t == nullptr) {
    return nullptr;
  }

  assert(t->BalanceFactor() <= 1);
  assert(t->BalanceFactor() >= -1);

  AvlNode* deleted = nullptr;
  int cmp = compare(key, t);
  if (cmp == 0) {
    deleted = t;
    t = nullptr;
    return deleted;
  }

  if (cmp < 0) {
    deleted = Erase<compare>(t->left, key);
  } else {
    deleted = Erase<compare>(t->right, key);
  }

  t->CalcHeight();

  int balance = t->BalanceFactor();
  if (balance > 1) {
    if (t->left->BalanceFactor() > 1) {  // Left-Left
      RotateRight(t);
    } else {  // Left-Right
      RotateLeft(t->left);
      RotateRight(t);
    }
  } else if (balance < -1) {
    if (t->right->BalanceFactor() < -1) {  // Right-Right
      RotateLeft(t);
    } else {  // Right-Left
      RotateRight(t->right);
      RotateLeft(t);
    }
  }

  return deleted;
}

template <typename Func>
void ForEachBottomUp(AvlNode* t, Func func) {
  if (t == nullptr) {
    return;
  }

  ForEachBottomUp(t->left, func);
  ForEachBottomUp(t->right, func);
  func(t);
}

template <typename Func>
AvlNode* Find(AvlNode* t, Func func) {
  if (t == nullptr) {
    return nullptr;
  }

  int cmp = func(t);
  if (cmp < 0) {
    return Find(t->left, func);
  } else if (cmp > 0) {
    return Find(t->right, func);
  }

  return t;
}

}  // namespace internal

template <AvlNodeCmp compare>
class IntrusiveAvlTree {
 public:
  IntrusiveAvlTree() = default;

  IntrusiveAvlTree(const IntrusiveAvlTree&) = delete;
  IntrusiveAvlTree& operator=(const IntrusiveAvlTree&) = delete;

  // Returns nullptr if insertion was successful. Otherwise, returns existing
  // node with the same key.
  AvlNode* Insert(AvlNode& node) {
    AvlNode* ret = internal::Insert<compare>(root_, &node);
    if (ret == nullptr) {
      ++size_;
    }
    return ret;
  }

  // Returns erased node.
  AvlNode* Erase(AvlNode& key) {
    AvlNode* ret = internal::Erase<compare>(root_, &key);
    if (ret != nullptr) {
      --size_;
    }
    return ret;
  }

  template <typename Func>
  AvlNode* Find(Func func) {
    return internal::Find<Func>(root_, func);
  }

  template <typename Func>
  void ForEachBottomUp(Func func) {
    internal::ForEachBottomUp(root_, func);
  }

  size_t size() const { return size_; }
  AvlNode* root() { return root_; }

 private:
  size_t size_ = 0;
  AvlNode* root_ = nullptr;
};
