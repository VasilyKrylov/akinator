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
    if (status != AKINATOR_OK)
        return AKINATOR_OK;
    

    // node_t *l1 = NodeCtor (strdup("Полторашка"));
    // node_t *r1 = NodeCtor (strdup("любит много спать"));
    // node_t *l2 = NodeCtor (strdup("Вадим"));
    // node_t *r2 = NodeCtor (strdup("Вова"));
    // tree.size = 5;
    
    // tree.root->left = l1;
    // tree.root->right = r1;

    // r1->left = l2;
    // r1->right = r2;

    // DEBUG_VAR ("%p", tree.root->left->left);
    // DEBUG_VAR ("%p\n", l1);

    // TREE_DUMP (&tree, "After hand assigning");

    // char *buffer = NULL;
    // int status = TreeLoadFromFile (&tree, treeSaveFileName, &buffer);
    // if (status != TREE_OK)
    // {
    //     TreeDtor (&tree);
    //     free (buffer);
    //     buffer = NULL;

    //     return status;
    // }

    // TREE_DUMP (&tree, "After loading from text file");

    AkinatorMenu (&akinator);

    // r2->right = tree.root;
    // ON_DEBUG (int res = TreeVerify (&tree));
    // DEBUG_LOG ("verify res = %d;", res);

    DEBUG_VAR ("%p", akinator.buffer);
    AkinatorDtor (&akinator);
}
