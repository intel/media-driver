/*
* Copyright (c) 2017-2021, Intel Corporation
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

/*
*  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

//!
//! \file     codechal_vdenc_vp9_g10.cpp
//! \brief    VP9 VDENC encoder for GEN10.
//!

#include "codechal_vdenc_vp9_g10.h"
#include "codeckrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g10.h"
#endif
#include "mhw_vdbox_vdenc_hwcmd_g10_X.h"

CodechalVdencVp9StateG10::CodechalVdencVp9StateG10(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalVdencVp9State(hwInterface, debugInterface, standardInfo)
{
    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_VP9_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_ENCODE_VP9_INIT_DSH_SIZE;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase = (uint8_t*)IGCODECKRN_G10;
#endif

    m_kuid = IDR_CODEC_VDENC_HME;

    pfnGetKernelHeaderAndSize = GetCommonKernelHeaderAndSize;

    // We need the DYS kernel inside AllVP9Enc_CNLA0, for SHME we need kernels inside
    // VDENC_HME_CNL_b0, so we need to allocate enough size in ISH for both

    uint8_t* binary = nullptr;
    uint32_t combinedKernelSize = 0;

    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        IDR_CODEC_VDENC_HME,
        &binary,
        &combinedKernelSize);
    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    uint32_t totalSize = combinedKernelSize;

    eStatus = CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        IDR_CODEC_AllVP9Enc,
        &binary,
        &combinedKernelSize);
    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    totalSize += combinedKernelSize;

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(totalSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
}

//!
//! \brief    Gets the kernel header/size from the kernel descriptor table
//! \details  Gets the kernel header/size from the kernel descriptor table
//! \param    PVOID binary
//!           [in] Pointer to the base of the combined kernel
//! \param    CODECHAL_ENC_OPERATION operation
//!           [in] Type of encode operation (in KDT kernel are grouped by operation)
//! \param    DWORD krnStateIdx
//!           [in] Index within the ENC operation block of the kernel to be loaded
//! \param    PVOID krnHeader
//!           [in] Pointer to the CODECHAL_KERNEL_HEADER stucture to be returned to caller
//! \param    PDWORD krnSize
//!           [in] Pointer to the kernel size to be returned to caller
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS CodechalVdencVp9StateG10::GetCommonKernelHeaderAndSize(
    void*        binary,
    EncOperation operation, //change it back to this EncOperation operation,
    uint32_t     krnStateIdx,
    void*        krnHeader,
    uint32_t*    krnSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    KernelHeaderEncode* kernelHeaderTable = (KernelHeaderEncode*)binary;
    PCODECHAL_KERNEL_HEADER invalidEntry = &(kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HMEDetection) + 1;
    uint32_t nextKrnOffset = *krnSize;
    krnStateIdx = 0;

    PCODECHAL_KERNEL_HEADER currKrnHeader;

    switch (operation)
    {
    case ENC_SCALING4X:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_DS4X_Frame;
        break;
    case VDENC_ME_P:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_P;
        break;
    case VDENC_ME_B:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_B;
        break;
    case VDENC_STREAMIN:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_Streamin;
        break;
    case VDENC_STREAMIN_HEVC:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_HEVC_Streamin;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencVp9StateG10::GetKernelHeaderAndSize(
    void         *binary,
    EncOperation operation,
    uint32_t     krnStateIdx,
    void         *krnHeader,
    uint32_t     *krnSize)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    KernelHeader* kernelHeaderTable = (KernelHeader*)binary;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if ((operation == ENC_SCALING4X) || (operation == ENC_SCALING2X))
    {
        currKrnHeader = &kernelHeaderTable->PLY_DSCALE;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->VP9_ME_P;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->VP9_Enc_I_32x32;
    }
    else if (operation == ENC_DYS)
    {
        currKrnHeader = &kernelHeaderTable->VP9_DYS;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    PCODECHAL_KERNEL_HEADER invalidEntry = &(kernelHeaderTable->VP9_DYS) + 1;
    uint32_t nextKrnOffset = *krnSize;
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

//!
//! \brief    Sets the 16x ME, 4x ME CURBE and loads it into the DSH
//! \details  Sets the 16x ME, 4x ME CURBE and loads it into the DSH using the
//!           parameters from the input kernel state.
//! \param    PMHW_STATE_HEAP_INTERFACE m_stateHeapInterface
//!           [in] StateHeap interface
//! \param    PCODECHAL_ENCODE_VDENC_ME_STATE vdencMeState
//!           [in] Parameters used for setting up the CURBE
//! \param    PCODECHAL_ENCODER pEncoder
//!           [in] Encode interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS CodechalVdencVp9StateG10::SetBindingTable(
    EncOperation encOperation,
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC encBindingTable)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encBindingTable);

    MOS_ZeroMemory(encBindingTable, sizeof(*encBindingTable));

    switch (encOperation)
    {
    case VDENC_ME_P:
        encBindingTable->dwNumBindingTableEntries = HmeEnd - HmeBegin;
        encBindingTable->dwBindingTableStartOffset = HmeBegin;
        break;
    case VDENC_ME_B:
        encBindingTable->dwNumBindingTableEntries = HmeEnd - HmeBegin;
        encBindingTable->dwBindingTableStartOffset = HmeBegin;
        break;
    case VDENC_STREAMIN:
        encBindingTable->dwNumBindingTableEntries = HmeEnd - HmeBegin;
        encBindingTable->dwBindingTableStartOffset = HmeBegin;
        break;
    case VDENC_STREAMIN_HEVC:
        encBindingTable->dwNumBindingTableEntries = HmeEnd - HmeBegin;
        encBindingTable->dwBindingTableStartOffset = HmeBegin;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < encBindingTable->dwNumBindingTableEntries; i++)
    {
        encBindingTable->dwBindingTableEntries[i] = i;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Sets the kernel params for resp kernel
//! \details  Sets the kernel params for the kernel
//!           requesed.
//! \param    PCODECHAL_ENCODER pEncoder
//!           [in] Encode interface
//! \param    EncOperation encOperation
//!           [in] Kernel requested
//! \param    MHW_KERNEL_PARAM*  kernelParams
//!           [in] Pointer of Kernel params struct to be updated
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS CodechalVdencVp9StateG10::SetKernelParams(
    EncOperation encOperation,
    MHW_KERNEL_PARAM* kernelParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();

    kernelParams->iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount = 1;

    switch (encOperation)
    {
    case VDENC_ME_P:
        kernelParams->iBTCount = HmeEnd - HmeBegin;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(VdencMeCurbe), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    case VDENC_ME_B:
        kernelParams->iBTCount = HmeEnd - HmeBegin;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(VdencMeCurbe), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    case VDENC_STREAMIN:
        kernelParams->iBTCount = HmeEnd - HmeBegin;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(VdencMeCurbe), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    case VDENC_STREAMIN_HEVC:
        kernelParams->iBTCount = HmeEnd - HmeBegin;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(VdencMeCurbe), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencVp9StateG10::InitKernelStateDys()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // DYS is not in the selected KUID platform (IDR_CODEC_VDENC_HME_CNL_b0),
    // so we get DYS from G10 CNL VP9 DP
    m_kuid = IDR_CODEC_AllVP9Enc;

    uint8_t* binary = nullptr;
    uint32_t combinedKernelSize = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        &combinedKernelSize));

    CODECHAL_KERNEL_HEADER currKrnHeader;
    uint32_t kernelSize = combinedKernelSize;
    // **Do NOT change this to m_encoder->pfnGetKernelHeaderAndSize() as that ptr is bound
    // to common (non-VP9 specific) VDEnc functionality for SHME and will not recognize DYS
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
        binary,
        ENC_DYS,
        0,
        &currKrnHeader,
        &kernelSize));

    PMHW_KERNEL_STATE kernelState = &m_dysKernelState;
    kernelState->KernelParams.iBTCount       = MOS_ALIGN_CEIL(m_dysNumSurfaces, m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    kernelState->KernelParams.iThreadCount   = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelState->KernelParams.iCurbeLength   = MOS_ALIGN_CEIL(m_dysStaticDataSize, m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
    kernelState->KernelParams.iBlockWidth    = CODECHAL_MACROBLOCK_WIDTH;
    kernelState->KernelParams.iBlockHeight   = CODECHAL_MACROBLOCK_HEIGHT;
    kernelState->KernelParams.iIdCount       = 1;
    kernelState->KernelParams.iSamplerCount  = 1;
    kernelState->KernelParams.iSamplerLength = m_stateHeapInterface->pStateHeapInterface->GetSizeofSamplerStateAvs();

    kernelState->dwCurbeOffset        = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelState->dwSamplerOffset      = MOS_ALIGN_CEIL(kernelState->dwCurbeOffset + kernelState->KernelParams.iCurbeLength, MHW_SAMPLER_STATE_AVS_ALIGN_G9);
    kernelState->KernelParams.pBinary = binary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelState->KernelParams.iSize   = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelState->KernelParams.iBTCount,
        &kernelState->dwSshSize,
        &kernelState->dwBindingTableSize));

    m_dysDshSize = kernelState->dwSamplerOffset +
        MOS_ALIGN_CEIL(kernelState->KernelParams.iSamplerLength * kernelState->KernelParams.iSamplerCount, MHW_SAMPLER_STATE_AVS_ALIGN);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelState));

    // Reset KUID back to generic VDEnc kernel if VDEnc is enabled
    m_kuid = IDR_CODEC_VDENC_HME;

    return eStatus;
}

//!
//! \brief    Initializes the kernel states for 4xME and StreamIn
//! \details  Initializes the kernel states for 4xME and StreamIn,
//!           and loads the kernel(s) into the ISH.
//! \param    PCODECHAL_ENCODER pEncoder
//!           [in] Encode interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS CodechalVdencVp9StateG10::InitKernelStateStreamin()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t kernelSize = 0;
    uint8_t* binary;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        &kernelSize));

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSize(
        binary,
        VDENC_STREAMIN_HEVC,
        0,
        &currKrnHeader,
        &kernelSize));

    PMHW_KERNEL_STATE kernelState = &m_vdencStreaminKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_STREAMIN_HEVC,
        &kernelState->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_STREAMIN_HEVC,
        &m_vdencStreaminKernelBindingTable));

    kernelState->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelState->KernelParams.pBinary = binary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelState->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelState->KernelParams.iBTCount,
        &kernelState->dwSshSize,
        &kernelState->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelState));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Initializes the kernel states for 16xME
//! \details  Initializes the kernel states for 16xME,
//!           and loads the kernel(s) into the ISH.
//! \param    PCODECHAL_ENCODER pEncoder
//!           [in] Encode interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS CodechalVdencVp9StateG10::InitKernelStateMe()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* binary;
    uint32_t kernelSize = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        &kernelSize));

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSize(
        binary,
        VDENC_ME_P,
        0,
        &currKrnHeader,
        &kernelSize));

    PMHW_KERNEL_STATE kernelState = &m_vdencMeKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_ME_P,
        &kernelState->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_ME_P,
        &m_vdencMeKernelBindingTable));

    kernelState->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelState->KernelParams.pBinary = binary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelState->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelState->KernelParams.iBTCount,
        &kernelState->dwSshSize,
        &kernelState->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelState));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencVp9StateG10::InitKernelStates()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    // DYS
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateDys());
    // VDEnc SHME (16x)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
    //Streamin Kernel initialization (streamin & 4x HME)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateStreamin());
#endif

    return MOS_STATUS_SUCCESS;
}

uint32_t CodechalVdencVp9StateG10::GetMaxBtCount()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_maxBtCount = 0;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    uint16_t btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

    if (m_hmeSupported)
    {
        uint32_t scalingBtCount = MOS_ALIGN_CEIL(
            m_scaling4xKernelStates[0].KernelParams.iBTCount,
            btIdxAlignment);

        uint32_t meBtCount = MOS_ALIGN_CEIL(
            m_vdencStreaminKernelState.KernelParams.iBTCount,
            btIdxAlignment);
        if (m_16xMeSupported)
        {
            meBtCount += MOS_ALIGN_CEIL(
                m_vdencMeKernelState.KernelParams.iBTCount,
                btIdxAlignment);
            scalingBtCount *= 2;
        }
        m_maxBtCount = scalingBtCount + meBtCount;
    }
#endif

    return m_maxBtCount;
}

bool CodechalVdencVp9StateG10::CheckSupportedFormat(
    PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool isColorFormatSupported = false;

    if (nullptr == surface)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return isColorFormatSupported;
    }

    switch (surface->Format)
    {
    case Format_NV12:
    case Format_NV21:
    case Format_P010:
        isColorFormatSupported = true;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_AYUV:
        isColorFormatSupported = (MOS_TILE_LINEAR == surface->TileType);
        break;
    case Format_A8R8G8B8:
        // On CNL RGB conversion is not studio range, intentionally let RGB fall-thru so driver calls Csc+Ds+Conversion kernel
    default:
        break;
    }

    return isColorFormatSupported;
}

MOS_STATUS CodechalVdencVp9StateG10::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    //Create and register huc Cmd Initializer
    m_hucCmdInitializer = MOS_New(CodechalCmdInitializer, this);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9State::Initialize(settings));

    m_dysVdencMultiPassEnabled = true;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    if (m_cscDsState)
    {
        m_cscDsState->EnableColor();
    }
#endif

    m_vdencBrcStatsBufferSize    = m_brcStatsBufSize;
    m_vdencBrcPakStatsBufferSize = m_brcPakStatsBufSize;
    m_brcHistoryBufferSize       = m_brcHistoryBufSize;

    /*
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    userFeatureData.i32Data = 1;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_HUC_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_hucEnabled = (userFeatureData.i32Data) ? true : false;
    */

    /* use the skuTable to check whether the Huc is enabled */
    m_hucEnabled = MEDIA_IS_SKU(m_skuTable, FtrEnableMediaKernels);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    userFeatureData.i32Data = 1;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;
    // For dynamic scaling, the SingleTaskPhaseSupported is set to true and it does not get restored
    // to the original value after encoding of the frame. So need to restore to the original state
    m_storeSingleTaskPhaseSupported = m_singleTaskPhaseSupported; //Save the SingleTaskPhase state here

    // HME enabled by default for VP9
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ME_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_hmeSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_16xME_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_16xMeSupported = (userFeatureData.i32Data) ? true : false;

    // disable superHME when HME is disabled
    if (m_hmeSupported == false)
    {
        m_16xMeSupported = false;
    }

    // Multi-Pass BRC: currently disabled by default
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_multipassBrcSupported = (userFeatureData.i32Data) ? true : false;

    // Adaptive Repak: currently disabled by default
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ADAPTIVE_REPAK_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_adaptiveRepakSupported = (userFeatureData.i32Data) ? true : false;

    // Initialize kernel State
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStates());

    // Get max binding table count
    m_maxBtCount = GetMaxBtCount();

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_BRC_DLL,
        &userFeatureData,
        m_osInterface->pOsContext);

    if (userFeatureData.i32Data)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_BRC_DLL_CUSTOMPATH,
            &userFeatureData,
            m_osInterface->pOsContext);

        if (!userFeatureData.i32Data)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MosUtilities::MosLoadLibrary(VP9SWBRCLIB, &m_swBrcMode)); // Load Dependency (use on RS1)
        }
        else
        {
            char path_buffer[256];
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_ZeroMemory(path_buffer, 256);
            userFeatureData.StringData.pStringData = path_buffer;
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_BRC_DLL_PATH,
                &userFeatureData,
                m_osInterface->pOsContext);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnLoadLibrary(m_osInterface, path_buffer, &m_swBrcMode)); // Mos_Specific_LoadLibrary / LoadLibraryEx (use on NOT RS1 or custom path)
        }
    }
#endif // (_DEBUG || _RELEASE_INTERNAL)

    return eStatus;

}

MOS_STATUS CodechalVdencVp9StateG10::SetupSegmentationStreamIn()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_segmentMapProvided && !m_hmeEnabled) // If we're not going to use the streamin surface leave now
    {
        return eStatus;
    }

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    MOS_LOCK_PARAMS lockFlagsReadOnly;
    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = 1;

    mhw_vdbox_vdenc_g10_X::VDENC_HEVC_VP9_STREAMIN_STATE_CMD *
    streamIn = (mhw_vdbox_vdenc_g10_X::VDENC_HEVC_VP9_STREAMIN_STATE_CMD *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(streamIn);

    // align to cache line size is OK since streamin state is padded to cacheline size - HW uses cacheline size to read, not command size
    uint32_t blockWidth   = MOS_ALIGN_CEIL(m_frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32;
    uint32_t blockHeight  = MOS_ALIGN_CEIL(m_frameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT) / 32;
    uint32_t streamInSize = blockHeight * blockWidth * CODECHAL_CACHELINE_SIZE;
    MOS_ZeroMemory(streamIn, streamInSize);

    // If segment map isn't provided then we unlock surface and exit function here.
    // Reason why check isn't done before function call is to take advantage of the fact that
    // we need the surface locked here if seg map is provided and we want it 0'd either way.
    // This saves us from doing 2 locks on this buffer per frame.
    if (!m_segmentMapProvided)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resVdencStreamInBuffer[m_currRecycledBufIdx]));
        return eStatus;
    }

    char *data = (char *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mbSegmentMapSurface.OsResource,
        &lockFlagsReadOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    // Frame level rasterization of segmentation index map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitZigZagToRasterLUT(m_frameHeight, m_frameWidth));

    // set seg ID's of streamin states
    for (uint32_t i = 0 ; i < blockHeight * blockWidth ; ++i)
    {
        uint32_t addrOffset = CalculateBufferOffset(
            m_mapBuffer[i],
            m_frameWidth,
            m_vp9PicParams->PicFlags.fields.seg_id_block_size,
            m_mbSegmentMapSurface.dwPitch);
        uint32_t segId  = *(data + addrOffset);
        streamIn[i].DW7.SegidEnable = 1;
        streamIn[i].DW7.Segid32X32016X1603Vp9Only = segId | (segId << 4) | (segId << 8) | (segId << 12);

        // TU functions copied from there.
        streamIn[i].DW0.Maxtusize = 3;

        streamIn[i].DW0.Maxcusize = 3;
        // For InterFrames we change the CUsize to 32x32 if we have sub 32 blocks with different segids in superblock
        if ((i % 4) == 3 && m_pictureCodingType == P_TYPE)
        {
            if (!(streamIn[i - 3].DW7.Segid32X32016X1603Vp9Only == streamIn[i - 2].DW7.Segid32X32016X1603Vp9Only &&
                streamIn[i - 2].DW7.Segid32X32016X1603Vp9Only == streamIn[i - 1].DW7.Segid32X32016X1603Vp9Only &&
                streamIn[i - 1].DW7.Segid32X32016X1603Vp9Only == streamIn[i].DW7.Segid32X32016X1603Vp9Only))
            {
                streamIn[i - 3].DW0.Maxcusize = streamIn[i - 2].DW0.Maxcusize = streamIn[i - 1].DW0.Maxcusize = streamIn[i].DW0.Maxcusize = 2;
            }
        }

        streamIn[i].DW0.Numimepredictors = CODECHAL_VDENC_NUMIMEPREDICTORS;

        switch (m_vp9SeqParams->TargetUsage)
        {
        case 1:     // Quality mode
        case 4:     // Normal mode
            streamIn[i].DW6.Nummergecandidatecu8X8 = 1;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 2;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 3;
            streamIn[i].DW6.Nummergecandidatecu64X64 = 4;
            break;
        case 7:     // Speed mode
            streamIn[i].DW0.Numimepredictors = 4;
            streamIn[i].DW6.Nummergecandidatecu8X8 = 0;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 2;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 2;
            streamIn[i].DW6.Nummergecandidatecu64X64 = 2;
            break;
        default:
            MHW_ASSERTMESSAGE("Invalid TU provided!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mbSegmentMapSurface.OsResource));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resVdencStreamInBuffer[m_currRecycledBufIdx]));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG10::InitZigZagToRasterLUT(uint32_t height, uint32_t width)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_mapBuffer && height == m_segStreamInHeight && width == m_segStreamInWidth)
    {
        // return here if pointer is valid (map init'd) and res not changed, no need to populate the map more than once
        return eStatus;
    }

    // Allocate space for zig-zag to raster LUT used for vdenc streamin (1 int32_t for every 32x32 block (pic 64 aligned))
    // We only do this the 1st time segmentation streamin is used in a sequence or res has changes since last allocated
    // We keep this map around until sequence is finished, it's deleted at device destruction.
    if (m_mapBuffer) // free previous if it exists - it may exist if this isn't first seg streamin frame, but res changed since allocated
    {
        MOS_FreeMemory(m_mapBuffer);
    }

    uint32_t align64Width32  = MOS_ALIGN_CEIL(width, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32;
    uint32_t align64Height32 = MOS_ALIGN_CEIL(height, CODEC_VP9_SUPER_BLOCK_HEIGHT) / 32;
    m_mapBuffer = (uint32_t*)MOS_AllocAndZeroMemory(align64Width32 * align64Height32 * sizeof(int32_t));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mapBuffer);
    m_segStreamInHeight = height;
    m_segStreamInWidth  = width;

    // initialize 64 aligned num 32 blocks for each streamin cmd, we will update last row/col for unaligned cases later
    uint32_t num32blocks = align64Width32 * align64Height32;
    for (uint32_t i = 0, uiCount = 0; i < num32blocks ; i += (align64Width32*2))
    {
        for (uint32_t j = i ; j < i+(align64Width32*2) ; j += 4)
        {
            m_mapBuffer[j]   = uiCount++;
            m_mapBuffer[j+1] = uiCount++;
        }
        for (uint32_t j = i+2 ; j < i+(align64Width32*2) ; j += 4)
        {
            m_mapBuffer[j]   = uiCount++;
            m_mapBuffer[j+1] = uiCount++;
        }
    }

    // ^ Zig-zag pattern filled to SB aligned (CEIL), if unaligned then we base seg ID address on previous row/column (data replication)
    uint32_t width32 = CODECHAL_GET_WIDTH_IN_BLOCKS(width, 32);
    if (width32 != align64Width32) // replicate last column
    {
        for (uint32_t i = (align64Width32*2)-1-2 ; i < num32blocks ; i += (align64Width32*2))
        {
            m_mapBuffer[i]   = m_mapBuffer[i-1];
            m_mapBuffer[i+2] = m_mapBuffer[i+1];
        }
    }

    uint32_t height32 = CODECHAL_GET_HEIGHT_IN_BLOCKS(height, 32);
    if (height32 != align64Height32) // replicate last row
    {
        for (uint32_t i = num32blocks - (align64Width32*2) + 2 ; i < num32blocks ; i += 4)
        {
            m_mapBuffer[i]   = m_mapBuffer[i-2];
            m_mapBuffer[i+1] = m_mapBuffer[i+1-2];
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG10::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    uint32_t dumpFormat = 0;
    CODECHAL_DEBUG_TOOL(
    //    CodecHal_DbgMapSurfaceFormatToDumpFormat(m_rawSurfaceToEnc->Format, &dumpFormat);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf"));
    if (m_lastRefPic)
    {
     //   CodecHal_DbgMapSurfaceFormatToDumpFormat(m_lastRefPic->Format, &dumpFormat);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_lastRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            "LastRefSurface"));
    }

    if (m_goldenRefPic)
    {
      //  CodecHal_DbgMapSurfaceFormatToDumpFormat(m_goldenRefPic->Format, &dumpFormat);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_goldenRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            "GoldenRefSurface"));
    }

    if (m_altRefPic)
    {
     //   CodecHal_DbgMapSurfaceFormatToDumpFormat(m_altRefPic->Format, &dumpFormat);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_altRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            "_AltRefSurface"));
    }
    );

    m_setRequestedEUSlices = ((m_frameHeight * m_frameWidth) >= m_ssdResolutionThreshold &&
        m_targetUsage <= m_ssdTargetUsageThreshold) ? true : false;

    m_hwInterface->m_numRequestedEuSlices = (m_setRequestedEUSlices) ?
        m_sliceShutdownRequestState : m_sliceShutdownDefaultState;

    // While this streamin isn't a kernel function, we 0 the surface here which is needed before HME kernel
    SetupSegmentationStreamIn();

    // Csc, Downscaling, and/or 10-bit to 8-bit conversion
    CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
    MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
    cscScalingKernelParams.bLastTaskInPhaseCSC =
        cscScalingKernelParams.bLastTaskInPhase4xDS = !(m_16xMeSupported || m_hmeEnabled);
    cscScalingKernelParams.bLastTaskInPhase16xDS = !(m_32xMeSupported || m_hmeEnabled);
    cscScalingKernelParams.bLastTaskInPhase32xDS = !m_hmeEnabled;

    m_firstTaskInPhase = true;
    if (m_cscDsState)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));
    }
    if (m_hmeEnabled)
    {
        //Initialize the ME struct
        VdencVmeState vdencMeState;
        InitMEState(&vdencMeState);

        if (m_16xMeSupported)
        {
            // P_HME kernel (16x HME)
            vdencMeState.b16xMeInUse = true;
            vdencMeState.b4xMeInUse = false;
            m_lastTaskInPhase = false;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(VdencHmeKernel(&vdencMeState));
        }

        //StreamIn kernel, 4xME
        vdencMeState.b16xMeInUse = false;
        vdencMeState.b4xMeInUse = true;
        vdencMeState.segmapProvided = m_segmentMapProvided;
        m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VdencHmeKernel(&vdencMeState));
    }

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
        m_waitForEnc = true;
    }
#endif

    return eStatus;
}
