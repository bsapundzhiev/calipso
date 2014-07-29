#ifndef _CPO_CORE_H
#define _CPO_CORE_H

#define LISTEN_BACKLOG	512

#ifndef MAXHOSTNAMELEN
#define	MAXHOSTNAMELEN 	80
#endif

#ifndef   MAXPATHLEN
#define   MAXPATHLEN      1024
#endif

#define NR_HOOKS 17

enum {
	REPLY_STATE_NONE = 0,
	REPLY_STATE_TRANSFER = 1,
	REPLY_STATE_DONE = 2,
};

#ifndef  OUTPUTBUFSZ
#define  OUTPUTBUFSZ	4096
#endif

#ifndef	INPUTBUFSZ
#define INPUTBUFSZ		OUTPUTBUFSZ
#endif

/**
 * pools
 */
#ifndef CALIPSO_DEFAULT_POOLSIZE
#define CALIPSO_DEFAULT_POOLSIZE	256
#endif

/**
 * struct listener
 */
struct  s_socket {

	List 	*	config;
	hash_t 	*	server;
	queue_t *	client;
    int		lsocket;
    int		state;
    int16_t		port;
    int		(*accept_callback)(void *);
    ssize_t		(*r)(calipso_client_t *, void *, size_t);
    ssize_t		(*w)(calipso_client_t *, const void *, size_t); 
	struct cpo_event_s *event; /*event */
#ifdef USE_SSL
	SSL_CTX	* ssl_ctx;
#endif
};

struct s_request {

	struct mpool *pool;
  	List *	config;
  	struct s_client * client;
  	struct s_reply * reply;
 	hash_t * header;
  	int(*request_hdl)(struct s_request *);
  	char *	method;
  	char *	uri;
  	char *	version;
	char *	querystring;
	char * 	host;
	char *	user; 
  	time_t	request_time;
	struct 	chunks * in_filter; /*request body*/
	struct 	chunk_ctx *header_buf;
	int 	body_length;
};

struct s_calipso {
	struct mpool *pool;
  	List * config;
	/*EXPERIMENTAL*/
  	struct s_process * currentproc;
  	struct s_process * mprocess;
  	/*ds_array_t *	nprocesses;*/
	struct bin_tree	*nprocesses;
	/*struct s_process * lprocess;*/
  	/*struct s_process * eprocess;*/
  	
  	hash_t * listeners;/* here are all 
				 	 	* listen socket struct's
				 	 	*/
    List   * m_hook[NR_HOOKS];	/* hooks */
    hash_t * m_handler;		/* server handlers */
  	int	(*mproc_model)(void);
  	int	(*nproc_model)(void);
	struct s_module *mod_table; 	/* module table */
  	void *	pw;			/* struct passwd */
  	void *	gr;			/* struct group	 */
  	time_t	start_time;
  	uid_t	uid;
  	gid_t	gid;
	cpo_log_t * log;
};


struct s_process {
	struct mpool *pool;
  	pid_t		pid;
  	int8_t		ptype;
  	int		psocket;
  	/* EXPERIMENTAL */
  	int		pcontrolsocket;
  	/* EXPERIMENTAL */
  	int32_t		pflags;
  	int32_t		pstate;
  	time_t		proc_time;
};


struct s_server {
	struct mpool *pool;
	List *	config;
    int	state;
    char	hostname[MAXHOSTNAMELEN];
    char	serverroot[MAXPATHLEN];
    char	documentroot[MAXPATHLEN];
	int16_t 	keep_alive_max; 			/* max persistent conn */
	//cpo_log_t * log;
};

struct s_client {
	struct mpool *pool;
    List * config;
    struct s_socket * listener;
    struct s_server * server;
    struct s_request * request;
    /*ds_fifo_t * pipeline;*/
	queue_t * pipeline; 	/*client pipeline method*/
    int csocket;
	struct sockaddr_in* info;
    time_t	connect_time;
	int8_t	keepalive; 	/*keepalive conn*/
	int8_t	keepalives;	/*persistent connections cntr*/
	int(*client_persistent_hdl)(struct s_client *); /*persistent handler*/
	timer_t ctmr;		/*conn timer*/
	int8_t done; /* client complete state */
	struct cpo_event_s *event; /*event */
	size_t pending_bytes; 
#ifdef USE_SSL
	SSL	* ssl;
#endif
};

struct s_reply {
	struct mpool *pool;
  	List *	config;
  	struct s_client *	client;
  	struct s_request *	request;
  	struct s_resource *	resource;
	hash_t *header;
  	int(*reply_hdl)(struct s_reply *);
  	int	state;  	/*handler state*/
  	int	status;		/*status code*/
  	int	bytes_sent;
  	int	bytes_to_send;
  	size_t	replybufsz;
	struct 	chunks * out_filter;
	size_t 	content_length;		/*reply lenght*/
};

/**
 * resource
 */
struct s_resource {
	struct mpool *pool;
  	char *	resource_path;
  	int	resource_fd;
  	void *	resource_stat;
	off_t 	offset;			/*file offset*/
};
/*
struct s_config {
        char *option;
        char *value;
};*/

/**
 * struct module
 */
struct s_module {
	const char *mod_name;
 	void *handler;
	/*int (*handle)(void*);*/
};

/**
 * forward struct pool
 */
//struct mpool;

/**
 * calipso type's
 */
typedef struct  s_calipso   calipso_t;
typedef struct 	s_socket	calipso_socket_t;
typedef struct 	s_server 	calipso_server_t;
//typedef struct 	_List    	calipso_config_t;
typedef struct 	s_client	calipso_client_t;
typedef struct 	s_request	calipso_request_t;
typedef struct	s_reply		calipso_reply_t;
typedef	struct 	mpool		calipso_pool_t;
typedef struct 	s_process	calipso_process_t;
typedef	struct 	s_resource 	calipso_resource_t;
typedef struct  s_module 	calipso_mod_t;

#endif 
