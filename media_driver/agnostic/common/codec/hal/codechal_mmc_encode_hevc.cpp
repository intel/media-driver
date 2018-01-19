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
//! \file     codechal_mmc_encode_hevc.cpp
//! \brief    Impelements the public interface for CodecHal Media Memory Compression
//!

#include "codechal_mmc_encode_hevc.h"

CodechalMmcEncodeHevc::CodechalMmcEncodeHevc(
    CodechalHwInterface    *hwInterface,
    void *standardState):
    CodecHalMmcState(hwInterface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_hevcState = (CodechalEncodeHevcBase *)standardState;
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hevcState);

    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface->GetSkuTable());
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

MOS_STATUS CodechalMmcEncodeHevc::SetPipeBufAddr(
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcState->m_hevcSeqParams);

    if (m_mmcEnabled &&
        m_hevcState->m_reconSurface.bCompressible &&
        m_hcpMmcEnabled &&
        !m_hevcState->m_disableReconMMCD)
    {
        pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_HORIZONTAL;

        // SAO 1st pass can always be executed as 1st pass even with 3 pass SAO
        // memory compression of reconstructed pixels should be disabled during SAO First Pass
        if (m_hevcState->m_hevcSeqParams->SAO_enabled_flag &&
            m_hevcState->IsFirstPass() &&
            m_hevcState->m_b2NdSaoPassNeeded)
        {
            pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        }
    }
    else
    {
        pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
    }

    CODECHAL_DEBUG_TOOL(
        m_hevcState->m_reconSurface.MmcState = pipeBufAddrParams->PreDeblockSurfMmcState;
    )

    return eStatus;
}

