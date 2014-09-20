#include "calipso.h"
#include "dav_fs.h"

#define NOT_IMPLEMENTED -1

static int dav_fs_delete();
static int dav_fs_put();
static int dav_fs_get();
static int dav_fs_get_name();
static int dav_fs_get_size();
static int dav_fs_get_last_modified();
static int dav_fs_create_file(const char *name, void *data);
static int dav_fs_create_directory(const char *name);

dav_fs_t mod_dav_fops = { 
	.path = NULL, 
	.delete = dav_fs_delete,
  	.put= dav_fs_put,
  	.get= dav_fs_get,
  	.get_name= dav_fs_get_name,
  	.get_size= dav_fs_get_size,
  	.get_last_modified= dav_fs_get_last_modified,
  	
  	.create_file= dav_fs_create_file, 
  	.create_directory= dav_fs_create_directory,
  	/*
  	//self array
  	struct dav_file_s (*get_children)();
  	//single child
  	struct dav_file_s (*get_child)(const char *name);*/
};

static int dav_fs_delete()
{
	return NOT_IMPLEMENTED;
}

static int dav_fs_put()
{
	return NOT_IMPLEMENTED;
}

static int dav_fs_get()
{
	return NOT_IMPLEMENTED;
}

static int dav_fs_get_name()
{
	return NOT_IMPLEMENTED;
}

static int dav_fs_get_size()
{
	return NOT_IMPLEMENTED;
}

static int dav_fs_get_last_modified()
{
	return NOT_IMPLEMENTED;
}

static int dav_fs_create_file(const char *name,void *data)
{
	return NOT_IMPLEMENTED;
}

static int dav_fs_create_directory(const char *name)
{
	return NOT_IMPLEMENTED;
}
