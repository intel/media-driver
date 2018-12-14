/* 
* Copyright (c) 2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE   4096
#define MAX_FILE_PATH 4096
#define MAX_FILE_NAME 260

#ifdef LINUX_
#define StrCmp strcasecmp
#else
#define StrCmp stricmp
#endif

int usage()
{
    fprintf(stderr, "KernelToHex <source file>\r\n"   \
                    "Convert krn into hex file (single file)\r\n\r\n");
    return -1;
}

bool KrnToHex(char *file_krn, char *file_hex)
{
    char     buffer[BUFFER_SIZE];
    char     aux_file_hex[MAX_FILE_PATH];
    char     line[40];
    FILE     *f_in, *f_out;

    unsigned int   *pbuffer;
    unsigned int    n_read;

    int      i;
    char     *pfile_handle;

    // Generate destination file name (if not provided)
    if (!file_hex)
    {
        file_hex = aux_file_hex;

        strncpy(file_hex, file_krn, MAX_FILE_PATH - 5);
        file_hex[MAX_FILE_PATH - 5] = 0;
        i = (int)strlen(file_hex);

        pfile_handle = strrchr(file_hex, '.');
        if (!pfile_handle)
        {
            pfile_handle = file_hex + i;
        }
        sprintf(pfile_handle, ".hex");
    }

    // Open files
    f_in  = fopen(file_krn, "rb");
    f_out = fopen(file_hex, "wb");

    // Check if files successfully open/created
    if ((f_in != NULL) && (f_out != NULL))
    {
        while (true)
        {
            n_read = fread(buffer, sizeof(char), BUFFER_SIZE, f_in);
            if (n_read == 0)
            {
                break;
            }

            pbuffer = (unsigned int *)buffer;

            for (; n_read >= 16; n_read -= 16, pbuffer+=4)
            {
                sprintf(line, "%08x %08x %08x %08x\r\n", pbuffer[0],  pbuffer[1], pbuffer[2], pbuffer[3]);
                fwrite(line, 1, 37, f_out);
            }

            if(n_read > 0)
            {
                for (; n_read > 4; n_read -= 4, pbuffer++)
                {
                    sprintf(line, "%08x ", pbuffer[0]);
                    fwrite(line, 1, 9, f_out);
                }
                sprintf(line, "%08x\r\n", pbuffer[0]);
                fwrite(line, 1, 10, f_out);
            }
        }
    }
    else
    {
        if (f_in == NULL)
        {
            fprintf(stderr, "Failed to open input file \"%s\"\r\n", file_krn);
        }

        if (f_out == NULL)
        {
            fprintf(stderr, "Failed to create output file \"%s\"\r\n", file_hex);
        }
    }

    // Add 128 bytes padding at the end of each biniary
    for (int i = 0; i < 8; i++)
    {
        sprintf(line, "%08x %08x %08x %08x\r\n", 0, 0, 0, 0);
        fwrite(line, 1, 37, f_out);
    }

    if (f_in != NULL)
    {
        fclose(f_in);
    }

    if (f_out != NULL)
    {
        fclose(f_out);
    }

    return true;
}

int main(int argc, char* argv[])
{
    char *p = NULL;

    if (argc == 2)
    {
        p = strrchr(argv[1], '.');

        if (p != NULL && StrCmp(p, ".krn") == 0)
        {
            KrnToHex(argv[1], NULL);
        }
    }
    else
    {
        return (usage());
    }

    return 0;
}
