#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
    if (argc < 5 || argc > 6)
    {
        printf("Uso: pesquisa <método> <quantidade> <situação> <chave> [-P]\n"); // explicando o uso
        return 1;
    }

    // transformando os argumentos de char -> int
    int method = atoi(argv[1]);
    int quantity = atoi(argv[2]);
    int situation = atoi(argv[3]);
    int key = atoi(argv[4]);
    int display_keys = 0;

    //caso exista argv[5], e esteja com "-P", ativa "display_keys", atribuindo o valor "1"
    if (argc == 6 && strcmp(argv[5], "-P") == 0)
        display_keys = 1;

    int tamanho_maximo = strlen(argv[2]) + 4 + 1; // nome + extensão + \0
    char *bin = (char *)malloc(tamanho_maximo * sizeof(char));
    if (bin == NULL) {
        printf("Erro ao alocar memória.\n");
        return 1;
    }
    strcpy(bin, argv[2]); // nome do arquivo de acordo com o tamanho
    
    // Adiciona a extensão .bin ao nome do arquivo
    strcat(bin, ".bin");

    //inicializando o arquivo
    FILE *file = fopen(bin, "rb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    Registry_t result; // inicializando resultado
    result.key = 0; // inicializar com algo pra prevenir erros...?

    // Medição de tempo de execução
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    switch(method)
    {
        case 1:
            result = indexedSequencialSearch(file, quantity, situation, key); // acesso sequencial indexado
            break;
        case 2:
            result = externalBinarySearchTree(file, quantity, situation, key); // árvore binária de pesquisa adequada à memória externa
            break;
        case 3:
            result = BTree(file, quantity, situation, key); // árvore B
            break;
        case 4:
            result = BTree_star(file, quantity, situation, key); // árvore B*
            break;
        default:
            printf("Método de pesquisa inválido.\n");
            break;
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    if(display_keys)
    {
        if(result.key != 0)
            printf("RESULTADO ACHADO:\nChave --> %d\nDado1 --> %s\nDado2 --> %d\nDado3 --> %s\n", result.key, result.dado1, result.dado2, result.dado3);
        else
            printf("Chave não encontrada no arquivo.\n");
    }

    fclose(file);
    free(bin);

    return 0;
}