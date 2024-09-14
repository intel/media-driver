/*
* Copyright (c) 2021-2024, Intel Corporation
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
//!
//! \file     encode_aqm_feature.cpp
//! \brief    Defines the common interface for aqm feature
//!

#include <iomanip>

#include "encode_aqm_feature.h"
#include "encode_feature_manager.h"
#include "encode_utils.h"
#include "media_perf_profiler.h"

namespace encode
{
EncodeAqmFeature::EncodeAqmFeature(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *               constSettings) : MediaFeature(constSettings),
                           m_hwInterface(hwInterface),
                           m_allocator(allocator)
{
    ENCODE_FUNC_CALL();

    m_featureManager = featureManager;

    auto encFeatureManager = dynamic_cast<EncodeFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    if (hwInterface)
    {
        m_mosCtx = hwInterface->GetOsInterface()->pOsContext;
        m_userSettingPtr = hwInterface->GetOsInterface()->pfnGetUserSettingInstance(hwInterface->GetOsInterface());
    }

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

#if (_DEBUG || _RELEASE_INTERNAL)
    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "VDAQM Enable",
        MediaUserSetting::Group::Sequence);

    m_metricsDumpMode   = outValue.Get<uint8_t>();

#endif

    if (m_metricsDumpMode > 0)
    {
        m_enabled = true;
    }
#if _MEDIA_RESERVED
#if USE_CODECHAL_DEBUG_TOOL
    MetricsDump();
#endif
#endif
}

EncodeAqmFeature::~EncodeAqmFeature()
{
    if (m_enabled)
    {
        FreeResources();
#if _MEDIA_RESERVED
#if USE_CODECHAL_DEBUG_TOOL
        CloseDumpFiles();
#endif
#endif
    }
}

uint32_t EncodeAqmFeature::EncodeAqmFeatureFunction0(uint32_t frameWidth, uint32_t frameHeight, uint8_t index)
{
    uint32_t width  = (frameWidth + (1 << index) - 1) >> index;
    uint32_t height = (frameHeight + (1 << index) - 1) >> index;
    return (MOS_ALIGN_CEIL(MOS_ALIGN_CEIL(width, 4) / 4, CL_SIZE_BYTES) * (MOS_ALIGN_CEIL(height, 4) / 4));
}

MOS_STATUS EncodeAqmFeature::Update(void *params)
{
    if (m_enabled)
    {
        ENCODE_CHK_STATUS_RETURN(AllocateResources());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAqmFeature::AllocateResources()
{
    ENCODE_FUNC_CALL();

    if (m_AllocatedResources)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;

    uint32_t rowstoreBufferSize[5] = {(m_basicFeature->m_oriFrameWidth / 4 + 1), 3391, 1665, 833, 417};
    for (int i = 0; i < 5; i++)
    {
        std::string bufName                 = "Index" + std::to_string(i) + "LineRowstoreBuffer";
        allocParamsForBufferLinear.dwBytes  = rowstoreBufferSize[i] * CL_SIZE_BYTES * m_numRowStore;
        allocParamsForBufferLinear.pBufName = &*bufName.begin();
        EncodeAqmFeatureMember0[i]          = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        EncodeAqmFeatureMember1[i]          = rowstoreBufferSize[i] * CL_SIZE_BYTES;
    }

    EncodeAqmFeatureMember2    = ((sizeof(AQM_Ouput_Format) + CL_SIZE_BYTES - 1) / CL_SIZE_BYTES) * CL_SIZE_BYTES * m_numTiles;
    EncodeAqmFeatureMember3[0] = 0;
    EncodeAqmFeatureMember3[1] = EncodeAqmFeatureFunction0(m_basicFeature->m_oriFrameWidth, m_basicFeature->m_oriFrameHeight, 1);
    EncodeAqmFeatureMember3[2] = EncodeAqmFeatureFunction0(m_basicFeature->m_oriFrameWidth, m_basicFeature->m_oriFrameHeight, 2);
    EncodeAqmFeatureMember3[3] = EncodeAqmFeatureFunction0(m_basicFeature->m_oriFrameWidth, m_basicFeature->m_oriFrameHeight, 3);
    EncodeAqmFeatureMember3[4] = EncodeAqmFeatureFunction0(m_basicFeature->m_oriFrameWidth, m_basicFeature->m_oriFrameHeight, 4);

    // allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    
    allocParamsForBufferLinear.dwBytes  = EncodeAqmFeatureMember2;
    allocParamsForBufferLinear.pBufName = "VdaqmBuffer0";
    m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::VdaqmBuffer0, allocParamsForBufferLinear, EncodeBasicFeature::m_uncompressedSurfaceNum);

    allocParamsForBufferLinear.dwBytes  = EncodeAqmFeatureMember3[1];
    allocParamsForBufferLinear.pBufName = "VdaqmBuffer1";
    m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::VdaqmBuffer1, allocParamsForBufferLinear, EncodeBasicFeature::m_uncompressedSurfaceNum);

    allocParamsForBufferLinear.dwBytes  = EncodeAqmFeatureMember3[2];
    allocParamsForBufferLinear.pBufName = "VdaqmBuffer2";
    m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::VdaqmBuffer2, allocParamsForBufferLinear, EncodeBasicFeature::m_uncompressedSurfaceNum);

    allocParamsForBufferLinear.dwBytes  = EncodeAqmFeatureMember3[3];
    allocParamsForBufferLinear.pBufName = "VdaqmBuffer3";
    m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::VdaqmBuffer3, allocParamsForBufferLinear, EncodeBasicFeature::m_uncompressedSurfaceNum);

    allocParamsForBufferLinear.dwBytes  = EncodeAqmFeatureMember3[4];
    allocParamsForBufferLinear.pBufName = "VdaqmBuffer4";
    m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::VdaqmBuffer4, allocParamsForBufferLinear, EncodeBasicFeature::m_uncompressedSurfaceNum);

    m_AllocatedResources = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAqmFeature::FreeResources()
{
    ENCODE_FUNC_CALL();

    if (m_AllocatedResources)
    {
        for (uint8_t index = 0; index < AQM_INDEX; index++)
            m_allocator->DestroyResource(EncodeAqmFeatureMember0[index]);
    }

    m_AllocatedResources = false;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_FRAME_START, EncodeAqmFeature)
{
    if (m_enabled)
    {
        params.aqmFrameStart = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_PIC_STATE, EncodeAqmFeature)
{
    if (m_enabled)
    {
        params.frameWidthInPixelMinus1  = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameWidth, 8) - 1;
        params.FrameHeightInPixelMinus1 = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameHeight, 8) - 1;
        params.vdaqmEnable              = m_enabled;
        params.tileBasedEngine          = m_tileBasedEngine;
        params.chromasubsampling        = m_basicFeature->m_chromaFormat - 1;
        params.aqmMode                  = m_aqmMode;
        params.sseEnable                = true;
#if _MEDIA_RESERVED
#define AQM_PIC_STATE_SETPAR_EXT
#include "encode_aqm_feature_ext.h"
#undef AQM_PIC_STATE_SETPAR_EXT
#else
        params.extSettings.emplace_back(
            [this](uint32_t *data) {
                data[2]  |= 0x1e;
                data[3]  |= 0x1a334d66;
                data[4]  |= 0x809ab3cd;
                data[5]  |= 0xe6000000;
                data[6]  |= 0x80001;
                data[7]  |= 0x700025;
                data[8]  |= 0x11000da;
                data[9]  |= 0x80001;
                data[10] |= 0x700025;
                data[11] |= 0x11000da;

                return MOS_STATUS_SUCCESS;
            });
#endif
        if (m_basicFeature->m_bitDepth == 8)
        {
            params.pixelbitdepth = 0;
        }
        else if (m_basicFeature->m_bitDepth == 10)
        {
            params.pixelbitdepth = 1;
        }
        else if (m_basicFeature->m_bitDepth == 12)
        {
            params.pixelbitdepth = 2;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_PIPE_BUF_ADDR_STATE, EncodeAqmFeature)
{
    if (m_enabled)
    {
        auto bIdx = m_basicFeature->m_currOriginalPic.FrameIdx;

        for (uint8_t index = 0; index < AQM_INDEX; index++)
        {
            params.AqmPipeBufAddrStatePar0[index] = EncodeAqmFeatureMember0[index];
            params.AqmPipeBufAddrStatePar1[index] = EncodeAqmFeatureMember1[index] * m_currPipeNum;
        }

        params.AqmPipeBufAddrStatePar4[0] = nullptr;
        params.AqmPipeBufAddrStatePar4[1] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer1, bIdx);
        params.AqmPipeBufAddrStatePar4[2] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer2, bIdx);
        params.AqmPipeBufAddrStatePar4[3] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer3, bIdx);
        params.AqmPipeBufAddrStatePar4[4] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer4, bIdx);
        params.AqmPipeBufAddrStatePar2    = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer0, bIdx);
    }

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS EncodeAqmFeature::UpdateFrameDisplayOrder(const uint16_t pictureCodingType, const uint32_t framePOC, const uint32_t gopPicSize)
{
    if (pictureCodingType == I_TYPE)
    {
        m_frameNumPrevious += m_gopSizePrevious;
    }
    uint32_t    displayOrderInGOP   = framePOC;
    uint32_t    displayOrderInSeq   = displayOrderInGOP + m_frameNumPrevious;
    m_gopSizePrevious               = gopPicSize;
    m_frameIdxQueue.push(displayOrderInSeq);
    return MOS_STATUS_SUCCESS;
}
#endif

#if _MEDIA_RESERVED
#define AQM_FEATURE_SOURCE_EXT
#include "encode_aqm_feature_ext.h"
#undef AQM_FEATURE_SOURCE_EXT
#endif 

MOS_STATUS EncodeAqmFeature::GetFrameMSE(AQM_Ouput_Format* pDataFrame, uint32_t (&MSE)[3])
{
    ENCODE_CHK_NULL_RETURN(pDataFrame);

    uint64_t SSE[3] = {};
    uint32_t areaSum = 0;

    // Get Tile MSE
    uint32_t offsetAqmTile = MOS_ALIGN_CEIL(sizeof(AQM_Ouput_Format), CL_SIZE_BYTES);
    for (uint32_t tileIdx = 0; tileIdx < m_numTiles && tileIdx < ENCODE_VDENC_MAX_TILE_NUM; tileIdx++)
    {
        AQM_Ouput_Format* pTileVdaqmInfo = (AQM_Ouput_Format*)((char*)pDataFrame + offsetAqmTile * tileIdx);
        ENCODE_CHK_NULL_RETURN(pTileVdaqmInfo);

        uint32_t area   =   m_tile_width[tileIdx] * m_tile_height[tileIdx];
        SSE[0]          +=  (uint64_t)pTileVdaqmInfo->SSEY * (uint64_t)area;
        SSE[1]          +=  (uint64_t)pTileVdaqmInfo->SSEU * (uint64_t)area;
        SSE[2]          +=  (uint64_t)pTileVdaqmInfo->SSEV * (uint64_t)area;
        areaSum         += area;
    }

    if (areaSum == 0)
    {
        ENCODE_ASSERTMESSAGE("frame pixel num cal by each tile is zero, sth must be wrong!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MSE[0] = (uint32_t)(SSE[0] / (uint64_t)areaSum);
    MSE[1] = (uint32_t)(SSE[1] / (uint64_t)areaSum);
    MSE[2] = (uint32_t)(SSE[2] / (uint64_t)areaSum);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAqmFeature::ReportQualityInfoFrame(uint32_t statBufIdx, EncodeStatusReportData& statusReportData)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PMOS_RESOURCE  pBuffer = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer0, statBufIdx);
    ENCODE_CHK_NULL_RETURN(pBuffer);

    AQM_Ouput_Format* pDataFrameStas = (AQM_Ouput_Format*)m_allocator->LockResourceForRead(pBuffer);
    ENCODE_CHK_NULL_RETURN(pDataFrameStas);

    ENCODE_CHK_STATUS_RETURN(GetFrameMSE(pDataFrameStas, statusReportData.MSE));

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(pBuffer));

    return eStatus;
}
}  // namespace encode
