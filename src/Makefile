# $Id: Makefile 159 2013-04-17 22:25:23Z borislav $
#*
#* Copyright (C) 2012 Borislav Sapundzhiev
#*
#* This file is released under the terms of GPL v2 and any later version.
#*
#*

ifndef OS
CFLAGS+= -DUSE_SSL
USE_SSL=1
endif

OBJS    = main.o\
 	calipso.o\
	array.o\
	list.o\
	btree.o\
	rbtree.o\
	queue.o\
	hash.o\
	dllist.o\
	http.o\
	config.o\
	socket.o\
	signal.o\
	cpo_io.o\
	cpo_aio.o\
	server.o\
	client.o\
	reply.o\
	request.o\
	resource.o\
	poll.o\
	epoll.o\
	event.o\
	mpool.o\
	process.o\
	mprocess.o\
	module.o\
	hooks.o\
	cplib/thread.o\
	cplib/cplib.o\
	cplib/xmalloc.o\
	cplib/cp_time.o\
	cplib/cpo_string.o\
	cplib/cpo_file.o\
	cplib/base64.o\
	chunks.o\
	timer.o\
	cpo_log.o

ifdef USE_SSL
	OBJS+=cpo_io_ssl.o
endif
#
# project related
#
ifndef VERSION
VERSION = $(shell cat ../VERSION)
CFLAGS+= -DVERSION="\"$(VERSION)\""
endif

ifndef OS
CFLAGS+= -DOS="\"$(shell uname)"\"
endif

TARGET 	= calipso
BINDIR =/usr/sbin

#
# if you need large file support 
# edit this line.
LFS 	= -D_FILE_OFFSET_BITS=64 -D_REENTRANT
#get system name


INCLUDE_DIR =./include 

STR_CFLAGS = -DDEBUG -D_GNU_SOURCE

CFLAGS += -g -O2 -Wall $(STR_CFLAGS) $(LFS) -I$(INCLUDE_DIR)
LDFLAGS += -rdynamic

#CFLAGS += -Werror -W -Wno-unused-parameter -pedantic -g -O0 -ansi -fno-exceptions 
#
# if your system needs any additional libraries (solaris, for example, 
# needs the ones commented out below), edit this line.
EXTRA_LIBS = -lpthread -ldl -lrt

ifdef USE_SSL
EXTRA_LIBS += -lssl -lcrypto
endif

#
# this line should build under os/2 using syslog from
# LIBS = -lsyslog -lsocket

LIBS = $(EXTRA_LIBS)

all:calipso $(OBJS) 	

strip:		
	strip $(TARGET)
clean: 		
	rm -f *.o
distclean:	
	rm -f *~ *.o tpool/*~ tpool/*.o cplib/*~ cplib/*.o $(TARGET)
test:
	strip $(TARGET)
	calipsoctl kill
	cp $(TARGET) /usr/sbin/
	calipsoctl start
dump:
	objdump -d -l $(TARGET)
lines:
	 wc -l `find . -type f -name '*.c'`

install:
	/bin/cp calipso $(DESTDIR)$(BINDIR)

#uninstall:
#	/bin/rm -f $(BINDIR)/calipso
#	/bin/rm -f $(BINDIR)/calipsoctl
#	/bin/rm -r /etc/calipso

calipso:${OBJS}

	${CC} ${LDFLAGS} -o $(TARGET) ${OBJS} ${LIBS}

