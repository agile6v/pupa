/*
 * Copyright (C) agile6v
 */



#include "php.h"
#include "php_ini.h"

zend_function_entry pupa_functions[] = {
    ZEND_FE(pupa_init,  NULL)
    ZEND_FE(pupa_get,   NULL)
    ZEND_FE(pupa_set,   NULL)
    ZEND_FE(pupa_del,   NULL)
    ZEND_FE(pupa_stats, NULL)
    {NULL, NULL, NULL}
};


zend_module_entry pupa_module_entry = {
    STANDARD_MODULE_HEADER,
    PUPA_MODULE_NAME,
    pupa_functions,
    PHP_MINIT(pupa),
    PHP_MSHUTDOWN(pupa),
    NULL,
    NULL,
    PHP_MINFO(pupa),
    PUPA_MODULE_VERSION,
    STANDARD_MODULE_PROPERTIES
};


PHP_MINIT_FUNCTION(pupa)
{
    return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(pupa)
{
    pupa_fini();

    return SUCCESS;
}


PHP_MINFO_FUNCTION(pupa)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "pupa support", "enabled");
    php_info_print_table_end();
}


ZEND_FUNCTION(pupa_init)
{

}

ZEND_FUNCTION(pupa_get)
{

}

ZEND_FUNCTION(pupa_set)
{

}

ZEND_FUNCTION(pupa_del)
{

}

ZEND_FUNCTION(pupa_stats)
{

}