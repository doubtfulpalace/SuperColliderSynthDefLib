//
//  NodeDefs.c
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/16/24.
//

#include "NodeDefs.h"
#include "SynthDefLua.h"

#define COMMA ,
#define NODEFUNC(FUNC_NAME, NAME, RATE, IS_CONTROL, NUM_INPUTS, INPUT_NAMES, DEFAULT_INPUT_VALUES, NUM_OUTPUTS, OUTPUT_RATES, SPECIAL_INDEX)\
static int FUNC_NAME(lua_State *L) {\
    const char *inputNames[] = INPUT_NAMES;\
    lua_Number defaultInputValues[] = DEFAULT_INPUT_VALUES;\
    lua_Integer outputRates[] = OUTPUT_RATES;\
    return newNode(L, NAME, RATE, IS_CONTROL, NUM_INPUTS, inputNames, defaultInputValues, NUM_OUTPUTS, outputRates, SPECIAL_INDEX);\
}\

NODEFUNC(SinOsc, "SinOsc", 2, false, 2, {"freq" COMMA "phase"}, {220 COMMA 0.0}, 1, {2}, 0);
NODEFUNC(Control, "Control", 1, true, 0, {}, {}, 1, {1}, 0);
NODEFUNC(Out, "Out", 2, false, 3, {"out" COMMA "left" COMMA "right"}, {0.6 COMMA 0.6 COMMA 0.6}, 0, {}, 0);
NODEFUNC(Mul, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 2);

static const struct luaL_Reg nodeSpecs[] = {
    {"SinOsc", SinOsc},
    {"Control", Control},
    {"Out", Out},
    {"Mul", Mul},
    {NULL, NULL}  /* sentinel */
};

int luaL_registerNodeDefs(lua_State *L) {
    int counter = 0;
    while (!(nodeSpecs[counter].func == NULL || nodeSpecs[counter].name == NULL)) {
        lua_register(L, nodeSpecs[counter].name, nodeSpecs[counter].func);
        counter++;
    }
    return 1;
}
