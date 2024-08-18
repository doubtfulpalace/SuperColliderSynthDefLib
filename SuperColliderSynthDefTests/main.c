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
    /* declare a file pointer */
    FILE    *infile;
    uint8_t *buffer;
    long    numbytes;
     
    /* open an existing file for reading */
    infile = fopen(path, "r");
     
    /* quit if the file does not exist */
    if (infile == NULL) {
        return NULL;
    }
    
    /* Get the number of bytes */
    fseek(infile, 0L, SEEK_END);
    numbytes = ftell(infile);
     
    /* reset the file position indicator to
    the beginning of the file */
    fseek(infile, 0L, SEEK_SET);
     
    /* grab sufficient memory for the
    buffer to hold the text */
    buffer = (uint8_t*)calloc(numbytes, sizeof(char));
     
    /* memory error */
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

void testLuaSynthDef(lua_State *L, const char *resultKey) {
    lua_getglobal(L, resultKey);
    SynthNodeSpec *node = (SynthNodeSpec*)lua_touserdata(L, -1);
    dumpSynthNodeSpec(stdout, *node);
}

void testStack(void) {
    int size = 10;
    NodeSpecStack *stack = (NodeSpecStack*)malloc(sizeof(NodeSpecStack));
    for (int i = 0; i < size; i++) {
        SynthNodeSpec *spec = (SynthNodeSpec*)malloc(sizeof(SynthNodeSpec));
        spec->id = i;
        pushNodeSpec(stack, spec);
    }
    for (int i = 0; i < size; i++) {
        SynthNodeSpec *spec = popNodeSpec(stack);
        printf("%d, ", spec->id);
        freeSynthNodeSpec(spec);
    }
    free(stack);
    printf("\n");
}

int main(int argc, const char * argv[]) {

    const char *synthCode = ""
        "return Out(Control(\"out\", 1), SinOsc(440), SinOsc(220, 0.3))";
    SynthDef *def1 = newSynthDef();
    parse_lua_synthdef(synthCode, "boop", def1);
    
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
    
    /* declare a file pointer */
    FILE    *outfile;
     
    /* open an existing file for reading */
    char *path = "/Users/timwalters/Library/Application Support/SuperCollider/synthdefs/scsdt.scsyndef";
    outfile = fopen(path, "w");
         
    fwrite(fileData, sizeof(uint8_t), dataSize, outfile);
    fclose(outfile);
    freeSynthDef(def1);
    free(fileData);

    SynthDefFileRep *fileRep2 = readSynthDefFile("/Users/timwalters/Library/Application Support/SuperCollider/synthdefs/scsdt.scsyndef");
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
