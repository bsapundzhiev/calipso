dnl
dnl $Id: config.m4 152 2013-02-09 17:00:33Z borislav $
dnl 

PHP_ARG_WITH(calipso, for Calipso SAPI support,
[  --with-calipso=SRCDIR    Build PHP as calipso SAPI module(EXPERIMENTAL)], no, no)

if test "$PHP_CALIPSO" != "no"; then
  if test ! -d $PHP_CALIPSO ; then
    AC_MSG_ERROR([You did not specify a directory])
  fi
#  PHP_BUILD_THREAD_SAFE
  CALIPSO_LIB_DIR="/usr/lib/calipso/php5"
  PHP_ADD_INCLUDE($PHP_CALIPSO/include)
  AC_DEFINE(HAVE_CALIPSO,1,[Whether you have calipso])
  PHP_SELECT_SAPI(calipso, shared, php_calipso.c mod_php5.c)
  mkdir -p $CALIPSO_LIB_DIR
  INSTALL_IT="\$(INSTALL) -m 0755 $SAPI_SHARED \$(INSTALL_ROOT)$CALIPSO_LIB_DIR"
fi

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
