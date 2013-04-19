#ifndef _MIME_H
#define _MIME_H

hash_t *mime_type;
int mime_load_file(hash_t *l, char const *fname );
int mime_add_new(hash_t *h, const char *ext, const char *type);
const char *mime_get_type_value(hash_t *h, char const *filename);
void mime_unalloc(hash_t *h);

#endif
