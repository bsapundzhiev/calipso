/* http_mime.c mtypes
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"
#include "http_mime.h"
#include "cplib.h"

#define DEFAULT_TYPE "text/plain"

#define MAX_MIME_LINE 1024

/**
 * parse Apache mime.types file
 */
void mime_parse_line(hash_t *h, char *input)
{
    int i, size;
    char **array = NULL;
    char mime_type[256], mime_name[256];

    if(!strstr(input, "\t")) {
        return;
    }
    sscanf(input, "%s %[^\n]", mime_type, mime_name );

    size = cpo_explode(&array, mime_name, ' ');

    for(i= 0; i < size; ++i) {
        mime_add_new(h, array[i], mime_type);
    }

    free(array);
}
/**
 * read line from conf file
 */
int mime_load_file(hash_t *l, char const *fname )
{
    FILE *f;
    int i=0,chr = 0;
    char *line;

    if ( (f=fopen(fname,"r")) == NULL ) {
        printf("cant open %s\n", fname);
        TRACE("mime_load_file: cant open %s\n", fname);
        exit(-1);
    }
    line = (char*)malloc( MAX_MIME_LINE );

    while (!feof(f) && i < MAX_MIME_LINE) {
        chr = fgetc(f);
        switch (chr) {
        case '#':
            do {
                chr=fgetc(f);
                if ( chr == EOF ) {
                    fclose(f);
                    return 0;
                }
            } while (chr!='\n');
            break;
        case EOF:
            break;
        case '\n':
            line[i]='\0';
            //printf("Line:%s\n",line);
            mime_parse_line(l,  line );
            i=0;
            break;
        default:
            line[i]= chr;
            i++;
            break;

        }
    }

    fclose(f);
    free(line);
    return 0;
}

const char *mime_get_type_value(hash_t *h, char const *filename)
{
    char* ext = NULL;
    const char *data;

    ext = (filename != NULL) ? strrchr( filename, '.' ) : NULL;
    if ( ext == NULL )
        return (DEFAULT_TYPE);

    while (*ext++ != '.')
        ;
    data = hash_table_get_data( h, ext );
    return( data != NULL ? data : DEFAULT_TYPE);
}

int mime_add_new(hash_t *h, const char *ext, const char *type)
{
    hash_table_insert( h,  ext,  strdup(type)  );
    return 0;
}

void mime_unalloc(hash_t *h)
{
    hash_node_t *node;
    size_t i;
    for (i = 0; i <  hash_table_get_size(h); i++) {
        node = h->nodes[i];
        while(node) {
            free(node->data);
            node->data  = NULL;
            node = node->next;
        }
    }

    hash_table_destroy(h);
}
