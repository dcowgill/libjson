#include "munit.h"
#include <stdlib.h>

int tests_run = 0;
int tests_failed = 0;

int mu_summarize()
{
    printf("%d of %d tests passed.\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 2 : 0;
}
