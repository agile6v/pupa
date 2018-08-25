/*
 * Copyright (C) agile6v
 */

#ifndef _PUPA_PHP_H
#define _PUPA_PHP_H

#include "pupa.h"
#include "pupa_config.h"

#define PUPA_MODULE_NAME        "pupa"
#define PUPA_MODULE_VERSION     "0.1"

PHP_MINIT_FUNCTION(pupa);
PHP_MSHUTDOWN_FUNCTION(pupa);
PHP_MINFO_FUNCTION(pupa);

ZEND_FUNCTION(pupa_init);
ZEND_FUNCTION(pupa_fini);
ZEND_FUNCTION(pupa_get);
ZEND_FUNCTION(pupa_set);
ZEND_FUNCTION(pupa_del);
ZEND_FUNCTION(pupa_stats);

extern zend_module_entry pupa_module_entry;

#endif //_PUPA_PHP_H
