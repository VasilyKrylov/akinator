#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>

#include "debug.h"
#include "tree_log.h"
#include "utils.h"
#include "float_math.h"

#include "tree.h"

int TreeCountNodes (node_t *node, size_t size, size_t *nodesCount);

// maybe: pass varInfo here for ERROR_LOG
node_t *NodeCtor (char *data)
{
    assert (data);

    node_t *node = (node_t *) calloc (1, sizeof(node_t));
    if (node == NULL)
    {
        ERROR_LOG ("Error allocating memory for new node - %s", strerror (errno));

        return NULL;
    }

    node->data = data;
    node->left = NULL;
    node->right = NULL;

    return node;
}

int TreeCtor (tree_t *tree, char *data
              ON_DEBUG (, varInfo_t varInfo))
{
    assert (tree);

    tree->root = NodeCtor (data);

#ifdef PRINT_DEBUG
    tree->varInfo = varInfo;

    int status = LogInit (&tree->log);
    if (status != TREE_OK)
        return status;

#endif // PRING_DEBUG

    TREE_DUMP (tree, "After ctor");

    return TREE_VERIFY (tree);
}
void TreeDtor (tree_t *tree)
{
#ifdef PRINT_DEBUG
    fprintf (tree->log.logFile, "%s", "</pre>\n");

    fclose (tree->log.logFile);
#endif // PRINT_DEBUG

    TreeDelete (tree->root);
}

void TreeDelete (node_t *node)
{
    assert (node);

    DEBUG_VAR ("%p", node);
    
    if (node->left != NULL)
    {
        TreeDelete (node->left);
    }
    if (node->right != NULL)
    {
        TreeDelete (node->right);
    }

    free (node->data);
    free (node);

    node = NULL;
}

int TreeVerify (tree_t *tree)
{
    int error = TREE_OK;

    if (tree == NULL)       return TREE_ERROR_NULL_STRUCT;
    if (tree->root == NULL) return TREE_ERROR_NULL_ROOT;

    size_t nodesCount = 0;
    error |= TreeCountNodes (tree->root, tree->size, &nodesCount);

    if (nodesCount < tree->size) error |= TREE_ERROR_NOT_ENOUGH_NODES;
    if (nodesCount > tree->size) error |= TREE_ERROR_TO_MUCH_NODES;

    return error;
}

int TreeCountNodes (node_t *node, size_t size, size_t *nodesCount)
{
    int status = TREE_OK;
    
    *nodesCount += 1;

    if (*nodesCount > size)
        return TREE_ERROR_TO_MUCH_NODES;

    if (node->left != NULL)
        status |= TreeCountNodes (node->left, size, nodesCount);

    if (node->right != NULL)
        status |= TreeCountNodes (node->right, size, nodesCount);

    return status;
}

int TreeSaveToFile (tree_t *tree)
{
    FILE *outputFile = fopen (treeFileName, "w");
    if (outputFile == NULL)
    {
        ERROR_LOG ("Error opening file \"%s\"", treeFileName);
        
        return TREE_ERROR_COMMON |
               COMMON_ERROR_OPENING_FILE;
    }

    int status = NodeSaveToFile (tree->root, outputFile);

    fclose (outputFile);

    return status;
}

int NodeSaveToFile (node_t *node, FILE *file)
{
    // NOTE: maybe add macro for printf to check it's return code
    fprintf (file, "(\"%s\"", node->data);

    if (node->left != NULL)
        NodeSaveToFile (node->left, file);
    else
        fprintf (file, "%s", " nil");

    if (node->right != NULL)
        NodeSaveToFile (node->right, file);
    else
        fprintf (file, "%s", " nil");

    fprintf (file, "%s", ")");

    return TREE_OK;
}