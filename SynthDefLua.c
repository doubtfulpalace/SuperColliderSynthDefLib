//
//  UGens.c
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/14/24.
//

#include <string.h>
#include "SynthDefLua.h"
#include "lua.h"
#include "lauxlib.h"
#include "math.h"
#include "Diagnostics.h"
#include "NodeStack.h"
#include "SuperColliderSynthDef.h"
#include "NodeDefs.h"

#define NUM_COMMON_NODE_ARGUMENTS 1
#define DUMP_STACK_LABEL "too many DJs"

UniqueIDSource *getIDSource(lua_State *L) {
    lua_pushstring(L, ID_SOURCE_KEY);
    lua_gettable(L, LUA_REGISTRYINDEX);
    UniqueIDSource *idSource = (UniqueIDSource*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return idSource;
}

NodeSpecStack *getNodeStack(lua_State *L) {
    lua_pushstring(L, NODE_STACK_KEY);
    lua_gettable(L, LUA_REGISTRYINDEX);
    NodeSpecStack *stack = (NodeSpecStack*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return stack;
}

SynthNodeSpec *newSynthNodeSpec(void) {
    return (SynthNodeSpec *)malloc(sizeof(SynthNodeSpec));
}

NodeHash nodeHash(SynthNodeSpec spec) {
    NodeHash hash = { 0 };
    for (int i = 0; i < 256; i++) {
        hash.hash[i] = 0;
    }
    int counter = 0;
    counter += sprintf(&hash.hash[counter], "%s", spec.name);
    counter += sprintf(&hash.hash[counter], "%d", spec.rate);
	if (spec.isConstant) {
        counter += sprintf(&hash.hash[counter], "Constant");
		union {
			lua_Number floatValue;
			lua_Integer intValue;
		} converter;
		converter.floatValue = spec.numberValue;
		counter += sprintf(&hash.hash[counter], "%x", converter.intValue);
 	}
	else if (spec.isControl) {
        counter += sprintf(&hash.hash[counter], "%s", spec.controlName);
	} else {
        for (int i = 0; i < spec.numInputs; i++) {
            counter += sprintf(&hash.hash[counter], " %d", spec.inputIDs[i]);
        }
	}
	return hash;
}

void freeSynthNodeSpec(SynthNodeSpec *spec) {
    free(spec->inputNames);
    free(spec->inputIDs);
    free(spec->outputRates);
    free(spec);
}

// ---- INTERNAL INIT FUNCTIONS
static void newConstantNode(lua_State *L, SynthNodeSpec *node, lua_Number value) {
    UniqueIDSource *idSource = getIDSource(L);
    lua_Integer id = nextUniqueID(idSource);
    node->id = id;
    node->isConstant = true;
    node->numberValue = value;
    node->isControl = false;
    node->controlName = "\0";
    node->name = "\0";
    node->rate = 0;
    node->numInputs = 0;
    node->numOutputs = 0;
    node->specialIndex = 0;
//    nodes[id] = node;
    NodeSpecStack *stack = getNodeStack(L);
    NodeHash hash = nodeHash(*node);
    lua_pushstring(L, hash.hash);
    lua_gettable(L, LUA_REGISTRYINDEX);
    if (lua_isnil(L, -1)) {
        lua_pushlightuserdata(L, node);
        lua_pushstring(L, nodeHash(*node).hash);
        lua_settable(L, LUA_REGISTRYINDEX);
        pushNodeSpec(stack, node);
    }
    lua_pop(L, 1);
}

static void registerNode(lua_State *L, SynthNodeSpec *node) {
    lua_pushstring(L, nodeHash(*node).hash);
    lua_pushlightuserdata(L, node);
    lua_settable(L, LUA_REGISTRYINDEX);
    NodeSpecStack *stack = getNodeStack(L);
    pushNodeSpec(stack, node);
}

static void getEquivalentNode(lua_State *L, SynthNodeSpec *node) {
    // will leave either equivalent node or nil on the stack
    NodeHash hash = nodeHash(*node);
    lua_pushstring(L, hash.hash);
    lua_gettable(L, LUA_REGISTRYINDEX);
}

static void constantNode(lua_State *L, lua_Integer *inputID, lua_Number value) {
    UniqueIDSource *idSource = getIDSource(L);
    lua_Integer id = nextUniqueID(idSource);
    SynthNodeSpec *node = newSynthNodeSpec();
    node->id = id;
    node->isConstant = true;
    node->numberValue = value;
    node->isControl = false;
    node->controlName = "\0";
    node->name = "\0";
    node->rate = 0;
    node->numInputs = 0;
    node->numOutputs = 0;
    node->specialIndex = 0;
//    nodes[id] = node;
    getEquivalentNode(L, node);
    if (lua_isnil(L, -1)) {
        registerNode(L, node);
        *inputID = node->id;
    } else {
        SynthNodeSpec *existingNode = lua_touserdata(L, -1);
        *inputID = existingNode->id;
        freeSynthNodeSpec(node);
    }
    lua_pop(L, 1);
}

static void newProtoNode(lua_State *L, SynthNodeSpec *node, const char *name, lua_Integer rate,
                         bool isControl, lua_Integer numInputs, const char **inputNames,
                         lua_Integer numOutputs, lua_Integer *outputRates, lua_Integer specialIndex)
{
    UniqueIDSource *idSource = getIDSource(L);
    lua_Integer id = nextUniqueID(idSource);
    node->id = id;
    node->name = name;
    node->rate = rate;
    node->isControl = isControl;
    node->numInputs = numInputs;
    node->inputIDs = (lua_Integer*)malloc(sizeof(lua_Integer) * numInputs);
    node->inputNames = (const char **)malloc(sizeof(const char*) * numInputs);
    for (int i = 0; i < node->numInputs; i++) {
        node->inputNames[i] = inputNames[i];
    }
    node->numOutputs = numOutputs;
    node->outputRates = (lua_Integer*)malloc(sizeof(lua_Integer) * numOutputs);
    for (int i = 0; i < node->numOutputs; i++) {
        node->outputRates[i] = outputRates[i];
    }
    node->specialIndex = specialIndex;
}

static void normalizeNodeInput(lua_State *L, lua_Integer *inputID, int index, lua_Number defaultValue) {
    if (lua_isnil(L, index)) {
        constantNode(L, inputID, defaultValue);
    } else if (lua_isuserdata(L, index)) {
        SynthNodeSpec *node = (SynthNodeSpec*)lua_touserdata(L, index);
        *inputID = node->id;
    } else {
        constantNode(L, inputID, lua_tonumber(L, index));
    }
}

int newNode(lua_State *L, const char *name, lua_Integer rate, bool isControl, lua_Integer numInputs, const char **inputNames, lua_Number *defaultInputValues, lua_Integer numOutputs, lua_Integer *outputRates, lua_Integer specialIndex) {
    int numArgs = lua_gettop(L);
    SynthNodeSpec *node = newSynthNodeSpec();
    newProtoNode(L, node, name, rate, isControl, numInputs, inputNames, numOutputs, outputRates, specialIndex);
    int numAdditionalArgs = node->isControl ? 2 : NUM_COMMON_NODE_ARGUMENTS;
    if (numArgs > node->numInputs + numAdditionalArgs) {
        dumpStack(L, "Too many arguments");
        return 0;
    }
    bool namedArguments = (numArgs == 1 && lua_istable(L, 1));
    if (node->isControl) {
        if (namedArguments) {
            lua_pushstring(L, CONTROL_NAME_KEY);
            lua_gettable(L, 1);
            if (!lua_isnil(L, -1)) {
                node->controlName = lua_tostring(L, -1);
            } else {
                node->controlName = "<control>";
            }
            lua_pop(L, 1);
            lua_pushstring(L, CONTROL_VALUE_KEY);
            lua_gettable(L, 1);
            node->numberValue = lua_tonumber(L, -1);
            lua_pop(L, 1);
        } else {
            node->controlName = lua_tostring(L, 1);
            node->numberValue = lua_tonumber(L, 2);
        }
    } else {
        if (namedArguments) {
            for (int i = 0; i < node->numInputs; i++) {
                lua_pushstring(L, inputNames[i]);
                lua_gettable(L, 1);
                normalizeNodeInput(L, &node->inputIDs[i], -1, defaultInputValues[i]);
            }
            lua_pushstring(L, RATE_KEY);
            lua_gettable(L, 1);
            if (!lua_isnil(L, -1) && !node->isControl) {
                node->rate = lua_tonumber(L, -1);
            }
            lua_pop(L, 1);
        } else {
            for (int i = 0; i < node->numInputs; i++) {
                normalizeNodeInput(L, &node->inputIDs[i], i+1, defaultInputValues[i]);
            }
            if (numArgs == node->numInputs + 1) {
                if (node->isControl) {
                    node->controlName = lua_tostring(L, numArgs);
                } else {
                    node->rate = lua_tonumber(L, numArgs);
                }
            }
        }
    }
    getEquivalentNode(L, node);
    if (lua_isnil(L, -1)) {
        registerNode(L, node);
        lua_pushlightuserdata(L, node);
    } else {
        // existing node already on stack
        freeSynthNodeSpec(node);
    }
    luaL_setmetatable(L, "SCNodeDef");
    return 1;
}
