/*
* Copyright (c) 2018, Intel Corporation
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
#include <cctype>
#include <string>
#include <stdio.h>
#include "devconfig.h"
#include "gtest/gtest.h"

using namespace std;

const char*        g_dirverPath;
vector<Platform_t> g_platform;

static bool ParseCmd(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    if (ParseCmd(argc, argv) == false)
    {
        return -1;
    }

    return RUN_ALL_TESTS();
}

static bool ParsePlatform(const char *str);
static bool ParseDriverPath(const char *str);

static bool ParseCmd(int argc, char *argv[])
{
    g_dirverPath = nullptr;
    g_platform.clear();

    for (int i = 1; i < argc; i++)
    {
        if (ParseDriverPath(argv[i]) == false && ParsePlatform(argv[i]) == false)
        {
            printf("ERROR\n    Bad command line parameter!\n\n");
            printf("USAGE\n    devult [driver_path] [platform_name...]\n\n");
            printf("DESCRIPTION\n    [driver_path]     : Use default driver relative path if not specify driver_path.\n"
                "    [platform_name...]: Select zero or more items from {SKL, BXT, BDW, CNL}.\n\n");
            printf("EXAMPLE\n    devult\n"
                "    devult ./build/media_driver/iHD_drv_video.so\n"
                "    devult skl\n"
                "    devult ./build/media_driver/iHD_drv_video.so skl\n"
                "    devult ./build/media_driver/iHD_drv_video.so skl cnl\n\n");
            return false;
        }
    }

    return true;
}

static bool ParsePlatform(const char *str)
{
    string tmpStr(str);

    for (auto i = tmpStr.begin(); i != tmpStr.end(); i++)
    {
        *i = toupper(*i);
    }

    for (int i = 0; i < (int)igfx_MAX; i++)
    {
        if (tmpStr.compare(g_platformName[i]) == 0)
        {
            g_platform.push_back((Platform_t)i);
            return true;
        }
    }

    return false;
}

static bool ParseDriverPath(const char *str)
{
    if (g_dirverPath == nullptr && strstr(str, "iHD_drv_video.so") != nullptr)
    {
        g_dirverPath = str;
        return true;
    }

    return false;
}
