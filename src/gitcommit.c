#define LIB_NAME    "GitCommit"
#define MODULE_NAME "gitcommit"

#include <dmsdk/sdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static char s_commit_hash[11] = {0};
static const char* COMMIT_FILE_PATH = "gitcommit/commitinfo";

static void RetrieveGitCommit() {
#if !defined(_WIN32) && !defined(_WIN64)
    system("mkdir -p gitcommit");
#endif
    char cmd[256];
#if defined(_WIN32) || defined(_WIN64)
    snprintf(cmd, sizeof(cmd),
        "git rev-parse HEAD > \"%s\" 2>nul",
        COMMIT_FILE_PATH);
#else
    snprintf(cmd, sizeof(cmd),
        "git rev-parse HEAD > \"%s\" 2>/dev/null",
        COMMIT_FILE_PATH);
#endif

    system(cmd);

    FILE* file = fopen(COMMIT_FILE_PATH, "r");
    if (!file) {
        return;
    }
    if (fgets(s_commit_hash, sizeof(s_commit_hash), file)) {
        char* nl = strchr(s_commit_hash, '\n');
        if (nl) *nl = '\0';
    }
    fclose(file);
}

static int LuaGetCommit(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushstring(L, s_commit_hash);
    return 1;
}

static const luaL_Reg Module_methods[] = {
    {"get_commit", LuaGetCommit},
    {0,0}
};

static void LuaInit(lua_State* L) {
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeGitCommit(dmExtension::AppParams*) {
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeGitCommit(dmExtension::Params* params) {
    RetrieveGitCommit();
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeGitCommit(dmExtension::AppParams*) {
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeGitCommit(dmExtension::Params*) {
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(GitCommit, LIB_NAME,
                     AppInitializeGitCommit, AppFinalizeGitCommit,
                     InitializeGitCommit, 0, 0, FinalizeGitCommit)
