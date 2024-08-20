# A pure C library to read and write SuperCollider synthdefs, using a Lua-based DSL.

Currently in the early proof-of-concept stage, this C library will eventually allow clients from any language that can import C libraries 
(e.g. Python, Ruby, Swift, etc.) to be able to read and write SuperCollider synthdefs, bypassing the SuperCollider language and environment.

Clients can use the `UGen` and `SynthDef` struct to author synthdefs in a "close-to-the-metal" style, or use the included Lua interpreter and functions 
as a DSL, for example:

```
controls = { Control("out", 0), Control("freq", 440), Control("amp", 0.125) }
osc1 = SinOsc(controls[2]) * controls[3]
return Out(controls[1], osc1, osc1)
```

Calling `parse_lua_synthdef` with this code turns it into a `SynthDef` struct, ready for conversion to data and transmission via OSC to `scsynth`.

The many things remaining to be done include:
- Multichannel expansion
- Reassignment of node values
- Error handling
- Defining the full suite of unit generators
- Optimization of SynthDef graphs (currently collapse equivalent nodes, should also remove unused nodes)

Code review and language design input from interested and knowledgable parties is very welcome.

