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
//! \file     codechal_mmc_decode_jpeg.cpp
//! \brief    Impelements the public interface for CodecHal Media Memory Compression
//!

#include "codechal_mmc_decode_jpeg.h"

CodechalMmcDecodeJpeg::CodechalMmcDecodeJpeg(
    CodechalHwInterface    *hwInterface, 
    void *standardState):
    CodecHalMmcState(hwInterface)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_jpegState = (CodechalDecodeJpeg *)standardState;
    CODECHAL_HW_ASSERT(m_jpegState);

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
            __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_ENABLE_ID,
            &userFeatureData);
        m_mmcEnabled = (userFeatureData.i32Data) ? true : false;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabled;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_compressibleId  = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSIBLE_ID;
    m_compressModeId  = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSMODE_ID;
#endif
}

MOS_STATUS CodechalMmcDecodeJpeg::SetPipeBufAddr(
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_jpegState->m_jpegPicParams);

    if (m_mmcEnabled && m_jpegState->m_destSurface.bCompressible)
    {
        if (((m_jpegState->m_jpegPicParams->m_chromaType == jpegYUV422H2Y) || (m_jpegState->m_jpegPicParams->m_chromaType == jpegYUV422H4Y)) && ((m_jpegState->m_destSurface.Format == Format_YUY2) || (m_jpegState->m_destSurface.Format == Format_UYVY)))
        {
            pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_HORIZONTAL;
        }
        else if ((m_jpegState->m_jpegPicParams->m_chromaType == jpegYUV420) && (m_jpegState->m_destSurface.Format == Format_NV12))
        {
            pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_VERTICAL;
        }
    }

    CODECHAL_DEBUG_TOOL(
        m_jpegState->m_destSurface.MmcState = pipeBufAddrParams->PreDeblockSurfMmcState;)

    return eStatus;
}

