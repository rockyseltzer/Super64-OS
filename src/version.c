#include "version.h"

#define ALTRA64_VERSION "3.6.9-s64.90"

const char *Altra64_GetVersionString(void)
{
    return ALTRA64_VERSION;
}

const char *Altra64_GetBuildDate(void)
{
    return __DATE__;
}
