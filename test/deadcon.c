#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>

int
sockndelay(int sfd, int on)
{

	return ((on) ? fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK):
		fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) & ~O_NONBLOCK));

}

int
tconnect(struct in_addr const *paddr, int port, int timeout)
{

	int sfd;
	struct sockaddr_in sin;
	fd_set wfds;
	struct timeval tv;

	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return (-1);
	}

	memset(&sin, 0, sizeof(sin));
	memcpy(&sin.sin_addr, &paddr->s_addr, 4);
	sin.sin_port = htons((short int) port);
	sin.sin_family = AF_INET;

	if (sockndelay(sfd, 1) == -1)
	{
		perror("sockndelay");
		close(sfd);
		return (-1);
	}

	if (connect(sfd, (struct sockaddr *) &sin, sizeof(sin)) == 0)
	{
		sockndelay(sfd, 0);
		return (sfd);
	}

	if ((errno != EINPROGRESS) && (errno != EWOULDBLOCK))
	{
		perror("connect");
		close(sfd);
		return (-1);
	}


	FD_ZERO(&wfds);
	FD_SET(sfd, &wfds);
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	if (select(sfd + 1, (fd_set *) 0, &wfds, (fd_set *) 0, &tv) <= 0)
	{
		perror("select");
		close(sfd);
		return (-1);
	}

	if (FD_ISSET(sfd, &wfds))
	{
		sockndelay(sfd, 0);
		return (sfd);
	}

	close(sfd);

	return (-1);
}

int
main(int argc, char *argv[])
{
	int ii;
	char *server;
	int port;
	int nconns, ccreat = 0;
	struct hostent * he;
	struct in_addr inadr;
	struct sockaddr_in sin;

	if (argc < 4)
	{
		printf("use: %s server port numconns\n", argv[0]);
		return (1);
	}

	server = argv[1];
	port = atoi(argv[2]);
	nconns = atoi(argv[3]);

	if (inet_aton(server, &inadr) == 0)
	{
		if ((he = gethostbyname(server)) == NULL)
		{
			fprintf(stderr, "unable to resolve: %s\n", server);
			return (-1);
		}

		memcpy(&inadr.s_addr, he->h_addr_list[0], he->h_length);
	}

	for (ii = 0; ii < nconns; ii++)
	{
		int sfd = tconnect(&inadr, port, 30);

		if (sfd != -1)
		{
			char const *req = "GET / HTTP/1.0\r\n";
			write(sfd, req, strlen(req));
			++ccreat;
		}
	}

	printf("%d connections created ...\n", ccreat);

	//while (1)
	//	sleep(10);

	return (0);

} 
