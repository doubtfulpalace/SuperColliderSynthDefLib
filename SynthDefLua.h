//
//  UGens.h
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/14/24.
//

#ifndef UGens_h
#define UGens_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "lua.h"
#include "lauxlib.h"
#include "SuperColliderSynthDef.h"

#define RATE_KEY "rate"
#define CONTROL_NAME_KEY "name"
#define CONTROL_VALUE_KEY "value"

// TODO
//typedef enum Rate {
//    scalar = 0,
//    control = 1,
//    audio = 2,
//    demand = 3
//} Rate;

typedef struct SynthNodeIOSpec {
} SynthNodeIOSpec;

typedef struct SynthNodeSpec {
    lua_Integer id;
    bool isConstant;
    lua_Number numberValue;
    const char *name;
    lua_Integer rate;
    bool isControl;
    const char *controlName;
    SynthNodeIOSpec ioSpec;
    lua_Integer numInputs;
    lua_Integer *inputIDs;
    lua_Integer numOutputs;
    const char **inputNames;
    lua_Number *defaultInputValues;
    lua_Integer *outputRates;
    lua_Integer specialIndex;
} SynthNodeSpec;

uint64_t nodeHash(SynthNodeSpec spec);

void freeSynthNodeSpec(SynthNodeSpec *spec);
int newNode(lua_State *L, const char *name, lua_Integer rate, bool isControl, lua_Integer numInputs,
            const char *inputNames[], lua_Number defaultInputValues[], lua_Integer numOutputs,
            lua_Integer outputRates[], lua_Integer specialIndex);

#endif /* UGens_h */
