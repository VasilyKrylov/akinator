#ifndef K_utils_H
#define K_utils_H

int SafeMkdir       (const char *fileName);
void ClearBuffer    ();
char *SkipSpaces    (char *buffer);
char *ReadFile      (const char *inputFileName, size_t *bufferLen);

#endif // K_utils_H