#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "tree.h"
#include "tree_log.h"
#include "akinator.h"

int main()
{
    akinator_t akinator = {};

    int status = AkinatorCtor (&akinator);

    DEBUG_VAR ("%d", status);
    
    if (status != TREE_OK)
        return status;
    
    AkinatorMenu (&akinator);

    // r2->right = tree.root;
    // ON_DEBUG (int res = TreeVerify (&tree));
    // DEBUG_LOG ("verify res = %d;", res);

    DEBUG_VAR ("%p", akinator.buffer);
    AkinatorDtor (&akinator);
}
