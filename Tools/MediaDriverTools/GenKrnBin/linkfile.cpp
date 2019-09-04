// Disable deprecated
#pragma warning(disable : 4995)

#include <assert.h>
#include <list>
#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include "linkfile.h"

#ifdef LINUX_
#include <strings.h>
#include <cstring>
#endif

#ifdef LINUX_
#define StrCmp strcasecmp
#else
#define StrCmp stricmp
#endif

using namespace std;

// scratch global variables for writing files
char *skipspace(char *ch)
{
    if (!ch) return NULL;

    // skip space chars
    while (isspace(*ch))
    {
        (*ch++) = 0;
    }

    // end of string
    if (*ch == 0)
    {
        ch = NULL;
    }

    return ch;
}

char *nextword(char *ch)
{
    if (!ch) return NULL;

    // skip current word (non-space chars)
    while ((*ch) && !isspace(*ch))
    {
        ch++;
    }

    // skip space chars after word, point to next word
    return skipspace(ch);
}

void CreateLinkFile(char *pDirectoryName, list <string> &kernels)
{
    list <string> labels;
    list <string>::iterator i;

    char LinkFileTxt[MAX_STRING_SIZE];
    char LinkFileBin[MAX_STRING_SIZE];
    char     *pText  = NULL;
    LinkData *pLink  = NULL;
    LinkFileHeader LinkHeader;
    FILE *hText = NULL;
    FILE *hBin  = NULL;
    unsigned int dwFileSize, dwBytesRead, dwBytesWrite;
    unsigned int dwKernelID, dwLabelID, dwOffset;
    bool bExport;
    bool bInline;
    int  lines;
    int  links = 0;
    int  exports = 0;
    int  imports = 0;
    char *ch;
    string name_krn;
    string name_hex;

    // Create full LinkFile binary name
#ifdef LINUX_
    sprintf(LinkFileTxt, "%s/%s", pDirectoryName, "LinkFile.txt");
#else
    sprintf(LinkFileTxt, "%s\\%s", pDirectoryName, "LinkFile.txt");
#endif  
    hText = fopen(LinkFileTxt, "r");
    if (hText == NULL) goto done;

    // Create full LinkFile binary name - remove pre-existing file
#ifdef LINUX_
    sprintf(LinkFileBin, "%s/%s", pDirectoryName, "LinkFile.krn");
#else
    sprintf(LinkFileBin, "%s\\%s", pDirectoryName, "LinkFile.krn");
#endif
    remove(LinkFileBin);

    // Allocate text buffer for reading
    fseek(hText, 0, SEEK_END);
    dwFileSize = ftell(hText);
    fseek(hText, 0, SEEK_SET);
    pText = (char *)malloc(dwFileSize + 1);
    if (!pText) goto done;
    
    memset(pText,0,dwFileSize + 1);

    // Read text file
    dwBytesRead = fread(pText, 1, dwFileSize, hText);
    pText[dwFileSize] = 0;

    // Count lines, split into strings
    for (lines = 0, ch = pText; (ch != NULL); lines++)
    {
        ch = strchr(ch, '\n');
        if (ch) *(ch++) = 0;
    }

    // Allocate binary file (from number of lines : 1 link entry per line)
    pLink = (LinkData *)malloc(lines * sizeof(LinkData));
    if (!pLink) goto done;
    memset(pLink, 0, lines * sizeof(LinkData));

    // Start parsing the file
    labels.empty();

    char *module;
    char *label;
    char *export_str;
    char *offset;
    for (ch = pText; lines > 0; lines--)
    {
        // Split the words
        module = ch;
        ch = ch + strlen(ch) + 1;
        module = skipspace(module);
        label  = nextword (module);
        export_str = nextword (label);
        offset = nextword (export_str);

        // Check for structure
        if (!(module && label && export_str && offset))
        {
            continue;
        }

        // Module search
        name_krn = module;
        name_hex = module;
        name_krn.append(".krn");
        name_hex.append(".hex");
        for (dwKernelID = 0, i = kernels.begin(); i != kernels.end(); dwKernelID++, i++)
        {
            if (StrCmp(i->c_str(), name_krn.c_str()) == 0) break;
            if (StrCmp(i->c_str(), name_hex.c_str()) == 0) break;
        }
        if (i == kernels.end())
        {
            fprintf(stderr, "Unresolved reference to %s\n", module);
            exit (-1);
        }

        // Label ID
        for (dwLabelID = 0, i = labels.begin(); i != labels.end(); dwLabelID++, i++)
        {
            if (strcmp(i->c_str(), label) == 0) break;
        }
        if (i == labels.end())
        {
            labels.push_back(label);
        }

        // Export/import
        if (strcmp(export_str, "export") == 0)
        {
            bExport = true;
            bInline = false;
        }
        else if (strcmp(export_str, "import") == 0)
        {
            bExport = false;
            bInline = false;
        }
        else if (strcmp(export_str, "include") == 0)
        {
            bExport = false;
            bInline = true;
        }
        else if (strcmp(export_str, "inline") == 0)
        {
            bExport = true;
            bInline = true;
        }
        else
        {
            fprintf(stderr, "Invalid Export/Import syntax\n");
            exit(-1);
        }

        // Offset in instructions
        dwOffset = atoi(offset);

        pLink[links].dwKernelID = dwKernelID;
        pLink[links].dwLabelID  = dwLabelID;
        pLink[links].bExport    = bExport;
        pLink[links].bInline    = bInline;
        pLink[links].dwOffset   = dwOffset << 2;
        links++;
        exports += bExport;
        imports += !bExport;
    }

    hBin = fopen(LinkFileBin, "wb");
    if (hBin == NULL) goto done;

    // Generate link header
    LinkHeader.dwVersion = LINKFILE_VERSION;
    LinkHeader.dwExports = exports;
    LinkHeader.dwImports = imports;
    LinkHeader.dwSize    = links * sizeof(LinkData);

    // Write header and link data
    fwrite(&LinkHeader, 1, sizeof(LinkHeader), hBin);
    fwrite(pLink, 1, links * sizeof(LinkData), hBin);

done:
    if (hText) fclose(hText);
    if (hBin)  fclose(hBin);
    if (pText) free(pText);
    if (pLink) free(pLink);
}

void DeleteLinkFile(char *pDirectoryName)
{
    char LinkFileBin[MAX_STRING_SIZE];

    // Create full LinkFile binary name - delete file
    sprintf(LinkFileBin, "%s\\%s", pDirectoryName, "LinkFile.krn");
    remove(LinkFileBin);
}
