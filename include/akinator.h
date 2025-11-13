#ifndef K_AKINATOR_H
#define K_AKINATOR_H

#include "tree.h"

#define MAGENTA_COLOR   "\33[35m"
#define COLOR_END       "\33[0m"

#define PRINT(format, ...) printf (MAGENTA_COLOR format COLOR_END, __VA_ARGS__)

enum akinatorError_t
{
    AKINATOR_OK                             = 0,
    AKINATOR_ERROR_WRONG_INPUT              = 1,

    AKINATOR_ERROR_TREE                     = 1 << 30,
    AKINATOR_ERROR_COMMON                   = 1 << 31,
};

struct akinator_t
{
    tree_t tree = {};

    char *buffer = NULL;
    size_t bufferLen = 0;
};

int AkinatorMenu        (akinator_t *akinator);
int AkinatorCtor        (akinator_t *akinator);
int AkinatorDtor        (akinator_t *akinator);
void AkinatorTreeDtor   (akinator_t *akinator);
void AkinatorTreeDelete (akinator_t *akinator, node_t **node);

#endif // K_AKINATOR_H