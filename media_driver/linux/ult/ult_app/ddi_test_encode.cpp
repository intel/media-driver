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
#include "ddi_test_encode.h"

TEST_F(MediaEncodeDdiTest, EncodeHEVC_DualPipe)
{
    EncTestData* pEncData = encTestFactory.GetEncTestData("HEVC-DualPipe");
    ExectueEncodeTest(pEncData);
    delete pEncData;
}

TEST_F(MediaEncodeDdiTest, EncodeAVC_DualPipe)
{
    EncTestData* pEncData = encTestFactory.GetEncTestData("AVC-DualPipe");
    ExectueEncodeTest(pEncData);
    delete pEncData;
}

void MediaEncodeDdiTest::ExectueEncodeTest(EncTestData* pEncData)
{
    vector<Platform_t> platforms = driverLoader.GetPlatforms();
    for (int i=0; i<driverLoader.GetPlatformNum(); i++)
    {
        if (encTestCfg.IsEncTestEnabled(DeviceConfigTable[platforms[i]], pEncData->GetFeatureID()))
        {
            EncodeExecute(pEncData, platforms[i]);
        }
    }
}

void MediaEncodeDdiTest::EncodeExecute(EncTestData* pEncData, Platform_t platform)
{
    int ret =0;
    VAConfigID m_config_id;
    VAContextID m_context_id;
    VAConfigAttrib* m_attribList;
    VAStatus va_status;
    VASurfaceStatus      surface_status;

    ret = driverLoader.InitDriver(platform); //So far we still use DeviceConfigTable to find the platform, as the libdrm mock use this. If we want to use vector Platforms, we would use vector in libdrm too.
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.InitDriver" << endl;

    //Query Attribute list for settings.
    //DdiMedia_GetConfigAttributes();

    //The attribute only use RCType and FEI function type in createconfig.
    ret = driverLoader.ctx.vtable->vaCreateConfig(&driverLoader.ctx, pEncData->GetFeatureID().profile, pEncData->GetFeatureID().entrypoint, (VAConfigAttrib *)&(pEncData->GetConfAttrib()[0]), pEncData->GetConfAttrib().size(),&m_config_id);
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaCreateConfig" << endl;

    vector<VASurfaceID> &resources = pEncData->GetResources();
    ret = driverLoader.ctx.vtable->vaCreateSurfaces2(&driverLoader.ctx, VA_RT_FORMAT_YUV420,pEncData->GetWidth(), pEncData->GetHeight(), &resources[0], resources.size(),(VASurfaceAttrib *)&(pEncData->GetSurfAttrib()[0]), pEncData->GetSurfAttrib().size() );
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaCreateSurfaces2" << endl;
    ret = driverLoader.ctx.vtable->vaCreateContext(&driverLoader.ctx,m_config_id, pEncData->GetWidth(), pEncData->GetHeight(),VA_PROGRESSIVE, &resources[0], resources.size(), &m_context_id);
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaCreateContext" << endl;

    for (int i=0; i<pEncData->num_frames; i++){ //loop several frames.

        ret = driverLoader.ctx.vtable->vaBeginPicture(&driverLoader.ctx,m_context_id,resources[0]);

        vector<vector<CompBufConif>> &compBufs=pEncData->GetCompBuffers();
        ret = driverLoader.ctx.vtable->vaCreateBuffer(&driverLoader.ctx, m_context_id, compBufs[i][0].BufType,	compBufs[i][0].BufSize, 1, compBufs[i][0].pData,&compBufs[i][0].BufID);
        EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaCreateBuffer" << endl;

        pEncData->UpdateCompBuffers(i);
        //In RenderPicture, it suppose all needed buffer has been created already.
        //Suppose the compBufs[0] is always EncCodedBuffer, so we won't render it.
        //If we render it, the ret is still Success, but would with log"not supported buffer type in vpgEncodeRenderPicture."
        for (int j=1; j<compBufs[i].size(); j++)
        {
            ret = driverLoader.ctx.vtable->vaCreateBuffer(&driverLoader.ctx, m_context_id, compBufs[i][j].BufType,  compBufs[i][j].BufSize, 1, compBufs[i][j].pData,&compBufs[i][j].BufID);
            EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaCreateBuffer" << endl;
            ret = driverLoader.ctx.vtable->vaRenderPicture(&driverLoader.ctx,m_context_id, &compBufs[i][j].BufID, 1);
            EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaRenderPicture" << endl;
        }
        ret = driverLoader.ctx.vtable->vaEndPicture(&driverLoader.ctx,m_context_id);
        EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaEndPicture" << endl;
        ret = driverLoader.ctx.vtable->vaSyncSurface(&driverLoader.ctx, resources[0]);
        EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaSyncSurface" << endl;
        do {
            ret = driverLoader.ctx.vtable->vaQuerySurfaceStatus(&driverLoader.ctx,resources[0], &surface_status);
        }while (surface_status != VASurfaceReady);
        /*
        //As MapBuffer would always call DdiEncode_StatusReport to read Status data written by HW, so we cannot call this function.
        ret = driverLoader.ctx.vtable->vaMapBuffer(&driverLoader.ctx,CompBufList[0].BufID,(void **) (&coded_buffer_segment));
        EXPECT_EQ (VA_STATUS_SUCCESS , ret );
        if(fp){
            fseek(fp, filesize, SEEK_SET);
            fwrite(coded_buffer_segment->buf, 1, coded_buffer_segment->size, fp);
        }
        filesize+=coded_buffer_segment->size;
        ret = driverLoader.ctx.vtable->vaUnmapBuffer(&driverLoader.ctx,CompBufList[0].BufID);
        */
        for (int j=0; j<compBufs[i].size(); j++)
        {
            ret = driverLoader.ctx.vtable->vaDestroyBuffer(&driverLoader.ctx, compBufs[i][j].BufID);
            EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaDestroyBuffer" << endl;
        }
      }


    ret = driverLoader.ctx.vtable->vaDestroySurfaces(&driverLoader.ctx, &resources[0], resources.size());
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaDestroySurfaces" << endl;
    ret = driverLoader.ctx.vtable->vaDestroyContext(&driverLoader.ctx , m_context_id);
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaDestroyContext" << endl;
    ret = driverLoader.ctx.vtable->vaDestroyConfig(&driverLoader.ctx, m_config_id );
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.ctx.vtable->vaDestroyConfig" << endl;
    ret = driverLoader.CloseDriver();
    //ret = driverLoader.ctx.vtable->vaTerminate(&driverLoader.ctx );
    EXPECT_EQ (VA_STATUS_SUCCESS , ret ) << "Platform = " << platform << ", Failed function = driverLoader.CloseDriver" << endl;

}

EncodeTestConfig::EncodeTestConfig()
{
    mapPlatformFeatureID[DeviceConfigTable[igfxCANNONLAKE]] = {
        TEST_Intel_Encode_HEVC,
        TEST_Intel_Encode_AVC
    };
    mapPlatformFeatureID[DeviceConfigTable[igfxSKLAKE]] = {
        TEST_Intel_Encode_HEVC,
        TEST_Intel_Encode_AVC
    };
    mapPlatformFeatureID[DeviceConfigTable[igfxBROXTON]] = {
        TEST_Intel_Encode_HEVC,
        TEST_Intel_Encode_AVC
    };
    mapPlatformFeatureID[DeviceConfigTable[igfxBROADWELL]] = {
        TEST_Intel_Encode_AVC
    };

}

EncodeTestConfig::~EncodeTestConfig()
{

}

bool EncodeTestConfig::IsEncTestEnabled(DeviceConfig platform, FeatureID featureId)
{
    bool bEnable = false;
    vector<FeatureID> FeatureIDArray = mapPlatformFeatureID[platform];

    for (int i=0; i< FeatureIDArray.size();i++)//each(FeatureID i in FeatureIDArray)
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
