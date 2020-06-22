#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <list>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <algorithm>

#define MAX_STRING_SIZE  4096
#define MAX_KERNEL_NAME_LENGTH 100
#define MAX_CACHE_STRING_LENGTH (MAX_KERNEL_NAME_LENGTH + 10)

using namespace std;

#define StrCmp strncasecmp

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        fprintf(stderr, "Usage: GenDmyHex.exe <kernel hex dir> <kernel header file> \n");
        exit(-1);
    }

    struct dirent* ent = NULL;
    
    char dirkrnhex[MAX_STRING_SIZE];
    FILE *pfkrnheader = NULL;
    DIR *pHexDir = opendir(argv[1]);

    if (!pHexDir)
    {
        fprintf(stderr, "Open kernel hex dir failed!\n");
        exit(-1);
    }

    if(sprintf(dirkrnhex, "%s", argv[1]) < 0)
    {
        fprintf(stderr, "Copy kernel hex dir failed!\n");
        exit(-1);
    }
    strcat(dirkrnhex, "/");

    if(!(pfkrnheader = fopen(argv[2], "r")))
    {
        fprintf(stderr, "Read kernel header file failed!\n");
        exit(-1);
    }

    ///////////////////////////////////////////////////////////
    //Get Full kernel list from pre-built kernele header file//
    ///////////////////////////////////////////////////////////

    list <string> KernelFullList;
    char scache[MAX_CACHE_STRING_LENGTH];

    while (fgets(scache, MAX_CACHE_STRING_LENGTH, pfkrnheader))
    {
        string strcache(scache);
        if (strcache.size() < 16)
        {
            continue;
        }
        string head = strcache.substr(8, 6);
        if (!head.compare("IDR_VP"))
        {
            strcache.erase(0, 15);
            strcache.erase(strcache.find(" "), strcache.size());
            if (!strcache.compare("TOTAL_NUM_KERNELS") ||
                !strcache.compare("KERNEL_NAMES") ||
                !strcache.compare("LINKFILE_HEADER") ||
                !strcache.compare("LINKFILE_VERSION") ||
                !strcache.compare("LinkFile")
                )
            {
                continue;
            }
            strcache.append(".hex");
            KernelFullList.push_back(strcache);
        }
    }

    fclose(pfkrnheader);

    ////////////////////////////////////////////////////////////////////////////
    // Remove a superset list of all the kernels according to generated kernel//
    ////////////////////////////////////////////////////////////////////////////

    char dirNewfile[MAX_STRING_SIZE];
    char dirOldfile[MAX_STRING_SIZE];

    while (NULL != (ent=readdir(pHexDir)))
    {      
        int n = strlen(ent->d_name);
        if (n < 4 || (StrCmp(ent->d_name + n - 4, ".hex", 4) != 0))
        {
            continue;
        }

        //Rename kernel name which contains "Dscale" to "DScale" except for "P010_444Dscale16_Buf*"
        string sfilename(ent->d_name);
        size_t pos = sfilename.find("Dscale");
        if (pos != string::npos)
        {
            string ssub = sfilename.substr(0, 9);
            if (ssub.compare("P010_444D"))
            {
                sfilename.replace(pos, 6, "DScale");
                strcpy(dirNewfile, dirkrnhex);
                strcpy(dirOldfile, dirkrnhex);
                strcat(dirOldfile, ent->d_name);
                strcat(dirNewfile, sfilename.c_str());
                if (rename(dirOldfile, dirNewfile))
                {
                    fprintf(stderr, "Rename file %s to %s failed!\n", dirOldfile, dirNewfile);
                    exit(-1);
                }
            }

        }

        // Remove the generated kernel
        list <string>::iterator iter = find(KernelFullList.begin(), KernelFullList.end(), sfilename);
        if (iter != KernelFullList.end())
        {
            KernelFullList.erase(iter);
        }
    }

    closedir(pHexDir);

    ////////////////////////////////////////////////////////////////////////////////////
    // Create a set of dummy kernel hex that are not included in the generated kernels//
    ////////////////////////////////////////////////////////////////////////////////////

    while (!KernelFullList.empty())
    {
        string skernelhex = KernelFullList.front();
        strcpy(dirNewfile, dirkrnhex);
        strcat(dirNewfile, skernelhex.c_str());
        FILE *pfkernelhex = NULL;
        if (!(pfkernelhex = fopen(dirNewfile, "wb")))
        {
            fprintf(stderr, "Create dummy kernel hex file failed!\n");
            exit(-1);
        }
        else
        {
            fclose(pfkernelhex);
        }
        KernelFullList.pop_front();
    }

    fprintf(stdout, "Create dummy kernel hex files successfully!\n");
    return 0;
}
