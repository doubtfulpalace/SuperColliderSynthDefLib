//
//  NodeDefs.c
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/16/24.
//

#include "NodeDefs.h"
#include "SynthDefLua.h"

// These need to be fleshed out
NODEFUNC(Control, "Control", CONTROL_RATE, true, 0, {}, {}, 1, {1}, 0);
NODEFUNC(Out, "Out", AUDIO_RATE, false, 3, {"out" COMMA "left" COMMA "right"}, {0.6 COMMA 0.6 COMMA 0.6}, 0, {}, 0);

NODEFUNC(Add, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 0);
NODEFUNC(Sub, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 1);
NODEFUNC(Mul, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 2);
NODEFUNC(Div, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 3);
NODEFUNC(Mod, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 4);
NODEFUNC(Pow, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 25);
NODEFUNC(BitwiseAnd, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 14);
NODEFUNC(BitwiseOr, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 15);
NODEFUNC(BitshiftLeft, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 26);
NODEFUNC(BitshiftRight, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 27);
NODEFUNC(UnsignedBitshiftRight, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 28);
// Lua maps > and >= to < and <= with swapped arguments, so we only define the latter
NODEFUNC(LessThan, "BinaryOpUGen", 2, false, AUDIO_RATE, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 8);
NODEFUNC(LessThanOrEqualTo, "BinaryOpUGen", AUDIO_RATE, false, 2, {"a" COMMA "b"}, {1 COMMA 1}, 1, {2}, 10);

static const struct luaL_Reg nodeSpecs[] = {
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

