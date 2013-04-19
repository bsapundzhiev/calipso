/* SSL IO
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"
#include "cpo_io_ssl.h"

int calipso_ssl_init() 
{
	TRACE("Init SSL...\n");
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	//ERR_load_BIO_strings();
	return CPO_OK;
}

void SSL_set_nonblocking(SSL *s)
{
    BIO_set_nbio(SSL_get_rbio(s),1);
    BIO_set_nbio(SSL_get_wbio(s),1);
}

ssize_t
cpo_io_ssl_read(calipso_client_t *client, void *buf, size_t len)
{
	return fd_ssl_read(client->ssl, buf, len);
}
ssize_t
cpo_io_ssl_write(calipso_client_t *client, const void *buf, size_t len)
{
	return fd_ssl_write(client->ssl, buf, len);
}

int cpo_io_ssl_is_handshake_done(calipso_client_t *client)
{
	printf("read_header_buff:SSL_State %s\n", SSL_state_string(client->ssl));

	if(!client->ssl) return 0;

	if( SSL_in_init(client->ssl) ) {
			
		int ret = SSL_do_handshake(client->ssl);
		TRACE("SSL_accept_state\n");
		if(ret < 0) {
		int err = SSL_get_error(client->ssl, ret);
			switch(err) {
				case SSL_ERROR_WANT_READ: 
				TRACE("SSL_ERROR_WANT_READ\n");
				break;
				case SSL_ERROR_WANT_WRITE:
				TRACE("SSL_ERROR_WANT_WRITE\n");
				break;
				default:
					/*XXX: no clue for redirect to HTTP */
				cpo_log_error(calipso->log, "connection error SSL_error(%d)", err );
				return calipso_client_connection_error(client);
			}
		}
		return 0;
	}

	return CPO_OK;
}

ssize_t
fd_ssl_write(SSL *ssl, const void *buf, size_t len)
{
    int cc;
    int total;
    total = 0;

    while (len > 0) {
		cc = SSL_write(ssl, buf, len);
		if(errno == EAGAIN) {
			printf("fd_ssl_write EWOULDBLOCK\n"); 
			//fcntl(fd, F_SETFL, ~O_NONBLOCK);
			//continue;
			return cc;
		}
        if (cc < 0 || errno == EINTR)
            return -1;
#ifdef _WIN32
        (char*)
#endif
        buf += cc;
        total += cc;
        len -= cc;
    }
    return total;
}

ssize_t
fd_ssl_read(SSL *ssl, void *buf, size_t len)
{
    int cc;
    int total;
    total = 0;

    while (len > 0) {
		cc = SSL_read(ssl, buf, len);
	
        if (cc < 0 ) {

			if(errno == EAGAIN || errno == EINTR) {
				printf("fd_ssl_read EWOULDBLOCK\n");
				return total;
			}
	
			int err = SSL_get_error(ssl, cc);
			printf("cc %d fd %d\n", cc, SSL_get_fd(ssl));
			printf("fd_ssl_read ssl_error %d\n", err);
			switch(err) {
			case SSL_ERROR_ZERO_RETURN: 
				printf("SSL_ERROR_ZERO_RETURN\n");
				break;
			case SSL_ERROR_WANT_READ:
				printf("SSL_ERROR_WANT_READ\n"); 
				break;
			case SSL_ERROR_WANT_WRITE:
				printf("SSL_ERROR_WANT_WRITE\n");
				break;
			default:
				ERR_print_errors_fp(stderr);
				exit(-1);	
			}
          	return total;
		}
        if (cc == 0)
            break;
#ifdef _WIN32
        (char*)
#endif
        buf += cc;
        total += cc;
        len -= cc;
    }
    return total;
}

ssize_t SSL_writev(SSL *ssl, const struct iovec *vector, int count)
{
    int i, n;
    ssize_t ret = 0;
    //int ssl_error;

    for (i = 0; i < count; ++i) {
        n = SSL_write(ssl, vector[i].iov_base, vector[i].iov_len);
        if (n <= 0) {
            //ssl_error = SSL_get_error(ssl, n);
            return ret == 0 ? n : ret;
        } else
            ret += n;
    }

    return ret;
}

int SSL_send_file(SSL * ssl, int rdesc /*IN_FD*/)
{
	ssize_t	readret;
	char buffer[OUTPUTBUFSZ];

    //readret = fd_read(rdesc, buffer, OUTPUTBUFSZ);
	readret = read(rdesc, buffer, OUTPUTBUFSZ);
    if (readret == -1) {
    	TRACE(" NEEDS_FIX - handle this correctly -> %s \n", strerror(errno));
        return (readret);
    } else {

      close(rdesc);

    }					
	
	return SSL_write(ssl, buffer, readret);
}

