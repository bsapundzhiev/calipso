/*
 *   Copyright (C) 2007 by Borislav Sapundzhiev <BSapundjiev@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#define ARCH	"win32"
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)
#define closesocket close
#define ARCH	"Linux"
#endif


/*-----------------------------*
 *-- Format for HTTP request --*
 *-----------------------------*/
#define HTTP_HEADER HTTP_HEADER_1

#define HTTP_HEADER_1 "GET %s HTTP/1.0\r\n\
Host: localhost\r\n\
User-Agent: gh (arch %s)\r\n\
Accept: */*\r\n\
Accept-Charset: us-ascii, ISO-8859-1, ISO-8859-2, ISO-8859-4, ISO-8895-5, ISO-88\
59-7, ISO-8895-9, ISO-8859-13, windows-1250, windows-1251, windows-1257, cp437,\
cp737, cp850, cp852, cp866, x-cp866-u, x-mac, x-mac-ce, x-kam-cs, koi8-r, koi8-u\
, TCVN-5712, VISCII, utf-8\r\n\
Connection: clos%c\r\n\r\n"

/* --- */

#define VER 	"0.4"
#define COPYSTR "Autor: (c) 2007 Borislav Sapundzhiev <bsapundjiev@gmail.com>"

#define ARRAY_SIZE(arr,array)\
	arr = sizeof(array) / sizeof(array[0]);

struct buf {
    int Socket;
    int rec;
    int sent;
    char szBuffer[1024];
} gh_buf;

void GetHTTP( char *args[] );
int parse_HTTP_header(int socket);
int HTTP_sockgets_trigger(int sock, char *buf, int len);
static void usage();

int
sockndelay(int sfd, int on)
{
    return ((on) ? fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK):
            fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) & ~O_NONBLOCK));

}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage();
        return (1);
    }

#ifdef _WIN32
    setmode(fileno(stdout), _O_BINARY);
#endif
    GetHTTP(argv);

    return (0);
}

int gh_connect_poll()
{
    int nRet;
    fd_set wfd, rfd;
    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;

    FD_ZERO(&wfd);
    FD_ZERO(&rfd);
    FD_SET(gh_buf.Socket, &wfd);
    //FD_SET(gh_buf.Socket, &rfd);
    
    for (;;) {

        if (select(gh_buf.Socket + 1, &rfd, &wfd, NULL, &tv) <= 0) {
            perror("select");
            close(gh_buf.Socket);
            return SOCKET_ERROR;
        }

        if (FD_ISSET(gh_buf.Socket, &wfd)) {
            //printf("write %s\n", gh_buf.szBuffer);

            nRet = send(gh_buf.Socket, gh_buf.szBuffer, strlen(gh_buf.szBuffer), 0);
            //printf("nRet %d\n", nRet);

            if (nRet == SOCKET_ERROR) {

                if ((errno != EINPROGRESS) && (errno != EWOULDBLOCK)) {
                    perror("send");
                    closesocket(gh_buf.Socket);
                    return SOCKET_ERROR;
                } else {
                    continue;
                }
            }
            
            memset(gh_buf.szBuffer, 0 , sizeof(gh_buf.szBuffer));
            FD_ZERO(&wfd);
            FD_SET(gh_buf.Socket, &rfd);
        }

        if (FD_ISSET(gh_buf.Socket, &rfd)) {
            //printf("read...\n");          
            nRet = recv(gh_buf.Socket, gh_buf.szBuffer, sizeof(gh_buf.szBuffer), 0);
            //nRet = http_parse_header(gh_buf.Socket);
            
            if (nRet == SOCKET_ERROR) {

                if ((errno != EINPROGRESS) && (errno != EWOULDBLOCK)) {
                    perror("recv");
                    closesocket(gh_buf.Socket);
                    return SOCKET_ERROR;
                }
            }
            /* Did the server close the connection? */
            if (nRet == 0) {
               
                break;
            }

            fwrite(gh_buf.szBuffer, nRet, 1, stdout);
        }
    }

    return 1;
}

void GetHTTP( char *args[] )
{
    int nRet;
    struct 	in_addr		iaHost;
    struct 	hostent  	*lpHostEntry;
    //struct 	servent    	*lpServEnt;
    struct 	sockaddr_in 	saServer;
    char *lpServerName;
    char *lpFileName;
    char *ptr;
    int port=0;


    char proto[64];
    char host[512];
    char urlpath[1024];
    char invalid_char = 'e';
    //ptr= (char*) malloc( 16 );

    if (sscanf(args[1],"%64[^\n:]://%512[^\n/?]%[^\n]",proto, host, urlpath) < 2) {
        printf("Error: invalid URL\n");
        return;
    }

    lpServerName = strtok(host,":");

    if (strlen(urlpath) > 1)
        lpFileName = urlpath;
    else
        lpFileName = "/";

    ptr = strtok( NULL , ":" );


    if (ptr != NULL )
        port = atoi(ptr);
    else
        port = (80);

    printf("resolving::lpServerName='%s', lpFileName='%s', port='%d'\n" , lpServerName, lpFileName, port );

    iaHost.s_addr = inet_addr(lpServerName);

    if (iaHost.s_addr == (INADDR_NONE) ) {

        lpHostEntry = gethostbyname(lpServerName);
    } else {	/*It was a valid IP address string*/
        lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
    }

    if (lpHostEntry == NULL) {
        perror("gethostbyname()");
        return;
    }

    /*Create a TCP/IP stream socket*/
    gh_buf.Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (gh_buf.Socket == INVALID_SOCKET) {
        perror("socket()");
        return;
    }

    if (sockndelay(gh_buf.Socket, 1) == -1) {
        perror("sockndelay");
        close(gh_buf.Socket);
        return;
    }

    saServer.sin_port = htons( port );
    saServer.sin_family = AF_INET;
    saServer.sin_addr = *( (struct in_addr*) *lpHostEntry-> h_addr_list );

    nRet = connect(gh_buf.Socket, (struct sockaddr*)&saServer, sizeof(struct sockaddr_in));

    if (nRet == SOCKET_ERROR) {

        if ((errno != EINPROGRESS) && (errno != EWOULDBLOCK)) {
            perror("connect");
            closesocket(gh_buf.Socket);
            return;
        }
    }

    //send invalid char
    if (args[2] && !strcmp(args[2],"-x")) {
        invalid_char = 0x1;
    }

    sprintf(gh_buf.szBuffer, HTTP_HEADER, lpFileName, ARCH, invalid_char);
    gh_connect_poll();

#if 0
    if (args[2] && !strcmp(args[2],"-d")) {
        parse_HTTP_header(Socket);
        closesocket(Socket);
        return;
    }
#endif

    closesocket(gh_buf.Socket);
}

int http_parse_header(int socket)
{

    int lines = 0;
    char header_line[1024];
    /* parse header */
    int ret;
    printf("---- HTTP HEADER> ----\n");
    while ( (ret = HTTP_sockgets_trigger( socket, header_line, 1024)) )
        printf("line %d => %s\n", lines++, header_line );
    printf("---- <HTTP HEADER ----\n\n");

    return ret;
}


/**
 * get connection bytes (read line \n\r)
 */
int HTTP_sockgets_trigger(int sock, char *buf, int len)
{
    char *ptr = buf;
    char *ptr_end = ptr + len - 1;

    while (ptr < ptr_end) {
        /* get readhandler here */
        switch ( recv(sock, ptr, 1,0)) {
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
            //printf("%s() failed: %s\n", __func__, strerror(errno));
            return -1;
        }
    }

    return len;

}

static void usage()
{
    printf("\n\
GetHttp -- http header dump program -- (%s) build (%s)\n\n\
%s \n\n\
Usage: [http://serv_name:port/path<?query>] -[d]\n\
\t-d - dump HTTP header only\n\
\t-x - set invalid char(0x1) in header\n\
\n\n\
",ARCH,__DATE__, COPYSTR );
}