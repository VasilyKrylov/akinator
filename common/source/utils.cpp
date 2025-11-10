#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "utils.h"

#include "debug.h"

int SafeMkdir (const char *fileName)
{
    struct stat st = {};
    if (stat (fileName, &st) != 0)
    {
        // rwxrwxr-x
        // idk WHY, but when I use rw-rw-r-- I can't create folder inside folder
        int status = mkdir (fileName, S_IRUSR | S_IWUSR | S_IXUSR |
                                      S_IRGRP | S_IWGRP | S_IXGRP |
                                      S_IROTH |           S_IXOTH);
        if (status != 0)
        {
            ERROR_LOG ("Error creating folder \"%s\" : %s",
                       fileName, strerror(errno));

            return COMMON_ERROR_CREATING_FILE;
        }
    }

    return COMMON_ERROR_OK;
}


void ClearBuffer() {
    int trash = '\0';

    while (trash != '\n' && trash != EOF) 
    {
        trash = getchar ();
    }
}