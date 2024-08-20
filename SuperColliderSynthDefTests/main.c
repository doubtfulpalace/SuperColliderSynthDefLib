//
//  main.c
//  SuperColliderSynthDefTests
//
//  Created by Tim Walters on 8/12/24.
//

#include <stdio.h>
#include "SuperColliderSynthDef.h"
#include "SynthDefLua.h"
#include "ParseLua.h"
#include "NodeStack.h"
#include "Diagnostics.h"

SynthDefFileRep *readSynthDefFile(char *path) {
    FILE *infile;
    uint8_t *buffer;
    long numbytes;
     
    infile = fopen(path, "r");
    if (infile == NULL) {
        return NULL;
    }
    
    fseek(infile, 0L, SEEK_END);
    numbytes = ftell(infile);
    fseek(infile, 0L, SEEK_SET);
     
    buffer = (uint8_t*)calloc(numbytes, sizeof(char));
    if (buffer == NULL) {
        return NULL;
    }
    
    fread(buffer, sizeof(uint8_t), numbytes, infile);
    fclose(infile);
    
    SynthDefError error = SynthDefError_NONE;
    
    SynthDefFileRep *fileRep = readSynthDefFileRep(buffer, &error);
    if (error != SynthDefError_NONE) { return NULL; }

    free(buffer);
    if (error != SynthDefError_NONE) { return NULL; }
    return(fileRep);
}

void testlua(lua_State *L, const char *resultKey) {
    lua_getglobal(L, resultKey);
    if (!lua_isnumber(L, -1)) {
        error(L, "result should be a number\n");
    }
    double result = (float)lua_tonumber(L, -1);
    printf("Result: %.6f\n", result);
}

int main(int argc, const char * argv[]) {
    const char *synthCode = ""
        "controls = { Control(\"out\", 0), Control(\"freq\", 440), Control(\"amp\", 0.125) }"
        "osc1 = SinOsc(controls[2]) * controls[3]"
        "osc2 = SinOsc(controls[2]) * controls[3]"
        "return Out(controls[1], osc1, osc2)";
    SynthDef *def1 = newSynthDef();
    parse_lua_synthdef(synthCode, "swifter2", def1);
    
    dumpSynthDef(stdout, *def1);
    
    SynthDefFileRep *fileRep = newSynthDefFileRep();
    fileRep->numSynthDefs = 1;
    fileRep->synthDefs = def1;
    uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t) * 100000);

    SynthDefError error;
    int dataSize = writeSynthDefFileRep(*fileRep, buffer, &error);
    if (error != SynthDefError_NONE) {
        return error;
    }
    
    uint8_t *fileData = (uint8_t*)malloc(dataSize);
    memcpy(fileData, buffer, dataSize);
    free(buffer);
    
    FILE *outfile;
    char *path = "/Users/timwalters/Library/Application Support/SuperCollider/synthdefs/scsdt.scsyndef";
    outfile = fopen(path, "w");
         
    fwrite(fileData, sizeof(uint8_t), dataSize, outfile);
    fclose(outfile);
    freeSynthDef(def1);
    free(fileData);

    SynthDefFileRep *fileRep2 = readSynthDefFile("/Users/timwalters/Library/Application Support/SuperCollider/synthdefs/swifter2.scsyndef");
    if (fileRep2 == NULL) {
        return 1;
    }
    for (int i = 0; i < fileRep2->numSynthDefs; i++) {
        dumpSynthDef(stdout, fileRep2->synthDefs[i]);
    }
    printf("Read %d synthdefs\n", fileRep2->numSynthDefs);
    freeSynthDefFileRep(fileRep2);
    return 0;
}
