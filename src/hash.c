/*
 Copyright (c) 2012 the authors listed at the following URL, and/or
 the authors of referenced articles or incorporated external code:
 http://en.literateprograms.org/Hash_table_(C)?action=history&offset=20100620072342

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define DEFAULT_HASH_SIZE 1024

static hash_size def_hashfunc(const char *key, hash_size size)
{
    hash_size hash = 0;

    while (*key)
        hash += (unsigned char) *key++;

    return hash % size;
}

/* hash key for hashtable -> Horner's rule */
static hash_size hash_table_hash(const char *key, hash_size size)
{
    hash_size result = 0;
    while (*key)
        result += (result * 32 + (unsigned char) *key++);
    return result % size;
}

hash_t *hash_table_create(hash_size size, hashfunc_ptr hashfunc)
{
    hash_t *hashtbl;

    size = (0 == size) ? DEFAULT_HASH_SIZE : size;

    hashtbl = (hash_t *) malloc(sizeof(hash_t));
    if (!hashtbl)
        return NULL;

    if (!(hashtbl->nodes = calloc(size, sizeof(struct hashnode_s*)))) {
        free(hashtbl);
        return NULL;
    }

    hashtbl->size = size;
    if (hashfunc)
        hashtbl->hashfunc = hashfunc;
    else
        hashtbl->hashfunc = def_hashfunc; //hash_table_hash;

    return hashtbl;
}

void hash_table_destroy(hash_t *hashtbl)
{
    hash_size n;
    struct hashnode_s *node, *oldnode;

    for (n = 0; n < hashtbl->size; ++n) {
        node = hashtbl->nodes[n];
        while (node) {
            free(node->key);
            oldnode = node;
            node = node->next;
            free(oldnode);
        }
    }
    free(hashtbl->nodes);
    free(hashtbl);
}

/* add elem if element exists replace */
int hash_table_update(hash_t *hashtbl, const char *key, void *data)
{
    struct hashnode_s *node;
    hash_size hash = hashtbl->hashfunc(key, hashtbl->size);

    node = hashtbl->nodes[hash];

    while (node) {
        if (!strcmp(node->key, key)) {
            node->data = data;
            return 0;
        }
        node = node->next;
    }

    return hash_table_insert(hashtbl, key, data);

}

/* add elem if exists resolve collision */
int hash_table_insert(hash_t *hashtbl, const char *key, void *data)
{
    struct hashnode_s *node;
    hash_size hash = hashtbl->hashfunc(key, hashtbl->size);

    //printf("hashtbl_insert() key=%s, hash=%d, data=%s\n", key, hash, (char*)data);

    /*node=hashtbl->nodes[hash];
     while(node) {
     if(!strcmp(node->key, key)) {
     node->data=data;
     return 0;
     }
     node=node->next;
     }*/

    if (!(node = malloc(sizeof(struct hashnode_s))))
        return -1;
    if (!(node->key = strdup(key))) {
        free(node);
        return -1;
    }
    node->data = data;
    node->next = hashtbl->nodes[hash];
    hashtbl->nodes[hash] = node;

    return 0;
}

int hash_table_remove(hash_t *hashtbl, const char *key)
{
    struct hashnode_s *node, *prevnode = NULL;
    hash_size hash = hashtbl->hashfunc(key, hashtbl->size);

    node = hashtbl->nodes[hash];
    while (node) {
        if (!strcmp(node->key, key)) {
            free(node->key);
            node->key = NULL;
            if (prevnode)
                prevnode->next = node->next;
            else
                hashtbl->nodes[hash] = node->next;
            free(node);
            return 0;
        }
        prevnode = node;
        node = node->next;
    }

    return -1;
}

void *hash_table_get_data(hash_t *hashtbl, const char *key)
{
    struct hashnode_s *node;
    hash_size hash = hashtbl->hashfunc(key, hashtbl->size);

    /*	fprintf(stderr, "hashtbl_get() key=%s, hash=%d\n", key, hash);*/

    node = hashtbl->nodes[hash];
    while (node) {
        if (!strcmp(node->key, key))
            return node->data;
        node = node->next;
    }

    return NULL;
}

int hash_table_resize(hash_t *hashtbl, hash_size size)
{
    hash_t newtbl;
    hash_size n;
    struct hashnode_s *node, *next;

    //newtbl.size=size;
    newtbl.hashfunc = hashtbl->hashfunc;

    if (!(newtbl.nodes = calloc(size, sizeof(struct hashnode_s*))))
        return -1;

    for (n = 0; n < hashtbl->size; ++n) {
        for (node = hashtbl->nodes[n]; node; node = next) {
            next = node->next;
            hash_table_insert(&newtbl, node->key, node->data);
            hash_table_remove(hashtbl, node->key);

        }
    }

    free(hashtbl->nodes);
    hashtbl->size = newtbl.size;
    hashtbl->nodes = newtbl.nodes;

    return 0;
}

hash_node_t *hash_get_first_entry(hash_t *hash_entry)
{
    hash_node_t *node = NULL;
    hash_size n;

    for (n = 0; n < hash_entry->size; ++n) {
        node = hash_entry->nodes[n];
        while (node) {
            if (node->key)
                return node;
            node = node->next;
        }
    }

    return NULL;
}

hash_node_t *hash_get_last_entry(hash_t *hash_entry)
{
    hash_node_t *node = NULL, *s = NULL;
    hash_size n;

    for (n = 0; n < hash_entry->size; ++n) {
        node = hash_entry->nodes[n];
        while (node) {
            if (node->key) {
                s = node;
            }
            node = node->next;
        }
    }

    return s;
}

hash_size hash_table_get_size(hash_t *hashtbl)
{
    return hashtbl->size;
}

/* debug */
void hash_table_display(hash_t *hashtbl)
{
    hash_size i;
    hash_node_t *node;

    for (i = 0; i < hash_table_get_size(hashtbl); i++) {
        node = hashtbl->nodes[i];
        while (node) {
            printf("%s() %s: %s\n", __FUNCTION__, node->key,
                   (char *) node->data);
            node = node->next;
        }
    }
}

