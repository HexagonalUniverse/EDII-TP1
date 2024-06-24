/*  <src/transparent-counter.c>
    
    (...)    
*/


#include "transparent-counter.h"


#if TRANSPARENT_COUNTER
#include <stdio.h>


struct __the_transparent_counter transparent_counter = { 0 };


/*  Displays the counter in the global transparent-counter. */
void transparent_counter_print(void)
{
#if TRANSPARENT_COUNTER
    printf("Counters:\n");
    printf("\treg:\n\t\t| read = %llu\n\t\t| read = %llu\n",
        (unsigned long long) transparent_counter.reg.read,
        (unsigned long long) transparent_counter.reg.write);

    printf("\tb:\n\t\t| read = %llu\n\t\t| write = %llu\n",
        (unsigned long long) transparent_counter.b.read,
        (unsigned long long) transparent_counter.b.write);

    printf("\tb*:\n\t\t| read = %llu\n\t\t| write = %llu\n",
        (unsigned long long) transparent_counter.bs.read,
        (unsigned long long) transparent_counter.bs.write);

    printf("\tebst:\n\t\t| read = %llu\n\t\t| write = %llu\n",
        (unsigned long long) transparent_counter.ebst.read,
        (unsigned long long) transparent_counter.ebst.write);

    printf("\terbt:\n\t\t| read = %llu\n\t\t| write = %llu\n",
        (unsigned long long) transparent_counter.erbt.read,
        (unsigned long long) transparent_counter.erbt.write);

    printf("\tcomparisons:\n\t\t| search = %llu\n\t\t| build = %llu\n",
        (unsigned long long) transparent_counter.comparisons.search,
        (unsigned long long) transparent_counter.comparisons.build);
#endif
}
#endif