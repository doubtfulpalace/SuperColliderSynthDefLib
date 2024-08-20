//
//  Diagnostics.h
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/16/24.
//

#ifndef Diagnostics_h
#define Diagnostics_h

#include "SynthDefLua.h"

void dumpStack(lua_State *L, char *label);
void dumpTable(lua_State *L, char *label);
void dumpSynthNodeSpec(FILE *file, SynthNodeSpec node);
void dumpSynthDef(FILE *file, const SynthDef def);

#include <stdio.h>

#endif /* Diagnostics_h */
