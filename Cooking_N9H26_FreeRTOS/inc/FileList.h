#ifndef FILELIST_H
#define FILELIST_H

#include <stdlib.h>

#define DEF_MAX_FILELIST_SIZE       256
typedef struct
{
    int     m_i32UsedNum;
    int     m_i32MaxSize;
    char *m_szFilePath[DEF_MAX_FILELIST_SIZE];
} S_FILELIST;

int filelist_create(const char *pcDirPath);
int filelist_destroy(void);
int filelist_dump(void);
S_FILELIST *filelist_getInstance(void);
const char *filelist_getFileName(int);

#endif  // Avoid multiple inclusion
