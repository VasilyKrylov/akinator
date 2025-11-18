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

static int TreeCountNodes       (node_t *node, size_t size, size_t *nodesCount);
static int TreeLoadNode         (tree_t *tree, node_t **node,
                                 char *buffer, char **curPos);

static int TreeLoadNodeAndFill  (tree_t *tree, node_t **node,
                                 char *buffer, char **curPos);

int TreeLoadSubNodes            (tree_t *tree, node_t **node,
                                 char *buffer, char **curPos);

// maybe: pass varInfo here for ERROR_LOG
node_t *NodeCtor (tree_t *tree)
{
    assert (tree);

    node_t *node = (node_t *) calloc (1, sizeof(node_t));
    if (node == NULL)
    {
        ERROR_LOG ("Error allocating memory for new node - %s", strerror (errno));

        return NULL;
    }

    DEBUG_LOG ("tree->size = %lu", tree->size);
    tree->size += 1;
    DEBUG_LOG ("tree->size = %lu", tree->size);

    node->data = NULL;
    node->left = NULL;
    node->right = NULL;

    return node;
}
void NodeFill (node_t *node, treeDataType data)
{
    assert (node);
    assert (data);

    // NOTE: check node->data or not?

    node->data = data;
}

int TreeCtor (tree_t *tree
              ON_DEBUG (, varInfo_t varInfo))
{
    assert (tree);

    tree->root = NULL;
    tree->size = 0;

    ON_DEBUG (
        tree->varInfo = varInfo;
    );

    int status = LogInit (&tree->log);
    DEBUG_VAR ("%d", status);
    if (status != TREE_OK)
        return status;


    return TREE_OK;
}

void TreeDtor (tree_t *tree)
{
    fprintf (tree->log.logFile, "%s", "</pre>\n");

    fclose (tree->log.logFile);

    TreeDelete (tree->root);
}

// TODO: занулять информацию и использовать это вместо isValid
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

    // if (node->dynamicAllocated)
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

    DEBUG_LOG ("nodesCount = %lu", nodesCount);
    DEBUG_LOG ("tree->size = %lu", tree->size);

    return error;
}

int TreeCountNodes (node_t *node, size_t size, size_t *nodesCount)
{
    int status = TREE_OK;
    
    *nodesCount += 1;

    if (node->data == NULL)
        status |= TREE_ERROR_NULL_DATA;

    if (*nodesCount > size)
        return status|
               TREE_ERROR_TO_MUCH_NODES;

    if (node->left != NULL)
        status |= TreeCountNodes (node->left, size, nodesCount);

    if (node->right != NULL)
        status |= TreeCountNodes (node->right, size, nodesCount);

    return status;
}

int TreeSaveToFile (tree_t *tree, const char *fileName)
{
    FILE *outputFile = fopen (fileName, "w");
    if (outputFile == NULL)
    {
        ERROR_LOG ("Error opening file \"%s\"", fileName);
        
        return TREE_ERROR_COMMON |
               COMMON_ERROR_OPENING_FILE;
    }

    int status = NodeSaveToFile (tree->root, outputFile);

    fclose (outputFile);

    return status;
}

int NodeSaveToFile (node_t *node, FILE *file)
{
    assert (node);
    assert (file);

    // NOTE: maybe add macro for printf to check it's return code
    fprintf (file, "(\"%s\"", node->data);

    if (node->left != NULL)
        NodeSaveToFile (node->left, file);
    else
        fprintf (file, "%s", "nil");

    if (node->right != NULL)
        NodeSaveToFile (node->right, file);
    else
        fprintf (file, "%s", "nil");

    fprintf (file, "%s", ")");

    return TREE_OK;
}


int TreeLoadFromFile (tree_t *tree, const char *fileName, char **buffer, size_t *bufferLen)
{
    assert (tree);
    assert (fileName);

    DEBUG_PRINT ("\n========== LOADING TREE FROM \"%s\" ==========\n", fileName);

    if (tree->root != NULL)
    {
        ERROR_LOG ("%s", "TREE_ERROR_LOAD_INTO_NOT_EMPTY");
        
        return TREE_ERROR_LOAD_INTO_NOT_EMPTY;
    }
    
    *buffer = ReadFile (fileName, bufferLen);
    if (buffer == NULL)
    {
        return TREE_ERROR_COMMON |
               COMMON_ERROR_READING_FILE;
    }
    char *curPos = *buffer;
    DEBUG_LOG ("*buffer = \'%p\';", *buffer);
    DEBUG_LOG ("*buffer = \'%s\';", *buffer);
    
    int status = TreeLoadNode (tree, &tree->root, *buffer, &curPos);

    if (status != TREE_OK)
    {
        ERROR_LOG ("%s", "Error in TreeLoadNode()");

        return TREE_ERROR_SAVE_FILE_SYNTAX;
    }

    TREE_DUMP (tree, "%s", "After load");

    return TREE_OK;
}


// file must begin with '(', I don't see any problem with it
// TODO: make a new function
int TreeLoadNode (tree_t *tree, node_t **node,
                  char *buffer, char **curPos)
{
    if (**curPos == '(')
    {
        *node = NodeCtor (tree); 
        if (node == NULL)
            return TREE_ERROR_CREATING_NODE;

        int status = TreeLoadNodeAndFill (tree, node, buffer, curPos);
        
        return status;
    }
    else if (strncmp (*curPos, "nil", sizeof("nil") - 1) == 0)
    {
        *curPos += sizeof ("nil") - 1;

        *node = NULL;

        return TREE_OK;
    }
    else 
    {
        ERROR_LOG ("%s", "Syntax error in tree dump file - uknown beginning of the node");
        ERROR_LOG ("curPos = \'%s\';", *curPos);

        return TREE_ERROR_SYNTAX_IN_SAVE_FILE;
    }
}

int TreeLoadNodeAndFill (tree_t *tree, node_t **node,
                         char *buffer, char **curPos)
{
    assert (tree);
    assert (node);
    assert (buffer);
    assert (curPos);
    assert (*curPos);

    (*curPos)++; // move after '('

    char *data = *curPos + 1; // + 1 to get after "
    
    NodeFill (*node, data);
    
    int readBytes = 0;
    sscanf (*curPos, "\"%*[^\"]\"%n", &readBytes); // NOTE: make version without this sscanf
    (*curPos)[readBytes - 1] = 0;

    DEBUG_VAR ("%s", data);

    *curPos += readBytes;
    
    NODE_DUMP (*node, &tree->log, "Created new node - \"%s\". \n"
                                  "curPos = \'%s\'", *curPos + 1, *curPos);

    int status = TreeLoadSubNodes (tree, node, buffer, curPos);
    if (status != TREE_OK)
        return status;
    
    if (**curPos != ')')
    {
        ERROR_LOG ("%s", "Syntax error in tree dump file - missing closing bracket ')'");
        ERROR_LOG ("curPos = \'%s\';", *curPos);

        return TREE_ERROR_SYNTAX_IN_SAVE_FILE;
    }

    (*curPos)++;

    return TREE_OK;
}
int TreeLoadSubNodes (tree_t *tree, node_t **node,
                      char *buffer, char **curPos)
{
    int status = TreeLoadNode (tree, &(*node)->left, buffer, curPos);
    if (status != TREE_OK)
        return status;

    if ((*node)->left != NULL)
    {
        NODE_DUMP ((*node)->left, &tree->log, "After creating left subtree. \n"
                                            "curPos = \'%s\'", *curPos);
    }
    
    status = TreeLoadNode (tree, &(*node)->right, buffer, curPos);
    if (status != TREE_OK)
        return status;

    if ((*node)->right != NULL)
    {
        NODE_DUMP ((*node)->right, &tree->log, "After creating right subtree. \n"
                                            "curPos = \'%s\'", *curPos);
    }
    
    return TREE_OK;
}

bool IsLeaf (node_t *node)
{
    assert (node);

    return node->left == NULL && node->right == NULL;
}

bool HasBothChildren (node_t *node)
{
    assert (node);

    return node->left != NULL && node->right != NULL;
}

bool HasOneChild (node_t *node)
{
    assert (node);

    return !IsLeaf (node) && !HasBothChildren (node);
}
