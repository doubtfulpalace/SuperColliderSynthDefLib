//
//  SuperColliderSynthDef.c
//  SuperColliderSynthDefLib
//
//  Created by Tim Walters on 8/12/24.
//

#include <stdio.h>
#include "SuperColliderSynthDef.h"

bool littleEndian(void) {
    int i = 1;
    char *p = (char *)&i;
    return p[0] == 1;
}

int writeSynthDefName(char string[], uint8_t *data, SynthDefError *error) {
    *error = 0;
    int counter = 0;
    unsigned long count = strlen(string);
    if (count > MAXNAMELEN) {
        *error = SynthDefError_ERROR;
        return 0;
    }
    data[0] = (uint8_t)count;
    counter++;
    for (int i = 0; i < count; i++) {
        data[counter] = string[i];
        counter++;
    }
    return counter;
}

int readSynthDefName(char **name, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    uint8_t rawLength = *data;
    int length = (int)rawLength;
    if ((int)length > MAXNAMELEN) {
        *error = SynthDefError_ERROR;
        return 0;
    }
    counter += 1;
    *name = (char *)malloc((length + 1) * sizeof(char));
    for (int i = 0; i < length; i++) {
        (*name)[i] = data[counter];
        counter++;
    }
    return counter;
}

int writeSynthDefUInt32(uint32_t value, uint8_t *data) {
    data[0] = (value >> 24) & 0xFF;
    data[1] = (value >> 16) & 0xFF;
    data[2] = (value >> 8) & 0xFF;
    data[3] = value & 0xFF;
    return 4;
}

int readSynthDefUInt32(uint32_t *value, uint8_t *data) {
    *value = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
    return 4;
}

int writeSynthDefUInt16(uint16_t value, uint8_t *data) {
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;
    return 2;
}

int readSynthDefUInt16(uint16_t *value, uint8_t *data) {
    *value = (data[0] << 8) + data[1];
    return 2;
}

int writeSynthDefFloat(float value, uint8_t *data) {
    union {
        uint32_t n;
        float x;
    } converter;
    converter.x = value;
    return writeSynthDefUInt32(converter.n, data);
}

int readSynthDefFloat(float *value, uint8_t *data) {
    union {
        uint32_t n;
        float x;
    } converter;
    int counter = readSynthDefUInt32(&converter.n, data);
    *value = converter.x;
    return counter;
}

int writeSynthDefWireSpec(WireSpec wireSpec, uint8_t *data) {
    int counter = 0;
    counter += writeSynthDefUInt32(wireSpec.index, data);
    counter += writeSynthDefUInt32(wireSpec.channel, data + counter);
    return counter;
}

int readSynthDefWireSpec(WireSpec *wireSpec, uint8_t *data) {
    int counter = 0;
    counter += readSynthDefUInt32(&wireSpec->index, data);
    counter += readSynthDefUInt32(&wireSpec->channel, data + counter);
    return counter;
}

int writeSynthDefRate(SynthDefRate rate, uint8_t *data) {
    *data = (uint8_t)rate;
    return 1;
}

int readSynthDefRate(SynthDefRate *rate, uint8_t *data) {
    uint8_t rawRate = *data;
    int intRate = (int)rawRate;
    switch(intRate) {
        case SynthDefRate_SCALAR:
        case SynthDefRate_CONTROL:
        case SynthDefRate_AUDIO:
        case SynthDefRate_DEMAND:
            *rate = intRate;
            break;
        default:
            *rate = SynthDefRate_CONTROL;
    }
    return 1;
}

int writeSynthDefControlSpec(ControlSpec controlSpec, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += writeSynthDefName(controlSpec.name, data, error);
    CHECK_ERROR(*error);
    counter += writeSynthDefUInt32(controlSpec.index, data + counter);
    return counter;
}

int readSynthDefControlSpec(ControlSpec *controlSpec, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += readSynthDefName(&controlSpec->name, data, error);
    CHECK_ERROR(*error)
    counter += readSynthDefUInt32(&controlSpec->index, data + counter);
    return counter;
}

int writeSynthDefVariant(Variant variant, uint32_t numControls, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += writeSynthDefName(variant.name, data, error);
    CHECK_ERROR(*error);
    for (int i = 0; i < numControls; i++) {
        counter += writeSynthDefFloat(variant.values[i], data + counter);
    }
    return counter;
}

int readSynthDefVariant(Variant *variant, uint32_t numControls, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += readSynthDefName(&variant->name, data, error);
    CHECK_ERROR(*error);
    variant->values = (float *)malloc(sizeof(float) * numControls);
    for (int i = 0; i < numControls; i++) {
        counter += readSynthDefFloat(&variant->values[i], data + counter);
    }
    return counter;
}

int writeSynthDefUGen(UGen ugen, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += writeSynthDefName(ugen.name, data + counter, error);
    CHECK_ERROR(*error);
    counter += writeSynthDefRate(ugen.rate, data + counter);
    counter += writeSynthDefUInt32(ugen.numInputs, data + counter);
    counter += writeSynthDefUInt32(ugen.numOutputs, data + counter);
    counter += writeSynthDefUInt16(ugen.specialIndex, data + counter);
    for (int i = 0; i < ugen.numInputs; i++) {
        counter += writeSynthDefWireSpec(ugen.inputs[i], data + counter);
    }
    for (int i = 0; i < ugen.numOutputs; i++) {
        counter += writeSynthDefRate(ugen.outputRates[i], data + counter);
    }
    return counter;
}

int readSynthDefUGen(UGen *ugen, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += readSynthDefName(&ugen->name, data + counter, error);
    CHECK_ERROR(*error);
    counter += readSynthDefRate(&ugen->rate, data + counter);
    counter += readSynthDefUInt32(&ugen->numInputs, data + counter);
    counter += readSynthDefUInt32(&ugen->numOutputs, data + counter);
    counter += readSynthDefUInt16(&ugen->specialIndex, data + counter);
    ugen->inputs = (WireSpec *)malloc(ugen->numInputs * sizeof(WireSpec));
    for (int i = 0; i < ugen->numInputs; i++) {
        counter += readSynthDefWireSpec(&ugen->inputs[i], data + counter);
    }
    ugen->outputRates = (SynthDefRate *)malloc(ugen->numOutputs * sizeof(SynthDefRate));
    for (int i = 0; i < ugen->numOutputs; i++) {
        counter += readSynthDefRate(&ugen->outputRates[i], data + counter);
    }
    return counter;
}

int writeSynthDef(SynthDef synthdef, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += writeSynthDefName(synthdef.name, data + counter, error);
    CHECK_ERROR(*error);
    counter += writeSynthDefUInt32(synthdef.numConstants, data + counter);
    for (int i = 0; i < synthdef.numConstants; i++) {
        counter += writeSynthDefFloat(synthdef.constants[i], data + counter);
    }
    counter += writeSynthDefUInt32(synthdef.numControlValues, data + counter);
    for (int i = 0; i < synthdef.numControlValues; i++) {
        counter += writeSynthDefFloat(synthdef.controlValues[i], data + counter);
    }
    counter += writeSynthDefUInt32(synthdef.numControls, data + counter);
    for (int i = 0; i < synthdef.numControls; i++) {
        counter += writeSynthDefControlSpec(synthdef.controls[i], data + counter, error);
        CHECK_ERROR(*error);
    }
    counter += writeSynthDefUInt32(synthdef.numUGens, data + counter);
    for (int i = 0; i < synthdef.numUGens; i++) {
        counter += writeSynthDefUGen(synthdef.ugens[i], data + counter, error);
        CHECK_ERROR(*error);
    }
    counter += writeSynthDefUInt16(synthdef.numVariants, data + counter);
    for (int i = 0; i < synthdef.numVariants; i++) {
        counter += writeSynthDefVariant(synthdef.variants[i], synthdef.numControls, data + counter, error);
        CHECK_ERROR(*error);
    }
    return counter;
}

int readSynthDef(SynthDef *synthdef, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += readSynthDefName(&synthdef->name, data + counter, error);
    CHECK_ERROR(*error);
    counter += readSynthDefUInt32(&synthdef->numConstants, data + counter);
    synthdef->constants = (float *)malloc(synthdef->numConstants * sizeof(float));
    for (int i = 0; i < synthdef->numConstants; i++) {
        counter += readSynthDefFloat(&synthdef->constants[i], data + counter);
    }
    counter += readSynthDefUInt32(&synthdef->numControlValues, data + counter);
    synthdef->controlValues = (float*)malloc(synthdef->numControlValues * sizeof(float));
    for (int i = 0; i < synthdef->numControlValues; i++) {
        counter += readSynthDefFloat(&synthdef->controlValues[i], data + counter);
    }
    counter += readSynthDefUInt32(&synthdef->numControls, data + counter);
    synthdef->controls = (ControlSpec*)malloc(synthdef->numControls * sizeof(ControlSpec));
    for (int i = 0; i < synthdef->numControls; i++) {
        counter += readSynthDefControlSpec(&synthdef->controls[i], data + counter, error);
        CHECK_ERROR(*error);
    }
    counter += readSynthDefUInt32(&synthdef->numUGens, data + counter);
    synthdef->ugens = (UGen*)malloc(synthdef->numUGens * sizeof(UGen));
    for (int i = 0; i < synthdef->numUGens; i++) {
        counter += readSynthDefUGen(&synthdef->ugens[i], data + counter, error);
        CHECK_ERROR(*error);
    }
    counter += readSynthDefUInt16(&synthdef->numVariants, data + counter);
    synthdef->variants = (Variant*)malloc(synthdef->numVariants * sizeof(Variant));
    for (int i = 0; i < synthdef->numVariants; i++) {
        counter += readSynthDefVariant(&synthdef->variants[i], synthdef->numControls, data + counter, error);
        CHECK_ERROR(*error);
    }
    return counter;
}

int writeSynthDefFileRep(SynthDefFileRep fileRep, uint8_t *data, SynthDefError *error) {
    int counter = 0;
    counter += writeSynthDefUInt32('SCgf', data);
    uint32_t version = 2;
    counter += writeSynthDefUInt32(version, data + counter);
    counter += writeSynthDefUInt16(fileRep.numSynthDefs, data + counter);
    for (uint16_t i = 0; i < fileRep.numSynthDefs; i++) {
        counter += writeSynthDef(fileRep.synthDefs[i], data + counter, error);
        CHECK_ERROR(*error);
    }
    return counter;
}

SynthDefFileRep *newSynthDefFileRep(void) {
    return (SynthDefFileRep*)malloc(sizeof(SynthDefFileRep));
}

SynthDefFileRep *readSynthDefFileRep(uint8_t *data, SynthDefError *error) {
    SynthDefFileRep *fileRep = newSynthDefFileRep();
    int counter = 0;
    uint32_t magic = 0;
    counter += readSynthDefUInt32(&magic, data + counter);
    if (magic != 'SCgf') {
        *error = SynthDefError_ERROR;
        return 0;
    }
    uint32_t version = 0;
    counter += readSynthDefUInt32(&version, data + counter);
    if (version != 2) {
        *error = SynthDefError_ERROR;
        return 0;
    }
    counter += readSynthDefUInt16(&fileRep->numSynthDefs, data + counter);
    fileRep->synthDefs = (SynthDef*)malloc(fileRep->numSynthDefs * sizeof(SynthDef));
    for (uint16_t i = 0; i < fileRep->numSynthDefs; i++) {
        counter += readSynthDef(&fileRep->synthDefs[i], data + counter, error);
        CHECK_ERROR(*error);
    }
    return fileRep;
}

void freeControlSpec(ControlSpec *controlSpec) {
    free(controlSpec->name);
}

void freeUGen(UGen *ugen) {
    free(ugen->name);
    free(ugen->inputs);
    free(ugen->outputRates);
}

void freeVariant(Variant *variant) {
    free(variant->name);
    free(variant->values);
}

SynthDef *newSynthDef(void) {
    return (SynthDef *)malloc(sizeof(SynthDef));
}

void freeSynthDef(SynthDef *def) {
    free(def->name);
    free(def->constants);
    free(def->controlValues);
    free(def->controls);
    free(def->ugens);
    free(def->variants);
    free(def);
}

void freeSynthDefFileRep(SynthDefFileRep *fileRep) {
    for (int i = 0; i < fileRep->numSynthDefs; i++) {
        freeSynthDef(&fileRep->synthDefs[i]);
    }
    free(fileRep);
}
