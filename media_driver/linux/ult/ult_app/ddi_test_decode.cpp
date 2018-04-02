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
#include "ddi_test_decode.h"

using namespace std;

TEST_F(MediaDecodeDdiTest, DecodeHEVCLong)
{
    DecTestData *pDecData = m_decDataFactory.GetDecTestData("HEVC-Long");
    ExectueDecodeTest(pDecData);
    delete pDecData;
}

TEST_F(MediaDecodeDdiTest, DecodeAVCLong)
{
    DecTestData *pDecData = m_decDataFactory.GetDecTestData("AVC-Long");
    ExectueDecodeTest(pDecData);
    delete pDecData;
}

void MediaDecodeDdiTest::ExectueDecodeTest(DecTestData *pDecData)
{
    vector<Platform_t> platforms = m_driverLoader.GetPlatforms();
    for (int i = 0; i < m_driverLoader.GetPlatformNum(); i++)
    {
        if (m_decTestCfg.IsDecTestEnabled(DeviceConfigTable[platforms[i]],
            pDecData->GetFeatureID()))
        {
            DecodeExecute(pDecData, platforms[i]);
        }
    }
}

void MediaDecodeDdiTest::DecodeExecute(DecTestData *pDecData, Platform_t platform)
{
    VAConfigID      config_id;
    VAContextID     context_id;
    VASurfaceStatus surface_status;

    // So far we still use DeviceConfigTable to find the platform, as the libdrm mock use this.
    // If we want to use vector Platforms, we would use vector in libdrm too.
    int ret = m_driverLoader.InitDriver(platform);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.InitDriver" << endl;

    // The attribute only use RCType and FEI function type in createconfig.
    ret = m_driverLoader.m_ctx.vtable->vaCreateConfig(&m_driverLoader.m_ctx,
        pDecData->GetFeatureID().profile, pDecData->GetFeatureID().entrypoint,
        (VAConfigAttrib *)&(pDecData->GetConfAttrib()[0]), pDecData->GetConfAttrib().size(), &config_id);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateConfig" << endl;

    vector<VASurfaceID> &resources = pDecData->GetResources();
    ret = m_driverLoader.m_ctx.vtable->vaCreateSurfaces2(&m_driverLoader.m_ctx, VA_RT_FORMAT_YUV420,
        pDecData->GetWidth(), pDecData->GetHeight(), &resources[0], resources.size(), nullptr, 0);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateSurfaces2" << endl;

    ret = m_driverLoader.m_ctx.vtable->vaCreateContext(&m_driverLoader.m_ctx, config_id, pDecData->GetWidth(),
        pDecData->GetHeight(),VA_PROGRESSIVE, &resources[0], resources.size(), &context_id);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateContext" << endl;

    for (int i = 0; i < pDecData->m_num_frames; i++)
    {
        // As BeginPicture would reset some parameters, so it should be called before RenderPicture.
        ret = m_driverLoader.m_ctx.vtable->vaBeginPicture(&m_driverLoader.m_ctx, context_id, resources[0]);
        EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
            << ", Failed function = m_driverLoader.m_ctx.vtable->vaBeginPicture" << endl;

        vector<vector<CompBufConif>> &compBufs = pDecData->GetCompBuffers();
        for (int j = 0; j < compBufs[i].size(); j++)
        {
            ret = m_driverLoader.m_ctx.vtable->vaCreateBuffer(&m_driverLoader.m_ctx, context_id,
                compBufs[i][j].bufType, compBufs[i][j].bufSize, 1, compBufs[i][j].pData, &compBufs[i][j].bufID);
            EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
                << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateBuffer" << endl;
        }

        pDecData->UpdateCompBuffers(i);
        for (int j = 0; j < compBufs[i].size(); j++)
        {
            // In RenderPicture, it suppose all needed buffer has been created already.
            ret = m_driverLoader.m_ctx.vtable->vaRenderPicture(&m_driverLoader.m_ctx,
                context_id, &compBufs[i][j].bufID, 1);
            EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
                << ", Failed function = m_driverLoader.m_ctx.vtable->vaRenderPicture" << endl;
        }

        ret = m_driverLoader.m_ctx.vtable->vaEndPicture(&m_driverLoader.m_ctx, context_id);
        EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
            << ", Failed function = m_driverLoader.m_ctx.vtable->vaEndPicture" << endl;

        do
        {
            ret = m_driverLoader.m_ctx.vtable->vaQuerySurfaceStatus(
                &m_driverLoader.m_ctx, resources[0], &surface_status);
        } while (surface_status != VASurfaceReady);

        for (int j = 0; j < compBufs[i].size(); j++)
        {
            ret = m_driverLoader.m_ctx.vtable->vaDestroyBuffer(&m_driverLoader.m_ctx, compBufs[i][j].bufID);
            EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
                << ", Failed function = m_driverLoader.m_ctx.vtable->vaDestroyBuffer" << endl;
        }
      }

    ret = m_driverLoader.m_ctx.vtable->vaDestroySurfaces(&m_driverLoader.m_ctx, &resources[0], resources.size());
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaDestroySurfaces" << endl;

    ret = m_driverLoader.m_ctx.vtable->vaDestroyContext(&m_driverLoader.m_ctx, context_id);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaDestroyContext" << endl;

    ret = m_driverLoader.m_ctx.vtable->vaDestroyConfig(&m_driverLoader.m_ctx, config_id);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaDestroyConfig" << endl;

    ret = m_driverLoader.CloseDriver();
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.CloseDriver" << endl;

    MemoryLeakDetector::Detect(m_driverLoader, platform);
}

DecodeTestConfig::DecodeTestConfig()
{
    m_mapPlatformFeatureID[DeviceConfigTable[igfxCANNONLAKE]] = {
        TEST_Intel_Decode_HEVC,
        TEST_Intel_Decode_AVC ,
    };
    m_mapPlatformFeatureID[DeviceConfigTable[igfxSKLAKE]]     = {
        TEST_Intel_Decode_HEVC,
        TEST_Intel_Decode_AVC ,
    };
    m_mapPlatformFeatureID[DeviceConfigTable[igfxBROXTON]]    = {
        TEST_Intel_Decode_HEVC,
        TEST_Intel_Decode_AVC ,
    };
    m_mapPlatformFeatureID[DeviceConfigTable[igfxBROADWELL]]  = {
        TEST_Intel_Decode_AVC ,
    };
}

bool DecodeTestConfig::IsDecTestEnabled(DeviceConfig platform, FeatureID featureId)
{
    const auto &featureIdArray = m_mapPlatformFeatureID[platform];
    for (int i = 0; i < featureIdArray.size(); i++)
    {
        // In fact, we may need to call QueryEntroyPoint to make sure it does have this config.
        // But we suppose this test is done in Caps test.
        // Otherwise, here is need to call QueryEntroyPoint to check if it's supported.
        if (featureId == featureIdArray[i])
        {
            return true;
        }
    }

    return false;
}
