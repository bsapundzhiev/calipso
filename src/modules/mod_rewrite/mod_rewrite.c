/* HTTP rule module
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <regex.h>
//#include "pcre.h"

#include <calipso.h>

#define REGEX_DEBUG
#define REGEX_MSG(regex, status)\
char msgbuf[1024];\
regerror(status, regex, msgbuf, sizeof(msgbuf));\
fprintf(stderr, "Regex failed: %s\n", msgbuf);\

static int	mod_rewrite_init(void);
static int  mod_rewrite_request(calipso_request_t *request);


static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
#ifdef REGEX_DEBUG
	REGEX_MSG(r,status);
#endif
        return CPO_ERR;
    }

    return CPO_OK;
}

int match(regex_t * r, const char *match) 
{

	int ret, reti = regexec(r, match, 0, NULL, 0);
    
	if( !reti ){
    	puts("Match");
		ret = CPO_OK;
   	}
    else if( reti == REG_NOMATCH ){
    	puts("No match");
		ret = CPO_ERR;
   	}
    else{
#ifdef REGEX_DEBUG
		REGEX_MSG(r,reti);
#endif
		ret = CPO_ERR;
 	}

	return ret;
}

int pm_init()
{
	calipso_register_hook(HOOK_INIT, mod_rewrite_init);
	calipso_register_hook(HOOK_REQUEST, mod_rewrite_request);

	return CPO_OK;
}

int pm_configure()
{
	TRACE("register module HOOK_CONFIGURE here\n");
	return 0;
}

int pm_exit()
{
	TRACE("exit\n");
	return CPO_OK;
}

static int mod_rewrite_init(void)
{
	return CPO_OK;
}

static int  mod_rewrite_request(calipso_request_t *request)
{
	regex_t r;
	TRACE("Start\n");

	compile_regex (&r, "^/dokuwiki/(data|conf|bin|inc)/");

	if ( match(&r, request->uri) ) {
		//test just deny
		calipso_reply_set_status(request->reply, HTTP_FORBIDDEN);
	} 

	regfree (& r);
	return CPO_OK;
}

