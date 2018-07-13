/*
 * Copyright (C) agile6v
 */

#ifdef __cplusplus
#include "lua.hpp"
#else
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#endif

#include "pupa.h"
#include "pupa_config.h"

#ifdef __cplusplus
extern "C"{
#endif

static int init(lua_State *L)
{
    char *filename;
    int   ret, key_count, type;

    filename = (char *) luaL_checkstring(L, 1);
    key_count = luaL_checknumber(L, 2);
    type = luaL_checknumber(L, 3);

    ret = pupa_init(filename, key_count, type);
    if (ret != PUPA_OK) {
        return luaL_error(L, "Failed to initialize pupa.");
    }

    lua_pushnil(L);

    return 1;
}

static int fini(lua_State *L)
{
    int ret;

    ret = pupa_fini();
    if (ret != PUPA_OK) {
        return luaL_error(L, "Failed to end the pupa.");
    }

    lua_pushnil(L);
    return 1;
}

static int get(lua_State *L)
{
    int         ret;
    pupa_str_t  key, value;

    key.data = (char *) luaL_checkstring(L, 1);
    key.len = strlen(key.data);

    ret = pupa_get(&key, &value);
    if (ret != PUPA_OK) {
        return luaL_error(L, "Failed to get %s.", key.data);
    }

    lua_pushstring(L, value.data);
    lua_pushnil(L);

    return 2;
}

static int set(lua_State *L)
{
    int         ret;
    pupa_str_t  key, value;

    key.data = (char *) luaL_checkstring(L, 1);
    key.len = strlen(key.data);

    value.data = (char *) luaL_checkstring(L, 2);
    value.len = strlen(value.data);

    ret = pupa_set(&key, &value);
    if (ret != PUPA_OK) {
        return luaL_error(L, "Failed to perform pupa_set.");
    }

    lua_pushnil(L);

    return 1;
}

static int delete(lua_State *L)
{
    int         ret;
    pupa_str_t  key;

    key.data = (char *) luaL_checkstring(L, 1);
    key.len = strlen(key.data);

    ret = pupa_del(&key);
    if (ret != PUPA_OK) {
        return luaL_error(L, "Failed to delete %s.", key.data);
    }

    lua_pushnil(L);

    return 1;
}

static int stats(lua_State *L)
{
    int         ret;
    pupa_str_t  stat;

    ret = pupa_stats(&stat);
    if (ret != PUPA_OK) {
        return luaL_error(L, "Failed to perform pupa_stats.");
    }

    lua_pushstring(L, stat.data);
    lua_pushnil(L);

    return 2;
}

static const struct luaL_Reg pupa_lua[] = {
    {"get",     get},
    {"set",     set},
    {"delete",  delete},
    {"stats",   stats},
    {"init",    init},
    {"fini",    fini},
    {NULL,      NULL}
};


int luaopen_pupa_lua(lua_State *L) {
    luaL_newlib(L, pupa_lua);
    return 1;
}

#ifdef __cplusplus
}
#endif