/**
 * well modules will look like this:
 * cm_init
 * cm_process_call
 * cm_exit
 */

#include <calipso.h>
#include "mod_fcgi.h"
#include "mod_xcgi.h"


#define   READ_FD	0
#define   WRITE_FD	1

static int mod_xcgi_init(void);
static int mod_xcgi_resource(calipso_request_t *request);
static int mod_xcgi_cgi_exec(calipso_request_t *request, const char * filename);

int mod_xcgi_parse_cgi_header(calipso_request_t *request, const char *hdr_line);
int mod_xcgi_readline(int fd, char *buf, int len);

int pm_init()
{
	TRACE("register: %s\n",__FILE__);
	calipso_register_hook(HOOK_INIT, mod_xcgi_init);
	calipso_register_hook(HOOK_RESOURCE, mod_xcgi_resource);
	return OK;
}

int pm_configure()
{
	TRACE("register module HOOK_CONFIGURE here\n");
	return 0;
}

int pm_exit()
{
	TRACE("register module HOOK_EXIT here\n");
	return 0;
}

int mod_xcgi_init()
{
	return 0;
}

int mod_xcgi_resource(calipso_request_t *request)
{
	const char *filename, *uri;
	
	calipso_reply_t *reply = calipso_request_get_reply(request);
	calipso_resource_t *resource = calipso_reply_get_resource(reply);
	int http_status  = calipso_reply_get_status(reply);

	if (calipso_resource_is_directory(resource) 
		||( calipso_http_status_is_error(http_status) )) {
		return (0);
	}

	uri = calipso_request_get_uri(request);
	filename = calipso_resource_get_path(resource);
	
	if( strncmp(uri, "/cgi-bin/", 9 ) == 0) {
		TRACE("exec GCI\n");
		/* TEST FCGI */
		if ( is_file_of(filename, ".php") ) {
			mod_xcgi_fcgi_exec(request, filename);
		} else {
			mod_xcgi_cgi_exec(request, filename);
		}
	}
	return 1;
}

int mod_xcgi_cgi_exec(calipso_request_t *request, const char * filename)
{
	int nExitCode = STILL_ACTIVE;
	char szBuffer[OUTPUTBUFSZ];
	HANDLE hProcess;
    int fdStdOut;
    int fdStdOutPipe[2];

	calipso_reply_t *reply;
	calipso_resource_t *resource;
	
	reply = calipso_request_get_reply(request);
	resource = calipso_reply_get_resource(reply);
	/* dont use resource */
	if(resource->resource_fd != -1) {
		close(resource->resource_fd);
		calipso_resource_set_file_descriptor(resource, -1);
	}

      // Create the pipe
    if(_pipe(fdStdOutPipe, OUTPUTBUFSZ, O_NOINHERIT) == -1)
         return   1;
	//fflush(stdout);
	fdStdOut = _dup(_fileno(stdout));

     // Duplicate write end of pipe to stdout file descriptor
     if(_dup2(fdStdOutPipe[WRITE_FD], _fileno(stdout)) != 0)
         return   2;

     // Close original write end of pipe
     _close(fdStdOutPipe[WRITE_FD]);

      // Spawn process
      //hProcess = (HANDLE)_spawnvp(P_NOWAIT, filename, (const char* const*)&filename);
	  hProcess  = (HANDLE)spawnl( P_NOWAIT, filename, filename, "_spawnl", "two", NULL );
      // Duplicate copy of original stdout back into stdout
      if(_dup2(fdStdOut, _fileno(stdout)) != 0)
         return   3;

      // Close duplicate copy of original stdout
      _close(fdStdOut);

      if(hProcess)
      {
         int nOutRead;
		 char header_line[1024];
		 while ( mod_xcgi_readline( fdStdOutPipe[READ_FD], header_line, 1024) > 0) {
			mod_xcgi_parse_cgi_header(request, header_line);
		 }
		 mod_xcgi_readline( fdStdOutPipe[READ_FD], header_line, 2);

         while   (nExitCode == STILL_ACTIVE)
         {
            nOutRead = _read(fdStdOutPipe[READ_FD], szBuffer, OUTPUTBUFSZ);
            if(nOutRead)
            {
               //fwrite(szBuffer, 1, nOutRead, stdout);
			   calipso_reply_print(reply, szBuffer, nOutRead);
            }

            if(!GetExitCodeProcess(hProcess,(unsigned long*)&nExitCode))
               return 4;
         }
      }
	
	return 0;
}

int mod_xcgi_readline(int fd, char *buf, int len)
{
        char *ptr = buf;
        char *ptr_end = ptr + len - 1;

        while (ptr < ptr_end) {
		/* get readhandler here */
                switch ( read(fd, ptr, 1)) {
                case 1:
                        if (*ptr == '\r')
                                continue;
                        else if (*ptr == '\n') {
                                *ptr = '\0';
                                return ptr - buf;
                        } else {
                                ptr++;
                                continue;
                        }
                case 0:
                        *ptr = '\0';
                        return ptr - buf;
                default:
                        printf("%s() failed: %s\n", __func__, strerror(errno));
                        return -1;
                }
        }

        return len;

}

int mod_xcgi_parse_cgi_header(calipso_request_t *request, const char *hdr_line)
{
	char *p;
	const char * header_name = hdr_line;
    char * header_content = p = strchr(header_name, ':');

    if (!p)  {
		return NOK;	
	}

	*p = '\0';
		do {
            header_content++;
		} while (*header_content == ' ');

	printf("cgi_hdr '%s' : '%s'\n", header_name, header_content);
	calipso_reply_set_header_value(request->reply, header_name, header_content);
   	*p = ':';
	return OK;
}
