//
//  ParseLua.c
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/14/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "ParseLua.h"
#include "SynthDefLua.h"
#include "NodeStack.h"
#include "NodeDefs.h"

void error(lua_State *L, const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, "%s", argp);
    va_end(argp);
    lua_close(L);
    exit(EXIT_FAILURE);
}

void parse_lua_synthdef(const char *luacode, const char* defName, SynthDef *def) {
    char* prefix = "resultFunc = function() ";
    char* infix = " end; ";
    char* suffix = " = resultFunc()";
    char wrappedCode
        [strlen(prefix) + strlen(luacode) + strlen(infix) + strlen(defName) + strlen(suffix) + 1
    ];
    sprintf(wrappedCode, "%s%s%s%s%s", prefix, luacode, infix, defName, suffix);
    
    NodeSpecStack *stack = newNodeSpecStack();
    UniqueIDSource *idSource = newUniqueIDSource();

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_registerNodeDefs(L);

    lua_pushstring(L, NODE_STACK_KEY);
    lua_pushlightuserdata(L, stack);
    lua_settable(L, LUA_REGISTRYINDEX);
    
    lua_pushstring(L, ID_SOURCE_KEY);
    lua_pushlightuserdata(L, idSource);
    lua_settable(L, LUA_REGISTRYINDEX);
    
    if (luaL_loadstring(L, wrappedCode) || lua_pcall(L, 0, 0, 0)) {
        error(L, "Lua error: %s", lua_tostring(L, -1));
    }
    
    parseSynthDefStack(L, defName, stack, def);
    lua_close(L);
    freeUniqueIDSource(idSource);
    freeNodeSpecStack(stack);
}
