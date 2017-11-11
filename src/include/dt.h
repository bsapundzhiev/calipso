/**
 * Description: Generic DataTypes
 * Autor: (c) 2006 Borislav Sapundjiev <BSapundjiev_AT_gmail[D0T]com>
 *
 */

#ifndef DT_H
#define DT_H

typedef struct _List 	List;

struct _List {
    void *data;
    List *next;
};

/*---list---*/
typedef int (*lcomparer)(const void *ptr1, const void *ptr2);
//int cmpfn_str_asc(const void *ptr1, const void *ptr2);
//int cmpfn_str_dsc(const void *ptr1, const void *ptr2);

List *	list_new(void);
List *	list_append(List *, void *);
List *	list_get_first_entry( List *list_entry );
List *	list_get_next_entry( List *list_entry );
void *	list_get_entry_value( List *list_entry );
List * 	list_get_entry( List *list, void * data);
int 	list_remove_entry(List *list);
size_t 	list_length (List *list);
void 	list_remove_data(List *L, void * data);
void 	list_delete(List *L);
void 	list_debug_dump( List *list );
List * 	list_find_prev(List *L, void * data);
List *	sort_list(List *L, lcomparer cmpfn);

/*---btree---*/
struct bin_tree {
    int 			size;
    int             key;
    void            *data;
    struct bin_tree	*left;
    struct bin_tree	*right;
};

typedef struct bin_tree btree_t;

btree_t *btree_new(void);
btree_t *btree_search(btree_t *root, int key);
btree_t *btree_append_node(btree_t *root, int key, void *data);
btree_t *btree_delete_node(btree_t* root, int num);
void 	*btree_get_data(btree_t *root, int key);
void	btree_print(btree_t *root);
void	btree_delete(btree_t *root);

/*!---btree---*/

#endif /* LLIST_H */
