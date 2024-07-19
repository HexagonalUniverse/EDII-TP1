/*  <data/src/data-gen.c>
    
    (...) */  

#define _FILE_OFFSET_BITS   64
#define _LARGEFILE_SOURCE  
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "../../src/include/registry.h"


#define MIN_ARGS 3
#define MAX_FILE_NAME 31
#define CONST_GAP_RANGE     2146

#define DATA_FILES_PATH "./temp/"

typedef enum{
    ORDERED_ASCENDING,
    ORDERED_ASCENDING_GAP,
    ORDERED_DESCENDING,
    ORDERED_DESCENDING_GAP,
    DISORDERED,
    DISORDERED_GAP
}GenMode;


#define large_fseek(_Stream, _Offset, _Origin)  fseeko64(_Stream, _Offset, _Origin)

static inline bool readRegistry(FILE* _file, uint64_t _pos, registry_t* _returnRegistry){
    large_fseek(_file, _pos * sizeof(registry_t), SEEK_SET);
    return fread(_returnRegistry, sizeof(registry_t), 1, _file) > 0;
}

static inline bool writeRegistry(FILE* _file, uint64_t _pos, registry_t* _Registry){
    large_fseek(_file, _pos * sizeof(registry_t), SEEK_SET);
    return fwrite(_Registry, sizeof(registry_t), 1, _file) > 0;
}



//Creating a random string
static inline char *createRandStr(char* _str, const uint64_t _strSize) {
    for (uint64_t i = 0; i < _strSize - 1; i++)
        _str[i] = rand() % 26 + 65;
    _str[_strSize - 1] = '\0';
    return _str;
}

// S = O(N), T = O(N)
//Creating random Data for registries
static inline void createData(registry_t* _reg) {
    _reg->data_1 = rand() % LONG_MAX;
    createRandStr(_reg->data_2, RD2_SIZE);
    createRandStr(_reg->data_3, RD3_SIZE);
}

static void shuffleKeys(FILE* _file, const uint64_t _noReg){
    uint64_t rand_pos;
    key_t aux;
    registry_t reg_1, reg_2;

    for(uint64_t i = 0; i < _noReg; i++){
        rand_pos = rand() % _noReg;
        readRegistry(_file, i, &reg_1);
        readRegistry(_file, rand_pos, &reg_2);

        aux = reg_1.key;
        reg_1.key = reg_2.key;
        reg_2.key = aux;
        writeRegistry(_file, i, &reg_1);
        writeRegistry(_file, rand_pos, &reg_2);
    }
}

// S = O(N), T = O(N)
//Creating keys for registries
void createRegistries(registry_t* _reg, const GenMode* _genMode, const uint64_t _noReg, FILE* _file) {
    srand(time(NULL));
    key_t aux;

    switch (*_genMode) {
        case ORDERED_ASCENDING:
            for(uint64_t i = 0; i < _noReg; i++){
                _reg->key = i;
                createData(_reg);
                writeRegistry(_file, i, _reg);
            }
            break;

        case ORDERED_ASCENDING_GAP:
            createData(_reg);
            _reg->key = rand() % CONST_GAP_RANGE + 1;
            writeRegistry(_file, 0, _reg);

            for (uint64_t i = 1; i < _noReg; i++) {
                aux = _reg->key;

                createData(_reg);
                _reg->key = aux + rand() % CONST_GAP_RANGE + 1;

                writeRegistry(_file, i, _reg);
            }
        break;

        case ORDERED_DESCENDING:
            for(uint64_t i = 0; i < _noReg; i++){
                _reg->key = _noReg - i;
                createData(_reg);
                writeRegistry(_file, i, _reg);
            }
            break;

        case ORDERED_DESCENDING_GAP:
            createData(_reg);
            _reg->key = INT_MAX - rand() % CONST_GAP_RANGE + 1;
            writeRegistry(_file, 0, _reg);

            for (uint64_t i = 1; i < _noReg; i++) {
                aux = _reg->key;

                createData(_reg);
                _reg->key = aux - rand() % CONST_GAP_RANGE + 1;

                writeRegistry(_file, i, _reg);
            }
        break;

        case DISORDERED:
            for(uint64_t i = 0; i < _noReg; i++){
                _reg->key = i;
                createData(_reg);
                writeRegistry(_file, i, _reg);
            }
            shuffleKeys(_file, _noReg);
            break;

        case DISORDERED_GAP:
            createData(_reg);
            _reg->key = rand() % CONST_GAP_RANGE + 1;
            writeRegistry(_file, 0, _reg);

            for (uint64_t i = 1; i < _noReg; i++) {
                aux = _reg->key;

                createData(_reg);
                _reg->key = aux + rand() % CONST_GAP_RANGE + 1;

                writeRegistry(_file, i, _reg);
            }

            shuffleKeys(_file, _noReg);
        break;
    }
}

/*
static void createRegistries(registry_t* _regs, const GenMode* _genMode, const uint64_t _noReg, FILE* _file){
    //createData(_regs, _noReg);
    createKey(_regs, _genMode, _noReg, _file);
}
*/

/*
static void 
__AEIOU(int argc, char ** argsv) {
#define _TEMPORARY_FILENAME     "destruidor.bin"

    registry_t buffer = { 0 };
    FILE * reg_file = fopen(_TEMPORARY_FILENAME, "w+b");
    if (reg_file == NULL)
        return;

    for (int i = 2; i < argc; i ++) {
        buffer.data_1 = rand() % LONG_MAX;
        createRandStr(buffer.data_2, RD2_SIZE);
        createRandStr(buffer.data_3, RD3_SIZE);

        buffer.key = atoi(argsv[i]);
        printf("> key: %d\n", buffer.key);
        fwrite(& buffer, sizeof(registry_t), 1, reg_file);
    }

    fclose(reg_file);
}
*/

static bool 
interpretArguments(int argsc, char** argsv, GenMode* _returnGenMode, uint64_t* _returnNoRegistries, char* _returnFileName){
    if(argsc < MIN_ARGS){
        printf("ERROR: Expected at least %d arguments, given %d\n", MIN_ARGS, argsc);
        printf("\tFormat: ./data-gen [GenMode] [num of registries] *[file-name]\n");
        printf("\n\tGenModes:\n\t[0] ORDERED_ASCENDING\n\t[1] ORDERED_ASCENDING_GAP\n\t[2] ORDERED_DESCENDING\n\t[3] ORDERED_DESCENDING_GAP\n\t[4] DISORDERED\n\t[5] DISORDERED_GAP\n\n");
        return false;
    }
    
    *_returnGenMode = atoi(argsv[1]);
    *_returnNoRegistries = atoll(argsv[2]);
    if(argsc > MIN_ARGS){
        strcpy(_returnFileName, argsv[3]);
    }else{
        switch(*_returnGenMode){
            case ORDERED_ASCENDING:
                sprintf(_returnFileName, "%sOA-data%s.bin", DATA_FILES_PATH, argsv[2]);
                break;
            case ORDERED_ASCENDING_GAP:
                sprintf(_returnFileName, "%sOAG-data%s.bin", DATA_FILES_PATH, argsv[2]);
                break;
            case ORDERED_DESCENDING:
                sprintf(_returnFileName, "%sOD-data%s.bin", DATA_FILES_PATH, argsv[2]);
                break;
            case ORDERED_DESCENDING_GAP:
                sprintf(_returnFileName, "%sODG-data%s.bin", DATA_FILES_PATH, argsv[2]);
                break;
            case DISORDERED:
                sprintf(_returnFileName, "%sD-data%s.bin", DATA_FILES_PATH, argsv[2]);
                break;
            case DISORDERED_GAP:
                sprintf(_returnFileName, "%sDG-data%s.bin", DATA_FILES_PATH, argsv[2]);
                break;
        }
    }
    return true;
}

// S = O(N), T = O(N).
int main(int argc, char ** argsv) {
    srand(time(NULL));
    
    /*
    if (argc > 1) {
        if(!strcmp(argsv[1], "AEIOU")){
            __AEIOU(argc, argsv);
            return 0;
        }
    }
    */

    GenMode genMode; uint64_t noRegistries; char fileName[MAX_FILE_NAME];
    if(!interpretArguments(argc, argsv, &genMode, &noRegistries, fileName)){
        return 1;
    }

    FILE *file = NULL;
    if((file = fopen(fileName, "w+b")) == NULL){
        printf("ERROR unnable to open file\n");
        return 1;
    }

    registry_t reg;
    createRegistries(&reg, &genMode, noRegistries, file);

    printf("Data generated at file: %s\n", fileName);

    fclose(file);
    return 0;
}
