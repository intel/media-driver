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
#include "ddi_test_decode.h"
TEST_F(MediaDecodeDdiTest, DecodeHEVCLong)
{
    DecTestData* pDecData = decDataFactory.GetDecTestData("HEVC-Long");
    ExectueDecodeTest(pDecData);
    delete pDecData;
}

TEST_F(MediaDecodeDdiTest, DecodeAVCLong)
{
    DecTestData* pDecData = decDataFactory.GetDecTestData("AVC-Long");
    ExectueDecodeTest(pDecData);
    delete pDecData;
}

void MediaDecodeDdiTest::ExectueDecodeTest(DecTestData* pDecData)
{
    vector<Platform_t> platforms = driverLoader.GetPlatforms();
    for (int i=0; i<driverLoader.GetPlatformNum(); i++)
    {
        if (decTestCfg.IsDecTestEnabled(DeviceConfigTable[platforms[i]], pDecData->GetFeatureID()))
        {
            DecodeExecute(pDecData, platforms[i]);
        }
    }
}

void MediaDecodeDdiTest::DecodeExecute(DecTestData* pDecData, Platform_t platform)
{
    int ret =0;
    VAConfigID m_config_id;
    VAContextID m_context_id;
    VAConfigAttrib* m_attribList;
    VAStatus va_status;
    VASurfaceStatus      surface_status;
    ret = driverLoader.InitDriver(platform); //So far we still use DeviceConfigTable to find the platform, as the libdrm mock use this. If we want to use vector Platforms, we would use vector in libdrm too.
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );
    //Query Attribute list for settings.
    //DdiMedia_GetConfigAttributes();
    
    //The attribute only use RCType and FEI function type in createconfig.
    ret = driverLoader.ctx.vtable->vaCreateConfig(&driverLoader.ctx, pDecData->GetFeatureID().profile, pDecData->GetFeatureID().entrypoint, (VAConfigAttrib *)&(pDecData->GetConfAttrib()[0]), pDecData->GetConfAttrib().size(),&m_config_id);
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );

    vector<VASurfaceID> &resources = pDecData->GetResources();
    ret = driverLoader.ctx.vtable->vaCreateSurfaces2(&driverLoader.ctx, VA_RT_FORMAT_YUV420,pDecData->GetWidth(), pDecData->GetHeight(), &resources[0], resources.size(),NULL, 0 );
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );
    ret = driverLoader.ctx.vtable->vaCreateContext(&driverLoader.ctx,m_config_id, pDecData->GetWidth(), pDecData->GetHeight(),VA_PROGRESSIVE, &resources[0], resources.size(), &m_context_id);
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );

    for (int i=0; i<pDecData->num_frames; i++){ //loop several frames.

        //As BeginPicture would reset some parameters, so it should be called before RenderPicture.
        ret = driverLoader.ctx.vtable->vaBeginPicture(&driverLoader.ctx,m_context_id,resources[0]);
        EXPECT_EQ (VA_STATUS_SUCCESS , ret );

        vector<vector<CompBufConif>> &compBufs=pDecData->GetCompBuffers();
        for (int j=0; j<compBufs[i].size(); j++)
        {
            ret = driverLoader.ctx.vtable->vaCreateBuffer(&driverLoader.ctx, m_context_id, compBufs[i][j].BufType,  compBufs[i][j].BufSize, 1, compBufs[i][j].pData,&compBufs[i][j].BufID);
            EXPECT_EQ (VA_STATUS_SUCCESS , ret );
        }
        pDecData->UpdateCompBuffers(i);
        //In RenderPicture, it suppose all needed buffer has been created already.
        for (int j=0; j<compBufs[i].size(); j++)
        {
            ret = driverLoader.ctx.vtable->vaRenderPicture(&driverLoader.ctx,m_context_id, &compBufs[i][j].BufID, 1);
            EXPECT_EQ (VA_STATUS_SUCCESS , ret );
        }
        ret = driverLoader.ctx.vtable->vaEndPicture(&driverLoader.ctx,m_context_id);
        EXPECT_EQ (VA_STATUS_SUCCESS , ret );
        //For decoder, SyncSurface would call GetStatusReport, so we don't call it now.
        //ret = driverLoader.ctx.vtable->vaSyncSurface(&driverLoader.ctx, resources[0]);//Do we need this?
        //EXPECT_EQ (VA_STATUS_SUCCESS , ret );
        do {
            ret = driverLoader.ctx.vtable->vaQuerySurfaceStatus(&driverLoader.ctx,resources[0], &surface_status);
        }while (surface_status != VASurfaceReady);

        for (int j=0; j<compBufs[i].size(); j++)
        {
            ret = driverLoader.ctx.vtable->vaDestroyBuffer(&driverLoader.ctx, compBufs[i][j].BufID);
            EXPECT_EQ (VA_STATUS_SUCCESS , ret );
        }
      }

    ret = driverLoader.ctx.vtable->vaDestroySurfaces(&driverLoader.ctx, &resources[0], resources.size());
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );
    ret = driverLoader.ctx.vtable->vaDestroyContext(&driverLoader.ctx , m_context_id);
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );
    ret = driverLoader.ctx.vtable->vaDestroyConfig(&driverLoader.ctx, m_config_id );
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );
    ret = driverLoader.CloseDriver();
    //ret = driverLoader.ctx.vtable->vaTerminate(&driverLoader.ctx );
    EXPECT_EQ (VA_STATUS_SUCCESS , ret );

}

DecodeTestConfig::DecodeTestConfig()
{
    mapPlatformFeatureID[DeviceConfigTable[igfxCANNONLAKE]] = {
        TEST_Intel_Decode_HEVC,
        TEST_Intel_Decode_AVC
    };
    mapPlatformFeatureID[DeviceConfigTable[igfxSKLAKE]] = {
        TEST_Intel_Decode_HEVC,
        TEST_Intel_Decode_AVC
    };
    mapPlatformFeatureID[DeviceConfigTable[igfxBROXTON]] = {
        TEST_Intel_Decode_HEVC,
        TEST_Intel_Decode_AVC
    };
    mapPlatformFeatureID[DeviceConfigTable[igfxBROADWELL]] = {
        TEST_Intel_Decode_AVC
    };

}

DecodeTestConfig::~DecodeTestConfig()
{
}

bool DecodeTestConfig::IsDecTestEnabled(DeviceConfig platform, FeatureID featureId)
{
    bool bEnable = false;
    vector<FeatureID> FeatureIDArray = mapPlatformFeatureID[platform];

    for (int i=0; i< FeatureIDArray.size();i++)
    {
        //Infact, we may need to call QueryEntroyPoint to make sure it does have this config. But we suppose this test is done in Caps test.
        //Otherwise, here is need to call QueryEntroyPoint to check if it's supported.
        if (featureId == FeatureIDArray[i]) 
        {
            bEnable = true;
            break;
        }
    }

    return bEnable;

}
