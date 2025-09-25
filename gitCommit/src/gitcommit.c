// gitcommit.c
#define LIB_NAME "gitCommit"
#define MODULE_NAME "gitCommit"

#include <dmsdk/sdk.h>
#include <dmsdk/dlib/log.h>
#include <assert.h>

static char s_commit_hash[11] = {0};
static const char *COMMIT_FILE_PATH = "commitinfo";

static void RetrieveGitCommit()
{
    char cmd[256];
#if defined(_WIN32) || defined(_WIN64)
    snprintf(cmd, sizeof(cmd),
             "git rev-parse --short=10 HEAD > \"%s\" 2>nul",
             COMMIT_FILE_PATH);
#else
    snprintf(cmd, sizeof(cmd),
             "git rev-parse --short=10 HEAD > \"%s\" 2>/dev/null",
             COMMIT_FILE_PATH);
#endif

    system(cmd);

    FILE *file = fopen(COMMIT_FILE_PATH, "r");
    if (!file)
        return;

    if (fgets(s_commit_hash, sizeof(s_commit_hash), file))
    {
        char *nl = strchr(s_commit_hash, '\n');
        if (nl)
            *nl = '\0';
    }
    fclose(file);
}

static int LuaGetCommit(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushstring(L, s_commit_hash);
    return 1;
}

// upvalue1 = original sys.get_config
// upvalue2 = revision string
static int LuaGetConfigOverride(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 1);
    const char* key = luaL_optstring(L, 1, NULL);

    if (key && strcmp(key, "project.revision") == 0) {
        lua_pushvalue(L, lua_upvalueindex(2)); 
        return 1;
    }

    lua_pushvalue(L, lua_upvalueindex(1));           
    if (lua_gettop(L) >= 1) lua_pushvalue(L, 1); else lua_pushnil(L); 
    if (lua_gettop(L) >= 2) lua_pushvalue(L, 2); else lua_pushnil(L); 
    lua_call(L, 2, 1);
    return 1;
}

static int LuaApplyRevisionOverride(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    if (s_commit_hash[0] == '\0')
    {
        return 0;
    }

    lua_getglobal(L, "sys"); 
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return 0;
    }

    lua_getfield(L, -1, "get_config");
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return 0;
    }

    lua_pushvalue(L, -1); 
    lua_remove(L, -2);    

    lua_pushstring(L, s_commit_hash);             
    lua_pushcclosure(L, LuaGetConfigOverride, 2);

    lua_setfield(L, -2, "get_config");
    lua_pop(L, 1);

    return 0;
}

static const luaL_Reg Module_methods[] = {
    {"get_commit", LuaGetCommit},
    {"apply_revision_override", LuaApplyRevisionOverride},
    {0, 0}};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

#if LUA_VERSION_NUM >= 502
    lua_newtable(L);                    
    luaL_setfuncs(L, Module_methods, 0);
    lua_setglobal(L, MODULE_NAME);
#else
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
#endif

    if (lua_gettop(L) != top)
    {
        dmLogError("gitCommit: LuaInit stack mismatch (%d -> %d)",
                   top, lua_gettop(L));
        lua_settop(L, top);
    }
}

dmExtension::Result AppInitializeGitCommit(dmExtension::AppParams *)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeGitCommit(dmExtension::Params* params) {
    RetrieveGitCommit();
    if (!params || !params->m_L) {
        dmLogError("gitCommit: no Lua state");
        return dmExtension::RESULT_OK; 
    }
    LuaInit(params->m_L);
    dmLogInfo("gitCommit: initialized, hash='%s'", s_commit_hash);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeGitCommit(dmExtension::AppParams *)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeGitCommit(dmExtension::Params *)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(gitCommit, LIB_NAME,
                     AppInitializeGitCommit, AppFinalizeGitCommit,
                     InitializeGitCommit, 0, 0, FinalizeGitCommit)
