/* $Id: mod_php5.c 152 2013-02-09 17:00:33Z borislav $ */
/*
 *   Copyright (C) 2008-2009 by Borislav Sapundjiev <BSapundjiev_AT_gmail[D0T]com> 
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details. 
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "calipso.h"
#include "rfc2616.h"
#include "cplib.h"
#include "mod_php5.h"

#include "php_calipso.h"


int mod_php5_register_hooks() 
{
	//calipso_register_handler("*/*.php", php_handler);
	calipso_register_hook(HOOK_INIT, (void *)mod_php5_init);
	calipso_register_hook(HOOK_CONFIGURE, (void *)mod_php5_configure);
	calipso_register_hook(HOOK_CHROOT, (void *)mod_php5_chroot);
	//calipso_register_hook(HOOK_PRIVILEGES, (void *)mod_php5_privileges);
	calipso_register_hook(HOOK_RESOURCE, (void *)mod_php5_resource);
}

int mod_php5_init(void) 
{
	return 1;
}

int mod_php5_configure(void)
{
	//configure php
	return 1;
}

int mod_php5_chroot(void) 
{
	const char *chrootenable;
	const char *chrootpath;	
	calipso_config_t *config;

	config = calipso_get_config();

	/* Check wether chroot()-ing is required, chroot is default behaviour */
	chrootenable = config_get_option(config, "chroot", NULL);

	if (chrootenable == NULL /*|| _config_is_enabled(chrootenable)*/) {
		if ( calipso_get_uid() )
			printf("Chroot() must be disabled as non privileged user.");

		chrootpath = config_get_option(config, "chrootpath", NULL);

		if (chrootpath == NULL)
			printf("Chroot() directive enabled, but no ChrootPath() set.");
		if (chroot(chrootpath) < 0)
			printf("chroot() call failure in %s.", __func__);
		if (chdir("/") < 0)
			printf("chdir() call failure in %s.", __func__);
	}

	return 1;
}

int mod_php5_privileges(void) 
{
	return 1;	
}

int mod_php5_request(calipso_request_t *request) 
{
	//TODO: use http request hook
	return 1;
}

int mod_php5_resource(calipso_request_t *request) 
{
	int fd, status;
	const char *filename = NULL;
	//struct stat sb;
	calipso_reply_t *reply;
	calipso_resource_t *resource;
	
	/* And stat the resource for other modules to know what type it is */
	reply = calipso_request_get_reply(request);
	resource = calipso_reply_get_resource(reply);
	filename = calipso_resource_get_path(resource);

  	status = calipso_reply_get_status(reply);
 	if (calipso_http_status_is_error(status)) {
		return 0;
	}
	//printf("%s %s ",__func__ , filename);
	//TODO: add mime
	if ( !is_file_of(filename, ".php") ) {
		return (1);
	}

	/* dont use resource */
	if(resource->resource_fd) {
		close(resource->resource_fd);
		calipso_resource_set_file_descriptor(resource, -1);
	}

	/* set the default handler */
	//calipso_request_set_handler(request, php_handler);
	if( access(filename, R_OK) == 0 ) {
	
		php_handler(request);
	} 
	else {
		 calipso_reply_set_status(reply, HTTP_FORBIDDEN);
	}
	
	return 1;	
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
