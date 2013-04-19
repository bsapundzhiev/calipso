/*remote tasks */
#include "bridge.h"
#include <calipso.h>

static u_int get_remote_ip(const char *hostname)
{
	u_int ip;
	struct hostent* hent;

	ip = inet_addr(hostname);
	if (ip == 0xffffffff) {
		hent = gethostbyname(hostname);
		if (hent == NULL)
			return INADDR_NONE;
		ip= *((unsigned long *)hent->h_addr);
	}
	return ip;
}

int cpo_bridge_init()
{
	return CPO_ERR;
}

int cpo_bridge_connect(calipso_request_t *r, const char * host , unsigned int port)
{
	int sockfd;
    struct sockaddr_in serv_addr;
	calipso_socket_t * listener = r->client->listener;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
        perror("ERROR opening socket");
		calipso_reply_set_status(r->reply, HTTP_BAD_GATEWAY);
		return CPO_ERR;
	}
 
	set_keep_alive(sockfd, 1);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = get_remote_ip(host);
    serv_addr.sin_port = htons(port);

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ 
        perror("ERROR connecting");
		calipso_reply_set_status(r->reply, HTTP_BAD_GATEWAY);
		return CPO_ERR;
	}

	return CPO_ERR;
}

int cpo_brige_process_up(calipso_request_t *r)
{
	return CPO_ERR;
}

int cpo_brige_process_down(calipso_request_t *r)
{
	return CPO_ERR;
}

//Handlers
int cpo_bridge_reply_handler(calipso_request_t *r)
{
	return CPO_ERR;
}

