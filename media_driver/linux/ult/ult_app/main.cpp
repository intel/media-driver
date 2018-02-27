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
// main.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <cctype>
#include <string>
#include "gtest/gtest.h"
#include "devconfig.h"

using namespace std;

const char* DirverPath;
vector<Platform_t> g_platform;

bool parseCmd(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    if (parseCmd(argc, argv) == false)
    {
        return -1;
    }

    return RUN_ALL_TESTS();
}

bool parsePlatform(const char *str);
bool parseDriverPath(const char *str);

bool parseCmd(int argc, char* argv[])
{
    DirverPath = nullptr;
    g_platform.clear();

    for (int i = 1; i < argc; i++)
    {
        if (parseDriverPath(argv[i]) == false && parsePlatform(argv[i]) == false)
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

bool parsePlatform(const char *str)
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

bool parseDriverPath(const char *str)
{
    if (DirverPath == nullptr && strstr(str, "iHD_drv_video.so") != nullptr)
    {
        DirverPath = str;
        return true;
    }

    return false;
}