/* btree.c - btree implementation
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "dt.h"

btree_t *
btree_new()
{
    btree_t *new_tree = malloc(sizeof(struct bin_tree));
    if (!new_tree)
        return NULL;
    new_tree->key = -1;
    new_tree->data = NULL;
    new_tree->left = NULL;
    new_tree->right = NULL;
    new_tree->size	= 0;
    return new_tree;
}

btree_t *
btree_append_node(btree_t *root, int key, void *data)
{
    btree_t *new_node;
    btree_t *node, **prevp;

    prevp = &root;

    while (*prevp) {
        node = *prevp;
        if (key == node->key) {
            printf("key exists %d\n", node->key);
            return root;
        }
        prevp = key < node->key ? &node->left : &node->right;
    }

    new_node = btree_new();

    if (!new_node) {
        printf("malloc() call error in %s\n", __FUNCTION__);
        return NULL;
    }

    new_node->key = key;
    new_node->data = data;

    *prevp = new_node;
    root->size++;
    return root;
}

btree_t *
btree_search(btree_t *root, int key)
{
    btree_t *node = root;

    while (node) {
        if (node->key == key)
            break;

        node = key < node->key ? node->left : node->right;
    }

    return node;
}

void btree_delete(btree_t *root)
{
    if (!root) {
        return;
    }

    btree_delete(root->left);
    btree_delete(root->right);
    free(root);
}

void *
btree_get_data(btree_t *root, int key)
{
    btree_t *node = NULL;
    node = btree_search(root, key);
    return (node && node->data) ? node->data : NULL;
}

btree_t *
btree_delete_node(btree_t *root, int num)
{
    btree_t *temp = NULL;
    if (root == NULL) {
        return NULL;
    } else if (num < root->key)
        root->left = btree_delete_node(root->left, num);
    else if (num > root->key)
        root->right = btree_delete_node(root->right, num);
    else {
        if (root->left != NULL && root->right != NULL) {
            temp = root->left->key < num ? root->left : root->right;
            root->key = temp->key;
            root->right = btree_delete_node(root->right, root->key);
        } else if (root->left == NULL) {
            temp = root;
            root = root->right;
        } else if (root->right == NULL) {
            temp = root;
            root = root->left;
        }

        free(temp);
    }
    root->size--;
    return root;
}

int btree_get_size(btree_t *root)
{
    return root->size;
}

void btree_print(btree_t *root)
{
    if (root) {
        btree_print(root->left);
        printf("%d : %p\n", root->key, root->data);
        btree_print(root->right);
    }
}

