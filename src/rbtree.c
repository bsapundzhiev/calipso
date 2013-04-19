/* red-black tree implementation
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This file is released under the terms of GPL v2 and any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "rbtree.h"

#define SENTINEL &tree->sentinel

static void rotateLeft(NodeType **root, NodeType *x, NodeType *sentinel) 
{
    // rotate node x to left

    NodeType *y = x->right;

    // establish x->right link
    x->right = y->left;
    if (y->left != sentinel) y->left->parent = x;

    // establish y->parent link
    if (y != sentinel) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
    } else {
        *root = y;
    }

    // link x and y
    y->left = x;
    if (x != sentinel) x->parent = y;
}

static void rotateRight(NodeType **root, NodeType *x, NodeType *sentinel) 
{
    // rotate node x to right

    NodeType *y = x->left;

    // establish x->left link
    x->left = y->right;
    if (y->right != sentinel) y->right->parent = x;

    // establish y->parent link
    if (y != sentinel) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
    } else {
       *root = y;
    }

    // link x and y
    y->right = x;
    if (x != sentinel) x->parent = y;
}

static void insertFixup(NodeType **root, NodeType *x, NodeType *sentinel) 
{
    // maintain red-black tree balance
    // after inserting node x

    // check red-black properties
    while (x != *root && x->parent->color == RED) {
        // we have a violation
        if (x->parent == x->parent->parent->left) {
            NodeType *y = x->parent->parent->right;
            if (y->color == RED) {

                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                // uncle is BLACK
                if (x == x->parent->right) {
                    // make x a left child
                    x = x->parent;
                    rotateLeft(root, x, sentinel);
                }

                // recolor and rotate
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateRight(root, x->parent->parent, sentinel);
            }
        } else {

            // mirror image of above code
            NodeType *y = x->parent->parent->left;
            if (y->color == RED) {

                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                // uncle is BLACK
                if (x == x->parent->left) {
                    x = x->parent;
                    rotateRight(root, x, sentinel);
                }
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateLeft(root, x->parent->parent, sentinel);
            }
        }
    }
    (*root)->color = BLACK;
}

// insert new node (no duplicates allowed)
RbtStatus rbtInsert(RBTreeType *tree, KeyType key, ValType val) 
{
	NodeType **root = &tree->root;

    NodeType *current, *parent, *x;

    // allocate node for data and insert in tree

    // find future parent
    current = *root;
    parent = 0;
    while (current != SENTINEL) {
        if (compEQ(key, current->key)) 
            return RBT_STATUS_DUPLICATE_KEY;
        parent = current;
        current = compLT(key, current->key) ?
            current->left : current->right;
    }

    // setup new node
    if ((x = malloc (sizeof(*x))) == 0)
        return RBT_STATUS_MEM_EXHAUSTED;
    x->parent = parent;
    x->left = SENTINEL;
    x->right = SENTINEL;
    x->color = RED;
    x->key = key;
    x->val = val;

    // insert node in tree
    if(parent) {
        if(compLT(key, parent->key))
            parent->left = x;
        else
            parent->right = x;
    } else {
        *root = x;
    }

    insertFixup(root, x, SENTINEL);

    return RBT_STATUS_OK;
}

static void deleteFixup(NodeType **root, NodeType *x, NodeType *sentinel) 
{
    // maintain red-black tree balance
    // after deleting node x

    while (x != *root && x->color == BLACK) {
        if (x == x->parent->left) {
            NodeType *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateLeft (root, x->parent, sentinel);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rotateRight (root, w, sentinel);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotateLeft (root, x->parent, sentinel);
                x = *root;
            }
        } else {
            NodeType *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateRight (root, x->parent, sentinel);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rotateLeft (root, w, sentinel);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotateRight (root, x->parent, sentinel);
                x = *root;
            }
        }
    }
    x->color = BLACK;
}

// delete node
RbtStatus rbtErase(RBTreeType *tree, NodeType * z) 
{
	NodeType **root = &tree->root;
    NodeType *x, *y;

    if (z->left == SENTINEL || z->right == SENTINEL) {
        // y has a SENTINEL node as a child
        y = z;
    } else {
        // find tree successor with a SENTINEL node as a child
        y = z->right;
        while (y->left != SENTINEL) y = y->left;
    }

    // x is y's only child
    if (y->left != SENTINEL)
        x = y->left;
    else
        x = y->right;

    // remove y from the parent chain
    x->parent = y->parent;
    if (y->parent)
        if (y == y->parent->left)
            y->parent->left = x;
        else
            y->parent->right = x;
    else
        *root = x;

    if (y != z) {
        z->key = y->key;
        z->val = y->val;
    }


    if (y->color == BLACK)
        deleteFixup (root, x, SENTINEL);

    free (y);

    return RBT_STATUS_OK;
}

// find key
NodeType *rbtFind(RBTreeType *tree, KeyType key) 
{
	NodeType **root = &tree->root;
    NodeType *current;
    current = *root;
    while(current != SENTINEL) {
        if(compEQ(key, current->key)) {
            return current;
        } else {
            current = compLT (key, current->key) ?
                current->left : current->right;
        }
    }
    return NULL;
}

// in-order walk of tree
void rbtInorder(NodeType **p, NodeType *sentinel, void (callback)(NodeType *)) 
{
    if (*p == sentinel) return;
    rbtInorder(&(*p)->left, sentinel,callback);
    callback(*p);
    rbtInorder(&(*p)->right, sentinel, callback);
}

// delete nodes depth-first
void rbtDelete(NodeType **p, NodeType *sentinel) 
{
    if (*p == sentinel) return;
    rbtDelete(&(*p)->left, sentinel);
    rbtDelete(&(*p)->right, sentinel);
    free(*p);
}

void rbtDel(RBTreeType *tree) 
{
	if(tree == NULL) return;
	rbtDelete(&tree->root, SENTINEL);
	free(tree);
}

void displayNode(NodeType *p) 
{
    printf("%d %d\n", p->key, (int)p->val);
}

RBTreeType * rbtNew() 
{
	RBTreeType *tree = malloc(sizeof(RBTreeType));
	if(tree == NULL) return NULL;
	//tree->compare = rbtCompare;
	// all leafs are sentinels
    tree->root = &tree->sentinel;
    tree->sentinel.left = &tree->sentinel;
    tree->sentinel.right = &tree->sentinel;
    tree->sentinel.parent = NULL;
    tree->sentinel.color = BLACK;
    tree->sentinel.key = 0;
    tree->sentinel.val = NULL;
	return tree;
}

