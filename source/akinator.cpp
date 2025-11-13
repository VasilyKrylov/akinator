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

int AskYesOrNo ();

int AskQuestion     (tree_t *tree, node_t *node);
int AskFinal        (tree_t *tree, node_t *node);
int AddNewCharacter (tree_t *tree, node_t *node, char *userCharacter, char *newQuestion);
int DescribeObject  (tree_t *tree);
int FindCharacter   (node_t *node, 
                     stack_t *currentPath, stack_t *rightPath,
                     char *character);

int AkinatorCtor (akinator_t *akinator)
{
    akinator->buffer = NULL;
    akinator->bufferLen = 0;
    
    int status = TREE_CTOR (&akinator->tree);
    if (status != TREE_OK)
        return AKINATOR_ERROR_TREE |
               status;

    akinator->tree.root = NodeCtor (&akinator->tree);
    if (akinator->tree.root == NULL)
        return AKINATOR_ERROR_COMMON |
               COMMON_ERROR_ALLOCATING_MEMORY;

    NodeFill (akinator->tree.root, strdup ("uknown character"));


    return AKINATOR_OK;
}

int AkinatorDtor (akinator_t *akinator)
{
    AkinatorTreeDtor (akinator);

    free (akinator->buffer);
    akinator->buffer = NULL;
    akinator->bufferLen = 0;

    return AKINATOR_OK;
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
    
    DEBUG_VAR ("%p", (*node)->data);
    DEBUG_VAR ("%p", akinator->buffer);
    DEBUG_VAR ("%p", akinator->buffer + akinator->bufferLen);
    if ((void *)(*node)->data < (void *)akinator->buffer || 
        (void *)(*node)->data > (void *)(akinator->buffer + akinator->bufferLen))
    {
        free ((*node)->data);
    }
    
    free (*node);
    *node = NULL;
}

int AkinatorMenu (akinator_t *akinator)
{
    PRINT ("%s", "Welcome to Akinator!\n");

    while (true)
    {
        // NOTE: maybe do enums
        PRINT  ("Choose the option:\n"
                "\t[0] - Exit\n"
                "\t[1] - Save the tree to \"%s\"\n"
                "\t[2] - Load the tree from \"%s\"\n"
                "\t[3] - Guess a character\n"
                "\t[4] - Dump the tree\n"
                // "\t[3] - Дать определение объекту\n"
                " > ",
                treeSaveFileName, treeSaveFileName);

        DEBUG_LOG ("tree->size = %lu", akinator->tree.size);

        int action = -1;
        int res = scanf ("%d", &action);
        ClearBuffer();
    
        if (res != 1)
        {
            ERROR_PRINT ("%s", "This is not a number, please, try again\n");
            
            continue;
        }
        
        switch (action)
        {
            case 0:
                return TREE_OK;

            case 1:
            {
                int status = TreeSaveToFile (&akinator->tree, treeSaveFileName);
                if (status != TREE_OK)
                    ERROR_PRINT ("%s", "Error saving the tree\n");
                else 
                    PRINT ("%s", "The tree saved successfully\n");
                
                break;
            }

            case 2:
            {
                akinator->tree.size = 0;
                AkinatorTreeDelete (akinator, &akinator->tree.root);

                if (akinator->buffer != NULL)
                    free (akinator->buffer);

                int status = TreeLoadFromFile (&akinator->tree, treeSaveFileName, &akinator->buffer, &akinator->bufferLen);
                if (status != TREE_OK)
                    ERROR_PRINT ("Error loading the tree.\nStatus code = %d", status);
                else 
                    PRINT ("%s", "The tree loaded successfully");

                break;
            }

            case 3:
                GuessCharacter (&akinator->tree);
                TREE_DUMP (&akinator->tree, "After guess");
                break;

            case 4:
            {
                int status = TreeDump (&akinator->tree, "User asked to dump the tree", 
                                       __FILE__, __LINE__, __func__);
                if (status != TREE_OK)
                    ERROR_PRINT ("Error loading the tree.\nStatus code = %d", status);
                else 
                    PRINT ("%s", "The tree loaded successfully");

                break;
            }

            default:
                ERROR_PRINT ("%s", "Uknown command code, try again");
                break;
        }
    }

    return AKINATOR_OK;
}

int AskYesOrNo()
{
    int answer = getchar();
    ClearBuffer();
    
    DEBUG_VAR ("%c", answer);
    
    if (answer < 0)
    {
        ERROR_PRINT ("Error reading user input - %s", strerror (errno));
        
        return AKINATOR_ERROR_COMMON |
               COMMON_ERROR_READING_INPUT;
    }

    answer = tolower (answer);

    if (answer == 'y') return USER_YES;
    if (answer == 'n') return USER_NO;
    
    return USER_UKNOWN;
}


// Recursive version of GuessCharacter. Removed from project

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
        while (leftAttemtts--)
        {
            DEBUG_LOG ("tree->size = %lu", tree->size);

            int status = AskFinal (tree, node);

            if (status == TREE_OK)
                return TREE_OK;
            
            DEBUG_VAR ("%d", status);
        }

        PRINT ("%s", "You've made to much attemts to decsribe character. Please, try again");
    }
    else 
    {
        ERROR_LOG ("node[%p] has only 1 child, this should never happened", node);
        
        return AKINATOR_ERROR_TREE | 
               TREE_ERROR_WRONG_NODE;
    }

    return AKINATOR_OK;
}

// RECURSIVE REALISATION

// int GuessCharacter (tree_t *tree)
// {
//     assert (tree);

//     AskQuestion (tree, tree->root);

//     return TREE_VERIFY (tree);
// }

// int AskQuestion (tree_t *tree, node_t *node)
// {
//     assert (node);
//     assert (tree);
    
//     if (IsLeaf (node))
//     {
//         AskFinal (tree, node);

//         return AKINATOR_OK;
//     }

//     PRINT ("Ваш персонаж %s? (input Y/y or N/n)\n"
//             " > ", node->data);

//     int answer = AskYesOrNo();

//     if (answer == USER_YES)
//     {
//         AskQuestion (tree, node->left);

//         return AKINATOR_OK;
//     }
//     else if (answer == USER_NO)
//     {   
//         AskQuestion (tree, node->right);

//         return AKINATOR_OK;
//     }
//     else if (answer == USER_UKNOWN)
//     {

//         AskQuestion (tree, node);

//         return AKINATOR_OK;
//     }
//     else 
//     {
//         return answer; // some error code here
//     }

//     return TREE_VERIFY (tree);
// }

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

            return AKINATOR_OK;
            break;

        case USER_NO:
        { // TODO: new function
            PRINT ("%s", "Who are you thinking of?\n"
                        " > ");

            char *userCharacter = NULL;
            size_t userCharacterLen = 0;
            int status = SafeReadLine (&userCharacter, &userCharacterLen, stdin);
            if (status != COMMON_ERROR_OK)
                return status;

            PRINT ("The difference between %s and %s that it is ...\n"
                    " > ", userCharacter, node->data);
            
            char *newQuestion = NULL;
            size_t newQuestionLen = 0;
            status = SafeReadLine (&newQuestion, &newQuestionLen, stdin);
            if (status != COMMON_ERROR_OK)
                return status;

            int res = AddNewCharacter (tree, node, userCharacter, newQuestion);

            DEBUG_LOG ("result of adding character = %d;", res);
            
            return res;
            break;
        }
        case USER_UKNOWN:
            ERROR_PRINT ("%s", "Sorry, you are wrong. You need to input Y/y or N/n\n");

            return AKINATOR_ERROR_WRONG_INPUT;

    
        default:
            assert (0 && "this should never happen");
            break;
    }

    return AKINATOR_OK;
}


int AddNewCharacter (tree_t *tree, node_t *node, char *userCharacter, char *newQuestion)
{
    assert (tree);
    assert (node);
    assert (userCharacter);
    assert (newQuestion);

    // NOTE: not the best option solution
    // read about lowercase wide chars in future
    // also I believe what user do not use leading spaces

    if (strncasecmp ("no", newQuestion, sizeof("no") - 1) == 0)
    {
        ERROR_PRINT ("%s", "You can't use no or don't in definition");

        return TREE_ERROR_INVALID_NEW_QUESTION;
    }

    node_t *userNode = NodeCtor (tree);
    NodeFill (userNode, userCharacter);

    if (userNode == NULL)
        return TREE_ERROR_COMMON |
               COMMON_ERROR_ALLOCATING_MEMORY;

    node_t *oldNode  = NodeCtor (tree);
    NodeFill (oldNode, node->data);
    if (userNode == NULL)
        return TREE_ERROR_COMMON |
               COMMON_ERROR_ALLOCATING_MEMORY;

    node->data  = newQuestion;
    node->left  = userNode;
    node->right = oldNode;

    int status = TREE_VERIFY (tree);
    if (status != TREE_OK)
        return AKINATOR_ERROR_TREE |
               status;

    return TREE_OK;
}

int DescribeObject (tree_t *tree)
{
    assert (tree);

    PRINT ("%s", "Description of who you want to get?\n"
                 " > ");

    char *character = NULL;
    size_t characterLen = 0;

    int status = SafeReadLine (&character, &characterLen, stdin);
    if (status != COMMON_ERROR_OK)
        return status;

    stack_t currentPath = {};
    STACK_CREATE (&currentPath, tree->size);

    stack_t rightPath = {};
    STACK_CREATE (&rightPath, tree->size);

    return TREE_OK;
}

// NOTE: hash will be in future version, ded
int FindCharacter (node_t *node, 
                   stack_t *currentPath, stack_t *rightPath,
                   char *character)
{
    if (IsLeaf (node) && strcmp (node->data, character) == 0)
    {
        DEBUG_LOG ("Found \"%s\"", node->data);

        stackDataType value = 0;
        while (StackPop (currentPath, &value) != TRYING_TO_POP_FROM_EMPTY_STACK)
        {
            StackPush (rightPath, value);
        }

    }

    // if (node->left)

    return TREE_OK;
}