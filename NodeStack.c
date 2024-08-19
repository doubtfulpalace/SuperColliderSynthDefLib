//
//  NodeStack.c
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/16/24.
//


#include <stdio.h>
#include <stdlib.h>
#include "NodeStack.h"
#include "Diagnostics.h"

NodeSpecStack *newNodeSpecStack(void) {
    return (NodeSpecStack*)malloc(sizeof(NodeSpecStack));
}

void freeNodeSpecStack(NodeSpecStack *stack) {
    while (!stackEmpty(stack)) {
        SynthNodeSpec *spec = popNodeSpec(stack);
        freeSynthNodeSpec(spec);
    }
    free(stack);
}

void pushNodeSpec(NodeSpecStack *stack, SynthNodeSpec *spec) {
    NodeSpecStackNode *node = (NodeSpecStackNode*)malloc(sizeof(NodeSpecStackNode));
    node->spec = spec;
    node->next = stack->stack;
    stack->stack = node;
}

SynthNodeSpec *popNodeSpec(NodeSpecStack *stack) {
    if (stack->stack == NULL) {
        return NULL;
    }
    SynthNodeSpec *spec = stack->stack->spec;
    stack->stack = stack->stack->next;
    return spec;
}

bool stackEmpty(NodeSpecStack *stack) {
    return stack->stack == NULL;
}

UniqueIDSource *newUniqueIDSource(void) {
    return (UniqueIDSource*)malloc(sizeof(UniqueIDSource));
}

void freeUniqueIDSource(UniqueIDSource *idSource) {
    free(idSource);
}

lua_Integer nextUniqueID(UniqueIDSource *idSource) {
    return idSource->currentID++;
}

void parseSynthDefStack(lua_State *L, const char *name, NodeSpecStack *stack, SynthDef *def) {
    NodeSpecStack *constantStack = newNodeSpecStack();
    NodeSpecStack *controlStack = newNodeSpecStack();
    NodeSpecStack *ugenStack = newNodeSpecStack();
    NodeSpecStack *stash = newNodeSpecStack();
    uint32_t numConstants = 0;
    uint32_t numControls = 0;
    uint32_t numUGens = 0;
    
    while (!stackEmpty(stack)) {
        SynthNodeSpec *spec = popNodeSpec(stack);
        if (spec->isConstant) {
            pushNodeSpec(constantStack, spec);
            numConstants++;
        } else if (spec->isControl) {
            pushNodeSpec(controlStack, spec);
            numControls++;
        } else {
            pushNodeSpec(ugenStack, spec);
            numUGens++;
        }
    }
    
    def->name = (char*)malloc(sizeof(name));
    strcpy(def->name, name);
    def->numConstants = numConstants;
    lua_Integer constantIDs[numConstants];
    def->constants = (float*)malloc(sizeof(float) * numConstants);
    for (int i = 0; i < numConstants; i++) {
        SynthNodeSpec *spec = popNodeSpec(constantStack);
        constantIDs[i] = spec->id;
        def->constants[i] = spec->numberValue;
        pushNodeSpec(stash, spec);
    }
    
    // TODO: find out when these differ
    def->numControlValues = numControls;
    def->numControls = numControls;
    def->controls = (ControlSpec*)malloc(sizeof(ControlSpec) * numControls);
    def->controlValues = (float*)malloc(sizeof(float) * numControls);
    lua_Integer controlIDs[numControls];
    for (int i = 0; i < numControls; i++) {
        SynthNodeSpec *spec = popNodeSpec(controlStack);
        controlIDs[i] = spec->id;
        def->controlValues[i] = spec->numberValue;
        def->controls[i].name = (char*)malloc(sizeof(spec->controlName));
        strcpy(def->controls[i].name, spec->controlName);
        def->controls[i].index = i;
        pushNodeSpec(stash, spec);
    }
    
    int totalUGens = numUGens + (numControls > 0 ? 1 : 0);
    def->numUGens = totalUGens;
    lua_Integer ugenIDs[numUGens];
    UGen *ugens = (UGen*)malloc(sizeof(UGen) * totalUGens);
    SynthNodeSpec *specs[numUGens];
    
    // make composite control
    ugens[0].name = "Control";
    ugens[0].numInputs = 0;
    ugens[0].numOutputs = numControls;
    ugens[0].outputRates = (uint32_t*)malloc(sizeof(uint32_t) * ugens[0].numOutputs);
    for (int i = 0; i < ugens[0].numOutputs; i++) {
        ugens[0].outputRates[i] = 1; // for now!
    }
    ugens[0].rate = 1;
    ugens[0].specialIndex = 0;
    
    // get everything but inputs, and populate the cross-references
    for (int i = 1; i < totalUGens; i++) {
        SynthNodeSpec *spec = popNodeSpec(ugenStack);
        ugenIDs[i-1] = spec->id;
        specs[i-1] = spec;
        ugens[i].name = (char*)malloc(sizeof(spec->name));
        strcpy(ugens[i].name, spec->name);
        ugens[i].numInputs = spec->numInputs;
        ugens[i].numOutputs = spec->numOutputs;
        ugens[i].outputRates = (uint32_t*)malloc(sizeof(uint32_t) * ugens[i].numOutputs);
        for (int j = 0; j < spec->numOutputs; j++) {
            ugens[i].outputRates[j] = spec->outputRates[j];
        }
        ugens[i].rate = spec->rate;
        ugens[i].specialIndex = spec->specialIndex;
        pushNodeSpec(stash, spec);
    }
    // now get the inputs
    for (int i = 1; i < totalUGens; i++) {
        uint32_t channel = 0; // for now!
        ugens[i].inputs = (WireSpec *)malloc(sizeof(WireSpec) * ugens[i].numInputs);
        for (int j = 0; j < ugens[i].numInputs; j++) {
            bool found = false;
            for (int k = 0; k < numConstants; k++) {
                if (specs[i-1]->inputIDs[j] == constantIDs[k]) {
                    ugens[i].inputs[j].index = -1;
                    ugens[i].inputs[j].channel = k;
                    found = true;
                    break;
                }
            }
            if (!found) {
                for (int k = 0; k < numControls; k++) {
                    if (specs[i-1]->inputIDs[j] == controlIDs[k]) {
                        ugens[i].inputs[j].index = 0;
                        ugens[i].inputs[j].channel = k;
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                for (int k = 0; k < numUGens; k++) {
                    if (specs[i-1]->inputIDs[j] == ugenIDs[k]) {
                        ugens[i].inputs[j].index = k + 1;
                        ugens[i].inputs[j].channel = channel;
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                // trouble in River City
            }
        }
    }
//    for (int i = 0; i < numUGens; i++) {
//        pushNodeSpec(stash, specs[i]);
//    }
    def->ugens = ugens;
    // may never support this in Lua
    def->numVariants = 0;
    freeNodeSpecStack(stash);
}
