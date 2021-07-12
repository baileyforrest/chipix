#pragma once

#include <assert.h>
#include <stddef.h>

#include "libc/macros.h"

typedef struct Tree {
  struct Tree* left;
  struct Tree* right;
  int height;
} Tree;

static inline int tree_height(Tree* t) { return t == NULL ? 0 : t->height; }

static inline void tree_calc_height(Tree* t) {
  t->height = MAX(int, tree_height(t->left), tree_height(t->right)) + 1;
}

static inline int tree_balance_factor(Tree* t) {
  return tree_height(t->left) - tree_height(t->right);
}

static inline void tree_rotate_right(Tree** t) {
  Tree* new_root = (*t)->left;
  Tree* rotated = new_root->right;

  new_root->right = *t;
  (*t)->left = rotated;

  tree_calc_height(*t);
  tree_calc_height(new_root);

  *t = new_root;
}

static inline void tree_rotate_left(Tree** t) {
  Tree* new_root = (*t)->right;
  Tree* rotated = new_root->left;

  new_root->left = *t;
  (*t)->right = rotated;

  tree_calc_height(*t);
  tree_calc_height(new_root);

  *t = new_root;
}

// - Returns -1 if l < r
// - Returns  1 if l > r
// - Returns  0 if l == r
typedef int (*TreeCompareCb)(Tree* l, Tree* r);

// Returns NULL if insertion was successful. Otherwise, returns existing node
// with the same key.
static inline Tree* tree_insert(Tree** t, Tree* node, TreeCompareCb compare) {
  if (*t != NULL) {
    assert(tree_balance_factor(*t) <= 1);
    assert(tree_balance_factor(*t) >= -1);
  }

  if (*t == NULL) {
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    *t = node;
    return NULL;
  }

  int cmp = compare(node, *t);
  if (cmp == 0) {
    return *t;
  }

  Tree* duplicate = NULL;
  if (cmp < 0) {
    duplicate = tree_insert(&(*t)->left, node, compare);
  } else if (cmp > 0) {
    duplicate = tree_insert(&(*t)->left, node, compare);
  }

  tree_calc_height(*t);

  int balance = tree_balance_factor(*t);
  if (balance > 1) {
    if (compare(node, (*t)->left) < 0) {  // Left-Left
      tree_rotate_right(t);
    } else {  // Left-Right
      tree_rotate_left(&(*t)->left);
      tree_rotate_right(t);
    }
  } else if (balance < -1) {
    if (compare(node, (*t)->right) > 0) {  // Right-Right
      tree_rotate_left(t);
    } else {  // Right-Left
      tree_rotate_right(&(*t)->right);
      tree_rotate_left(t);
    }
  }

  return duplicate;
}

static inline Tree* tree_find(Tree* t, Tree* key, TreeCompareCb compare) {
  if (t == NULL) {
    return NULL;
  }

  assert(tree_balance_factor(t) <= 1);
  assert(tree_balance_factor(t) >= -1);

  int cmp = compare(key, t);
  if (cmp < 0) {
    return tree_find(t->left, key, compare);
  } else if (cmp > 0) {
    return tree_find(t->right, key, compare);
  }

  return t;
}

static inline Tree* tree_erase(Tree** t, Tree* key, TreeCompareCb compare) {
  if (*t == NULL) {
    return NULL;
  }

  assert(tree_balance_factor(*t) <= 1);
  assert(tree_balance_factor(*t) >= -1);

  Tree* deleted = NULL;
  int cmp = compare(key, *t);
  if (cmp == 0) {
    deleted = *t;
    *t = NULL;
    return deleted;
  }

  if (cmp < 0) {
    deleted = tree_erase(&(*t)->left, key, compare);
  } else {
    deleted = tree_erase(&(*t)->right, key, compare);
  }

  tree_calc_height(*t);

  int balance = tree_balance_factor(*t);
  if (balance > 1) {
    if (tree_balance_factor((*t)->left) > 1) {  // Left-Left
      tree_rotate_right(t);
    } else {  // Left-Right
      tree_rotate_left(&(*t)->left);
      tree_rotate_right(t);
    }
  } else if (balance < -1) {
    if (tree_balance_factor((*t)->right) < -1) {  // Right-Right
      tree_rotate_left(t);
    } else {  // Right-Left
      tree_rotate_right(&(*t)->right);
      tree_rotate_left(t);
    }
  }

  return deleted;
}
