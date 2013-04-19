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

#ifndef _IO_SSL_H
#define _IO_SSL_H

int calipso_ssl_init() ;

void SSL_set_nonblocking(SSL *s);

ssize_t
cpo_io_ssl_read(calipso_client_t *client, void *buf, size_t len);

ssize_t
cpo_io_ssl_write(calipso_client_t *client, const void *buf, size_t len);

int cpo_io_ssl_is_handshake_done(calipso_client_t *client);

ssize_t
fd_ssl_write(SSL *ssl, const void *buf, size_t len);

ssize_t
fd_ssl_read(SSL *ssl, void *buf, size_t len);

ssize_t SSL_writev(SSL *ssl, const struct iovec *vector, int count);
int SSL_send_file(SSL * ssl, int rdesc /*IN_FD*/);

#endif

