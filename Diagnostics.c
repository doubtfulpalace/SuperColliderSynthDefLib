//
//  Diagnostics.c
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/16/24.
//

#include <stdio.h>
#include <lauxlib.h>
#include <lualib.h>

#include "Diagnostics.h"

void indent(FILE *file, int level) {
    for (int i = 0; i < level; i++) {
        fprintf(file, "  ");
    }
}

void dumpStack(lua_State *L, char *label) {
    printf("----%s\n", label);
    for (int i = 0; i < lua_gettop(L); i++) {
        int stackIndex = (i+1) * -1;
        printf("%d: %s (", stackIndex, lua_typename(L, lua_type(L, stackIndex)));
        if (lua_isstring(L, stackIndex)) {
            printf("%s", lua_tostring(L, stackIndex));
        } else {
            printf("%f", lua_tonumber(L, stackIndex));
        }
        printf(")\n");
    }
}

void dumpTable(lua_State *L, char *label) {
    printf("----%s\n", label);
    lua_pushnil(L);  /* first key */
    while (lua_next(L, -2) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */
    if (lua_isstring(L, -2)) {
        printf("Key: (%s, %s)", lua_typename(L, lua_type(L, -2)), lua_tostring(L, -2));
    } else {
        printf("Key: (%s, %f)", lua_typename(L, lua_type(L, -2)), lua_tonumber(L, -2));
    }
    printf("; ");
    if (lua_isstring(L, -1)) {
        printf("Value: (%s, %s)", lua_typename(L, lua_type(L, -1)), lua_tostring(L, -1));
    } else {
        printf("Value: (%s, %f)", lua_typename(L, lua_type(L, -1)), lua_tonumber(L, -1));
    }
    printf("\n");
    /* removes 'value'; keeps 'key' for next iteration */
    lua_pop(L, 1);
    }
}
 
void dumpUGen(FILE *file, const UGen ugen, int level) {
    indent(file, level);
    fprintf(file, "%s (rate %d) inputs: ", ugen.name, ugen.rate);
    for(int i = 0; i < ugen.numInputs; i++) {
        fprintf(file, "[%d,%d] ", ugen.inputs[i].index, ugen.inputs[i].channel);
    }
    fprintf(file, "outputs: [");
    for (int i = 0; i < ugen.numOutputs; i++) {
        fprintf(file, "%d ", ugen.outputRates[i]);
    }
    fprintf(file, "], specialIndex: %d", ugen.specialIndex);
    fprintf(file, "\n");
}

void dumpSynthDef(FILE *file, const SynthDef def) {
    fprintf(file, "Synth/def %s\n", def.name);
    fprintf(file, "  Constants: ");
    for (int i = 0; i < def.numConstants; i++) {
        fprintf(file, "%f ", def.constants[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "  ControlValues: ");
    for (int i = 0; i < def.numControls; i++) {
        fprintf(file, "%f ", def.controlValues[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "  Controls (%d): ", def.numControls);
    for (int i = 0; i < def.numControls; i++) {
        fprintf(file, "[%s %d] ", def.controls[i].name, def.controls[i].index);
    }
    fprintf(file, "\n");
    fprintf(file, "  UGens (%d):\n", def.numUGens);
    for (int i = 0; i < def.numUGens; i++) {
        fprintf(file, "    %d ", i + def.numControls);
        dumpUGen(file, def.ugens[i], 0);
    }
}

void dumpSynthNodeSpec(FILE *file, SynthNodeSpec node) {
    fprintf(file, "ID: %d ", node.id);
    if (node.isConstant) {
        fprintf(file, "%f\n", node.numberValue);
    } else if (node.isControl) {
        fprintf(file, "%s(name: %s, rate %d)\n", node.name, node.controlName, node.rate);
    } else {
        fprintf(file, "%s(rate: %d, output rates: [", node.name, node.rate);
        for (int i = 0; i < node.numOutputs; i++) {
            fprintf(file, "%d", node.outputRates[i]);
            if (i < node.numOutputs - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "], specialIndex: %d)\n", node.specialIndex);
        for (int i = 0; i < node.numInputs; i++) {
            fprintf(file, "%s: %d; ", node.inputNames[i], node.inputIDs[i]);
        }
        fprintf(file, "\n");
    }
}
