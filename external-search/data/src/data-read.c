#include <stdio.h>
#include <stdlib.h>

#include "../../src/include/registry.h"
#include "../../src/include/paging.h"

void printRegistry(registry_t* reg){
    printf("\tkey: %-5d | data1: %-10lld | data2: %.4s | data3: %.4s\n", reg->key, reg->data_1, reg->data_2, reg->data_3);
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("ERRO:\n\tFormato: ./read [arquivo]\n");
        return 1;
    }

    FILE* file;
    if((file = fopen(argv[1], "rb")) == NULL){
        printf("ERRO: Não foi possível abrir o arquivo\n");
        return 1;
    }

    registry_t reg[REGPAGE_ITENS];
    int page_index = 0;
    int q_itens;
    while((q_itens = fread(&reg, sizeof(registry_t), REGPAGE_ITENS, file))){
        printf("Page read index: %d\n", page_index++);
        for(int i = 0; i < q_itens; i++){
            printRegistry(&reg[i]);
        }
        printf("q_itens: %d\n\n", q_itens);
    }

    fclose(file);

    return 0;
}