/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Borislav Sapundzhiev  <bsapundjiev@gmail.com>                |
  +----------------------------------------------------------------------+
*/

/* $Id: php_calipso.h 26 2012-04-12 23:23:05Z borislav $ */

#ifndef PHP_CALIPSO_H
#define PHP_CALIPSO_H

#include "php.h"

/***************************************************************
 * MODULE
 ***************************************************************/
PHPAPI int	pm_init();
PHPAPI void	pm_exit();

/* execute server context */
int php_handler(calipso_request_t *request);


#endif /*PHP_CALIPSO_H*/
