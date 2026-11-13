#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void __assert_func(const char *file, int line, const char *func, const char *expr)
{
    printf("assertion failed: %s (%s:%d in %s)\n", expr ? expr : "?", file ? file : "?", line, func ? func : "?");
    for (;;)
    {
    }
}
