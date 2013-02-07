#ifndef __INCLUDED_LIBJSON_UNIT__
#define __INCLUDED_LIBJSON_UNIT__

#include <stdio.h>

#define mu_assert(test)                                 \
    do {                                                \
        if (!(test)) {                                  \
            fprintf(stderr, "TEST FAILED (%s:%d)\n",    \
                    __FILE__, __LINE__);                \
            tests_failed++;                             \
            return;                                     \
        }                                               \
    } while (0)

#define mu_run_test(test) do { test(); tests_run++; } while (0)

extern int tests_run;
extern int tests_failed;

int mu_summarize();

#endif
