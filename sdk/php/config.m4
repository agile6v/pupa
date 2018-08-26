dnl this file for pupa extension

PHP_ARG_WITH(pupa, for pupa support,
[  --with-pupa             Include pupa support.])


if test "$PHP_PUPA" = "yes"; then
    PHP_SUBST(PHP_PUPA_SHARED_LIBADD)

    PHP_ADD_INCLUDE(/usr/local/include/pupa)

    PHP_ADD_LIBRARY_WITH_PATH(pupa, /usr/lib64, PHP_PUPA_SHARED_LIBADD)

    PHP_NEW_EXTENSION(php_pupa, pupa_php.c, $ext_shared)
fi



