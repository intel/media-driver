/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_vdenc_stream_in.cpp
//! \brief    Defines the common interface for encode AVC VDEnc Stream-in
//!

#include "encode_avc_vdenc_stream_in_feature.h"
#include "encode_avc_brc.h"
#include "media_avc_feature_defs.h"
#include "mos_os_cp_interface_specific.h"

namespace encode
{
const size_t AvcVdencStreamInState::byteSize = sizeof(AvcVdencStreamInState);

AvcVdencStreamInFeature::AvcVdencStreamInFeature(
    MediaFeatureManager* featureManager,
    EncodeAllocator* allocator,
    CodechalHwInterfaceNext *hwInterface,
    void* constSettings) :
    MediaFeature(constSettings),
    m_hwInterface(hwInterface),
    m_allocator(allocator)
{
    m_featureManager = featureManager;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_featureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
}

AvcVdencStreamInFeature::~AvcVdencStreamInFeature()
{
    ENCODE_FUNC_CALL();
}

MOS_STATUS AvcVdencStreamInFeature::Init(void* setting)
{
    ENCODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencStreamInFeature::Update(void* setting)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

    // Allocate buffer if it doesn't exists
    if (!m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum))
    {
        if (m_basicFeature->m_picWidthInMb == 0 || m_basicFeature->m_picHeightInMb == 0)
        {
            ENCODE_ASSERTMESSAGE("m_picWidthInMb or m_picHeightInMb is equal to zero. BasicFeature->Update() must be call before StreamIn->Update()\n");
            return MOS_STATUS_UNINITIALIZED;
        }

        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type = MOS_GFXRES_BUFFER;
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Format = Format_Buffer;
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocParams.bIsPersistent = true;
        allocParams.dwMemType = MOS_MEMPOOL_VIDEOMEMORY;

        m_widthInMb = m_basicFeature->m_picWidthInMb;
        m_heightInMb = m_basicFeature->m_picHeightInMb;

        allocParams.dwBytes = MOS_ALIGN_CEIL(m_widthInMb * m_heightInMb * AvcVdencStreamInState::byteSize, CODECHAL_CACHELINE_SIZE);
        allocParams.pBufName = "AVC VDEnc StreamIn Data Buffer";

        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::StreamInBuffer, allocParams));
    }

    m_streamInBuffer = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
    ENCODE_CHK_NULL_RETURN(m_streamInBuffer);

    m_updated = true; // For check that Stream-In must be updated before other features, which will use it
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencStreamInFeature::Enable()
{
    ENCODE_FUNC_CALL();

    if (!m_updated)
    {
        return MOS_STATUS_UNINITIALIZED;
    }

    m_enabled = true;
    return MOS_STATUS_SUCCESS;
}

void AvcVdencStreamInFeature::Reset()
{
    ENCODE_FUNC_CALL();
    m_updated = m_enabled = false;
}

MOS_STATUS AvcVdencStreamInFeature::Clear()
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_UNINITIALIZED;
    }

    uint8_t *pData = (uint8_t*)Lock();
    ENCODE_CHK_NULL_RETURN(pData);

    MOS_ZeroMemory(pData, m_heightInMb * m_widthInMb * AvcVdencStreamInState::byteSize);
    return Unlock();
}

AvcVdencStreamInState* AvcVdencStreamInFeature::Lock()
{
    ENCODE_FUNC_CALL();
    return m_enabled ? (AvcVdencStreamInState*)m_allocator->LockResourceForWrite(m_streamInBuffer) : nullptr;
}

MOS_STATUS AvcVdencStreamInFeature::Unlock()
{
    ENCODE_FUNC_CALL();
    return m_enabled ? m_allocator->UnLock(m_streamInBuffer) : MOS_STATUS_UNINITIALIZED;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcVdencStreamInFeature::Dump(CodechalDebugInterface* itf, const char* bufName)
{
    ENCODE_FUNC_CALL();
    if (m_enabled)
    {
        return itf->DumpBuffer(
            m_streamInBuffer,
            CodechalDbgAttr::attrStreamIn,
            bufName,
            m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES);
    }
    return MOS_STATUS_SUCCESS;
}
#endif

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, AvcVdencStreamInFeature)
{
    if (m_enabled)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);

        params.streamInBuffer = m_streamInBuffer;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcVdencStreamInFeature)
{
    params.streamIn = m_enabled;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcVdencStreamInFeature)
{
    auto picParams = m_basicFeature->m_picParam;

    auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    params.roiEnable             = m_enabled && picParams->EnableRollingIntraRefresh == ROLLING_I_DISABLED &&
                                    (picParams->NumDirtyROI && brcFeature->IsVdencBrcEnabled() ||
                                     picParams->NumROI && picParams->bNativeROI ||
                                     picParams->TargetFrameSize > 0 && !m_basicFeature->m_lookaheadDepth);  // TCBRC (for AdaptiveRegionBoost)

    params.mbLevelQpEnable      = m_enabled && picParams->EnableRollingIntraRefresh == ROLLING_I_DISABLED &&
                                   (picParams->NumROI && !picParams->bNativeROI || m_basicFeature->m_mbQpDataEnabled);

    if (m_enabled)
    {
        PMOS_INTERFACE osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);
        MEDIA_WA_TABLE *pWaTable   = osInterface->pfnGetWaTable(osInterface);
        ENCODE_CHK_NULL_RETURN(pWaTable);

        if (MEDIA_IS_WA(pWaTable, Wa_15013906446))
        {
#if _MEDIA_RESERVED
    params.vdencAvcImgStatePar1 = 0;
#else
    params.extSettings.emplace_back(
        [this](uint32_t *data) {
            data[2] &= 0xffffff7f;
            return MOS_STATUS_SUCCESS;
        });
#endif  // _MEDIA_RESERVED
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, AvcVdencStreamInFeature)
{
    if (params.function == BRC_UPDATE)
    {
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

        // Input regions
        if (m_basicFeature->m_picParam->NumROI && !m_basicFeature->m_picParam->bNativeROI)  // Only for BRC non-native ROI
        {
            if (m_hwInterface->GetOsInterface()->osCpInterface != nullptr &&
                m_hwInterface->GetOsInterface()->osCpInterface->IsCpEnabled())
            {
                ENCODE_ASSERTMESSAGE("Non-native BRC ROI doesn't supports in CP case");
                return MOS_STATUS_UNIMPLEMENTED;
            }
            params.regionParams[9].presRegion = m_streamInBuffer;
        }

        // Output regions
        if (m_basicFeature->m_picParam->NumROI && !m_basicFeature->m_picParam->bNativeROI)  // Only for BRC non-native ROI
        {
            if (m_hwInterface->GetOsInterface()->osCpInterface != nullptr &&
                m_hwInterface->GetOsInterface()->osCpInterface->IsCpEnabled())
            {
                ENCODE_ASSERTMESSAGE("Non-native BRC ROI doesn't supports in CP case");
                return MOS_STATUS_UNIMPLEMENTED;
            }
            params.regionParams[10].presRegion = m_streamInBuffer;
            params.regionParams[10].isWritable = true;
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
