/*
 * Copyright (C) agile6v
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include "pupa_php.h"

zend_function_entry pupa_functions[] = {
    ZEND_FE(pupa_init,  NULL)
    ZEND_FE(pupa_fini,  NULL)
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

#ifdef COMPILE_DL_PHP_PUPA
ZEND_GET_MODULE(pupa)
#endif

PHP_MINIT_FUNCTION(pupa)
{
    REGISTER_LONG_CONSTANT("PUPA_RO", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("PUPA_RW", 2, CONST_CS | CONST_PERSISTENT);

    return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(pupa)
{
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
    int         ret;
    strsize_t   key_len;
    long        key_count, op_type;
    char       *fileName;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll",
                &fileName, &key_len, &key_count, &op_type) == FAILURE)
    {
        RETURN_FALSE;
    }

    ret = pupa_init(fileName, (int) key_count, (int) op_type);
    if (ret != PUPA_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


ZEND_FUNCTION(pupa_fini)
{
    int ret;

    ret = pupa_fini();
    if (ret != PUPA_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


ZEND_FUNCTION(pupa_get)
{
    int         ret;
    strsize_t   key_len;
    char       *p_key;
    pupa_str_t  key, value;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                &p_key, &key_len) == FAILURE)
    {
        RETURN_NULL();
    }

    key.data = p_key;
    key.len = key_len;

    ret = pupa_get(&key, &value);
    if (ret != PUPA_OK) {
        RETURN_NULL();
    }

    _RETURN_STRING(value.data);
}


ZEND_FUNCTION(pupa_set)
{
    int         ret;
    char       *p_key, *p_val;
    strsize_t   key_len, value_len;
    pupa_str_t  key, value;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                &p_key, &key_len, &p_val, &value_len) == FAILURE)
    {
        RETURN_NULL();
    }

    key.data = p_key;
    key.len = key_len;

    value.data = p_val;
    value.len = value_len;

    ret = pupa_set(&key, &value);
    if (ret != PUPA_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


ZEND_FUNCTION(pupa_del)
{
    int         ret;
    strsize_t   key_len;
    char       *p_key;
    pupa_str_t  key;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                &p_key, &key_len) == FAILURE)
    {
        RETURN_NULL();
    }

    key.data = p_key;
    key.len = key_len;

    ret = pupa_del(&key);
    if (ret != PUPA_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


ZEND_FUNCTION(pupa_stats)
{
    int         ret;
    pupa_str_t  stat;

    ret = pupa_stats(&stat);
    if (ret != PUPA_OK) {
        RETURN_NULL();
    }

    _RETURN_STRING(stat.data);
}