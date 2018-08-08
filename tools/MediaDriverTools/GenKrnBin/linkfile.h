#ifndef __LINKFILE_H__
#define __LINKFILE_H__

#include <list>

using namespace std;

#define MAX_STRING_SIZE  4096

#pragma pack(4)
#define LINKFILE_VERSION 0x00010000
typedef struct tagLinkFileHeader
{
    unsigned int    dwVersion;
    unsigned int    dwSize;
    unsigned int    dwImports;
    unsigned int    dwExports;
} LinkFileHeader;

typedef struct tagLinkData
{
    unsigned int    dwKernelID    : 16;     // Kernel ID
    unsigned int    dwLabelID     : 16;     // Label ID
    unsigned int    bExport       : 1;      // 0 - import; 1 - export;
    unsigned int    bResolved     : 1;      // MBZ
    unsigned int    dwOffset      : 20;     // Offset in DWORDs
    unsigned int    bInline       : 1;      // 0 - none; 1 - inline;
    unsigned int                  : 9;      // MBZ
} LinkData;
#pragma pack()

void CreateLinkFile(char *pDirectoryName, list <string> &kernels);
void DeleteLinkFile(char *pDirectoryName);

#endif // __LINKFILE_H__
