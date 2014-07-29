/* cpo_io.c - io routines
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"

ssize_t cpo_io_read(calipso_client_t * client, void *buf, size_t len)
{
    return fd_read(client->csocket, buf, len);
}

ssize_t cpo_io_write(calipso_client_t * client, const void *buf, size_t len)
{
    return fd_write(client->csocket, buf, len);
}

ssize_t fd_write(int fd, const void *buf, size_t len)
{
    int cc;
    int total;
    total = 0;
    while (len > 0) {

        cc = send(fd, buf, len, 0);

        if (cc < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                printf("fd_write EWOULDBLOCK cc %d total %d errno %d\n", cc,
                       total, errno);
                //return cc;
                break;
            }

            printf("fd_write cc %d %s\n", cc, strerror(errno));
            return cc;
        }
        buf = (u_char*) buf + cc;
        total += cc;
        len -= cc;
    }

    return total;
}

ssize_t fd_read(int fd, void *buf, size_t len)
{
    int cc;
    int total;
    total = 0;

    while (len > 0) {

        cc = recv(fd, buf, len, 0);

        if (cc < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                printf("rd_read EWOULDBLOCK cc %d total %d errno %d\n", cc,
                       total, errno);
                //return cc;
                break;
            }

            printf("fd_read cc%d %s\n", cc, strerror(errno));
            return cc;
        }

        if (cc == 0)
            break;

        buf = (u_char*) buf + cc;
        total += cc;
        len -= cc;
    }
    return total;
}

/**
 *[\@ my buffer read
 */
#ifdef _WIN32
long int calipso_sendfile(int out_fd, int in_fd, size_t size )
{
    char *buf;
    long int cc,ww;
    long int total;
    total = 0;

    buf = malloc( OUTPUTBUFSZ + 1);
    size = min(OUTPUTBUFSZ , size);
    while ( size > 0 ) {
        if (( cc = read( in_fd, buf, OUTPUTBUFSZ) ) < 0 ||
                ( ww = fd_write( out_fd, buf, cc ) ) < 0 ) {
            total = -1;
            printf("ERROR:calipso_sendfile: %d Last %d\n ",errno, GetLastError());
            if (WSAEWOULDBLOCK == GetLastError())
                errno = EAGAIN;
            break;
        } else if ( cc == 0 || out_fd < 0 ) {
            break;
        }

        total += cc;
        size -= cc;
    }
    free(buf);
    return total;
}
#else
long int calipso_sendfile(int out_fd, int in_fd, size_t size)
{
    char *buf;
    long int cc, ww;
    long int total;
    total = 0;

    buf = malloc( OUTPUTBUFSZ + 1);
    while (size > 0) {

        if ((cc = read(in_fd, buf, OUTPUTBUFSZ)) < 0
                || (ww = fd_write(out_fd, buf, cc)) < 0) {
            total = -1;
            break;
        } else if (cc == 0 || out_fd < 0) {
            break;
        }

        total += cc;
        size -= cc;
    }
    free(buf);
    return total;
}
#endif

#ifdef LINUX
extern ssize_t splice();
//XXX: Linux-specific system calls splice,vmsplice
#ifndef SPLICE_F_MOVE
#define SPLICE_F_MOVE (0x01) /* move pages instead of copying */
#endif
#ifndef SPLICE_F_NONBLOCK
#define SPLICE_F_NONBLOCK (0x02) /* don't block on the pipe splicing (but */
#endif
#ifndef SPLICE_F_MORE
#define SPLICE_F_MORE (0x04) /* expect more data */
#endif
#ifndef SPLICE_F_GIFT
#define SPLICE_F_GIFT (0x08) /* pages passed in are a gift */
#endif
ssize_t splice_sendfile(int infd, int outfd, off_t *offset, ssize_t size)
{
    int p[2];
    ssize_t total = 0;
    int ret, written;

    if ((total = pipe(p)) == -1)
        goto out;

    ret = splice(infd, offset, p[1], NULL, size,
                 SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
    if (ret < 0) {
        total = ret;
        goto out;
    }

    while (total < ret) {
        written = splice(p[0], NULL, outfd, NULL, ret,
                         SPLICE_F_MORE | SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
        if (written < 0) {
            perror("splice()");
            if (errno == EAGAIN)
                continue;
            total = ret;
            goto out;
        }

        total += written;
    }
out:
    close(p[0]);
    close(p[1]);
    return total;
}
#endif

