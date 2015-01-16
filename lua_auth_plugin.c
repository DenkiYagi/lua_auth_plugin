#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <mosquitto_plugin.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define unused  __attribute__((unused))

int topic_matches_sub(lua_State *lua)
{
    const char *sub = luaL_checkstring(lua, -2);
    const char *topic = luaL_checkstring(lua, -1);

    bool result;
    mosquitto_topic_matches_sub(sub, topic, &result);
    lua_pushboolean(lua, result);
    return 1;
}

void push_auth_opts(lua_State *lua, struct mosquitto_auth_opt *auth_opts, int auth_opt_count)
{
    lua_newtable(lua);

    for (int i = 0; i < auth_opt_count; i++) {
        lua_pushstring(lua, auth_opts[i].value);
        lua_setfield(lua, -2, auth_opts[i].key);
    }
}

int mosquitto_auth_plugin_version(void)
{
    return MOSQ_AUTH_PLUGIN_VERSION;
}

int mosquitto_auth_plugin_init(void **user_data, struct mosquitto_auth_opt *auth_opts, int auth_opt_count)
{
    char *module_name;

    for (int i = 0; i < auth_opt_count; i++) {
        if (!strcmp(auth_opts[i].key, "auth_file")) {
            module_name = strdup(auth_opts[i].value);
        }
    }

    if (module_name == NULL) {
        printf("auth_opt_auth_file config param missing\n");
        exit(1);
    }

    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);
    if( luaL_loadfile(lua, module_name) || lua_pcall(lua, 0, 0, 0) ) {
        printf("cant open\n");
        printf("error : %s\n", lua_tostring(lua, -1) );
        return MOSQ_ERR_UNKNOWN;
    }

    lua_register(lua, "topic_matches_sub", topic_matches_sub);
    lua_pushinteger(lua, MOSQ_ACL_NONE);
    lua_setglobal(lua, "MOSQ_ACL_NONE");
    lua_pushinteger(lua, MOSQ_ACL_READ);
    lua_setglobal(lua, "MOSQ_ACL_READ");
    lua_pushinteger(lua, MOSQ_ACL_WRITE);
    lua_setglobal(lua, "MOSQ_ACL_WRITE");

    lua_getglobal(lua, "plugin_init");
    push_auth_opts(lua, auth_opts, auth_opt_count);
    if(lua_pcall(lua, 1, 0, 0) != 0) {
        printf("plugin_init call failed\n");
        printf("error : %s\n", lua_tostring(lua, -1) );
        lua_pop(lua,1);
        return MOSQ_ERR_UNKNOWN;
    }

    *user_data = lua;

    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_plugin_cleanup(void *user_data, struct mosquitto_auth_opt *auth_opts unused, int auth_opt_count unused)
{
    lua_State *lua = (lua_State *)user_data;
    lua_getglobal(lua, "plugin_cleanup");
    push_auth_opts(lua, auth_opts, auth_opt_count);
    if(lua_pcall(lua, 1, 0, 0) != 0) {
        printf("plugin_cleanup call failed\n");
        printf("error : %s\n", lua_tostring(lua, -1) );
        lua_pop(lua,1);
        return MOSQ_ERR_UNKNOWN;
    }

    lua_close(lua);

    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_security_init(void *user_data, struct mosquitto_auth_opt *auth_opts unused, int auth_opt_count unused, bool reload)
{
    lua_State *lua = (lua_State *)user_data;
    lua_getglobal(lua, "security_init");
    push_auth_opts(lua, auth_opts, auth_opt_count);
    lua_pushboolean(lua, reload);
    if(lua_pcall(lua, 2, 0, 0) != 0) {
        printf("security_init call failed\n");
        printf("error : %s\n", lua_tostring(lua, -1) );
        lua_pop(lua,1);
        return MOSQ_ERR_UNKNOWN;
    }

    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_security_cleanup(void *user_data, struct mosquitto_auth_opt *auth_opts unused, int auth_opt_count unused, bool reload)
{
    lua_State *lua = (lua_State *)user_data;
    lua_getglobal(lua, "security_cleanup");
    push_auth_opts(lua, auth_opts, auth_opt_count);
    lua_pushboolean(lua, reload);
    if(lua_pcall(lua, 2, 0, 0) != 0) {
        printf("security_cleanup call failed\n");
        printf("error : %s\n", lua_tostring(lua, -1) );
        lua_pop(lua,1);
        return MOSQ_ERR_UNKNOWN;
    }

    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_acl_check(void *user_data, const char *clientid, const char *username, const char *topic, int access)
{
    lua_State *lua = (lua_State *)user_data;
    lua_getglobal(lua, "acl_check");
    lua_pushstring(lua, clientid);
    lua_pushstring(lua, username);
    lua_pushstring(lua, topic);
    lua_pushinteger(lua, access);
    if(lua_pcall(lua, 4, 1, 0) != 0) {
        printf("acl_check call failed\n");
        printf("error : %s\n", lua_tostring(lua, -1) );
        lua_pop(lua,1);
        return MOSQ_ERR_UNKNOWN;
    }

    int result = lua_toboolean(lua, -1);
    lua_pop(lua,1);
    return result ? MOSQ_ERR_SUCCESS : MOSQ_ERR_ACL_DENIED;
}

int mosquitto_auth_unpwd_check(void *user_data, const char *username, const char *password)
{
    lua_State *lua = (lua_State *)user_data;
    lua_getglobal(lua, "unpwd_check");
    lua_pushstring(lua, username);
    lua_pushstring(lua, password);
    if(lua_pcall(lua, 2, 1, 0) != 0) {
        printf("unpwd_check call failed\n");
        printf("error : %s\n", lua_tostring(lua, -1) );
        lua_pop(lua,1);
        return MOSQ_ERR_UNKNOWN;
    }

    int result = lua_toboolean(lua, -1);
    lua_pop(lua,1);
    return result ? MOSQ_ERR_SUCCESS : MOSQ_ERR_ACL_DENIED;
}

int mosquitto_auth_psk_key_get(void *user_data unused, const char *hint unused, const char *identity unused, char *key unused, int max_key_len unused)
{
    return MOSQ_ERR_AUTH;
}
