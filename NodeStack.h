//
//  NodeStack.h
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/16/24.
//

#include "SynthDefLua.h"
#include "lua.h"

#ifndef NodeStack_h
#define NodeStack_h

#define ID_SOURCE_KEY "SCSDL_IDSource"
#define NODE_STACK_KEY "SCSDL_NodeStack"

typedef struct NodeSpecStackNode {
    SynthNodeSpec *spec;
    struct NodeSpecStackNode* next;
} NodeSpecStackNode;

typedef struct NodeSpecStack {
    NodeSpecStackNode *temp;
    NodeSpecStackNode *stack;
} NodeSpecStack;

typedef struct UniqueIDSource {
    lua_Integer currentID;
} UniqueIDSource;

NodeSpecStack *newNodeSpecStack(void);
void freeNodeSpecStack(NodeSpecStack *stack);

void pushNodeSpec(NodeSpecStack *stack, SynthNodeSpec *spec);
SynthNodeSpec *popNodeSpec(NodeSpecStack *stack);
bool stackEmpty(NodeSpecStack *stack);

UniqueIDSource *newUniqueIDSource(void);
void freeUniqueIDSource(UniqueIDSource *idSource);

lua_Integer nextUniqueID(UniqueIDSource *idSource);

void parseSynthDefStack(lua_State *L, const char *resultKey, NodeSpecStack *stack, SynthDef *def);

#include <stdio.h>

#endif /* Stack_h */
