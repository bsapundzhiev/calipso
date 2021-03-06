#*
#* Copyright (C) 2012 Borislav Sapundzhiev
#*
#* This file is released under the terms of GPL v2 and any later version.
#*
#*

ifndef OS
#debug
CFLAGS+= -DUSE_SSL
USE_SSL=1
USE_POLL=1
USE_EPOLL=1
#STRICT=1
OS=$(shell uname -s)
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
	server.o\
	client.o\
	reply.o\
	request.o\
	resource.o\
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
	cpo_log.o

ifdef USE_SSL
	OBJS+=cpo_io_ssl.o
endif

ifdef USE_POLL
	OBJS+=poll.o
endif
ifdef USE_EPOLL
	OBJS+=epoll.o
endif

ifdef USE_KQUEUE
	OBJS+=kqueue.o
endif

ifeq ($(OS), Darwin)
	CFLAGS += -I../os/macosx/
	OBJS+=../os/macosx/timer.o ../os/macosx/compat.o
else
	OBJS+=timer.o
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

ifdef STRICT
CFLAGS += -Werror -W -Wno-unused-parameter #-pedantic -fno-exceptions #-g -O0 -ansi 
endif
#
# if your system needs any additional libraries (solaris, for example, 
# needs the ones commented out below), edit this line.
EXTRA_LIBS = -lpthread -ldl



ifdef USE_SSL
EXTRA_LIBS += -lssl -lcrypto
endif

ifeq ($(OS), Linux)
	EXTRA_LIBS += -lrt
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

