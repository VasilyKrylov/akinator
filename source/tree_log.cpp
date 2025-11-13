#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>

#include "tree.h"
#include "utils.h"

#include "tree_log.h"

static size_t imageCounter = 0;

const char * const kBlack       = "#000000";
const char * const kGray        = "#ebebe0";

const char * const kViolet      = "#cc99ff";
const char * const kBlue        = "#66ccff";

const char * const kGreen       = "#99ff66";
const char * const kDarkGreen   = "#33cc33";
const char * const kYellow      = "#ffcc00";

const char * const kHeadColor   = kViolet;
const char * const kFreeColor   = kDarkGreen;
const char * const kEdgeNormal  = kBlack;

void LeftRootRight      (node_t *node, FILE *graphFile);
int TreeDumpImg         (node_t *node, treeLog_t *log);
int DumpMakeConfig      (node_t *node, treeLog_t *log);
int DumpMakeImg         (node_t *node, treeLog_t *log);

int LogInit (treeLog_t *log)
{
    time_t t = time (NULL);
    struct tm tm = *localtime (&t);

    snprintf (log->logFolderPath, kFileNameLen, "%s%d-%02d-%02d_%02d:%02d:%02d/",
              kParentDumpFolderName,
              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
              tm.tm_hour,        tm.tm_min,     tm.tm_sec);

    snprintf (log->logFilePath,   kFileNameLen, "%s%s",
              log->logFolderPath, kLogFileName);

    snprintf (log->imgFolderPath, kFileNameLen, "%s%s",
              log->logFolderPath, kImgFolderName);

    snprintf (log->dotFolderPath, kFileNameLen, "%s%s",
              log->logFolderPath, kDotFolderName);


    if (SafeMkdir (kParentDumpFolderName) != TREE_OK)
        return TREE_ERROR_COMMON | 
               COMMON_ERROR_CREATING_FILE;

    if (SafeMkdir (log->logFolderPath) != TREE_OK)
        return TREE_ERROR_COMMON | 
               COMMON_ERROR_CREATING_FILE;

    if (SafeMkdir (log->imgFolderPath) != TREE_OK)
        return TREE_ERROR_COMMON | 
               COMMON_ERROR_CREATING_FILE;

    if (SafeMkdir (log->dotFolderPath) != TREE_OK)
        return TREE_ERROR_COMMON | 
               COMMON_ERROR_CREATING_FILE;

    log->logFile = fopen (log->logFilePath, "w");
    if (log->logFile == NULL)
    {
        ERROR_LOG ("Error opening file \"%s\"", log->logFilePath);
        
        return TREE_ERROR_COMMON |
               COMMON_ERROR_OPENING_FILE;
    }

    fprintf (log->logFile, "%s", "<pre>\n");

    return TREE_OK;
}

int NodeDump (node_t *node, treeLog_t *log,
              const char *file, int line, const char *func,
              const char *format, ...)
{
    assert(node);
    assert(log);
    assert(file);
    assert(func);
    assert(format);

    fprintf (log->logFile,
             "<h3>NODE DUMP called at %s:%d:%s(): <font style=\"color: green;\">",
             file, line, func);

    va_list args = {};
    va_start (args, format);
    vfprintf (log->logFile, format, args);
    va_end   (args);
    
    fprintf (log->logFile,
             "%s",
             "</font></h3>\n");
    
    TREE_DO_AND_CHECK (TreeDumpImg (node, log));

    fprintf (log->logFile, "%s", "<hr>\n\n");

    return TREE_OK;
}

int TreeDump (tree_t *tree, const char *comment,
              const char *file, int line, const char *func)
{
    assert(tree);
    assert(comment);
    assert(file);
    assert(func);
    
    DEBUG_PRINT ("%s", "\n");
    DEBUG_LOG ("comment = \"%s\";", comment);

    fprintf (tree->log.logFile,
             "<h3>TREE DUMP called at %s:%d:%s(): <font style=\"color: green;\">%s</font></h3>\n",
             file, line, func, comment);
    ON_DEBUG (
        fprintf (tree->log.logFile,
                 "%s[%p] initialized in {%s:%d}\n",
                 tree->varInfo.name, tree, tree->varInfo.file, tree->varInfo.line);
    );
    fprintf (tree->log.logFile,
             "tree->size = %lu;\n",
             tree->size);

    TREE_DO_AND_CHECK (TreeDumpImg (tree->root, &tree->log));

    fprintf (tree->log.logFile, "%s", "<hr>\n\n");
    
    return TREE_OK;
}

int TreeDumpImg (node_t *node, treeLog_t *log)
{
    assert (node);
    assert (log);

    TREE_DO_AND_CHECK (DumpMakeConfig (node, log));

    TREE_DO_AND_CHECK (DumpMakeImg (node, log));

    return TREE_OK;
}

void LeftRootRight (node_t *node, FILE *graphFile) // TODO rename 
{
    assert (node);
    assert (graphFile);

    // fprintf (graphFile,
    //         "\tnode%p [shape=Mrecord; style=\"filled\"; fillcolor=\"%s\"; "
    //         "label = \"{ptr = [%p] | data = [%s] | left = [%p] | right = [%p] }\"];\n",
    //         node, kBlue,
    //         node, node->data, node->left, node->right);


    fprintf (graphFile,
            "\tnode%p [shape=Mrecord; style=\"filled\"; fillcolor=\"%s\"; "
            "label = \"{\\\"%s\\\" | ptr = [%p] | left = [%p] | right = [%p] }\"];\n",
            node, kBlue,
            node->data, node, node->left, node->right);

    DEBUG_VAR ("%p", node);
    DEBUG_LOG ("\t node->left: %p", node->left);
    DEBUG_LOG ("\t node->right: %p", node->right);

    if (node->left != NULL)
    {
        DEBUG_LOG ("\t %p->%p [label = \"Yes\"];\n", node, node->left);

        fprintf (graphFile, "\tnode%p->node%p [label = \"Yes\"]\n", node, node->left);
        
        LeftRootRight (node->left, graphFile);
    }    

    if (node->right != NULL)
    {
        DEBUG_LOG ("\t %p->%p [label = \"No\"];\n", node, node->left);

        fprintf (graphFile, "\tnode%p->node%p [label = \"No\"];\n", node, node->right);
        
        LeftRootRight (node->right, graphFile);
    }
}

int DumpMakeConfig (node_t *node, treeLog_t *log)
{
    assert (node);
    assert (log);

    imageCounter++;

    char graphFilePath[kFileNameLen + 22] = {};
    snprintf (graphFilePath, kFileNameLen + 22, "%s%lu.dot", log->dotFolderPath, imageCounter);

    DEBUG_VAR ("%s", graphFilePath);

    FILE *graphFile  = fopen (graphFilePath, "w");
    if (graphFile == NULL)
    {
        ERROR_LOG ("Error opening file \"%s\"", graphFilePath);

        return TREE_ERROR_COMMON |
               COMMON_ERROR_OPENING_FILE;
    }

    fprintf (graphFile  ,   "digraph G {\n"
                            // "\tsplines=ortho;\n"
                            // "\tnodesep=0.5;\n"
                            "\tnode [shape=octagon; style=\"filled\"; fillcolor=\"#ff8080\"];\n");

    LeftRootRight (node, graphFile);

    fprintf (graphFile, "%s", "}");
    fclose (graphFile);

    return TREE_OK;
}
int DumpMakeImg (node_t *node, treeLog_t *log)
{
    assert (node);
    assert (log);

    char imgFileName[kFileNameLen] = {};
    snprintf (imgFileName, kFileNameLen, "%lu.svg", imageCounter);

    const size_t kMaxCommandLen = 256;
    char command[kMaxCommandLen] = {};

    snprintf (command, kMaxCommandLen, "dot %s%lu.dot -T svg -o %s%s", 
              log->dotFolderPath, imageCounter,
              log->imgFolderPath, imgFileName);

    int status = system (command);
    DEBUG_VAR ("%d", status);
    if (status != 0)
    {
        ERROR_LOG ("ERROR executing command \"%s\"", command);
        
        return TREE_ERROR_COMMON |
               COMMON_ERROR_RUNNING_SYSTEM_COMMAND;
    }

    fprintf (log->logFile,
             "<img src=\"img/%s\" hieght=\"500px\">\n",
             imgFileName);

    DEBUG_VAR ("%s", command);

    return TREE_OK;
}
