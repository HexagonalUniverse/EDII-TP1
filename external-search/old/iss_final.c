#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iss_final.h"

// funções de Paginação ()
void unloadPage(Page *page) // descarregar página
{
    free(page->dados);
    free(page);
}

Page *loadPage(FILE *file, int page_id, int number_of_items) // carregar página
{
    if(file != NULL)
    {
        Page *page = (Page*) malloc(sizeof(Page));
        if (page == NULL)
        {
            printf("Erro de alocação de memória.\n");
            return NULL;
        }

        page->dados = (registry_t*) malloc(number_of_items * sizeof(registry_t));
        if (page->dados == NULL)
        {
            printf("Erro de alocação de memória.\n");
            free(page);
            return NULL;
        }
        for (int i = 0; i < number_of_items; i++)
            page->dados[i].key = 0;

        // long int position = ftell(file);
        // printf("Posição atual do indicador de posição 1: %ld bytes\n", position);

        fseek(file, sizeof(registry_t) * page_id, SEEK_SET);
        // position = ftell(file);
        // printf("Posição atual do indicador de posição 2: %ld bytes\n", position);
        
        size_t registries_read = fread(page->dados, sizeof(registry_t), number_of_items, file);
        if (registries_read != number_of_items)
        {
            if (feof(file))
                printf("Alcançado o final do arquivo.\n");
            else if (ferror(file))
                printf("Erro de leitura do arquivo.\n");
            
            unloadPage(page);
            return NULL;
        }

        // position = ftell(file);
        // printf("Posição atual do indicador de posição 3: %ld bytes\n", position);
        // for (int i = 0; i < number_of_items; i++)
        //     printf("key  -> %d\n", page->dados[i].key);

        return page;
    }

    return NULL;
}


int *indexTable(FILE *file, int number_of_items, int *index_table_size)
{
    fseek(file, 0, SEEK_END);
    size_t end_of_file = ftell(file);
    int number_of_pages = end_of_file / (sizeof(registry_t) * number_of_items);

    //printf("number_of_pages -> %d\n", number_of_pages);

    int *index_table = malloc(number_of_pages * sizeof(int));

    for (int i = 0; i < number_of_pages; i++)
    {
        fseek(file, sizeof(registry_t) * i * number_of_items, SEEK_SET);
        fread(&index_table[i], sizeof(int), 1, file);
        //printf("index_table[%d] -> %d\n", i, index_table[i]);
    }

    *index_table_size = number_of_pages;
    return index_table;
}

// funções de pesquisa sequencial ordenada crescente e descrescente
registry_t orderedUpSequencialSearch(FILE *file, int number_of_items, int *index_table, int index_table_size, int key)
{
    Page *page;
    int i = 0;

    while (index_table[i] < key && i < index_table_size) i++;
    //printf("i -> %d\n", i);
    
    if ((page = loadPage(file, (i-1) * number_of_items, number_of_items)) != NULL)
    {
        for (int j = 0; j < number_of_items; j++) // itera enquando j < number_of_items
        {
            //printf("j -> %d\n", j);
            if(page->dados[j].key == 0 || key < page->dados[j].key) // caso algum elemento seja 0, significa que, à partir daquele item, não tem itens inicializados
            {
                printf("Chave não encontrada!\n"); // item não achado
                unloadPage(page);
                return (registry_t){0}; // retorna um tipo "registry_t" com elementos nulos
            }
            else if(key == page->dados[j].key)
            {
                registry_t temp = page->dados[j];
                unloadPage(page);
                return temp; // item achado
            }
        }
    }
    
    printf("Chave não encontrada!\n");
    return (registry_t){0};
}

registry_t orderedDownSequencialSearch(FILE *file, int number_of_items, int *index_table, int index_table_size, int key)
{
    Page *page;
    int i = 0;

    while (index_table[i] > key && i < index_table_size) i++;
    
    if ((page = loadPage(file, (i-1) * number_of_items, number_of_items)) != NULL)
    {
        for (int j = 0; j < number_of_items; j++) // itera enquando j < number_of_items
        {
            if(page->dados[j].key == 0 || key > page->dados[j].key) // caso algum elemento seja 0, significa que, à partir daquele item, não tem itens inicializados
            {
                printf("Chave não encontrada!\n"); // item não achado
                unloadPage(page);
                return (registry_t){0}; // retorna um tipo "registry_t" com elementos nulos
            }
            else if(key == page->dados[j].key)
            {
                registry_t temp = page->dados[j];
                unloadPage(page);
                return temp; // item achado
            }
        }
    }
    
    printf("Chave não encontrada!\n");
    return (registry_t){0};
}

// função de Busca Sequencial
registry_t indexedSequencialSearch(FILE *file, int situation, int key) // o arquivo, o item a ser achado, e se está ordenado ou não
{
    int index_table_size, number_of_items = 5;
    int *index_table = indexTable(file, number_of_items, &index_table_size);
    registry_t result = (registry_t){0};

    switch (situation)
    {
    case 1:
        result = orderedUpSequencialSearch(file, number_of_items, index_table, index_table_size, key);
        return result;
        free(index_table);
        break;
    case 2:
        result = orderedDownSequencialSearch(file, number_of_items, index_table, index_table_size, key);
        return result;
        free(index_table);
        break;
    default:
        printf("Operação inexistente!\n");
        return (registry_t){0};
        free(index_table);
        break;
    }
}


// funções de Busca Binária
int binarySearch(registry_t *dados, int left, int right, int key, int situation) // fazer o esquema de ordenação
{
    if(left <= right)
    {
        int mid = (left + right) / 2;

        if(dados[mid].key == key)
            return mid;
        switch (situation)
        {
        case 1: // ordenado
            if(key > dados[mid].key)
                return binarySearch(dados, mid + 1, right, key, situation);
            else
                return binarySearch(dados, left, mid - 1, key, situation);
            break;
        case 2: // desordenado
            if(key < dados[mid].key)
                return binarySearch(dados, mid + 1, right, key, situation);
            else
                return binarySearch(dados, left, mid - 1, key, situation);
            break;
        }
    }

    return -1;
}

// funções de pesquisa binária ordenada crescente e descrescente
registry_t orderedUpBinarySearch(FILE *file, int number_of_items, int *index_table, int index_table_size, int key)
{
    Page *page;
    int i = 0, target_index = 0;

    while (index_table[i] < key && i < index_table_size) i++;
    //printf("i -> %d\n", i);
    
    if ((page = loadPage(file, (i-1) * number_of_items, number_of_items)) != NULL)
    {
        if(page->dados[0].key != 0) // caso o primeiro elemento seja 0, significa que, à partir daquele item, não tem itens inicializados
        {
            target_index = binarySearch(page->dados, 0, number_of_items, key, 1);

            if(target_index != -1)
            {
                registry_t temp = page->dados[target_index];
                unloadPage(page);
                return temp; // item achado
            }
            else
            {
                printf("Chave não encontrada!\n"); // item não achado
                unloadPage(page);
                return (registry_t){0};
            }
        }
    }
    
    printf("Chave não encontrada!\n");
    return (registry_t){0};
}

registry_t orderedDownBinarySearch(FILE *file, int number_of_items, int *index_table, int index_table_size, int key)
{
    Page *page;
    int i = 0, target_index = 0;

    while (index_table[i] > key && i < index_table_size) i++;
    //printf("i -> %d\n", i);
    
    if ((page = loadPage(file, (i-1) * number_of_items, number_of_items)) != NULL)
    {
        if(page->dados[0].key != 0) // caso o primeiro elemento seja 0, significa que, à partir daquele item, não tem itens inicializados
        {
            target_index = binarySearch(page->dados, 0, number_of_items, key, 2);

            if(target_index != -1)
            {
                registry_t temp = page->dados[target_index];
                unloadPage(page);
                return temp; // item achado
            }
            else
            {
                printf("Chave não encontrada!\n"); // item não achado
                unloadPage(page);
                return (registry_t){0};
            }
        }
    }
    
    printf("Chave não encontrada!\n");
    return (registry_t){0};
}

// função de Busca Binária
registry_t indexedBinarySearch(FILE *file, int situation, int key) // o arquivo, o item a ser achado, e se está ordenado ou não
{
    int index_table_size, number_of_items = 5;
    int *index_table = indexTable(file, number_of_items, &index_table_size);
    registry_t result = (registry_t){0};

    switch (situation)
    {
    case 1:
        result = orderedUpBinarySearch(file, number_of_items, index_table, index_table_size, key);
        return result;
        free(index_table);
        break;
    case 2:
        result = orderedDownBinarySearch(file, number_of_items, index_table, index_table_size, key);
        return result;
        free(index_table);
        break;
    default:
        printf("Operação inexistente!\n");
        return (registry_t){0};
        free(index_table);
        break;
    }
}

/*
int main(void)
{
    FILE *file = fopen("nothing.bin", "rb"); // Abre o arquivo em modo de leitura binária
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    int key = 1;

    //registry_t result = indexedSequencialSearch(file, 2, key); // Pesquisa sequencial
    registry_t result = indexedBinarySearch(file, 1, key); // Pesquisa binária
    if(result.key != 0)
        printf("Resultado achado --> %d\n", result.key);

    fclose(file);

    return 0;
}
*/