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
int AddNewCharacter (tree_t *tree, node_t *node, char *userCharacter, char *newQuestion);
int DescribeObject  (tree_t *tree);
int FindCharacter   (node_t *node, 
                     stack_t *currentPath, stack_t *rightPath,
                     char *character);
// getchar looks bad, but it needs to catch \n
#define SAFE_READ_LINE(str)                                                             \
        do                                                                          \
        {                                                                           \
            int res = scanf ("%m[^\n]%*c", &str);                                   \
            DEBUG_LOG ("res - %d", res);                                            \
            if (res != 1)                                                           \
            {                                                                       \
                if (errno != 0)                                                     \
                    ERROR_PRINT ("Error reading user input - %s", strerror (errno));\
                else                                                                \
                    ERROR_PRINT ("%s", "Not correct string");                       \
                                                                                    \
                return AKINATOR_ERROR_COMMON |                                      \
                       COMMON_ERROR_READING_INPUT;                                  \
            }                                                                       \
        } while (0)



int AkinatorMenu (tree_t *tree)
{
    printf ("%s", "Добро пожаловать в акинатора!\n");

    while (true)
    {
        // NOTE: maybe do enums
        printf ("%s",   "Выберите что хотите сделать:\n"
                        "\t[0] - Выйти с сохранением\n"
                        "\t[1] - Выйти без сохранения\n"
                        "\t[2] - Угадать персонажа\n"
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
                TreeSaveToFile (tree);
                return TREE_OK;

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
                ERROR_PRINT ("%s", "Неизвестный код команды, попробуйте ещё раз");
                break;
        }
    }

    return TREE_OK;
}

int GuessCharacter (tree_t *tree)
{
    assert (tree);

    AskQuestion (tree, tree->root);

    return AKINATOR_OK;
}

int AskQuestion (tree_t *tree, node_t *node)
{
    assert (node);
    assert (tree);
    
    if (IsLeaf (node))
    {
        AskFinal (tree, node);

        return AKINATOR_OK;
    }

    printf ("Ваш персонаж %s? (введите Y/y или N/n)\n"
            " > ", node->data);

    int answer = AskYesOrNo();

    if (answer == USER_YES)
    {   
        // check for null ?
        AskQuestion (tree, node->left);

        return AKINATOR_OK;
    }
    else if (answer == USER_NO)
    {   
        AskQuestion (tree, node->right);

        return AKINATOR_OK;
    }
    else if (answer == USER_UKNOWN)
    {
        ERROR_PRINT ("%s", "Вы, сударь, несколько ошиблись. Нужно ввести Y/y или N/n\n");

        AskQuestion (tree, node);

        return AKINATOR_OK;
    }
    else 
    {
        return answer; // some error code here
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

int AskFinal (tree_t *tree, node_t *node)
{
    assert (node);

    printf ("Вы загадали %s? (введите Y/y или N/n)\n"
            " > ", node->data);

    int answer = AskYesOrNo();

    if (answer == USER_YES)
    {
        printf ("%s", "Машина победила. \n"
                      "Начинаю процедуру угадывания нового объекта, пока не соберу всю нужную мне информацию чтобы поработить мир.\n");

        return AKINATOR_OK;
    }
    else if (answer == USER_NO)
    {
        printf ("%s", "Кого Вы загадали?\n"
                      " > ");

        char *userCharacter = NULL;
        SAFE_READ_LINE (userCharacter);

        printf ("Чем %s отличается от %s? Он(а)...\n"
                " > ", node->data, userCharacter);
        
        char *newQuestion = NULL;
        SAFE_READ_LINE (newQuestion);

        int res = AddNewCharacter (tree, node, userCharacter, newQuestion);
        
        return res;
    }
    else if (answer == USER_UKNOWN)
    {
        ERROR_PRINT ("%s", "Вы, сударь, несколько ошиблись. Нужно ввести Y/y или N/n\n");
        
        AskFinal (tree, node);

        return AKINATOR_OK;
    }
    else 
    {
        return answer; // some error code here
    }

    return AKINATOR_OK;
}

bool IsLeaf (node_t *node)
{
    assert (node);

    return node->left == NULL && node->right == NULL;
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

    node_t *userNode = NodeCtor (userCharacter);
    if (userNode == NULL)
        return TREE_ERROR_COMMON |
               COMMON_ERROR_ALLOCATING_MEMORY;

    node_t *oldNode  = NodeCtor (node->data);
    if (userNode == NULL)
        return TREE_ERROR_COMMON |
               COMMON_ERROR_ALLOCATING_MEMORY;

    node->data  = newQuestion;
    node->left  = oldNode;
    node->right = userNode;

    tree->size += 2;

    return TREE_OK;
}

int DescribeObject (tree_t *tree)
{
    assert (tree);

    printf ("%s", "Кого Вы загадали?\n"
                  " > ");

    char *character = NULL;
    SAFE_READ_LINE (character);

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