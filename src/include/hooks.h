#ifndef _HOOKS_H
#define _HOOKS_H

#define DEBUG_HOOKS

enum cpo_server_hooks
{
	HOOK_LOAD_DYNAMIC = 0,
	HOOK_INIT = 1,
	HOOK_CONFIGURE = 2,	
	HOOK_CHROOT = 3,
	HOOK_PRIVILEGES = 4,
	HOOK_CONNECT = 5,
	HOOK_DISCONNECT = 6,
	HOOK_REQUEST = 7,
	HOOK_TRANSLATE = 8,
	HOOK_RESOURCE = 9,
	HOOK_AUTH_ID_CHECK = 10,
	HOOK_AUTH_ACCESS_CHECK = 11,
	HOOK_ACCESS_CHECK = 12,
	HOOK_MIME = 13,
	HOOK_REPLY = 14,
	HOOK_LOG = 15,
	HOOK_END = 16,
 	/* hook count */
	NR_HOOKS
};

void 
calipso_register_hook(int hook, void *callback_fn );

int
calipso_trigger_hook(int hook, ...);

const char *
get_hook_byid(const unsigned int id);

void 
calipso_hook_dump(int hook);

int 
calipso_hook_get_NR_HOOKS();

#endif 
