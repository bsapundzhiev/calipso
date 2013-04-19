 /* config.h - Simple Config Parser
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* config option */
typedef void (*cfg_fptr)(void* , void*);

typedef struct s_cfg {
	char * name;	/* option name */
	void * val;		/* default value */
	cfg_fptr p;		/* handler */
} cfg_t;

typedef struct s_conf_ctx { 
	char *block;
	char *option; 
	char *value;
} conf_ctx_t;

typedef struct 	_List    	calipso_config_t;

enum {
	CTX_BLOCK_BEGIN,
	CTX_BLOCK_END,
	CTX_BLOCK_VAL
};

calipso_config_t *calipso_config_alloc(void);
int config_parse_file(calipso_config_t *config, char const *fname );
void config_set_option(calipso_config_t *config, const char *opt, const char *val , const char *section);
const char *config_get_option(calipso_config_t *config, const char *opt, const char *section);
int config_get_noption(calipso_config_t *config, const char *opt , const char *section);
double config_get_foption(calipso_config_t *config, const char *opt , const char *section);
void config_unalloc(calipso_config_t * config);
void config_list_dump(calipso_config_t *config);
int config_get_state_ctx( conf_ctx_t *c);

#endif /*!_CONFIG_H*/
