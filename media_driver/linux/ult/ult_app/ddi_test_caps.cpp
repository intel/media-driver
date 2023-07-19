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
#include <algorithm>
#include <string>
#include "ddi_test_caps.h"

using namespace std;

bool CompareFeatureIDTable(vector<FeatureID> &currentTable, vector<FeatureID> &refTable)
{
    if (currentTable.size() != refTable.size())
    {
        return false;
    }

    sort(currentTable.begin(), currentTable.end());
    sort(refTable.begin()    , refTable.end());

    return equal(currentTable.begin(), currentTable.end(), refTable.begin());
}

int Test_QueryConfigProfiles(VADriverContextP ctx, vector<FeatureID> &queriedFeatureIDTable)
{
    VAProfile    profile;
    VAEntrypoint entrypoints[10];
    int          num_profiles;
    int          num_entrypoint;
    int          max_num_profiles = ctx->max_profiles;
    VAProfile    *profile_list    = nullptr;

    profile_list = (VAProfile *)malloc(max_num_profiles * sizeof(VAProfile));
    if (profile_list == nullptr)
    {
        return -1;
    }

    int ret = ctx->vtable->vaQueryConfigProfiles(ctx, profile_list, &num_profiles);
    if (ret)
    {
        free(profile_list);
        return -1;
    }

    for (int i = 0; i < num_profiles; i++)
    {
        profile = profile_list[i];
        ret     = ctx->vtable->vaQueryConfigEntrypoints(ctx, profile, entrypoints, &num_entrypoint);

        if (ret == VA_STATUS_ERROR_UNSUPPORTED_PROFILE)
        {
            continue;
        }
        else if (ret)
        {
            free(profile_list);
            return -1;
        }
        else
        {
            for (int j = 0; j < num_entrypoint; j++)
            {
                queriedFeatureIDTable.push_back({profile, entrypoints[j]});
            }
        }
    }

    free(profile_list);

    return ret;
}

TEST_F(MediaCapsDdiTest, DecodeEncodeProfile)
{
    vector<Platform_t> platforms = m_driverLoader.GetPlatforms();

    for (int i = 0; i < m_driverLoader.GetPlatformNum(); i++)
    {
        vector<FeatureID> queriedFeatureIDTable;
        vector<FeatureID> refFeatureIDTable = m_capsData.GetRefFeatureIDTable(DeviceConfigTable[platforms[i]]);

        // So far we still use DeviceConfigTable to find the platform, as the libdrm mock use this.
        // If we want to use vector Platforms, we would use vector in libdrm too.
        int ret = m_driverLoader.InitDriver(platforms[i]);
        EXPECT_EQ(VA_STATUS_SUCCESS , ret) << "Platform = " << g_platformName[platforms[i]]
            << ", Failed function = m_driverLoader.InitDriver" << endl;

        ret = Test_QueryConfigProfiles(&m_driverLoader.m_ctx, queriedFeatureIDTable);
        EXPECT_EQ(VA_STATUS_SUCCESS , ret) << "Platform = " << g_platformName[platforms[i]]
            << ", Failed function = Test_QueryConfigProfiles" << endl;

        EXPECT_TRUE((CompareFeatureIDTable(queriedFeatureIDTable, refFeatureIDTable))) << "Platform = "
            << g_platformName[platforms[i]] << ", Failed function = CompareFeatureIDTable" << endl;

        ret = m_driverLoader.CloseDriver();
        EXPECT_EQ (VA_STATUS_SUCCESS , ret) << "Platform = " << g_platformName[platforms[i]]
            << ", Failed function = m_driverLoader.CloseDriver" << endl;
    }
}

int testfunction(int a)
{
    return a + 1;
}

TEST_F(MediaCapsDdiTest, TraceInterfaceTest)
{
    //EXPECT_EQ(0, _TR_COUNT()); //compiler dependent behavior
    EXPECT_EQ(1, _TR_COUNT(1));
    EXPECT_EQ(2, _TR_COUNT(1, 2));
    EXPECT_EQ(10, _TR_COUNT(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));

    int i = 1;
    char b = 2;
    uint64_t l = 3;
    char a[] = {1, 2};

    {
        TR_FILL_PARAM(i, b);
        EXPECT_EQ(5, sizeof(_buf));
        EXPECT_EQ(1, *(int *)_buf);
    }
    {
        TR_FILL_PARAM(b, l);
        EXPECT_EQ(9, sizeof(_buf));
        EXPECT_EQ(2, *(char *)_buf);
    }
    {
        TR_FILL_PARAM(l, a);
        EXPECT_EQ(10, sizeof(_buf));
        EXPECT_EQ(3, *(uint64_t *)_buf);
    }
    {
        TR_FILL_PARAM(b, (int)l);
        EXPECT_EQ(5, sizeof(_buf));
        EXPECT_EQ(3, *(int *)(_buf + 1));
    }
    {
        TR_FILL_PARAM(b, *(short *)a);
        EXPECT_EQ(3, sizeof(_buf));
        EXPECT_EQ(0x201, *(short *)(_buf + 1));
    }
    {
        TR_FILL_PARAM(b, testfunction(2));
        EXPECT_EQ(5, sizeof(_buf));
        EXPECT_EQ(3, *(int *)(_buf + 1));
    }
}
