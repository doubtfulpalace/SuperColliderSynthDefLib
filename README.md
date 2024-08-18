# A pure C library to read and write SuperCollider synthdefs, using a Lua-based DSL.

Currently in the early proof-of-concept stage, this C library will eventually allow clients from any language that can import C libraries 
(e.g. Python, Ruby, Swift, etc.) to be able to read and write SuperCollider synthdefs, bypassing the SuperCollider language and environment.

Clients can use the `UGen` and `SynthDef` struct to author synthdefs in a "close-to-the-metal" style, or use the included Lua interpreter and functions 
as a DSL, for example:

```
outControl = Control("out", 1)
osc1 = SinOsc(440)
osc2 = SinOsc{freq=220, phase=0.5}
return Out(outControl, osc1, osc2)
```

Calling `parse_lua_synthdef` with this code turns it into a `SynthDef` struct, ready for conversion to data and transmission via OSC to `scsynth`.

The many things remaining to be done include:
- Multichannel expansion
- Special operators such as `*` and `+`, which will require entries in the Lua functions' metatable
- Reassignment of node values, which (I think) will require Lua uservalues to be added to the functions
- Defining the full suite of unit generators
- Optimization of SynthDef graphs (e.g. removing redundant constants, controls, and UGens

Code review and language design input from interested and knowledgable parties is very welcome.
