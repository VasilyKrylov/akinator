#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "akinator.h"

#include "tree.h"
#include "utils.h"
#include "stack.h"

enum userAnswer_t {
    USER_YES    = 1,
    USER_NO     = 0,
    USER_UKNOWN = -1
};

enum nodeDirection
{
    NODE_DIR_UKNOWN = -1,
    NODE_DIR_LEFT  = 1,
    NODE_DIR_RIGHT = 0
};

enum MenuActoins
{
    M_EXIT      = 0,
    M_SAVE      = 1,
    M_LOAD      = 2,
    M_GUESS     = 3,
    M_DUMP      = 4,
    M_DESCRIBE  = 5,
    M_COMPARE   = 6,
};

static int AskYesOrNo ();

// int AskQuestion         (tree_t *tree, node_t *node);
static int GuessCharacter          (tree_t *tree);
static int AskFinal                (tree_t *tree, node_t *node);
static int AskDifferenceAndAdd     (tree_t *tree, node_t *node);
static int AddNewCharacter         (tree_t *tree, node_t *node, char *userCharacter, char *newQuestion);

static int DescribeCharacter       (tree_t *tree);
static int FindCharacterAndPrintError       (tree_t *tree, stack_t *nodePath, char *character);
static int FindCharacter           (node_t *node, 
                                    stack_t *nodePath,
                                    const char *character);
static int PrintDecription         (node_t *node,  stack_t *nodePath,
                                    char *character);
static int CompareCharacters       (tree_t *tree);
void CompareCharactersClear        (stack_t *firstPath, stack_t *secondPath,
                                    char **firstCharacter, char **secondCharacter);
static int GetCharactersToCompare  (char **firstCharacter, char **secondCharacter);
bool IsSameDirection               (stack_t *firstPath, stack_t *secondPath);
static int PrintCommonDescription  (node_t **lca, stack_t *firstPath, stack_t *secondPath);
static int MoveAndPrint            (node_t **node, int direction);

static int MenuGetAction           (int *action);

static int MenuDumpTree            (akinator_t *akinator);
static int MenuSaveTree            (akinator_t *akinator);
static int MenuLoadTree            (akinator_t *akinator);
static int MenuGuessCharacter      (akinator_t *akinator);
static int MenuDescribeCharacter   (akinator_t *akinator);
static int MenuCompareCharacters   (akinator_t *akinator);

int AkinatorCtor (akinator_t *akinator)
{
    akinator->buffer = NULL;
    akinator->bufferLen = 0;
    
    int status = TREE_CTOR (&akinator->tree);
    if (status != TREE_OK)
        return status;

    akinator->tree.root = NodeCtor (&akinator->tree);
    if (akinator->tree.root == NULL)
        return TREE_ERROR_CREATING_NODE;

    NodeFill (akinator->tree.root, strdup ("uknown character"));


    return TREE_OK;
}

int AkinatorDtor (akinator_t *akinator)
{
    DEBUG_VAR ("%p", akinator->buffer);
    DEBUG_VAR ("%p", akinator->buffer + akinator->bufferLen);

    AkinatorTreeDtor (akinator);

    free (akinator->buffer);
    akinator->buffer = NULL;
    akinator->bufferLen = 0;

    return TREE_OK;
}

void AkinatorTreeDtor (akinator_t *akinator)
{
    fprintf (akinator->tree.log.logFile, "%s", "</pre>\n");

    fclose (akinator->tree.log.logFile);

    AkinatorTreeDelete (akinator, &akinator->tree.root);
}

void AkinatorTreeDelete (akinator_t *akinator, node_t **node)
{
    assert (node);
    assert (*node);
    
    if ((*node)->left != NULL)
    {
        AkinatorTreeDelete (akinator, &(*node)->left);
    }
    if ((*node)->right != NULL)
    {
        AkinatorTreeDelete (akinator, &(*node)->right);
    }
    
    DEBUG_VAR ("\t%p", (*node)->data);
    if ((void *)(*node)->data < (void *)akinator->buffer || 
        (void *)(*node)->data > (void *)(akinator->buffer + akinator->bufferLen))
    {
        free ((*node)->data);
    }
    
    free (*node);
    *node = NULL;
}

int MenuGetAction (int *action)
{
    PRINT  ("\n"
            "Choose the option:\n"
            "\t[%d] - Exit\n"
            "\t[%d] - Save the tree to \"%s\"\n"
            "\t[%d] - Load the tree from \"%s\"\n"
            "\t[%d] - Guess character\n"
            "\t[%d] - Dump the tree\n"
            "\t[%d] - Describe the character\n"
            "\t[%d] - Compare characters\n"
            " > ",
            M_EXIT, 
            M_SAVE, treeSaveFileName,
            M_LOAD, treeSaveFileName,
            M_GUESS, 
            M_DUMP, 
            M_DESCRIBE,
            M_COMPARE);

    int res = scanf ("%d", action);
    ClearBuffer();

    if (res != 1)
    {
        ERROR_PRINT ("%s", "This is not a number, please, try again\n");
        
        return TREE_ERROR_COMMON |
               COMMON_ERROR_WRONG_USER_INPUT;
    }

    return TREE_OK;
}

int AkinatorMenu (akinator_t *akinator)
{
    PRINT ("%s", "Welcome to Akinator!\n");

    while (true)
    {
        int action = -1;
        int status = MenuGetAction (&action);
        if (status != TREE_OK)
            continue;
        
        switch (action) 
        {
            case M_EXIT: PRINT ("%s", "Goodbye!\n");         return TREE_OK;

            case M_SAVE:        status = MenuSaveTree           (akinator);     break;
            case M_LOAD:        status = MenuLoadTree           (akinator);     break;
            case M_GUESS:       status = MenuGuessCharacter     (akinator);     break;
            case M_DUMP:        status = MenuDumpTree           (akinator);     break;
            case M_DESCRIBE:    status = MenuDescribeCharacter  (akinator);     break;
            case M_COMPARE:     status = MenuCompareCharacters  (akinator);     break;
            
            default:
                ERROR_PRINT ("%s", "Uknown command code, try again");
                break;
        }

        if (status != TREE_OK)
        {
            DEBUG_LOG ("Error occured\n"
                       "Status code = %d", status);
        }
        else
        {
            DEBUG_LOG ("%s", "Action performed successfully!\n");
        }
    }

    return TREE_OK;
}

int MenuSaveTree (akinator_t *akinator)
{
    return TreeSaveToFile (&akinator->tree, treeSaveFileName);
}

int MenuLoadTree (akinator_t *akinator)
{
    akinator->tree.size = 0;
    AkinatorTreeDelete (akinator, &akinator->tree.root);

    free (akinator->buffer);
    akinator->buffer = NULL;

    int status = TreeLoadFromFile (&akinator->tree, treeSaveFileName, &akinator->buffer, &akinator->bufferLen);

    return status;
}

int MenuDumpTree (akinator_t *akinator)
{
    return TreeDump (&akinator->tree, "User asked to dump the tree", 
                     __FILE__, __LINE__, __func__);
}

int MenuGuessCharacter (akinator_t *akinator)
{
    return GuessCharacter (&akinator->tree);
}

int MenuDescribeCharacter (akinator_t *akinator)
{
    return DescribeCharacter (&akinator->tree);
}

int MenuCompareCharacters   (akinator_t *akinator)
{
    return CompareCharacters (&akinator->tree);
}

int AskYesOrNo()
{
    int answer = getchar();
    ClearBuffer();
    
    DEBUG_VAR ("%c", answer);
    
    if (answer < 0)
    {
        ERROR_PRINT ("Error reading user input - %s", strerror (errno));
        
        return TREE_ERROR_COMMON |
               COMMON_ERROR_READING_INPUT;
    }

    answer = tolower (answer);

    if (answer == 'y') return USER_YES;
    if (answer == 'n') return USER_NO;
    
    return USER_UKNOWN;
}

int GuessCharacter (tree_t *tree)
{
    assert (tree);
    
    DEBUG_LOG ("tree->size = %lu", tree->size);

    node_t *node = tree->root;

    while (HasBothChildren (node))
    {
        PRINT ("Is it %s? (input Y/y or N/n)\n"
               " > ", node->data);
        
        int answer = AskYesOrNo();

        switch (answer)
        {
            case USER_YES:
                node = node->left;
                break;

            case USER_NO:
                node = node->right;
                break;

            case USER_UKNOWN:
                ERROR_PRINT ("%s", "Sorry, you are wrong. You need to input Y/y or N/n\n");
                break;
    
            default:
                assert (0 && "this should never happen");
                break;
        }
    }

    if (IsLeaf (node)) 
    {
        int leftAttemtts = 5;
        int status = TREE_OK;

        while (leftAttemtts--)
        {
            DEBUG_LOG ("tree->size = %lu", tree->size);

            status = AskFinal (tree, node);

            if (status == TREE_OK)
                return TREE_OK;
            
            DEBUG_VAR ("%d", status);
        }

        PRINT ("%s", "You've made to much attemts to decsribe character. Please, try again");

        return status;
    }
    else 
    {
        ERROR_LOG ("node[%p] has only 1 child, this should never happened", node);
        
        return TREE_ERROR_INVALID_NODE;
    }

    return TREE_OK;
}

int AskFinal (tree_t *tree, node_t *node)
{
    assert (node);

    PRINT ("Are you thinking about %s? (input Y/y or N/n)\n"
            " > ", node->data);

    int answer = AskYesOrNo();

    switch (answer)
    {
        case USER_YES:        
            PRINT ("%s", "I won. \n");
            break;

        case USER_NO:
            return AskDifferenceAndAdd (tree, node);
        
        case USER_UKNOWN:
            ERROR_PRINT ("%s", "Sorry, you are wrong. You need to input Y/y or N/n\n");

            return TREE_ERROR_COMMON | 
                   COMMON_ERROR_WRONG_USER_INPUT;
        
        case TREE_ERROR_COMMON | COMMON_ERROR_READING_INPUT:
            return answer;
            
        default:
            assert (0 && "this should never happen");
            break;
    }

    return TREE_OK;
}

int AskDifferenceAndAdd (tree_t *tree, node_t *node)
{
    assert (tree);
    assert (node);

    PRINT ("%s", "Who are you thinking of?\n"
                " > ");

    char *userCharacter = NULL;
    int status = SafeReadLine (&userCharacter);

    if (status != COMMON_ERROR_OK)
        return TREE_ERROR_COMMON | status;

    PRINT ("The difference between %s and %s that it is ...\n"
            " > ", userCharacter, node->data);
    
    char *newQuestion = NULL;
    status = SafeReadLine (&newQuestion);

    if (status != COMMON_ERROR_OK)
        return TREE_ERROR_COMMON | status;

    int res = AddNewCharacter (tree, node, userCharacter, newQuestion);

    DEBUG_LOG ("result of adding character = %d;", res);
    
    return res;
}

int AddNewCharacter (tree_t *tree, node_t *node, char *userCharacter, char *newQuestion)
{
    assert (tree);
    assert (node);
    assert (userCharacter);
    assert (newQuestion);

    if (strncasecmp ("no", newQuestion, sizeof("no") - 1) == 0)
    {
        ERROR_PRINT ("%s", "You can't use no or don't in definition");

        return TREE_ERROR_INVALID_NEW_QUESTION;
    }

    node_t *userNode = NodeCtor (tree);
    if (userNode == NULL)
        return TREE_ERROR_CREATING_NODE;
    
    NodeFill (userNode, userCharacter);

    node_t *oldNode  = NodeCtor (tree);
    if (userNode == NULL)
        return TREE_ERROR_CREATING_NODE;
    
    NodeFill (oldNode, node->data);

    node->data  = newQuestion;
    node->left  = userNode;
    node->right = oldNode;

    int status = TREE_VERIFY (tree);
    if (status != TREE_OK)
        return status;

    return TREE_OK;
}

int DescribeCharacter (tree_t *tree)
{
    assert (tree);

    PRINT ("%s", "Description of what you want to get?\n"
                 " > ");

    char *character = NULL;

    int status = SafeReadLine (&character);
    if (status != COMMON_ERROR_OK)
        return TREE_ERROR_COMMON |
               status;

    stack_t nodePath = {};
    STACK_CREATE (&nodePath, tree->size);

    status = FindCharacterAndPrintError (tree, &nodePath, character);
    if (status != TREE_NODE_FOUND)
    {
        StackDtor (&nodePath);
        free (character);

        return status;
    }

    status = PrintDecription (tree->root, &nodePath, character);
    if (status != TREE_OK)
    {
        StackDtor (&nodePath);
        free (character);

        return status;
    }

    StackDtor (&nodePath);
    free (character);

    return TREE_OK;
}

int FindCharacterAndPrintError (tree_t *tree, stack_t *nodePath, char *character)
{
    assert (tree);
    assert (character);

    int status = FindCharacter (tree->root, nodePath, character);
    STACK_DUMP (*nodePath, "Just to see the stack with path from root to node");

    switch (status)
    {
        case TREE_ERROR_INVALID_NODE:
            ERROR_PRINT ("%s", "Error in tree structure.\n"
                               "Please, make dump and contact with the developer.");
            break;

        case TREE_NODE_NOT_FOUND:
            ERROR_PRINT ("I don't know character \"%s\"\n"
                         "You can add it using \"Guess character\"", 
                         character);
            break;

        case TREE_NODE_FOUND:
            break;
    
        default:
            assert (0 && "This should never happen");
            break;
    }

    return status;
}

int FindCharacter (node_t *node,
                   stack_t *nodePath,
                   const char *character)
{
    assert (node);
    assert (nodePath);
    assert (character);
    
    if (IsLeaf (node))
    {
        if (strcmp (node->data, character) == 0)
        {
            DEBUG_LOG ("Found [%p] \"%s\"", node, node->data);
            
            return TREE_NODE_FOUND;
        }
        
        return TREE_NODE_NOT_FOUND;
    }

    if (HasOneChild (node))
    {
        ERROR_LOG ("Node[%p] with data \"%s\" has only 1 child", node, node->data);
        ERROR_LOG ("\t node->left  = [%p]", node->left);
        ERROR_LOG ("\t node->right = [%p]", node->right);
    
        return TREE_ERROR_INVALID_NODE;
    }

    int status = FindCharacter (node->left, nodePath, character);
    if (status == TREE_NODE_FOUND)
    {
        StackPush (nodePath, NODE_DIR_LEFT);

        return status;
    }

    status = FindCharacter (node->right, nodePath, character);
    if (status == TREE_NODE_FOUND)
    {
        StackPush (nodePath, NODE_DIR_RIGHT);

        return status;
    }
    
    return TREE_NODE_NOT_FOUND;
}

// FIXME: FIXME
int PrintDecription (node_t *node,  stack_t *nodePath,
                     char *character)
{
    assert (node);
    assert (nodePath);
    assert (character);

    PRINT ("%s - ", character);

    while (HasBothChildren (node) && nodePath->size != 0)
    {
        int direction = 0;
        int status = StackPop (nodePath, &direction);

        if (status != OK)
        {
            StackPrintError (status);

            return status;
        }

        status = MoveAndPrint (&node, direction);
        if (status != TREE_OK)
            return status;

        if (HasBothChildren (node)) 
            PRINT (", ");
    }
    PRINT ("\n");

    int status = TREE_OK;
    if (!IsLeaf (node))
    {
        ERROR_LOG ("StackPath is empty, but node[%p] isn't leaf", node);
        ERROR_LOG ("\t node->left  = [%p]", node->left);
        ERROR_LOG ("\t node->right = [%p]", node->right);

        status |= TREE_ERROR_INVALID_NODE;
    }
    if (nodePath->size != 0)
    {
        StackDump (nodePath, "Stack should be empty", 
                    __FILE__, __LINE__, __func__);

        ERROR_LOG ("StackPath is empty, but node[%p] isn't leaf", node);
        ERROR_LOG ("\t node->left  = [%p]", node->left);
        ERROR_LOG ("\t node->right = [%p]", node->right);

        status |= TREE_ERROR_INVALID_NODE;
    }

    return status;
}

int CompareCharacters (tree_t *tree)
{
    assert (tree);

    stack_t firstPath = {};
    STACK_CREATE (&firstPath, tree->size);

    stack_t secondPath = {};
    STACK_CREATE (&secondPath, tree->size);

    char *firstCharacter = NULL;
    char *secondCharacter = NULL;

    int status = GetCharactersToCompare (&firstCharacter, &secondCharacter);
    if (status != TREE_OK)
    {
        CompareCharactersClear (&firstPath, &secondPath, &firstCharacter, &secondCharacter);

        return status;
    }

    status = FindCharacterAndPrintError (tree, &firstPath, firstCharacter);
    if (status != TREE_NODE_FOUND)
    {
        CompareCharactersClear (&firstPath, &secondPath, &firstCharacter, &secondCharacter);

        return status;
    }

    status = FindCharacterAndPrintError (tree, &secondPath, secondCharacter);
    if (status != TREE_NODE_FOUND)
    {
        CompareCharactersClear (&firstPath, &secondPath, &firstCharacter, &secondCharacter);

        return status;
    }

    node_t *lca = tree->root;
    PrintCommonDescription (&lca, &firstPath, &secondPath);

    PrintDecription (lca, &firstPath, firstCharacter);
    PrintDecription (lca, &secondPath, secondCharacter);

    CompareCharactersClear (&firstPath, &secondPath, &firstCharacter, &secondCharacter);

    return TREE_OK;
}

void CompareCharactersClear (stack_t *firstPath, stack_t *secondPath,
                             char **firstCharacter, char **secondCharacter)
{
    assert (firstPath);
    assert (secondPath);

    assert (firstCharacter);
    assert (secondCharacter);

    assert (*firstCharacter);
    assert (*secondCharacter);

    StackDtor (firstPath);
    StackDtor (secondPath);

    free (*firstCharacter);
    free (*secondCharacter);

    *firstCharacter  = NULL;
    *secondCharacter = NULL;
}

int GetCharactersToCompare (char **firstCharacter, char **secondCharacter)
{
    assert (firstCharacter);
    assert (secondCharacter);
    
    PRINT ("%s", "First character to compare:\n"
                  " > ");

    int status = SafeReadLine (firstCharacter);
    if (status != COMMON_ERROR_OK)
        return TREE_ERROR_COMMON |
               status;

    PRINT ("%s", "Second character to compare:\n"
                  " > ");

    status = SafeReadLine (secondCharacter);
    if (status != COMMON_ERROR_OK)
        return TREE_ERROR_COMMON |
               status;

    return TREE_OK;
}

int PrintCommonDescription (node_t **lca, stack_t *firstPath, stack_t *secondPath)
{
    assert (lca);
    assert (firstPath);
    assert (secondPath);

    if (IsSameDirection (firstPath, secondPath))
    {
        PRINT ("Both characters are: ");
    }
    else
    {
        PRINT ("Both characters don't have common parameters\n");

        return TREE_OK;
    }

    while (firstPath->size != 0 && secondPath->size != 0)
    {
        stackDataType firstDir  = NODE_DIR_UKNOWN;
        stackDataType secondDir = NODE_DIR_UKNOWN;

        StackPop (firstPath,  &firstDir);
        StackPop (secondPath, &secondDir);

        DEBUG_VAR (STACK_FORMAT_STRING, firstDir);
        DEBUG_VAR (STACK_FORMAT_STRING, secondDir);

        int status = MoveAndPrint (lca, firstDir);
        if (status != TREE_OK)
            return status;

        if (IsSameDirection (firstPath, secondPath))
            PRINT (", ");
        else 
            break;
    }

    PRINT ("\n");

    return TREE_OK;
}

bool IsSameDirection (stack_t *firstPath, stack_t *secondPath)
{
    assert (firstPath);
    assert (secondPath);

    stackDataType firstDir  = NODE_DIR_UKNOWN;
    stackDataType secondDir = NODE_DIR_UKNOWN;

    StackTop (firstPath,  &firstDir);
    StackTop (secondPath, &secondDir);

    return firstDir == secondDir;
}

int MoveAndPrint (node_t **node, int direction)
{
    assert (node);
    assert (*node);

    if (!HasBothChildren (*node)) 
        return TREE_ERROR_INVALID_NODE;

    if (direction == NODE_DIR_LEFT)
    {
        PRINT ("%s", (*node)->data);

        *node = (*node)->left;
    }
    else if (direction == NODE_DIR_RIGHT)
    {
        PRINT ("not %s", (*node)->data);

        *node = (*node)->right;
    }
    else
    {
        ERROR_LOG ("Value poped from stack path is " STACK_FORMAT_STRING ", but only %d and %d are allowed",
                    direction, NODE_DIR_LEFT, NODE_DIR_RIGHT);
        
        return TREE_ERROR_INVALID_PATH;
    }
    
    return TREE_OK;
}