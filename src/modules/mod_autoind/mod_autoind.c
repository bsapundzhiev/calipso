/* mod_autoind.c index generator
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <dirent.h>
#include <libgen.h> /* dirname */
#include "calipso.h"
#include "mod_autoind.h"
#include "chunks.h"

#define SPACE_CHAR 		0x20
#define ETALON_SPACE 	40

enum {INDEX_TEXT, INDEX_HTML};

#define INDEX_TYPE 			INDEX_TEXT
#define SHOW_SERVER_TOKENS 	NOK
#define PARENT_DIR_NAME		"parent directory"

char *indheadfoot[] = {
		"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n"	
		"<html><head><title>Index of %s</title>\n"
		"</style></head><body>"
		"<h1>Index of %s</h1><hr><pre>\n",
		"</pre><hr>%s\n</body></html>\n",
};

static char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *indexes[]= {
	"index.html",
	"index.htm",
	"index.php"
};

typedef struct s_dentry {
	char 	*name;
	time_t  mtime;
    off_t   size;
	int 	is_dir;
} cpo_autoind_entry_t;

static int	mod_autoind_init(void);
static int	mod_autoind_resource(calipso_request_t *);
static int  mod_autoind_translate(calipso_request_t *request);
static int	mod_autoind_reply(calipso_request_t *);
static int 	mod_autoind_make_index_table(calipso_reply_t * reply, const char *uri, int prop);
static int 	mod_autoindex_cmp_entries(const void *one, const void *two);
static char *add_space(char *word, int len , char maska);

int pm_init()
{
	TRACE("register: %s\n",__FILE__);
	calipso_register_handler("filesystem/directory", mod_autoind_reply);
	calipso_register_hook(HOOK_INIT, mod_autoind_init);
	calipso_register_hook(HOOK_TRANSLATE, mod_autoind_translate);	
	calipso_register_hook(HOOK_RESOURCE, mod_autoind_resource);	
	return OK;
}

int pm_exit() 
{
	TRACE("Exit\n");
	return OK;
}

static int mod_autoind_init(void)
{
	//config
	return 1;
}

static int mod_autoind_translate(calipso_request_t *request)
{
	calipso_reply_t *reply = calipso_request_get_reply(request);
	calipso_resource_t *resource = calipso_reply_get_resource(reply);
	int http_status  = calipso_reply_get_status(reply);
	char * uri, *directory;
	int i;

	if( calipso_http_status_is_error(http_status) ) {
		return NOK;
	}
	
	uri = calipso_request_get_uri(request);
	directory = calipso_resource_get_path(resource);

	if( cpo_strlen( strrchr(uri,'/') ) == 1 )
	{
		chdir(directory);

		for(i=0; i < (sizeof(indexes) / sizeof(indexes[0])); i++ )
		{
			if( access(indexes[i], F_OK) == 0 ) {	
				strcat(directory, indexes[i]);
				calipso_resource_set_path(resource, directory);
				break;
			}
		}
	}

	return OK;
}

static int mod_autoind_resource(calipso_request_t *request)
{
	char *uri; 
	calipso_reply_t *reply = calipso_request_get_reply(request);
	calipso_resource_t *resource = calipso_reply_get_resource(reply);
	int http_status  = calipso_reply_get_status(reply);

	if (!calipso_resource_is_directory(resource) 
		||( calipso_http_status_is_error(http_status) 
		&& http_status != HTTP_FORBIDDEN )) {
		return NOK;
	}

	if(calipso_resource_is_directory(reply->resource))  {		
			
		uri = calipso_request_get_uri(request);

		if( cpo_strlen( strrchr(uri,'/') ) == 1 ) {
			calipso_reply_set_status(reply, HTTP_OK);
		} else {
			calipso_reply_set_status(reply, HTTP_MOVED_PERMANENTLY);
		}

		mod_autoind_reply(request);
	}

	return OK;
}

static int mod_autoind_reply(calipso_request_t *request)
{
	char *uri;
	calipso_reply_t * reply;
	int http_status;
	reply = calipso_request_get_reply(request);
	uri   = calipso_request_get_uri(request);
	
	calipso_reply_set_header_value(reply, "Content-Type", "text/html");

	http_status  = calipso_reply_get_status(reply);
	
	if (HTTP_OK == http_status) {

		mod_autoind_make_index_table(reply, uri, INDEX_TYPE);
	} else {

		strcat(uri, "/");
		calipso_reply_set_header_value(reply, "Location", uri);
	}
	
	return OK;
}

static int mod_autoind_make_index_table(calipso_reply_t * reply, const char *uri, int prop)
{
	DIR *dir;
	struct stat sb;
	struct tm tm;
	char date[26];
	struct dirent *ent;

	cpo_array_t arr;
	cpo_autoind_entry_t *entry;
	int i,dname_len;
	char *dname;
	const char * directory = calipso_resource_get_path(reply->resource);

	struct chunk_ctx * cb  = chunk_ctx_alloc(reply->pool);

	arr.elem_size = sizeof(cpo_autoind_entry_t);
	arr.v = calloc(40 , sizeof(cpo_autoind_entry_t) );
	arr.num = 0;
	arr.max = 32;


	if ((dir = opendir(directory)) == NULL) { 

		calipso_reply_set_status(reply, HTTP_FORBIDDEN);
		return CPO_ERR;
	}
	
	while ((ent = readdir(dir)) != NULL) {
		
		if(!strncmp(ent->d_name, ".", 1)) {		
			continue;			
		}
		
		if( stat(ent->d_name , &sb) != -1) {
		
			entry = cpo_array_push(&arr);
			entry->name  = cpo_pool_strdup(reply->pool, ent->d_name);
			entry->mtime = sb.st_mtime; 
			entry->size =  sb.st_size;
			entry->is_dir =  S_ISDIR(sb.st_mode);
		}
	}

	closedir(dir);

	cpo_array_qsort(&arr, mod_autoindex_cmp_entries);

	chunk_ctx_printf(reply->pool, cb, indheadfoot[0], uri, uri);

	if(strcmp(uri,"/") ) {		
		char dir[FILENAME_MAX];
		int len = strlen(uri)+1;
		strncpy(dir, uri, len);
		dir[len]= '\0';
		 
		chunk_ctx_printf(reply->pool, cb, "<a href=\"%s\">%s</a>%s %40s\n", 
			dirname(dir), PARENT_DIR_NAME, add_space(PARENT_DIR_NAME, 
			sizeof(PARENT_DIR_NAME), SPACE_CHAR), "-");
	}

    for(i =0; i < arr.num; i++)
	{
		entry = cpo_array_get_at(&arr, i);
		dname_len = cpo_strlen(entry->name);

		if(ETALON_SPACE < dname_len) {
			int dots;
			dname = cpo_pool_strdup(reply->pool, entry->name);
			
			for(dots = 4; dots >= 1; dots--) {
				dname[ETALON_SPACE-dots] = ((dots == 1) ? '>' : '.');
			}
			
			dname[ETALON_SPACE]='\0';
			dname_len = ETALON_SPACE;
		} else {
			dname = entry->name;
		}

		if(dname) {

			cpo_gmtime(&tm, &entry->mtime);

        	cpo_snprintf(date, sizeof(date), "%02d-%s-%d %02d:%02d ",
            	tm.tm_mday, months[tm.tm_mon ],	tm.tm_year, tm.tm_hour, tm.tm_min);

			if(entry->is_dir) {
				
				chunk_ctx_printf(reply->pool, cb, "<a href=\"%s%s/\">%s/</a>%s%10s %20s\n",
					uri, entry->name , dname, add_space(dname, dname_len, SPACE_CHAR), date, "-");
		    } else {
				
				chunk_ctx_printf(reply->pool, cb, "<a href=\"%s%s\">%s</a>%s %s %20.llu\n",
					uri, entry->name , dname, add_space(dname, dname_len ,SPACE_CHAR), date, (uintmax_t) entry->size);
			}
			//free dname
		}
    }

	chunk_ctx_printf(reply->pool, cb, indheadfoot[1], "");

	CNUNKS_ADD_TAIL_CTX(reply->out_filter, cb);

	free(arr.v);

	return CPO_OK;
}

static char *add_space(char *word, int len , char maska)
{
	int k, distance = 0 , etalon = ETALON_SPACE;
    static char tmp[ETALON_SPACE + 1];
	
    distance = etalon - len;

    memset(tmp, 0, sizeof(tmp));
    for(k=0; k < distance; k++)
    	tmp[k] = maska;
    return(tmp);

}

static int mod_autoindex_cmp_entries(const void *one, const void *two)
{
    cpo_autoind_entry_t *first = (cpo_autoind_entry_t *) one;
    cpo_autoind_entry_t *second = (cpo_autoind_entry_t *) two;

    if (first->is_dir && !second->is_dir) {
        return -1;
    }

    if (!first->is_dir && second->is_dir) {
        return 1;
    }

    return (int) strcmp(first->name, second->name);
}

