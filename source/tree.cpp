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
node_t *TreeLoadNode (char *buffer, char **curPos
                      ON_DEBUG(, treeLog_t *treeLog));

// maybe: pass varInfo here for ERROR_LOG
node_t *NodeCtor (char *data)
{
    // assert (data);

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

// void TreeDelete (node_t *node)
// {
//     assert (node);

//     DEBUG_LOG ("node [%p] -> data = \"%s\";", node, node->data);

//     node_t *leftNode = node->left;
//     node_t *rightNode = node->right;

//     DEBUG_LOG ("\tleftNode [%p]", leftNode);
//     DEBUG_LOG ("\trightNode [%p]", rightNode);

//     free (node->data);
//     free (node);
    
//     node = NULL;
    
//     if (leftNode != NULL)
//     {
//         TreeDelete (leftNode);
//     }
//     if (rightNode != NULL)
//     {
//         TreeDelete (rightNode);
//     }
// }

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
        fprintf (file, "%s", " nil");

    if (node->right != NULL)
        NodeSaveToFile (node->right, file);
    else
        fprintf (file, "%s", " nil");

    fprintf (file, "%s", ")");

    return TREE_OK;
}

// check if error really occured
#define SAFE_READ_CHAR(charVar, file) \
    do {                                                    \
        charVar = fgetc (file);                             \
        if (charVar < 0)                                    \
        {                                                   \
            ERROR_PRINT ("Error reading user input - %s",   \
                         strerror (errno));                 \
                                                            \
            return TREE_ERROR_COMMON |                      \
                   COMMON_ERROR_READING_INPUT;              \
        }                                                   \
    } while (0)

int TreeLoadFromFile (tree_t *tree, const char *fileName)
{
    assert (tree);
    assert (fileName);

    // TreeDtor (tree);
    // TREE_CTOR (tree, NULL);

    size_t bufferLen = 0;
    char *buffer = ReadFile (fileName, &bufferLen);
    if (buffer == NULL)
    {
        return TREE_ERROR_COMMON |
               COMMON_ERROR_READING_FILE;
    }
    char *curPos = buffer;

    DEBUG_LOG ("\n========== LOADING TREE FROM \"%s\" ==========\n", fileName);
    DEBUG_LOG ("buffer = \'%s\';", buffer);
    
    free (tree->root);
    tree->root = TreeLoadNode (buffer, &curPos 
                               ON_DEBUG(, &tree->log));

    if (tree->root == NULL)
        return TREE_ERROR_SAVE_FILE_SYNTAX;
    
    free (buffer);

    return TREE_OK;
}


// file must begin with '(', I don't see any problem with it
node_t *TreeLoadNode (char *buffer, char **curPos
                      ON_DEBUG(, treeLog_t *treeLog))
{
    if (**curPos == '(')
    {
        (*curPos)++;

        int readBytes = 0;
        char *data = NULL;
        sscanf (*curPos, "\"%m[^\"]\"%n", &data, &readBytes);
        *curPos += readBytes;

        node_t *node = NodeCtor (data); // to get after "
        if (node == NULL)
            return NULL; // FIXME: how to catch this error?
        
        NODE_DUMP (node, treeLog, "Created new node - \"%s\". \ncurPos = \'%s\'", data, *curPos);

        node->left = TreeLoadNode (buffer, curPos
                                   ON_DEBUG(, treeLog));
        if (node->left != NULL)
            NODE_DUMP (node->left, treeLog, "After creating left subtree. \ncurPos = \'%s\'", *curPos);
        
        node->right = TreeLoadNode (buffer, curPos
                                    ON_DEBUG(, treeLog));
        if (node->right != NULL)
            NODE_DUMP (node->right, treeLog, "After creating right subtree. \ncurPos = \'%s\'", *curPos);

            
        if (**curPos != ')')
        {
            ERROR_LOG ("%s", "Syntax error in tree dump file - missing closing bracket ')'");
            ERROR_LOG ("curPos = \'%s\';", *curPos);
        }

        (*curPos)++;
            
        return node;
    }
    else if (strncmp (*curPos, "nil", sizeof("nil") - 1) == 0)
    {
        *curPos += sizeof ("nil") - 1;

        return NULL;
    }
    else 
    {
        ERROR_LOG ("%s", "Syntax error in tree dump file - uknown beginning of the node");
        ERROR_LOG ("curPos = \'%s\';", *curPos);

        return NULL; // FIXME: how to catch this error
    }
}





// int TreeLoadNode (node_t *node, FILE *file)
// {
//     assert (node);
//     assert (file);

//     // NOTE: make a macro
//     int nodeStart = fgetc (file);
//     SAFE_READ_CHAR (nodeStart, file);

//     if (nodeStart == '"') // looks horrible
//     {
//         int res = fscanf (file, "%m[^\"]%*c", &node->data);
//         DEBUG_LOG ("res - %d", res);
//         if (res != 1)
//         {
//             if (ferror (file))
//                 ERROR_PRINT ("Error reading user input - %s", strerror (errno));
//             else
//                 ERROR_PRINT ("%s", "Not correct string in tree dump file");

//             return TREE_ERROR_COMMON |
//                    COMMON_ERROR_READING_INPUT;
//         }
//     }


//     int leftNodeStart = 0;
//     SAFE_READ_CHAR (leftNodeStart, file);

//     if (nodeStart == '(')
//     {
//         node->left = NodeCtor (NULL);
//         if (node->left == NULL)
//             return TREE_ERROR_COMMON |
//                    COMMON_ERROR_REALLOCATING_MEMORY;
        
//         TreeLoadNode (node->left, file);

//         return TREE_ERROR_INVALID_SAVE_FILE;
//     }
    
// }