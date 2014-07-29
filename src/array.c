/* array.c - data array
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include "array.h"

static int
cpo_array_setsize(cpo_array_t *a, int elements);

cpo_array_t *
cpo_array_create(int size, int elem_size)
{
    cpo_array_t *a = malloc(sizeof(cpo_array_t));
    if (a == NULL)
        return NULL;

    a->v = NULL;
    a->num = a->max = 0;
    a->elem_size = elem_size;

    return a;
}

static int cpo_array_preallocate(cpo_array_t *a, int elements)
{
    void *newv;
    int newmax = a->max;

    assert(a->num >= 0 && a->num <= a->max);

    while (elements > newmax) {
        newmax = (newmax + 1) * 2;
    }

    newv = malloc(newmax * a->elem_size);

    if (newv == NULL)
        return ENOMEM;

    memcpy(newv, a->v, a->num * a->elem_size);
    //need pool
    if (a->v)
        free(a->v);

    a->v = newv;
    a->max = newmax;
    return 0;
}

static int cpo_array_setsize(cpo_array_t *a, int elements)
{
    int result;

    assert(a->num >= 0 && a->num <= a->max);

    if (elements > a->max) {
        result = cpo_array_preallocate(a, elements);
        if (result) {
            return result;
        }
    }

    a->num = elements;
    return 0;
}

void *
cpo_array_get_at(cpo_array_t *a, int index)
{
    void *elt;
    assert(a->num <= a->max);
    assert(index >= 0 && index <= a->num);

    elt = (unsigned char*) a->v + a->elem_size * index;
    return elt;
}

void *
cpo_array_push(cpo_array_t *a)
{
    int ix, result;
    void * elt;
    ix = a->num;

    result = cpo_array_setsize(a, ix + 1);
    if (result) {
        return NULL;
    }

    elt = (unsigned char*) a->v + a->elem_size * ix;
    return elt;
}

void *
cpo_array_insert_at(cpo_array_t *a, int index)
{
    int nmove;

    if (index >= a->num) {
        cpo_array_setsize(a, index);
    }

    if (index < a->num) {
        nmove = (a->num) - (index);
        memmove(a->v, (unsigned char*) a->v + index - 1, nmove * a->elem_size);
    }
    a->num++;
    return cpo_array_get_at(a, index);
}

void *
cpo_array_remove(cpo_array_t *a, int index)
{
    int nmove;
    void *elt;
    assert(a->num <= a->max);
    assert(index >= 0 && index < a->num);

    nmove = a->num - index;

    memmove(a->v, (unsigned char*) a->v + index, nmove * a->elem_size);

    elt = (unsigned char *) a->v + (a->num * a->elem_size);
    a->num--;
    return elt;
}

void cpo_array_destroy(cpo_array_t *a)
{
    if (a->v)
        free(a->v);
    free(a);
}

void cpo_array_qsort(cpo_array_t *a,
                     int (*cmp_func)(const void *, const void *))
{
    qsort(a->v, a->num, a->elem_size, cmp_func);
}

void *cpo_array_bsearch(cpo_array_t *ar, const void *key,
                        int (*compar)(const void *, const void *))
{
    return bsearch(key, ar->v, ar->num, ar->elem_size, compar);
}

int array_cmp_int_asc(const void *a, const void *b)
{
    return (*(int*) a - *(int*) b);
}

int array_cmp_int_dsc(const void *a, const void *b)
{
    return (*(int*) b - *(int*) a);
}

int array_cmp_str_asc(const void *a, const void *b)
{
    return strcmp((char *) a, (char *) b);
}

int array_cmp_str_dsc(const void *a, const void *b)
{
    return strcmp((char *) b, (char *) a);
}

/* d */
void cpo_array_dump_int(cpo_array_t *arr)
{
    int i = 0;
    for (i = 0; i < arr->num; i++) {
        int x = (int) cpo_array_get_at(arr, i);
        printf("%d\n", x);
    }
}

void cpo_array_dump_str(cpo_array_t *arr)
{
    int i = 0;
    for (i = 0; i < arr->num; i++) {
        char *x = cpo_array_get_at(arr, i);
        printf("%s\n", x);
    }
}
