/* config.c - Simple Config Parser
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */
/*
 * syntax:
 *	key value #first option
 *	key=value;
 *	section {
 *		key=value;
 *		key val2
 *	}
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "dt.h"
#include "cplib.h"

#define MAXCONFLINELEN 	1024
#define MAXOPTLEN 		80

#define BLOCK_BEGIN 	"{"
#define BLOCK_END		"}"
#define DELIM_EQU		"="
#define DELIM_SPACE		" "
#define DELIM_TAB		"\t"

#define CONF_CTX_OPT	"context"
#define CONF_BEGIN_CTX 	"begin"
#define CONF_END_CTX 	"end"

#ifdef _WIN32 
#define OPTCMP stricmp
#else
#define OPTCMP	strcasecmp
#endif 

enum {
	PERR_LINE, 
	PERR_DELIM,
	PERR_PAIR,
	PERR_BEGIN,
	PERR_END,
	PERR_BLOCK,
};

static char _block[MAXOPTLEN];
static void parser_error(int nline, unsigned short err);


calipso_config_t *
calipso_config_alloc()
{
    return list_new();
}

void
config_set_option(calipso_config_t *config, const char *opt, const char *val , const char *section)
{
    conf_ctx_t *c = (conf_ctx_t*)malloc( sizeof( conf_ctx_t ) );

	if(c == NULL) return;

	c->block = section ? strdup(section) : NULL;
	c->option = opt ? strdup(opt) : NULL;
	c->value = val ? strdup(val): NULL;

    config = list_append( config, c );
}

static int block_begin(const char *line)
{
	char *s = strstr(line, BLOCK_BEGIN);
	return (s != NULL);
}

static int block_end(const char *line)
{
	char *s = strstr(line, BLOCK_END);
	return (s != NULL); 
}

int config_get_state_ctx( conf_ctx_t *c)
{
	if(!strcmp(c->option, CONF_CTX_OPT)) {

		if(!strcmp(c->value, CONF_BEGIN_CTX))
			return CTX_BLOCK_BEGIN;
		if(!strcmp(c->value, CONF_END_CTX))
			return 	CTX_BLOCK_END;
	}

	return CTX_BLOCK_VAL;
}


static int config_mark_ctx(calipso_config_t *config, short state) 
{
	if( strlen(_block) > 0 ) {
		
		switch(state) {
			case CTX_BLOCK_BEGIN:
				config_set_option(config, CONF_CTX_OPT, CONF_BEGIN_CTX,  _block);
			break;
			case CTX_BLOCK_END:
				config_set_option(config, CONF_CTX_OPT, CONF_END_CTX,  _block);
			break;
		}
	}

	return 1;
}

static void
config_parse_line(calipso_config_t *config , char *line, int nline)
{
	char option[MAXOPTLEN]= {0}, value[MAXOPTLEN] = {0};
	char *s, *delim = NULL;

    if(strlen(line) > MAXOPTLEN ) 
		parser_error(nline, PERR_LINE);

	//printf("line '%s' %d\n", line, nline); 
	if(block_begin(line)) {

		if(!strlen(_block)) {
			s = strtok(line, BLOCK_BEGIN);
			if(s) { 
				strncpy(_block, s, MAXOPTLEN);
				trim(_block);
				//mark context begin 
				config_mark_ctx(config, CTX_BLOCK_BEGIN);
			} else 	parser_error(nline, PERR_BLOCK);
		}
		else parser_error(nline, PERR_BEGIN);
		//printf("begin '%s'\n", _block);
		return;
	}

	if(block_end(line)) {
	
		if(strlen(_block) > 0) {
			//mark context end 
			config_mark_ctx(config, CTX_BLOCK_END);
			memset(_block, 0 , MAXOPTLEN);
		}
		else parser_error(nline, PERR_END);
		//printf("end '%s'\n", _block);
		return;
	}

	if(strstr(line , DELIM_EQU)) 
		delim = DELIM_EQU;
	else if(strstr(line , DELIM_SPACE))
		delim = DELIM_SPACE;
	else if(strstr(line, DELIM_TAB))
		delim = DELIM_TAB;
	else 
		parser_error(nline, PERR_DELIM);

	s = strtok (line, delim); 

	if (s) {
		strncpy (option, s, MAXOPTLEN);
	}

	s = strtok (NULL, delim);

    if (s) {
		strncpy (value, s, MAXOPTLEN);
	}

    if ( strlen(option) && strlen(value)) {
		trim(option);
		trim(value);
		config_set_option(config, option, value, strlen(_block) ? _block : NULL );
	} else {
		parser_error(nline, PERR_PAIR);
	}
}

int
config_parse_file(calipso_config_t *config, char const *fname )
{
    FILE *f;
    int i = 0, chr = 0, lines = 0;
	char line[MAXCONFLINELEN];
 	char parse = 0;

    if ( (f = fopen(fname,"r")) == NULL ) {
        printf("Error : '%s' %s \n",fname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (!feof(f)) {
        chr = fgetc(f);
        switch (chr) {
        case '<':
        case '#':
			while (chr!='\n' && chr != EOF) {
                chr= fgetc(f);
			}
			if(i > 0) {
				parse= 1;
			}
            break;
        case EOF:
			break;
        case '\n':
		case ';':
			parse = 1;
			break;
        default:
            line[i]= chr; 
            i++;
            break;
        }

		if('\n' == chr) {
        	lines++;
		}

		if(parse) {
		 	line[i]='\0';
			if(i > 1 || line[0] =='{' || line[0]=='}') {
				config_parse_line(config, line, lines);
			}
         	parse = i = 0;
		}
    }

	if(strlen(_block))
		parser_error(lines, PERR_END);

    fclose(f);   
    return lines;
}

const char *config_get_option(calipso_config_t *config, const char *opt , const char *section)
{
	conf_ctx_t *cfg;
    config = list_get_first_entry( config );

    while ( config ) {
        cfg = list_get_entry_value(config);

		if(!section) {
        	if (!OPTCMP( cfg->option , opt) ) {
            	return (const char *)cfg->value;
			}
		} else if(cfg->block) {
			if (!OPTCMP(cfg->block, section) && !OPTCMP( cfg->option , opt)) {
            	return (const char *)cfg->value;
			}
		}

        config = list_get_next_entry( config );
    }
    return (NULL);
}

int config_get_noption(calipso_config_t *config, const char *opt , const char *section)
{
	const char *val = config_get_option(config, opt , section );
	return val ? atoi(val) : 0;
}

double config_get_foption(calipso_config_t *config, const char *opt , const char *section)
{
	const char *val = config_get_option(config, opt , section);
	return val ? atof(val) : 0.0;
}

void
config_list_dump(calipso_config_t *config)
{
    int i = 1;
    conf_ctx_t *conf;

    config = list_get_first_entry( config );

    while ( config !=NULL ) {
        conf = list_get_entry_value( config );
		printf("%4d. %-20s => %-40s @ %s\n", i, conf->option, conf->value, conf->block);
        config = list_get_next_entry( config );
        i++;
	}
}

void config_unalloc(calipso_config_t * config) 
{
    conf_ctx_t *c;
	calipso_config_t * p;
    config = list_get_first_entry( config );
	p = config;

    while ( config !=NULL ) {
        c = list_get_entry_value( config );
		if(c) {	
			if(c->block)
				free(c->block);
			if(c->option)
				free(c->option);
			if(c->value);
				free(c->value);
			free(c);
		}
        config = list_get_next_entry( config );	
    }

	list_delete( p );
	free(p);
}

calipso_config_t *
calipso_config_get_parent(calipso_config_t * parent )
{
    return parent;
}

int
calipso_config_set_parent(calipso_config_t * parent , calipso_config_t * config)
{
    return ((parent = config)!= NULL);
}

static void parser_error(int nline, unsigned short err) 
{	
	fprintf(stderr, "parser error @ line: %d ", nline);
	
	switch (err) {
		case PERR_LINE:
			fprintf(stderr, "Line too long!\n");
        break;
		case PERR_DELIM:
			fprintf(stderr, "Unknown delimiter or block quote\n");
        break;
		case PERR_PAIR:
			fprintf(stderr, "Missing value pair!\n");
		break;
		case PERR_BEGIN:
			fprintf(stderr, "Section is already open!\n");
        break;
		case PERR_END:
			fprintf(stderr, "No open section or missing end quote!\n");
		break;
		case PERR_BLOCK:
			fprintf(stderr, "Open section without name!\n");
		break;
		default:
			fprintf(stderr, "Unknown error!\n");
    }

	exit(EXIT_FAILURE);
}

