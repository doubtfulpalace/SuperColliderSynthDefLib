//
//  SuperColliderSynthDef.h
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/12/24.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "lua.h"
//#include "ParseLua.h"
//#include "SynthDefLua.h"

#ifndef SuperColliderSynthDef_h
#define SuperColliderSynthDef_h

#define MAXVARIANTNAMELEN 32
#define MAXVARIANTS 32
#define MAXNAMELEN 256
#define MAXINPUTS 64
#define MAXOUTPUTS 64
#define MAXCONSTANTS 256
#define MAXCONTROLS 64
#define MAXUGENS 256
#define MAXDEFSINFILE 2

#define CHECK_ERROR(error) if (error != SynthDefError_NONE) { return 0; }

bool littleEndian(void);

typedef struct WireSpec {
    uint32_t index;
    uint32_t channel;
} WireSpec;

typedef struct ControlSpec {
    char *name;
    uint32_t index;
} ControlSpec;

typedef struct Variant {
    char *name;
    float *values;
} Variant;

//typedef enum SynthDefRate {
//    SynthDefRate_SCALAR = 0,
//    SynthDefRate_CONTROL = 1,
//    SynthDefRate_AUDIO = 2,
//    SynthDefRate_DEMAND = 3
//} SynthDefRate;

typedef enum SynthDefError {
    SynthDefError_NONE = 0,
    SynthDefError_ERROR = 1
} SynthDefError;

typedef struct UGen {
    char *name;
    uint32_t numInputs;
    WireSpec *inputs;
    uint32_t numOutputs;
    uint32_t *outputRates;
    uint32_t rate;
    uint16_t specialIndex;
} UGen;

typedef struct SynthDef {
    char *name;
    uint32_t numConstants;
    float *constants;
    uint32_t numControlValues;
    float *controlValues;
    uint32_t numControls;
    ControlSpec *controls;
    uint32_t numUGens;
    UGen *ugens;
    uint16_t numVariants;
    Variant *variants;
} SynthDef;

typedef struct SynthDefFileRep {
    uint16_t numSynthDefs;
    SynthDef *synthDefs;
} SynthDefFileRep;

SynthDef *newSynthDef(void);
void freeSynthDef(SynthDef *def);

int writeSynthDef(SynthDef synthdef, uint8_t *data, SynthDefError *error);
int readSynthDef(SynthDef *synthdef, uint8_t *data, SynthDefError *error);

int writeSynthDefFileRep(SynthDefFileRep fileRep, uint8_t *data, SynthDefError *error);
SynthDefFileRep *readSynthDefFileRep(uint8_t *data, SynthDefError *error);
SynthDefFileRep *newSynthDefFileRep(void);
void freeSynthDefFileRep(SynthDefFileRep *fileRep);

#endif /* SuperColliderSynthDef_h */
