/*  <src/transparent-counter.c>
    
    (...)    
*/


#include "transparent-counter.h"


#if TRANSPARENT_COUNTER
#include <stdio.h>


extern struct __the_transparent_counter transparent_counter = { 0 };


/*  Displays the counter in the global transparent-counter. */
void transparent_counter_print(void)
{
#if TRANSPARENT_COUNTER
    printf("Counters:\n");
    printf("\treg:\n\t\t| read = %" PRIu64 "\n\t\t| read = %" PRIu64 "\n",
        transparent_counter.reg.read,
        transparent_counter.reg.write);

    printf("\tb:\n\t\t| read = %" PRIu64 "\n\t\t| write = %" PRIu64 "\n",
        transparent_counter.b.read,
        transparent_counter.b.write);

    printf("\tb*:\n\t\t| read = %" PRIu64 "\n\t\t| write = %" PRIu64 "\n",
        transparent_counter.bs.read,
        transparent_counter.bs.write);

    printf("\tebst:\n\t\t| read = %" PRIu64 "\n\t\t| write = %" PRIu64 "\n",
        transparent_counter.ebst.read,
        transparent_counter.ebst.write);

    printf("\terbt:\n\t\t| read = %" PRIu64 "\n\t\t| write = %" PRIu64 "\n",
        transparent_counter.erbt.read,
        transparent_counter.erbt.write);

    printf("\tcomparisons:\n\t\t| search = %" PRIu64 "\n\t\t| build = %" PRIu64 "\n",
        transparent_counter.comparisons.search,
        transparent_counter.comparisons.build);
#endif
}
#endif