/* double linked list
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This file is released under the terms of GPL v2 and any later version.
 *
 */

#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <stdlib.h>
#include "dllist.h"

static void _dllist_release(dllist_t *l);

dllist_t *dllist_new(void *d)
{
    dllist_t *p = NULL;

    p = (dllist_t *) calloc(1, sizeof(dllist_t));
    if (p == NULL)
        return NULL;

    p->data = d;
    p->next = NULL;
    p->prev = NULL;

    return p;
}

dllist_t *dllist_head(dllist_t *l)
{
    dllist_t *p;

    if (l == NULL)
        return NULL;

    for (p = l; p->prev != NULL; p = p->prev)
        ;

    return p;
}

dllist_t *dllist_tail(dllist_t *l)
{
    dllist_t *p;

    if (l == NULL)
        return NULL;

    for (p = l; p->next != NULL; p = p->next)
        ;

    return p;
}

dllist_t *dllist_insert_head(dllist_t *l, void *d)
{
    dllist_t *newNode = NULL;

    newNode = dllist_new(d);
    if (l == NULL)
        return newNode;

    newNode->next = l;
    l->prev = newNode;

    return newNode;
}

dllist_t *dllist_list_prepend(dllist_t *l, dllist_t *n)
{
    if (l == NULL)
        return n;

    if (n != NULL) {
        n->next = l;
        n->prev = l->prev;
    }

    if (l->prev != NULL)
        l->prev->next = n;
    l->prev = n;

    return n;
}

dllist_t *dllist_append(dllist_t *l, dllist_t *n)
{
    if (l == NULL)
        return n;

    if (n != NULL) {
        n->prev = l;
        n->next = l->next;
    }

    if (l->next != NULL)
        n->next->prev = n;
    l->next = n;

    return n;
}

dllist_t *dllist_concat(dllist_t *a, dllist_t *b)
{
    dllist_t *p;

    if (a == NULL)
        return b;
    if (b == NULL)
        return a;

    for (p = a; p->next != NULL; p = p->next)
        ;

    p->next = b;
    b->prev = p;

    for (p = a; p->prev != NULL; p = p->prev)
        ;

    return p;
}

dllist_t *dllist_insert_tail(dllist_t *l, void *d)
{
    dllist_t *newNode = NULL, *runNode = l;
    newNode = dllist_new(d);

    if (l == NULL)
        return newNode;

    while (runNode->next != NULL)
        runNode = runNode->next;

    runNode->next = newNode;
    newNode->prev = runNode;

    return l;
}

dllist_t *dllist_insert_before(dllist_t *l, void *d, void * pos)
{
    dllist_t *newNode = NULL, *runNode = l;

    newNode = dllist_new(d);

    if (l == NULL)
        return newNode;

    runNode = dllist_find(l, pos);

    if (runNode == NULL) {
        _dllist_release(newNode);
        return l;
    }

    if (runNode->prev == NULL) {	//insert head node
        runNode->prev = newNode;
        newNode->next = runNode;
        l = newNode;
    } else {
        newNode->next = runNode;
        newNode->prev = runNode->prev;
        runNode->prev = newNode;
        newNode->prev->next = newNode;
    }

    return l;
}

void dllist_display(dllist_t *l, list_show_cb cb)
{
    dllist_t *runNode = l;

    while (runNode != NULL) {
        if (cb == NULL)
            printf("%p\n", runNode->data);
        else
            cb(runNode->data);
        runNode = runNode->next;
    }
}

void dllist_relase_list(dllist_t *l)
{
    dllist_t *p, *n;

    if (l == NULL)
        return;
    if (l->prev != NULL)
        l->prev->next = NULL;

    p = l;
    while (p != NULL) {
        n = p->next;
        _dllist_release(p);
        p = n;
    }
}

static void _dllist_release(dllist_t *l)
{
    if (l == NULL)
        return;

    free(l);
}

dllist_t * dllist_find(dllist_t *l, void *d)
{
    dllist_t *p;

    if (l == NULL)
        return NULL;

    for (p = l; p != NULL; p = p->next) {
        if (p->data == d)
            return p;
    }

    return NULL;
}

dllist_t *dllist_chop(dllist_t *l)
{
    dllist_t *p;

    if (l == NULL)
        return NULL;
    p = l->next;
    if (p != NULL)
        p->prev = NULL;

    _dllist_release(l);
    return p;
}

dllist_t * dllist_find_release(dllist_t *l, void *d)
{
    dllist_t *p;

    if (l == NULL)
        return NULL;

    if (l->data == d) {
        p = l->next;
        if (p != NULL)
            p->prev = NULL;
        _dllist_release(l);
        return p;
    }

    for (p = l->next; p != NULL; p = p->next) {
        if (p->data == d) {
            p->prev->next = p->next;
            if (p->next != NULL)
                p->next->prev = p->prev;
            _dllist_release(p);
            return l;
        }
    }

    return l;
}

dllist_t *dllist_reverse(dllist_t *l)
{
    dllist_t *x, *s, *r;

    if (l == NULL)
        return NULL;

    x = l->prev;
    r = l;
    s = l->next;

    while (s != NULL) {
        s = r->next;
        r->next = r->prev;
        r->prev = s;
        if (s != NULL)
            r = s;
    }

    if (x != NULL) {
        x->next = r;
        r->prev = x;
    }

    return r;
}

int dllist_count(dllist_t *l)
{
    int n;
    dllist_t *p;

    n = 0;
    for (p = l; p != NULL; p = p->next)
        n++;
    return n;
}

