#include <limits.h>
#include <string.h>
#include <time.h>
#include "../src/commons.h"

#define PARAMETER           0       //Alterar para definir a ordem dos registros: 0 - crescente, 1 - decrescente, 2 - aleatório
#define NUM_REG             1000    // Alterar para definir o número de registros
#define NAME_BIN            "1000A.bin" 

#define CONST_GAP_RANGE     2146

char *createRandStr(char *data, int number) {
    for (int i = 0; i < number - 1; i++)
        data[i] = rand() % (122 - 65 + 1) + 65;
    data[number - 1] = '\0';
    return data;
}

// S = O(N), T = O(N)
void createData(registry_t *_reg) {
    for (int i = 0; i < NUM_REG; i++) {
        _reg[i].data_1 = rand() % LONG_MAX;
        strcpy(_reg[i].data_2, createRandStr(_reg[i].data_2, RD2_SIZE));
        strcpy(_reg[i].data_3, createRandStr(_reg[i].data_3, RD3_SIZE));
    }
}

// S = O(N), T = O(N)
void createKey(registry_t *_reg) {
    srand(time(NULL));
    switch (PARAMETER) {
        case 0:
            for (int i = 0; i < NUM_REG; i++) {
                if (i == 0)
                    _reg[i].key = rand() % CONST_GAP_RANGE + 1;
                else
                    _reg[i].key = _reg[i - 1].key + rand() % CONST_GAP_RANGE + 1;
            }
        break;
        case 1:
            for (int i = 0; i < NUM_REG; i++) {
                if (i == 0)
                    _reg[i].key = INT_MAX - rand() % CONST_GAP_RANGE + 1;
                else
                    _reg[i].key = _reg[i - 1].key - rand() % CONST_GAP_RANGE + 1;
            }
        break;
        case 2:
            for (int i = 0; i < NUM_REG; i++) {
                if (i == 0)
                    _reg[i].key = rand() % CONST_GAP_RANGE + 1;
                else
                    _reg[i].key = _reg[i - 1].key + rand() % CONST_GAP_RANGE + 1;
            }
            int j;
            int aux;
            for (int i = 0; i < NUM_REG; i++) {
                j = rand() % NUM_REG;
                aux = _reg[i].key;
                _reg[i].key = _reg[j].key;
                _reg[j].key = aux;
            }
        break;
    }
}



static void 
__AEIOU(int argc, char ** argsv) {
#define _TEMPORARY_FILENAME     "../temp/destruidor.bin"

    registry_t buffer = { 0 };
    FILE * reg_file = fopen(_TEMPORARY_FILENAME, "w+b");
    if (reg_file == NULL)
        return;

    for (int i = 1; i < argc; i ++) {
        buffer.data_1 = rand() % LONG_MAX;
        createRandStr(buffer.data_2, RD2_SIZE);
        createRandStr(buffer.data_3, RD3_SIZE);

        buffer.key = atoi(argsv[i]);
        printf("> key: %d\n", buffer.key);
        fwrite(& buffer, sizeof(registry_t), 1, reg_file);
    }

    fclose(reg_file);
}


// S = O(N), T = O(N).
int main(int argc, char ** argsv) {
    srand(time(NULL));
    if (argc > 1) {
        __AEIOU(argc, argsv);
        return 0;
    }

    registry_t *reg = calloc(NUM_REG, sizeof(registry_t));

    createKey(reg);
    createData(reg);

    // todo: to verify correct opening...
    FILE *arq = fopen("reg.bin", "wb");
    fwrite(reg, sizeof(registry_t), NUM_REG, arq);
    fclose(arq);

    free(reg);
    return 0;
}
