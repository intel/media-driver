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
#include "ddi_test_encode.h"

using namespace std;

TEST_F(MediaEncodeDdiTest, EncodeHEVC_DualPipe)
{
    m_GpuCmdFactory = g_gpuCmdFactoryEncodeHevcDualPipe;
    EncTestData *pEncData = m_encTestFactory.GetEncTestData("HEVC-DualPipe");
    ExectueEncodeTest(pEncData);
    delete pEncData;
}

TEST_F(MediaEncodeDdiTest, EncodeAVC_DualPipe)
{
    m_GpuCmdFactory = g_gpuCmdFactoryEncodeAvcDualPipe;
    EncTestData *pEncData = m_encTestFactory.GetEncTestData("AVC-DualPipe");
    ExectueEncodeTest(pEncData);
    delete pEncData;
}

void MediaEncodeDdiTest::ExectueEncodeTest(EncTestData *pEncData)
{
    vector<Platform_t> platforms = m_driverLoader.GetPlatforms();
    for (int i = 0; i < m_driverLoader.GetPlatformNum(); i++)
    {
        if (m_encTestCfg.IsEncTestEnabled(DeviceConfigTable[platforms[i]],
            pEncData->GetFeatureID()))
        {
            CmdValidator::GpuCmdsValidationInit(m_GpuCmdFactory, platforms[i]);
            EncodeExecute(pEncData, platforms[i]);
        }
    }
}

void MediaEncodeDdiTest::EncodeExecute(EncTestData *pEncData, Platform_t platform)
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
        pEncData->GetFeatureID().profile, pEncData->GetFeatureID().entrypoint,
        (VAConfigAttrib *)&(pEncData->GetConfAttrib()[0]), pEncData->GetConfAttrib().size(), &config_id);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateConfig" << endl;

    vector<VASurfaceID> &resources = pEncData->GetResources();
    ret = m_driverLoader.m_ctx.vtable->vaCreateSurfaces2(&m_driverLoader.m_ctx, VA_RT_FORMAT_YUV420,
        pEncData->GetWidth(), pEncData->GetHeight(), &resources[0], resources.size(),
        (VASurfaceAttrib *)&(pEncData->GetSurfAttrib()[0]), pEncData->GetSurfAttrib().size());
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateSurfaces2" << endl;

    ret = m_driverLoader.m_ctx.vtable->vaCreateContext(&m_driverLoader.m_ctx, config_id, pEncData->GetWidth(),
        pEncData->GetHeight(), VA_PROGRESSIVE, &resources[0], resources.size(), &context_id);
    EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
        << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateContext" << endl;

    for (int i = 0; i < pEncData->m_num_frames; i++)
    {
        ret = m_driverLoader.m_ctx.vtable->vaBeginPicture(&m_driverLoader.m_ctx, context_id,resources[0]);

        vector<vector<CompBufConif>> &compBufs = pEncData->GetCompBuffers();
        ret = m_driverLoader.m_ctx.vtable->vaCreateBuffer(&m_driverLoader.m_ctx, context_id, compBufs[i][0].bufType,
            compBufs[i][0].bufSize, 1, compBufs[i][0].pData, &compBufs[i][0].bufID);
        EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
            << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateBuffer" << endl;

        pEncData->UpdateCompBuffers(i);
        for (int j = 1; j < compBufs[i].size(); j++)
        {
            ret = m_driverLoader.m_ctx.vtable->vaCreateBuffer(&m_driverLoader.m_ctx, context_id,
                compBufs[i][j].bufType, compBufs[i][j].bufSize, 1, compBufs[i][j].pData, &compBufs[i][j].bufID);
            EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
                << ", Failed function = m_driverLoader.m_ctx.vtable->vaCreateBuffer" << endl;

            // In RenderPicture, it suppose all needed buffer has been created already.
            // Suppose the compBufs[0] is always EncCodedBuffer, so we won't render it.
            // If we render it, the ret is still Success, but would with log"not supported
            // buffer type in vpgEncodeRenderPicture."
            ret = m_driverLoader.m_ctx.vtable->vaRenderPicture(&m_driverLoader.m_ctx,
                context_id, &compBufs[i][j].bufID, 1);
            EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
                << ", Failed function = m_driverLoader.m_ctx.vtable->vaRenderPicture" << endl;
        }

        ret = m_driverLoader.m_ctx.vtable->vaEndPicture(&m_driverLoader.m_ctx, context_id);
        EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
            << ", Failed function = m_driverLoader.m_ctx.vtable->vaEndPicture" << endl;

        ret = m_driverLoader.m_ctx.vtable->vaSyncSurface(&m_driverLoader.m_ctx, resources[0]);
        EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
            << ", Failed function = m_driverLoader.m_ctx.vtable->vaSyncSurface" << endl;

        do
        {
            ret = m_driverLoader.m_ctx.vtable->vaQuerySurfaceStatus(&m_driverLoader.m_ctx,
                resources[0], &surface_status);
        } while (surface_status != VASurfaceReady);

        for (int j = 0; j < compBufs[i].size(); j++)
        {
            ret = m_driverLoader.m_ctx.vtable->vaDestroyBuffer(&m_driverLoader.m_ctx, compBufs[i][j].bufID);
            EXPECT_EQ(VA_STATUS_SUCCESS, ret) << "Platform = " << g_platformName[platform]
                << ", Failed function = m_driverLoader.m_ctx.vtable->vaDestroyBuffer" << endl;
        }
      }

    ret = m_driverLoader.m_ctx.vtable->vaDestroySurfaces(&m_driverLoader.m_ctx,
        &resources[0], resources.size());
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
}

EncodeTestConfig::EncodeTestConfig()
{
    m_mapPlatformFeatureID[DeviceConfigTable[igfxSKLAKE]]     = {
        TEST_Intel_Encode_HEVC,
        TEST_Intel_Encode_AVC ,
    };
    m_mapPlatformFeatureID[DeviceConfigTable[igfxBROXTON]]    = {
        TEST_Intel_Encode_HEVC,
        TEST_Intel_Encode_AVC ,
    };
    m_mapPlatformFeatureID[DeviceConfigTable[igfxBROADWELL]]  = {
        TEST_Intel_Encode_AVC ,
    };
}

bool EncodeTestConfig::IsEncTestEnabled(DeviceConfig platform, FeatureID featureId)
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
