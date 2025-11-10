#ifndef K_TREE_LOG_H
#define K_TREE_LOG_H

struct tree_t;

const char kParentDumpFolderName[] = "dump/";
const char kImgFolderName[]        = "img/";
const char kDotFolderName[]        = "dot/";
const char kLogFileName[]          = "log.html";
const char kGraphFileName[]        = "dot.txt";

const size_t kLogFolderPathLen       = 44;
const size_t kFileNameLen            = 64;

struct treeLog_t
{
    char logFolderPath [kLogFolderPathLen] = {}; // dump/[date-time]
    char imgFolderPath [kFileNameLen]      = {}; // dump/[date-time]/img
    char dotFolderPath [kFileNameLen]      = {}; // dump/[date-time]/dot
    char logFilePath   [kFileNameLen]      = {}; // dump/[date-time]/log.html

    FILE *logFile  = NULL;
};

int LogInit             (treeLog_t *log);
int TreeDump            (tree_t *tree, const char *comment,
                         const char *_FILE, int _LINE, const char * _FUNC);

#endif // K_TREE_LOG_H