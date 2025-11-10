#ifndef K_AKINATOR_H
#define K_AKINATOR_H

#include "tree.h"

enum akinatorError_t
{
    AKINATOR_OK                             = 0,

    AKINATOR_ERROR_COMMON                   = 1 << 31
};

int GuessCharacter (tree_t *tree);
int AkinatorMenu (tree_t *tree);

#endif // K_AKINATOR_H