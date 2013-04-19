/* list.c - linked list implementation
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

List *list_new(void)
{
    List *new_list = malloc(sizeof(List));
	if(new_list == NULL) return NULL;

    new_list->data = NULL;
    new_list->next = NULL;

    return new_list;
}


List *list_append(List *l, void *data)
{
    if (l->data == NULL) {
        l->data = data;
        return l;
    }

    while (l->next) {
        l = l->next;
    }

    l->next = list_new();
    l->next->data = data;
    l->next->next = NULL;

    return l;
}

/* get first node in list */
List *list_get_first_entry( List *list_entry )
{
    return list_entry;
}

/* generic: next node */
List *list_get_next_entry( List *list_entry )
{
    return	list_entry->next;
}

/*generic: get data form list*/
void *list_get_entry_value( List *list_entry )
{
    return list_entry->data;
}


List *list_get_entry( List *list, void * data)
{
    List *list_entry;
    list_entry = list_get_first_entry(list);

    while (list_entry) {
        if ( list_entry->data == data )
            return list_entry;

        list_entry = list_get_next_entry( list_entry );
    }

    return NULL;
}

//FIXME:
int list_remove_entry(List *list_entry)
{

    List *temp = list_entry;
    List *prev;

    if (temp->next) {
        prev = temp;
        temp = temp->next;
        prev->next = temp->next;
        free (temp);
        return 1;  /* Success */
    }

    free(temp);

    return 0;

}

/* Reverse order of elements in list. LIFO */
/* TODO: more testing here */
List * list_reverse (List * start)
{
    List *cur, *succ;
    if (!(start && start->next && (cur = start->next->next)))
        return start;
    start->next->next = NULL;
    while (cur) {
        succ = cur->next;
        cur->next = start->next;
        start->next = cur;
        cur = succ;
    }
    return start;
}

/* Append a list to another.
 * If successful, the second list will become empty but not freed.
 */
/* TODO: more testing here */
List * list_merge (List * start, List *	tail)
{
    List * temp = start;

    if (!start) {
        printf("List: Trying to append list %p to a nonexisting list\n", tail);
        return NULL;
    }

    if (!(tail && tail->next))
        return start;

    while (temp->next)
        temp = temp->next;

    temp->next = tail->next;
    tail->next = NULL;		/* tail is now an empty list */
    return start;
}

/* for debuging propose */
void list_debug_dump( List *list )
{
    List *tmp;
    int i = 0;
	tmp = list;
    
    while (tmp) {
        printf("list node %d, <%p>\n",
               i++, tmp->data );
        tmp = tmp->next;
    }

}

void list_debug_dump_str( List *list )
{
    List *tmp;
    int i = 0;
	tmp = list;
    
    while (tmp) {
        printf("list node %d, <%s>\n",
               i++, (const char*)tmp->data );
        tmp = tmp->next;
    }

}
/* element count */
size_t list_length (List *list)
{
    size_t n;
    for (n = 0; list; ++n)
        list = list->next;
    return n;
}

/* Return true if L is empty */
int list_is_empty( List *L )
{
    return L->next == NULL;
}

/* Return true if P is empty */
int list_is_last( List * P )
{
    return P->next == NULL;
}

/* find data in list retrn current entry */
List * list_find(List * L , void * data )
{
    List * p;
    p = L->next;

    while ( p != NULL && p->data != data )
        p = p->next;
    return p;
}

/* If data is not found, then next field of returned value is NULL */
List * list_find_prev(List *L , void * data)
{
    List * P;
    P = L;

    while ( P->next != NULL && P->next->data != data )
        P = P->next;
    return P;
}

/* remove node form list */
void list_remove_data(List * L, void * data)
{
    List *p, *tmpcell;

    p = list_find_prev( L , data );

    if ( !list_is_last( p ) ) {	/* data is found; delete it */
        tmpcell = p->next;
        p->next = tmpcell->next;  /* Bypass deleted cell */
        free( tmpcell );
    } else {
        if ( L->next == NULL ) {
            L->data = NULL;
            return;
        } else {	/*swap 1st and 2st*/
            p = list_get_first_entry( L );
            tmpcell = p->next;
            p->data = tmpcell->data;

            if (!p->next->next)
                p->next = NULL;
            else
                p->next = tmpcell->next;
            free( tmpcell );
        }
    }


}

/* delete List */
void list_delete( List *L )
{
    List * p, * tmp;

    p = L->next;  /* Header assumed */
    L->data = NULL;
    L->next = NULL;

    while ( p != NULL ) {
        tmp = p->next;
        free( p );
        p = tmp;
    }
}
/*
int cmpfn_str_asc(const void *ptr1, const void *ptr2) 
{
	return strcmp(ptr1, ptr2); 
}

int cmpfn_str_dsc(const void *ptr1, const void *ptr2)
{
	return !cmpfn_str_asc(ptr1, ptr2);
}
*/
List *sort_list(List *L, lcomparer cmpfn) 
{

    List *tmpPtr = L;
    List *tmpNxt = L->next;

    void * tmp;

    while(tmpNxt != NULL){
           while(tmpNxt != tmpPtr){
                    if( cmpfn( tmpNxt->data , tmpPtr->data) < 0){
                            tmp = tmpPtr->data;
                            tmpPtr->data = tmpNxt->data;
                            tmpNxt->data = tmp;
                    }
                    tmpPtr = tmpPtr->next;
            }
            tmpPtr = L;
            tmpNxt = tmpNxt->next;
    }

	return tmpPtr; 
}

