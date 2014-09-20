#ifndef __DAV_FS_H
#define __DAV_FS_H


typedef struct dav_fs_s {
	char *path;
	/*file*/
	int (*delete)();
  	int (*put)();
  	int (*get)();
  	int (*get_name)();
  	int (*get_size)();
  	int (*get_last_modified)();
  
  	/*dir*/
  	int (*create_file)(const char *name,void *data);
  	int (*create_directory)(const char *name);
  	//self array
  	struct dav_file_s (*get_children)();
  	//single child
  	struct dav_file_s (*get_child)(const char *name);

} dav_fs_t;

#endif