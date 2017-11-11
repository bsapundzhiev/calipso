/* red-black tree
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This file is released under the terms of GPL v2 and any later version.
 *
 */
#ifndef _RBTREE_H
#define _RBTREE_H

typedef int KeyType;            // type of key
typedef void* ValType;

// how to compare keys
#define compLT(a,b) (a < b)
#define compEQ(a,b) (a == b)


typedef enum {
    RBT_STATUS_OK,
    RBT_STATUS_MEM_EXHAUSTED,
    RBT_STATUS_DUPLICATE_KEY,
    RBT_STATUS_KEY_NOT_FOUND
} RbtStatus;

typedef enum { BLACK, RED } nodeColor;

typedef struct NodeTag {
    struct NodeTag *left;       // left child
    struct NodeTag *right;      // right child
    struct NodeTag *parent;     // parent
    nodeColor color;            // node color (BLACK, RED)
    KeyType key;                // key used for searching
    ValType val;                // data related to key
} NodeType;

typedef struct TreeTag {
    struct NodeTag *root;
    struct NodeTag sentinel;
} RBTreeType;


RBTreeType * rbtNew();
RbtStatus rbtInsert(RBTreeType *tree, KeyType key, ValType val);
RbtStatus rbtErase(RBTreeType *tree, NodeType * z);
NodeType *rbtFind(RBTreeType *tree, KeyType key);
void rbtDel(RBTreeType *tree);
void rbtDelete(NodeType **p, NodeType *sentinel);
void rbtInorder(NodeType **p, NodeType *sentinel, void (callback)(NodeType *));
#endif
