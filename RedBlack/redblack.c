#include <stdio.h>
#include <malloc.h>
#include <stdlib.h> // for exit()
#include "redblack.h"

typedef struct rbnode {
  struct rbnode *left;
  struct rbnode *right;
  struct rbnode *parent;
  int val;
};

static struct rbnode *binary_tree = NULL;

static struct rbnode *_add_to_tree(struct rbnode *tree, int val)
{
  if (tree == NULL) {
    tree = malloc(sizeof(struct rbnode));
    if (tree == NULL) {
      // Whee, we just silently croak when we're out of memory
      exit(1);
    }
    tree->left = NULL;
    tree->right = NULL;
    tree->val = val;
  }
  else if (tree->val > val) {
    // Add val to the left
    if (tree->left == NULL) {
      tree->left = _add_to_tree(NULL, val);
    }
    else {
      (void) _add_to_tree(tree->left, val);
    }
  }
  else {
    // Add val to the right - I don't care if they're equal
    if (tree->right == NULL) {
      tree->right = _add_to_tree(NULL, val);
    }
    else {
      (void) _add_to_tree(tree->right, val);
    }
  }
  return tree;
}

void add_to_tree(int val)
{
  binary_tree = _add_to_tree(binary_tree, val);
}

static void _print_tree(struct rbnode *tree)
{
  if (tree == NULL) return;
  _print_tree(tree->left);
  printf("%d, ", tree->val);
  _print_tree(tree->right);
}

void print_tree()
{
  _print_tree(binary_tree);
  printf("\n");
}
