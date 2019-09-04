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
//! \file     codechal_kernel_base.cpp
//! \brief    Defines base class for all kernels
//! \details  Kernel base class abstracts all common functions and definitions
//!           for all kernels, each kernel class should inherit from kernel base
//!

#include "codechal_kernel_base.h"
#include "codeckrnheader.h"
#include "hal_oca_interface.h"

CodechalKernelBase::CodechalKernelBase(CodechalEncoderState *encoder) :
        m_encoder(encoder),
        m_firstTaskInPhase(encoder->m_firstTaskInPhase),
        m_lastTaskInPhase(encoder->m_lastTaskInPhase),
        m_singleTaskPhaseSupported(encoder->m_singleTaskPhaseSupported),
        m_renderContextUsesNullHw(encoder->m_renderContextUsesNullHw),
        m_groupIdSelectSupported(encoder->m_groupIdSelectSupported),
        m_fieldScalingOutputInterleaved(encoder->m_fieldScalingOutputInterleaved),
        m_vdencEnabled(encoder->m_vdencEnabled),
        m_groupId(encoder->m_groupId),
        m_maxBtCount(encoder->m_maxBtCount),
        m_vmeStatesSize(encoder->m_vmeStatesSize),
        m_storeData(encoder->m_storeData),
        m_verticalLineStride(encoder->m_verticalLineStride),
        m_downscaledWidthInMb4x(encoder->m_downscaledWidthInMb4x),
        m_downscaledHeightInMb4x(encoder->m_downscaledHeightInMb4x),
        m_downscaledWidthInMb16x(encoder->m_downscaledWidthInMb16x),
        m_downscaledHeightInMb16x(encoder->m_downscaledHeightInMb16x),
        m_downscaledWidthInMb32x(encoder->m_downscaledWidthInMb32x),
        m_downscaledHeightInMb32x(encoder->m_downscaledHeightInMb32x),
        m_mode(encoder->m_mode),
        m_pictureCodingType(encoder->m_pictureCodingType),
        m_frameWidth(encoder->m_frameWidth),
        m_frameHeight(encoder->m_frameHeight),
        m_frameFieldHeight(encoder->m_frameFieldHeight),
        m_standard(encoder->m_standard),
        m_walkerMode(encoder->m_walkerMode)
{
    m_osInterface        = encoder->GetOsInterface();
    m_hwInterface        = encoder->GetHwInterface();
    m_debugInterface     = encoder->GetDebugInterface();
    m_miInterface        = m_hwInterface->GetMiInterface();
    m_renderInterface    = m_hwInterface->GetRenderInterface();
    m_stateHeapInterface = m_renderInterface->m_stateHeapInterface->pStateHeapInterface;
}

CodechalKernelBase::~CodechalKernelBase()
{
    for (auto &it : m_kernelStatePool)
    {
        MOS_Delete(it.second);
    }
    m_kernelStatePool.clear();

    for (auto &it : m_surfacePool)
    {
        if (it.second != nullptr)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &it.second->OsResource);
            MOS_Delete(it.second);
        }
    }
    m_surfacePool.clear();
}

MOS_STATUS CodechalKernelBase::GetKernelBinaryAndSize(
    uint8_t * kernelBase,
    uint32_t  kernelUID,
    uint8_t **kernelBinary,
    uint32_t *size)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelBase);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelBinary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(size);

    if (kernelUID >= IDR_CODEC_TOTAL_NUM_KERNELS)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto kernelOffsetTable = (uint32_t *)kernelBase;
    auto binaryBase        = (uint8_t *)(kernelOffsetTable + IDR_CODEC_TOTAL_NUM_KERNELS + 1);

    *size         = kernelOffsetTable[kernelUID + 1] - kernelOffsetTable[kernelUID];
    *kernelBinary = (*size) > 0 ? binaryBase + kernelOffsetTable[kernelUID] : nullptr;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelBase::Initialize(
    KernelBinaryCallback callback,
    uint8_t *            binaryBase,
    uint32_t             kernelUID)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(callback);
    CODECHAL_ENCODE_CHK_NULL_RETURN(binaryBase);

    m_callback           = callback;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_renderInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface);

    uint32_t binarySize = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelBinaryAndSize(binaryBase, kernelUID, &m_kernelBinary, &binarySize));
    if (binarySize == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelBase::CreateKernelState(
    MHW_KERNEL_STATE **           kernelState,
    uint32_t                      kernelIndex,
    EncOperation                  operation,
    uint32_t                      kernelOffset)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_callback);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_kernelBinary);

    CODECHAL_ENCODE_CHK_NULL_RETURN((*kernelState) = MOS_New(MHW_KERNEL_STATE));
    m_kernelStatePool.insert(std::make_pair(kernelIndex, *kernelState));

    CODECHAL_KERNEL_HEADER kernelHeader;
    uint32_t               kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_callback(m_kernelBinary, operation, kernelOffset, &kernelHeader, &kernelSize));

    (*kernelState)->KernelParams.iBTCount          = GetBTCount();
    (*kernelState)->KernelParams.iThreadCount      = m_renderInterface->GetHwCaps()->dwMaxThreads;
    (*kernelState)->KernelParams.iCurbeLength      = GetCurbeSize();
    (*kernelState)->KernelParams.iBlockWidth       = CODECHAL_MACROBLOCK_WIDTH;
    (*kernelState)->KernelParams.iBlockHeight      = CODECHAL_MACROBLOCK_HEIGHT;
    (*kernelState)->KernelParams.iIdCount          = 1;
    (*kernelState)->KernelParams.iInlineDataLength = GetInlineDataLength();

    (*kernelState)->dwCurbeOffset        = m_stateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    (*kernelState)->KernelParams.pBinary = m_kernelBinary + (kernelHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    (*kernelState)->KernelParams.iSize   = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->CalculateSshAndBtSizesRequested(
        (*kernelState)->KernelParams.iBTCount,
        &(*kernelState)->dwSshSize,
        &(*kernelState)->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_renderInterface->m_stateHeapInterface, (*kernelState)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelBase::Run()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    AddPerfTag();

    MHW_KERNEL_STATE *kernelState = GetActiveKernelState();
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ? m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->RequestSshSpaceForCmdBuf(maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->VerifySpaceAvailable());
    }

    auto stateHeapInterface = m_renderInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SetInterfaceDescriptor(1, &idParams));

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = GetMediaStateType();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbe(kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(encFunctionType, MHW_DSH_TYPE, kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(encFunctionType, MHW_ISH_TYPE, kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(encFunctionType, kernelState)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SetBindingTable(kernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSurfaces(&cmdBuffer, kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(encFunctionType, MHW_SSH_TYPE, kernelState)));

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));

    InitWalkerCodecParams(walkerCodecParams);

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SubmitBlocks(kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->UpdateGlobalCmdBufId());
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(&cmdBuffer, encFunctionType)));

    m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase);
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelBase::CleanUpResource(PMOS_RESOURCE resource, PMOS_ALLOC_GFXRES_PARAMS allocParam)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(resource);
    CODECHAL_ENCODE_CHK_NULL_RETURN(allocParam);

    MOS_LOCK_PARAMS lockFlag;
    memset(&lockFlag, 0, sizeof(lockFlag));
    lockFlag.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, resource, &lockFlag);
    if (data == 0)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    if (allocParam->Format == Format_Buffer)
    {
        memset(data, 0, allocParam->dwBytes);
    }
    else if (allocParam->Format == Format_Buffer_2D)
    {
        memset(data, 0, allocParam->dwHeight * allocParam->dwWidth);
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, resource);

    return MOS_STATUS_SUCCESS;
}

PMOS_SURFACE CodechalKernelBase::GetSurface( uint32_t surfaceId )
{
    auto it = m_surfacePool.find(surfaceId);
    if (it != m_surfacePool.end())
    {
        return it->second;
    }
    return nullptr;
}

MOS_STATUS CodechalKernelBase::AllocateSurface(PMOS_ALLOC_GFXRES_PARAMS param, PMOS_SURFACE surface, uint32_t surfaceId)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(param);
    CODECHAL_ENCODE_CHK_NULL_RETURN(surface);
    m_surfacePool.insert(std::make_pair(surfaceId, surface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_osInterface->pfnAllocateResource(
            m_osInterface,
            param,
            &surface->OsResource));
    CleanUpResource(&surface->OsResource, param);

    return MOS_STATUS_SUCCESS;
}
