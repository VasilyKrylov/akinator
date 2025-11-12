#ifndef K_AKINATOR_H
#define K_AKINATOR_H

#include "tree.h"

#define MAGENTA_COLOR     "\33[35m"
#define COLOR_END       "\33[0m"

#define PRINT(format, ...) printf (MAGENTA_COLOR format COLOR_END, __VA_ARGS__)

enum akinatorError_t
{
    AKINATOR_OK                             = 0,
    AKINATOR_ERROR_WRONG_INPUT              = 1,

    AKINATOR_ERROR_TREE                     = 1 << 30,
    AKINATOR_ERROR_COMMON                   = 1 << 31,
};

int GuessCharacter (tree_t *tree);
int AkinatorMenu (tree_t *tree);

#endif // K_AKINATOR_H