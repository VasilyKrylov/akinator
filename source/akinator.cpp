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
bool IsLeaf         (node_t *node);
bool HasBothChildren(node_t *node);
int AddNewCharacter (tree_t *tree, node_t *node, char *userCharacter, char *newQuestion);
int DescribeObject  (tree_t *tree);
int FindCharacter   (node_t *node, 
                     stack_t *currentPath, stack_t *rightPath,
                     char *character);
                     
// NOTE: question about errno // FIXME: why macro // TODO: remove if errno != 0
#define SAFE_READ_LINE(str, strLen, stream);                                        \
        do                                                                          \
        {                                                                           \
            ssize_t res = getline (&str, &strLen, stream);                          \
            DEBUG_LOG ("res - %zd", res);                                           \
            if (res < 0)                                                            \
            {                                                                       \
                ERROR_PRINT ("Error reading user input - %s", strerror (errno));    \
                free (str);                                                         \
                                                                                    \
                return AKINATOR_ERROR_COMMON |                                      \
                       COMMON_ERROR_READING_INPUT;                                  \
            }                                                                       \
        } while (0)



int AkinatorMenu (tree_t *tree)
{
    PRINT ("%s", "Добро пожаловать в акинатора!\n");

    while (true)
    {
        // NOTE: maybe do enums
        PRINT  ("%s",   "Choose the option:\n"
                        "\t[0] - Exit\n"
                        "\t[1] - Save the tree\n"
                        "\t[2] - Guess a character\n"
                        // "\t[3] - Дать определение объекту\n"
                        " > ");
        
        int action = -1;
        int res = scanf ("%d", &action);
        ClearBuffer();
    
        if (res != 1)
        {
            ERROR_PRINT ("%s", "Вы ввели не число, попробуйте ещё раз");
            
            continue;
        }
        
        switch (action)
        {
            case 0:
            {
                int status = TreeSaveToFile (tree, treeSaveFileName);
                if (status != TREE_OK)
                    ERROR_PRINT ("%s", "Error saving the tree ");
                else 
                    PRINT ("%s", "The tree saved successfully");
                
                break;
            }

            case 1:
                return TREE_OK;

            case 2:
                GuessCharacter (tree);
                TREE_DUMP (tree, "After guess");
                break;

            case 3:
                DescribeObject (tree);
                break;

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
                assert (0);
                break;
        }
    }

    if (IsLeaf (node)) 
    {
        int leftAttemtts = 5;
        while (leftAttemtts--)
        {
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

    if (answer == USER_YES) // TODO switch
    {
        PRINT ("%s", "I won. \n");

        return AKINATOR_OK;
    }
    else if (answer == USER_NO)
    {
        PRINT ("%s", "Who are you thinking of?\n"
                      " > ");

        char *userCharacter = NULL;
        size_t userCharacterLen = 0;
        SAFE_READ_LINE (userCharacter, userCharacterLen, stdin);

        PRINT ("The difference between %s and %s that it...\n"
                " > ", node->data, userCharacter);
        
        char *newQuestion = NULL;
        size_t newQuestionLen = 0;
        SAFE_READ_LINE (newQuestion, newQuestionLen, stdin);

        int res = AddNewCharacter (tree, node, userCharacter, newQuestion);

        DEBUG_VAR ("%d", res);
        
        return res;
    }
    else if (answer == USER_UKNOWN)
    {
        ERROR_PRINT ("%s", "Sorry, you are wrong. You need to input Y/y or N/n\n");

        return AKINATOR_ERROR_WRONG_INPUT;
    }
    else 
    {
        assert (0);
    }

    return AKINATOR_OK;
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

int AddNewCharacter (tree_t *tree, node_t *node, char *userCharacter, char *newQuestion)
{
    assert (tree);
    assert (node);
    assert (userCharacter);
    assert (newQuestion);

    // NOTE: not the best option solution
    // read about lowercase wide chars in future
    // also I believe what user do not use leading spaces
    if (strncmp ("Не", newQuestion, sizeof("Не") - 1) == 0 ||
        strncmp ("не", newQuestion, sizeof("не") - 1) == 0 ||
        strncmp ("НЕ", newQuestion, sizeof("НЕ") - 1) == 0 ||
        strncmp ("нЕ", newQuestion, sizeof("нЕ") - 1) == 0)
    {
        ERROR_PRINT ("%s", "Нельзя использовать \"не\", \"нет\" и т.д. в определении признака");

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
    node->left  = oldNode;
    node->right = userNode;

    tree->size += 2;

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
    SAFE_READ_LINE (character, characterLen, stdin);

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