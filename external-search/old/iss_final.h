#ifndef ISS_FINAL_H
#define ISS_FINAL_H

#define PAGE_SIZE 6016

typedef struct registry_t // estrutura de dados
{
    int key;
    long dado1;
    char dado2[1000];
    char dado3[5000];
} registry_t;

typedef struct page // estrutura de página
{
    registry_t *dados;
    int page_id;
} Page;

void unloadPage(Page*);

Page *loadPage(FILE*, int, int);

int *indexTable(FILE*, int, int*);

registry_t orderedUpSequencialSearch(FILE*, int, int*, int, int);

registry_t orderedDownSequencialSearch(FILE*, int, int*, int, int);

registry_t indexedSequencialSearch(FILE*, int, int);

int binarySearch(registry_t*, int, int, int, int);

registry_t orderedUpBinarySearch(FILE*, int, int*, int, int);

registry_t orderedUpBinarySearch(FILE*, int, int*, int, int);

registry_t indexedBinarySearch(FILE*, int, int);

#endif