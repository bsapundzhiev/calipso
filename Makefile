.EXPORT_ALL_VARIABLES:

#Build settings
#SSL usage 
USE_SSL=1
#USE_POLL=1
#default event model
#USE_EPOLL=1 
#PLATFORM=armv6l
#CC=gcc
-include Config

TOPDIR:=$(shell pwd)
ifndef CALIPSO_INC
CALIPSO_INC:=$(TOPDIR)/include
endif
ifndef CALIPSO_LIB
CALIPSO_INC:=$(TOPDIR)/libs
endif

CFLAGS+= -D_REENTRANT -D_GNU_SOURCE

ifdef USE_SSL
CFLAGS+= -DUSE_SSL
endif
###################################
# project related
###################################
ifndef VERSION
VERSION = $(shell cat ./VERSION)
CFLAGS+= -DVERSION="\"$(VERSION)\""
endif

ifndef PLATFORM
PLATFORM= $(shell uname -m)
endif

ifndef OS
OS = $(shell uname -s)
CFLAGS+= -DOS="\"$(OS)/$(PLATFORM)"\"
endif

all:
	@make -C src
	@make -C src/modules

test:
	@make -C test
	
lines:
	wc -l `find . -name *.c` | grep total
	wc -l `find . -name *.h` | grep total

clean:
	@make -C src clean
	@make -C src/modules clean

distclean:
#	@make -C src distclean
#	@make -C src/modules distclean
	find . -name \*~ -o -name \*.o | xargs rm -f 
	find . -name \*.so | xargs rm -f

install:
	@make -C src install
	/bin/cp ./doc/calipsoctl $(DESTDIR)/usr/bin/
	/bin/mkdir -p $(DESTDIR)/etc
	/bin/cp -f ./doc/calipso.conf $(DESTDIR)/etc/
	/bin/cp -f ./doc/mime.types $(DESTDIR)/etc/
