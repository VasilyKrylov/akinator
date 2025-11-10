#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "tree.h"
#include "tree_log.h"
#include "akinator.h"

int main()
{
    tree_t tree;
    TREE_CTOR (tree, strdup("Животное"));

    node_t *l1 = NodeCtor (strdup("Полторашка"));
    node_t *r1 = NodeCtor (strdup("Любит много спать"));
    node_t *l2 = NodeCtor (strdup("Вадим"));
    node_t *r2 = NodeCtor (strdup("Вова"));
    tree.size = 5;
    
    tree.root->left = l1;
    tree.root->right = r1;

    r1->left = l2;
    r1->right = r2;

    DEBUG_VAR ("%p", tree.root->left->left);
    DEBUG_VAR ("%p\n", l1);

    TREE_DUMP (&tree, "After hand assigning");

    AkinatorMenu (&tree);

    // r2->right = tree.root;5
    ON_DEBUG (int res = TreeVerify (&tree));
    DEBUG_LOG ("verify res = %d;", res);

    TreeDtor (&tree);
}
