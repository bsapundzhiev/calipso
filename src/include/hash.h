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

#ifndef _HASH_H
#define _HASH_H

#include <sys/types.h>

typedef size_t hash_size;
typedef hash_size (*hashfunc_ptr)(const char *, hash_size);

typedef struct hashnode_s {
	char *key;
	void *data;
	struct hashnode_s *next;
} hash_node_t;

typedef struct hashtbl_s {
	hash_size size; //buckets
	hash_node_t **nodes;
	hashfunc_ptr hashfunc;
} hash_t;


hash_t *hash_table_create(hash_size size, hashfunc_ptr hashfunc);
void hash_table_destroy(hash_t *hashtbl);
int hash_table_update(hash_t *hashtbl, const char *key, void *data);
int hash_table_insert(hash_t *hashtbl, const char *key, void *data);
int hash_table_remove(hash_t *hashtbl, const char *key);
void *hash_table_get_data(hash_t *hashtbl, const char *key);
int hash_table_resize(hash_t *hashtbl, hash_size size);
/*misc*/
hash_node_t *hash_get_first_entry( hash_t *hash_entry );
hash_node_t *hash_get_last_entry( hash_t *hash_entry );
hash_size hash_table_get_size(hash_t *hashtbl);

#endif /*_HASH_H */
