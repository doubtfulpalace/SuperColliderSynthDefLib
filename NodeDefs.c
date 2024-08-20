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
NODEFUNC(Add, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 0);
NODEFUNC(Sub, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 1);
NODEFUNC(Mul, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 2);
NODEFUNC(Div, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 3);
NODEFUNC(Mod, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 4);
NODEFUNC(Pow, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 25);
NODEFUNC(BitwiseAnd, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 14);
NODEFUNC(BitwiseOr, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 15);
NODEFUNC(BitshiftLeft, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 26);
NODEFUNC(BitshiftRight, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 27);
NODEFUNC(UnsignedBitshiftRight, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 28);
// Lua maps > and >= to < and <= with swapped arguments, so we only define the latter
NODEFUNC(LessThan, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 8);
NODEFUNC(LessThanOrEqualTo, "BinaryOpUGen", 2, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 10);

static const struct luaL_Reg nodeSpecs[] = {
    {"SinOsc", SinOsc},
    {"Control", Control},
    {"Out", Out},
    {"UnsignedBitshiftRight", UnsignedBitshiftRight},
    {NULL, NULL}  /* sentinel */
};

int registerSCNodeDefMetaTable(lua_State *L) {
	luaL_Reg reg[] = {
		{"__add", Add},
		{"__sub", Sub},
		{"__mul", Mul},
		{"__div", Div},
		{"__mod", Mod},
		{"__pow", Pow},
		{"__band", BitwiseAnd},
		{"__bor", BitwiseOr},
		{"__shl", BitshiftLeft},
		{"__shr", BitshiftRight},
		// +>> not available in Lua, callers must call UGen explicitly
		{"__lt", LessThan},
		{"__le", LessThanOrEqualTo},
		{NULL, NULL}
	};
	
	luaL_newmetatable(L, "SCNodeDef");
	luaL_setfuncs(L, reg, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	
	return 1;
}

int luaL_registerNodeDefs(lua_State *L) {
    int counter = 0;
    while (!(nodeSpecs[counter].func == NULL || nodeSpecs[counter].name == NULL)) {
        lua_register(L, nodeSpecs[counter].name, nodeSpecs[counter].func);
        counter++;
    }
    registerSCNodeDefMetaTable(L);
    return 1;
}

