#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct registry_t // estrutura de dados
{
    int key;
    
    long data_1;
    char data_2[1000];
    char data_3[5000];
} registry_t;

typedef struct page // estrutura de página
{
    registry_t *dados;
    int page_id;
} Page;

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

        size_t bytes_read = fread(page->dados, sizeof(registry_t), number_of_items, file);
        if (bytes_read != number_of_items)
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

// função de Busca Sequencial
registry_t indexedSequencialSearch(FILE *file, int quantity, int situation, int key) // o arquivo, o item a ser achado, e se está ordenado ou não
{
    int i = 0, number_of_items = 5;
    Page * page;

    while (i < number_of_pages && array[i] < key)
        i++;

    if ((page = loadPage(file, i * number_of_items, number_of_items)) != NULL)
    {
        for (int j = 0; j < number_of_items; j++) // itera enquando j < number_of_items
        {
            if(page->dados[j].key == 0) // caso algum elemento seja 0, significa que, à partir daquele item, não tem itens inicializados
            {
                printf("Chave não encontrada!\n"); // item não achado
                unloadPage(page);
                return (registry_t){0}; // retorna um tipo "registry_t" com elementos nulos
            }
            
            switch (situation) // verifica qual o tipo de ordenação
            {
                case 1: // orenado crescente
                    if(key == page->dados[j].key)
                    {
                        registry_t temp = page->dados[j];
                        unloadPage(page);
                        return temp; // item achado
                    }
                    else if(key < page->dados[j].key) // se for menor que os próximos itens, ou seja, já passou de onde estaria localizado a chave
                    {
                        printf("Chave não encontrada!\n"); // item não achado
                        unloadPage(page);
                        return (registry_t){0};
                    }
                    break;

                case 2: // orenado decrescente
                    if(key == page->dados[j].key)
                    {
                        registry_t temp = page->dados[j];
                        unloadPage(page);
                        return temp; // item achado
                    }
                    else if(key > page->dados[j].key) // se for maior que os próximos itens, ou seja, já passou de onde estaria localizado a chave
                    {
                        printf("Chave não encontrada!\n"); // item não achado
                        unloadPage(page);
                        return (registry_t){0};
                    }
                    break;

                case 3: // desorenado
                    if(key == page->dados[j].key)
                    {
                        registry_t temp = page->dados[j];
                        unloadPage(page);
                        return temp; // item achado
                    }
                    break;
            }
        }
        unloadPage(page); // descarrega página
        i++; // próxima página
    }
    
    printf("Chave não encontrada!\n");
    return (registry_t){0};
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

registry_t indexedBinarySearch(FILE *file, int quantity, int situation, int key) // o arquivo, o item a ser achado, e se está ordenado ou não
{
    int i = 0, number_of_items = 5, target_index = number_of_items - 1;
    Page *page;

    while ((page = loadPage(file, i * number_of_items, number_of_items)) != NULL)
    {
        target_index = number_of_items - 1;

        if(page->dados[0].key == 0) // caso algum elemento seja 0, significa que, à partir daquele item, não tem itens inicializados
        {
            printf("Chave não encontrada!\n"); // item não achado
            unloadPage(page);
            return (registry_t){0}; // retorna um tipo "registry_t" com elementos nulos
        }

        switch (situation) // verifica qual o tipo de ordenação
        {
            case 1: // orenado crescente
                if(key > page->dados[target_index].key) // verifica o último elemento de uma vez, para ver se o item está no raio da página analisada
                {
                    unloadPage(page);
                    i++;
                    continue;
                }

                target_index = binarySearch(page->dados, 0, number_of_items, key, situation);

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
                break;

            case 2: // orenado decrescente
                if(key < page->dados[target_index].key) // verifica o último elemento de uma vez, para ver se o item está no raio da página analisada
                {
                    unloadPage(page);
                    i++;
                    continue;
                }

                target_index = binarySearch(page->dados, 0, number_of_items, key, situation);

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
                break;
        }
        unloadPage(page); // descarrega página
        i++; // próxima página
    }
    
    printf("Chave não encontrada!\n");
    return (registry_t){0};
}

int main()
{
    FILE *file = fopen("nothing.bin", "rb");
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    int key = 1;

    registry_t result = indexedSequencialSearch(file, 100, 1, key); // Pesquisa sequencial
    //registry_t result = indexedBinarySearch(file, 100, 1, key); // Pesquisa binária
    if(result.key != 0)
        printf("Resultado achado --> %d\n", result.key);

    fclose(file);

    return 0;
}