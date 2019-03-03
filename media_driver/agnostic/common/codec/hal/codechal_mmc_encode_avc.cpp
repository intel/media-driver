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
//!
//! \file     codechal_mmc_encode_avc.cpp
//! \brief    Impelements the public interface for CodecHal Media Memory Compression
//!

#include "codechal_mmc_encode_avc.h"

CodechalMmcEncodeAvc::CodechalMmcEncodeAvc(
    CodechalHwInterface    *hwInterface,
    void *standardState):
    CodecHalMmcState(hwInterface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_avcState = (CodechalEncodeAvcBase *)standardState;
    CODECHAL_HW_ASSERT(m_avcState);

    CODECHAL_HW_ASSERT(hwInterface);
    CODECHAL_HW_ASSERT(hwInterface->GetSkuTable());
    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrMemoryCompression))
    {
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = m_mmcEnabled;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_ENABLE_ID,
            &userFeatureData);
        m_mmcEnabled = (userFeatureData.i32Data) ? true : false;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabled;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_compressibleId  = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSIBLE_ID;
    m_compressModeId  = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSMODE_ID;
#endif
}

MOS_STATUS CodechalMmcEncodeAvc::SetPipeBufAddr(
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcState->m_avcPicParam);

     bool framePic = (CodecHal_PictureIsFrame(m_avcState->m_avcPicParam->CurrOriginalPic)) &&
        !m_avcState->m_avcPicParam->FieldCodingFlag &&
        !m_avcState->m_avcPicParam->FieldFrameCodingFlag;
     auto suppressReconPic = ((!m_avcState->m_refList[m_avcState->m_currReconstructedPic.FrameIdx]->bUsedAsRef) &&
        m_avcState->m_suppressReconPicSupported);

    // MMC is only enabled for frame encoding
    if (m_mmcEnabled &&
        m_avcState->m_reconSurface.bCompressible &&
        framePic && !suppressReconPic)
    {
        pipeBufAddrParams->PostDeblockSurfMmcState = m_avcState->m_deblockingEnabled ? MOS_MEMCOMP_HORIZONTAL : MOS_MEMCOMP_VERTICAL;
        pipeBufAddrParams->PreDeblockSurfMmcState = pipeBufAddrParams->PostDeblockSurfMmcState;
    }

    CODECHAL_DEBUG_TOOL(
        m_avcState->m_reconSurface.MmcState = pipeBufAddrParams->PreDeblockSurfMmcState;
    );

    if (m_avcState->m_vdencEnabled)
    {
        // MMC is only enabled for frame encoding and VDEnc doesn't support field/mbaff encoding.
        if (m_mmcEnabled)
        {
            pipeBufAddrParams->Ps4xDsSurfMmcState = m_avcState->m_deblockingEnabled ? MOS_MEMCOMP_HORIZONTAL : MOS_MEMCOMP_VERTICAL;
        }
    }

    return eStatus;
}

