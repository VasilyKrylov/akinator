#ifndef K_TREE_H
#define K_TREE_H

#include <stdio.h>

#include "tree_log.h"
#include "debug.h"

typedef double treeDataType;

const size_t kMaxCommentLen         = 64;
const size_t kTreeMaxLen            = (1UL << 32);

const char treeFileName[]           = "tree.txt";

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
#define TREE_CTOR(treeName, data) TreeCtor (&treeName, data,                        \
                                            varInfo_t{.name = #treeName,            \
                                                      .file = __FILE__,             \
                                                      .line = __LINE__,             \
                                                      .func = __func__})
#define TREE_DUMP(treeName, comment) TreeDump (treeName, comment, __FILE__, __LINE__, __func__)

#define TREE_VERIFY(tree) TREE_OK; // FIXME
#else

#define TREE_CTOR(treeName, data) TreeCtor (&treeName, data) // NOTE: removed ;
#define TREE_VERIFY(tree) TREE_OK;
#define TREE_DUMP(tree, comment) 

#endif // PRING_DEBUG

struct node_t
{
    char *data;

    node_t *left = NULL;
    node_t *right = NULL;

    // bool deleted = false; 
    // for TreeDelete()
};
struct tree_t
{
    node_t *root = NULL;

    size_t size = 0;

#ifdef PRINT_DEBUG
    varInfo_t varInfo = {};

    treeLog_t log = {};
#endif
};

enum treeError_t
{
    TREE_OK                             = 0,
    TREE_ERROR_NULL_STRUCT              = 1 << 0,
    TREE_ERROR_NULL_ROOT                = 1 << 1,
    TREE_ERROR_NOT_ENOUGH_NODES         = 1 << 2,
    TREE_ERROR_TO_MUCH_NODES            = 1 << 3,
    TREE_ERROR_INVALID_NEW_QUESTION     = 1 << 4,
    // TREE_ERROR_CYCLE

    TREE_ERROR_COMMON                   = 1 << 31
};

int TreeCtor            (tree_t *tree, char *data
                         ON_DEBUG (, varInfo_t varInfo));
node_t *NodeCtor        (char *data);
void TreeDelete         (node_t *node);
void TreeDtor           (tree_t *tree);
int TreeVerify          (tree_t *tree);
int TreeSaveToFile      (tree_t *tree);
int NodeSaveToFile      (node_t *node, FILE *file);


#endif //K_TREE_H