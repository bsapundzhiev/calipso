/*
 *  A Queue (FIFO)  implementation using doubly-linked list.
 *  based on page 70, section 2.4
 *  "Data Structures and Algorithms by Aho, Hopcraft and Ullman".
 */

#include <stdio.h>
#include <stdlib.h>
#include "dllist.h"
#include "queue.h"

static void remove_element(struct dlqlist* s, struct dllist* d);

enum {
    VAL_SUCC = 0, VAL_ERR = 1
};

int queue_enqueue(struct dlqlist* s, void *data)
{
    int ret;
    if (NULL == s) {
        fprintf(stderr, "IN: %s @ %d: Invalid Args\n", __FILE__, __LINE__);
        ret = VAL_ERR;
    } else if (NULL == s->head && NULL == s->tail) {
        struct dllist* p = malloc(1 * sizeof *p);
        if (NULL == p) {
            fprintf(stderr, "IN: %s @%d: Out of Memory\n", __FILE__, __LINE__);
            ret = VAL_ERR;
        } else {
            p->data = data;
            p->prev = p->next = NULL;

            s->head = s->tail = p;
            ret = VAL_SUCC;
        }
    } else if (NULL == s->head || NULL == s->tail) {
        fprintf(stderr, "IN: %s @%d: head/tail is null", __FILE__, __LINE__);
        ret = VAL_ERR;
    } else {
        struct dllist* p = malloc(1 * sizeof *p);
        if (NULL == p) {
            fprintf(stderr, "IN: %s @%d: Out of Memory\n", __FILE__, __LINE__);
            ret = VAL_ERR;
        } else {
            p->data = data;
            p->prev = p->next = NULL;

            s->tail->next = p;
            p->prev = s->tail;
            s->tail = p;
            ret = VAL_SUCC;
        }
    }

    return ret;
}

int queue_dequeue(struct dlqlist* s)
{
    int ret;
    if (NULL == s) {
        fprintf(stderr, "IN: %s @ %d: Invalid Args\n", __FILE__, __LINE__);
        ret = VAL_ERR;
    } else if (NULL == s->head && NULL == s->tail) {
        printf("Nothing to Dequeue()\n");
        ret = VAL_SUCC;
    } else if (NULL == s->head || NULL == s->tail) {
        fprintf(stderr, "IN: %s @%d: head/tail is null", __FILE__, __LINE__);
        ret = VAL_ERR;
    } else {
        struct dllist* p = s->head;
        if (NULL == s->head->next && NULL == s->tail->next) {
            s->head = s->tail = NULL;
        } else {
            s->head = s->head->next;
        }

        free(p);
        ret = VAL_SUCC;
    }

    return ret;
}

int queue_remove(struct dlqlist*s, void * elem)
{
    int ret;
    if (NULL == s) {
        fprintf(stderr, "IN: %s @ %d: Invalid Args\n", __FILE__, __LINE__);
        ret = VAL_ERR;
    } else if (NULL == s->head && NULL == s->tail) {
        printf("Nothing to Dequeue()\n");
        ret = VAL_SUCC;
    } else if (NULL == s->head || NULL == s->tail) {
        fprintf(stderr, "IN: %s @%d: Serious  head/tail", __FILE__, __LINE__);
        ret = VAL_ERR;
    } else {
        struct dllist* p = s->head;
        //--------------
        if (p->data == elem) {
            queue_dequeue(s);
            return VAL_SUCC;
        }
        if (s->tail->data == elem) {
            p = s->tail;
            remove_element(s, p);
            return VAL_SUCC;
        }
        //!-------------------
        for (; p; p = p->next) {
            if (elem == p->data) {
                remove_element(s,p);
                break;
            }
        }
        ret = VAL_SUCC;
    }

    return ret;
}
/*
void remove_element(struct dlqlist* s, struct dllist* d)
{
    if (NULL == d->next && (NULL == s->head->next && NULL == s->tail->next)) {
        s->head = s->tail = NULL;
    } else if ((NULL == d->next) && d->prev) {
        s->tail = d->prev;
        d->prev->next = NULL;
    } else if (d->next && (NULL == d->prev)) {
        s->head = d->next;
        s->head->prev = NULL;
    } else {
        d->prev->next = d->next;
        d->next->prev = d->prev;
    }

    free(d);
}*/

void remove_element(struct dlqlist* s, struct dllist* d)
{
    if (NULL == d->next) {
        s->tail = d->prev;
    } else {
        d->next->prev = d->prev;
    }

    if (NULL == d->prev) {
        s->head = d->next;
    } else {
        d->prev->next = d->next;
    }

    free(d);
}

void queue_print(struct dlqlist* s)
{
    if (NULL == s) {
        fprintf(stderr, "IN: %s @ %d: Invalid Args\n", __FILE__, __LINE__);
    } else if (NULL == s->head && NULL == s->tail) {
        printf("Nothing to print\n");
    } else if (NULL == s->head || NULL == s->tail) {
        fprintf(stderr, "IN: %s @%d: head/tail is null", __FILE__, __LINE__);
    } else {
        struct dllist* p = s->head;
        while (p) {
            printf("data = %p\n", p->data);
            p = p->next;
        }
    }
}

struct dlqlist * queue_new() {
    struct dlqlist* s = malloc(1 * sizeof *s);
    if (NULL == s) {
        fprintf(stderr, "IN: %s @%d: Out of Memory\n", __FILE__, __LINE__);
        return NULL;
    }

    s->head = s->tail = NULL;
    return s;
}

struct dlqlist* queue_delete(struct dlqlist* s) {
    while (s->head) {
        queue_dequeue(s);
    }

    return s;
}

