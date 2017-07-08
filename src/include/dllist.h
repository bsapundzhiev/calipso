#ifndef _DLLIST_H
#define _DLLIST_H

typedef int(*list_show_cb)(void*);

typedef struct dllist {
    void * data;
    struct dllist *next;
    struct dllist *prev;
} dllist_t;

dllist_t *dllist_new(void *d);

dllist_t *dllist_head(dllist_t *l);
dllist_t *dllist_tail(dllist_t *l);

dllist_t *dllist_insert_head(dllist_t *l, void *d);
dllist_t *dllist_insert_tail(dllist_t *l, void *d);

dllist_t *dllist_prepend(dllist_t *l, dllist_t *n);
dllist_t *dllist_append(dllist_t *l, dllist_t *n);
dllist_t *dllist_concat(dllist_t *a, dllist_t *b);

dllist_t *dllist_chop(dllist_t *l);

void dllist_relase_list(dllist_t *l);
dllist_t *dllist_find(dllist_t *l, void *d);
dllist_t *dllist_find_release(dllist_t *l, void *d);
dllist_t *dllist_reverse(dllist_t *l);
dllist_t *dllist_insert_before(dllist_t *l, void *d, void * pos);

int dllist_count(dllist_t *l);
void dllist_display(dllist_t *l, list_show_cb cb);

#endif /*_DLLIST_H*/

