#include <errno.h>
#include <string.h>
#include <cstdio>
#include <stdlib.h>

void errorCheck(int retVal, int fd, const char* additionalMessage)
{
    if(retVal != -1)
    {
        return;
    }
    dprintf(fd, "Encountered error: %s\n", strerror(errno));
    exit(-1);
}