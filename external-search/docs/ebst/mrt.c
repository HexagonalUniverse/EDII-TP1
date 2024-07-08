/*  <docs/ebst/mrt.c>

    A showcase program for seeing the procedures by its definition on the report. */


#include <stdio.h>


typedef int R;
typedef struct { int r, x, y; } E;

#define R_SIZE  10
static R Registries[R_SIZE] = { 0 };
static E EBST[R_SIZE] = { 0 };


void recursive_ebst_by_mrt(int i, int j, int k)
{
    const int m = ((j - i) >> 1) + i;
    int x = (m > i) ? (k + 1) : (- 1), 
        y = (j > m) ? (k + m - i + 1) : (- 1);

    if (m > i) 
        recursive_ebst_by_mrt(i, m - 1, k + 1);
    
    if (j > m)
        recursive_ebst_by_mrt(m + 1, j, k + 1 + m - i);
    
    EBST[k - 1] = (E) { Registries[m - 1], x, y };
    printf("E[%d] = (%d, %d, %d)\n",
        k, Registries[m - 1], x, y);
}


int main(void)
{
    for (int i = 0; i < R_SIZE; i++) {        
        Registries[i] = i + 1;
        printf("%d -> ", Registries[i]);
    }
    printf("\n");

    recursive_ebst_by_mrt(0, R_SIZE - 1, 0);
    for (int i = 1; i <= R_SIZE; i++) {
        printf("(%d, %d, %d) -> ", 
            EBST[i].r, EBST[i].x, EBST[i].y);
    }
    printf("\n");

    return 0;
}

