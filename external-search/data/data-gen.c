#include <limits.h>
#include <string.h>
#include <time.h>
#include "../src/registry.h"

#define MIN_ARGS 3
#define MAX_FILE_NAME 31
#define CONST_GAP_RANGE     2146

typedef enum{
    ORDERED_ASCENDING,
    ORDERED_ASCENDING_GAP,
    ORDERED_DESCENDING,
    ORDERED_DESCENDING_GAP,
    DISORDERED,
    DISORDERED_GAP
}GenMode;

//Creating a random string
static char *createRandStr(char* _str, const uint64_t _strSize) {
    for (int i = 0; i < _strSize - 1; i++)
        _str[i] = rand() % 26 + 65;
    _str[_strSize - 1] = '\0';
    return _str;
}

// S = O(N), T = O(N)
//Creating random Data for registries
static void createData(registry_t* _reg, const uint64_t* _noReg) {
    for (int i = 0; i < *_noReg; i++) {
        _reg[i].data_1 = rand() % LONG_MAX;
        createRandStr(_reg[i].data_2, RD2_SIZE);
        createRandStr(_reg[i].data_3, RD3_SIZE);
    }
}

static void shuffleKeys(registry_t* _reg, const uint64_t* _noReg){
    int j, aux;
    for (int i = 0; i < *_noReg; i++) {
        j = rand() % *_noReg;
        aux = _reg[i].key;
        _reg[i].key = _reg[j].key;
        _reg[j].key = aux;
    }
}

// S = O(N), T = O(N)
//Creating keys for registries
static void createKey(registry_t* _reg, const GenMode* _genMode, const uint64_t* _noReg) {
    srand(time(NULL));
    switch (*_genMode) {
        case ORDERED_ASCENDING:
            for(int i = 0; i < *_noReg; i++){
                _reg[i].key = i;
            }
            break;

        case ORDERED_ASCENDING_GAP:
            for (int i = 0; i < *_noReg; i++) {
                if (i == 0)
                    _reg[i].key = rand() % CONST_GAP_RANGE + 1;
                else
                    _reg[i].key = _reg[i - 1].key + rand() % CONST_GAP_RANGE + 1;
            }
        break;

        case ORDERED_DESCENDING:
            for(int i = 0; i < *_noReg; i++){
                _reg[i].key = *_noReg - i;
            }
            break;

        case ORDERED_DESCENDING_GAP:
            for (int i = 0; i < *_noReg; i++) {
                if (i == 0)
                    _reg[i].key = INT_MAX - rand() % CONST_GAP_RANGE + 1;
                else
                    _reg[i].key = _reg[i - 1].key - rand() % CONST_GAP_RANGE + 1;
            }
        break;

        case DISORDERED:
            for(int i = 0; i < *_noReg; i++){
                _reg[i].key = i;
            }
            shuffleKeys(_reg, _noReg);
            break;

        case DISORDERED_GAP:
            for (int i = 0; i < *_noReg; i++) {
                if (i == 0)
                    _reg[i].key = rand() % CONST_GAP_RANGE + 1;
                else
                    _reg[i].key = _reg[i - 1].key + rand() % CONST_GAP_RANGE + 1;
            }
            shuffleKeys(_reg, _noReg);
        break;
    }
}

static void createRegistries(registry_t* _regs, const GenMode* _genMode, const uint64_t* _noReg){
    createData(_regs, _noReg);
    createKey(_regs, _genMode, _noReg);
}

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


static bool 
interpretArguments(int argsc, char** argsv, GenMode* _returnGenMode, uint64_t* _returnNoRegistries, char* _returnFileName){
    if(argsc < MIN_ARGS){
        printf("ERROR: Expected at least %d arguments, given %d\n", MIN_ARGS, argsc);
        printf("\tFormat: ./data-gen [GenMode] [num of registries] *[file-name]\n");
        printf("\n\tGenModes:\n\t[0] ORDERED_ASCENDING\n\t[1] ORDERED_ASCENDING_GAP\n\t[2] ORDERED_DESCENDING\n\t[3] ORDERED_DESCENDING_GAP\n\t[4] DISORDERED\n\t[5] DISORDERED_GAP\n\n");
        return false;
    }

    *_returnGenMode = atoi(argsv[1]);
    *_returnNoRegistries = atoi(argsv[2]);
    if(argsc > MIN_ARGS){
        strcpy(_returnFileName, argsv[3]);
    }else{
        switch(*_returnGenMode){
            case ORDERED_ASCENDING:
                sprintf(_returnFileName, "OA-data%s.bin", argsv[2]);
                break;
            case ORDERED_ASCENDING_GAP:
                sprintf(_returnFileName, "OAG-data%s.bin", argsv[2]);
                break;
            case ORDERED_DESCENDING:
                sprintf(_returnFileName, "OD-data%s.bin", argsv[2]);
                break;
            case ORDERED_DESCENDING_GAP:
                sprintf(_returnFileName, "ODG-data%s.bin", argsv[2]);
                break;
            case DISORDERED:
                sprintf(_returnFileName, "D-data%s.bin", argsv[2]);
                break;
            case DISORDERED_GAP:
                sprintf(_returnFileName, "DG-data%s.bin", argsv[2]);
                break;
        }
    }
    return true;
}

// S = O(N), T = O(N).
int main(int argc, char ** argsv) {
    srand(time(NULL));

    if (argc > 1) {
        if(!strcmp(argsv[1], "AEIOU")){
            __AEIOU(argc, argsv);
            return 0;
        }
    }

    GenMode genMode; uint64_t noRegistries; char fileName[MAX_FILE_NAME];
    if(!interpretArguments(argc, argsv, &genMode, &noRegistries, fileName)){
        return 1;
    }

    registry_t *regs = calloc(noRegistries, sizeof(registry_t));
    createRegistries(regs, &genMode, &noRegistries);

    FILE *file = NULL;
    if((file = fopen(fileName, "wb")) == NULL){
        printf("ERROR unnable to open file\n");
        return 1;
    }
    fwrite(regs, sizeof(registry_t), noRegistries, file);
    printf("Data generated at file: %s\n", fileName);

    free(regs);
    fclose(file);
    return 0;
}