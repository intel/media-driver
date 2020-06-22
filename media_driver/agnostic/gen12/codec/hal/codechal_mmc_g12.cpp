/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_mmc_g12.cpp
//! \brief    Impelements the public interface for Gen12 CodecHal Media Memory Compression
//!

#include "codechal_hw.h"
#include "codechal_mmc_g12.h"
#include "mhw_mi_g12_X.h"

CodecHalMmcStateG12::CodecHalMmcStateG12(CodechalHwInterface  *hwInterface) :
    CodecHalMmcState(hwInterface)
{
    CODECHAL_HW_FUNCTION_ENTER;

    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrE2ECompression))
    {
        if (hwInterface->m_enableCodecMmc)
        {
            m_mmcEnabled = true;
        }
        else
        {
            m_mmcEnabled = false;
        }

        // overwrite if "Enable Codec MMC" exists
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = m_mmcEnabled;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE_ID,
            &userFeatureData);
        m_mmcEnabled = (userFeatureData.i32Data) ? true : false;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabled;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    }
}

void CodecHalMmcStateG12::InitDecodeMmcEnable(CodechalHwInterface    *hwInterface)
{
    CODECHAL_HW_ASSERT(hwInterface);
    CODECHAL_HW_ASSERT(hwInterface->GetSkuTable());
    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrE2ECompression))
    {
        bool decodeMmcEnabled = true;
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = decodeMmcEnabled;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_ENABLE_ID,
            &userFeatureData);
        decodeMmcEnabled = (userFeatureData.i32Data) ? true : false;

        m_mmcEnabledForComponent = m_mmcEnabled && decodeMmcEnabled;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabledForComponent;
        userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_compressibleId = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSIBLE_ID;
    m_compressModeId = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSMODE_ID;
#endif
}

void CodecHalMmcStateG12::InitEncodeMmcEnable(CodechalHwInterface    *hwInterface)
{
    CODECHAL_HW_ASSERT(hwInterface);
    CODECHAL_HW_ASSERT(hwInterface->GetSkuTable());
    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrE2ECompression))
    {
        bool encodeMmcEnabled = true;
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = encodeMmcEnabled;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_ENABLE_ID,
            &userFeatureData);
        encodeMmcEnabled = (userFeatureData.i32Data) ? true : false;

        m_mmcEnabledForComponent = m_mmcEnabled && encodeMmcEnabled;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabledForComponent;
        userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_compressibleId = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSIBLE_ID;
    m_compressModeId = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSMODE_ID;
#endif
}

MOS_STATUS CodecHalMmcStateG12::SetSurfaceParams(
    PCODECHAL_SURFACE_CODEC_PARAMS surfaceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_mmcEnabled)
    {
        CODECHAL_HW_CHK_NULL_RETURN(surfaceParams->psSurface);
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(
            m_osInterface,
            &surfaceParams->psSurface->OsResource,
            &surfaceParams->psSurface->MmcState));
    }
    else
    {
        surfaceParams->psSurface->MmcState = MOS_MEMCOMP_DISABLED;
    }
    return eStatus;
}

MOS_STATUS CodecHalMmcStateG12::SetSurfaceState(
    PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_HW_CHK_NULL_RETURN(surfaceStateParams);
    CODECHAL_HW_CHK_NULL_RETURN(surfaceStateParams->psSurface);

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_mmcEnabled)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &surfaceStateParams->psSurface->OsResource, &surfaceStateParams->mmcState));
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface,
            &surfaceStateParams->psSurface->OsResource, &surfaceStateParams->dwCompressionFormat));
    }
    else
    {
        surfaceStateParams->mmcState = MOS_MEMCOMP_DISABLED;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalMmcStateG12::SendPrologCmd(
    MhwMiInterface      *miInterface,
    MOS_COMMAND_BUFFER  *cmdBuffer,
    MOS_GPU_CONTEXT     gpuContext)
{
    CODECHAL_HW_CHK_NULL_RETURN(miInterface);
    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    if (m_mmcEnabled)
    {
        uint64_t auxTableBaseAddr = m_osInterface->pfnGetAuxTableBaseAddr(m_osInterface);
        if (auxTableBaseAddr)
        {
            MHW_MI_LOAD_REGISTER_IMM_PARAMS lriParams;
            MOS_ZeroMemory(&lriParams, sizeof(MHW_MI_LOAD_REGISTER_IMM_PARAMS));

            if (MOS_RCS_ENGINE_USED(gpuContext))
            {
                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseLow;
                lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseHigh;
                lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseLow;
                lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseHigh;
                lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));
            }
            else if (MOS_VECS_ENGINE_USED(gpuContext))
            {
                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVe0AuxTableBaseLow;
                lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVe0AuxTableBaseHigh;
                lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));
            }
            else
            {
                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd0AuxTableBaseLow;
                lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd0AuxTableBaseHigh;
                lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd2AuxTableBaseLow;
                lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

                lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd2AuxTableBaseHigh;
                lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
                CODECHAL_HW_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}
