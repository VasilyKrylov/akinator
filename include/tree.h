#ifndef K_TREE_H
#define K_TREE_H

#include <stdio.h>

#include "tree_log.h"
#include "debug.h"

typedef char * treeDataType; // FIXME: use this
const char * const treeDataTypeStr = "char *";
#define TREE_FORMAT_STRING "%s"

const size_t kMaxCommentLen         = 64;
const size_t kTreeMaxLen            = (1UL << 32);

const char treeSaveFileName[]       = "tree.txt";

#define TREE_DO_AND_CHECK(action)           \
        do                                  \
        {                                   \
            int status = action;            \
            DEBUG_VAR("%d", status);        \
                                            \
            if (status != TREE_OK)          \
                return status;              \
        } while (0)                       

#define TREE_DO_AND_CLEAR(action, clearAction)      \
        do                                          \
        {                                           \
            int status = action;                    \
            DEBUG_VAR("%d", status);                \
                                                    \
            if (status != TREE_OK)            \
            {                                       \
                clearAction;                        \
                                                    \
                return status;                      \
            }                                       \
        } while (0)                       

#ifdef PRINT_DEBUG

struct varInfo_t
{
    const char *name = NULL;
    const char *file = NULL;
    int line         = 0;
    const char *func = NULL;
};
// NOTE Change name to VarInfoTree_t (?)
#define TREE_CTOR(treeName) TreeCtor (treeName,                                 \
                                        varInfo_t{.name = #treeName,            \
                                                  .file = __FILE__,             \
                                                  .line = __LINE__,             \
                                                  .func = __func__})
#define TREE_DUMP(treeName, comment) TreeDump (treeName, comment, __FILE__, __LINE__, __func__)
#define NODE_DUMP(nodeName, treeLog, format, ...) NodeDump (nodeName, treeLog,              \
                                                            __FILE__, __LINE__, __func__,   \
                                                            format, __VA_ARGS__)

#define TREE_VERIFY(tree) TreeVerify (tree) 
#else

#define TREE_CTOR(treeName) TreeCtor (treeName) 
#define TREE_VERIFY(tree) TREE_OK;
#define TREE_DUMP(tree, comment) 
#define NODE_DUMP(nodeName, treeLog, format, ...) 

#endif // PRING_DEBUG

struct node_t
{
    treeDataType data;

    node_t *left = NULL;
    node_t *right = NULL;

    // bool dynamicAllocated = 0;

    // bool deleted = false; 
    // for TreeDelete()
};
struct tree_t
{
    node_t *root = NULL;

    size_t size = 0;

    treeLog_t log = {};

#ifdef PRINT_DEBUG
    varInfo_t varInfo = {};
#endif
};

enum treeError_t
{
    TREE_OK                             = 0,
    TREE_ERROR_NULL_STRUCT              = 1 << 0,
    TREE_ERROR_NULL_ROOT                = 1 << 1,
    TREE_ERROR_NULL_DATA                = 1 << 2,
    TREE_ERROR_NOT_ENOUGH_NODES         = 1 << 3,
    TREE_ERROR_TO_MUCH_NODES            = 1 << 4,
    TREE_ERROR_INVALID_NEW_QUESTION     = 1 << 5,
    TREE_ERROR_SAVE_FILE_SYNTAX         = 1 << 6,
    TREE_ERROR_LOAD_INTO_NOT_EMPTY      = 1 << 7,
    TREE_ERROR_INVALID_NODE             = 1 << 8,


    TREE_NODE_FOUND                     = 1 << 9,
    TREE_NODE_NOT_FOUND                 = 1 << 10,

    TREE_ERROR_COMMON                   = 1 << 31
};

int TreeCtor            (tree_t *tree
                         ON_DEBUG (, varInfo_t varInfo));
node_t *NodeCtor        (tree_t *tree);
void NodeFill           (node_t *node, treeDataType data);
void TreeDelete         (node_t *node);
void TreeDtor           (tree_t *tree);
int TreeVerify          (tree_t *tree);
int TreeLoadFromFile    (tree_t *tree, const char *fileName, char **bufer, size_t *bufferLen);
int TreeSaveToFile      (tree_t *tree, const char *fileName);
int NodeSaveToFile      (node_t *node, FILE *file);
bool IsLeaf             (node_t *node);
bool HasBothChildren    (node_t *node);


#endif //K_TREE_H