//
//  ParseLua.h
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/14/24.
//

#include <stdbool.h>
#include "NodeStack.h"
#include "SuperColliderSynthDef.h"

#ifndef ParseLua_h
#define ParseLua_h

void error(lua_State *L, const char *fmt, ...);
void parse_lua_synthdef(const char *luacode, const char *resultKey, SynthDef *def);

#endif /* ParseLua_h */
