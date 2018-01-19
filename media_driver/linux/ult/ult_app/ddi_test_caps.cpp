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
#include "ddi_test_caps.h"
#include<algorithm>
#include <string>

bool CompareFeatureIDTable(vector<FeatureID> &currentTable, vector<FeatureID> &refTable)
{
    if (currentTable.size() != refTable.size())
    {
        return false;
    }
    sort(currentTable.begin(), currentTable.end());
    sort(refTable.begin(), refTable.end());
    return equal(currentTable.begin(), currentTable.end(), refTable.begin());
}

int Test_QueryConfigProfiles(VADriverContextP ctx, vector<FeatureID> &queriedFeatureIDTable){
    VAProfile profile, *profile_list = NULL;
    int num_profiles, max_num_profiles, i;
    VAEntrypoint entrypoint, entrypoints[10];
    int num_entrypoint;
    int ret =0;

    max_num_profiles = ctx->max_profiles;
    profile_list = (VAProfile*)malloc(max_num_profiles * sizeof(VAProfile));

    if (!profile_list) {
        printf("Failed to allocate memory for profile list\n");
        return -1;
    }
    ret = ctx->vtable->vaQueryConfigProfiles(ctx, profile_list, &num_profiles);
    if (ret)
        return -1;

    for (i = 0; i < num_profiles; i++) {
        char *profile_str;

        profile = profile_list[i];
        ret= ctx->vtable->vaQueryConfigEntrypoints(ctx, profile, entrypoints,
                                             &num_entrypoint);
        if (ret == VA_STATUS_ERROR_UNSUPPORTED_PROFILE)
            continue;
        else if (ret)
            return -1;
        for (int j = 0; j < num_entrypoint; j++)
        {
            queriedFeatureIDTable.push_back({profile, entrypoints[j]});
        }
    }

    return ret;
}
TEST_F(MediaCapsDdiTest, DecodeEncodeProfile)
{
    int ret=0;
    int i=0;
    vector<Platform_t> platforms = driverLoader.GetPlatforms();

    for (i=0; i<driverLoader.GetPlatformNum(); i++)
    {
       vector<FeatureID> refFeatureIDTable = capsData.GetRefFeatureIDTable(DeviceConfigTable[platforms[i]]);//(platforms[i]);//
       vector<FeatureID> queriedFeatureIDTable;
       ret = driverLoader.InitDriver(platforms[i]); //So far we still use DeviceConfigTable to find the platform, as the libdrm mock use this. If we want to use vector Platforms, we would use vector in libdrm too.
       EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platforms[i] << ", Failed function = driverLoader.InitDriver" << endl;
       ret = Test_QueryConfigProfiles(&driverLoader.ctx, queriedFeatureIDTable);
       EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platforms[i] << ", Failed function = Test_QueryConfigProfiles" << endl;
       EXPECT_TRUE((CompareFeatureIDTable(queriedFeatureIDTable, refFeatureIDTable))) << "Platform = " << platforms[i] << ", Failed function = CompareFeatureIDTable" << endl;
       ret = driverLoader.CloseDriver();
       //ret = driverLoader.ctx.vtable->vaTerminate(&driverLoader.ctx);
       EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platforms[i] << ", Failed function = driverLoader.CloseDriver" << endl;
    }
}
