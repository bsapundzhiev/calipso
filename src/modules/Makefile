# $Id: Makefile 153 2013-03-17 21:36:34Z borislav $
.EXPORT_ALL_VARIABLES:

all:
	@make -C mod_proxy
	@make -C mod_http
	@make -C mod_dummy
	@make -C mod_autoind
ifdef USE_SSL
	@make -C mod_ssl
endif
	@make -C mod_rewrite

clean:
	@make -C mod_proxy clean
	@make -C mod_http clean
	@make -C mod_dummy clean
	@make -C mod_autoind clean
ifdef USE_SSL
	@make -C mod_ssl clean
endif
	@make -C mod_rewrite clean

install:
