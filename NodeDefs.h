//
//  NodeDefs.h
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/16/24.
//

#ifndef NodeDefs_h
#define NodeDefs_h

#include "lua.h"

#define COMMA ,
#define NODEFUNC(FUNC_NAME, NAME, RATE, IS_CONTROL, NUM_INPUTS, INPUT_NAMES, DEFAULT_INPUT_VALUES, NUM_OUTPUTS, OUTPUT_RATES, SPECIAL_INDEX)\
static int FUNC_NAME(lua_State *L) {\
    const char *inputNames[] = INPUT_NAMES;\
    lua_Number defaultInputValues[] = DEFAULT_INPUT_VALUES;\
    lua_Integer outputRates[] = OUTPUT_RATES;\
    return newNode(L, NAME, RATE, IS_CONTROL, NUM_INPUTS, inputNames, defaultInputValues, NUM_OUTPUTS, outputRates, SPECIAL_INDEX);\
}\

typedef enum Rate {
	SCALAR_RATE = 0,
	CONTROL_RATE = 1,
	AUDIO_RATE = 2,
	DEMAND_RATE = 3
} Rate;

int luaL_registerNodeDefs(lua_State *L);

#include <stdio.h>

#endif /* NodeDefs_h */
