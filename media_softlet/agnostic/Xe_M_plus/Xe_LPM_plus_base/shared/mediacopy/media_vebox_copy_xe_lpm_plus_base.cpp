/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_vebox_copy_xe_lpm_plus_base.cpp
//! \brief    Common Copy interface and structure used in Vebox Engine
//! \details  Common Copy interface and structure used in Vebox Engine
#include "media_vebox_copy_xe_lpm_plus_base.h"
#include "mhw_utilities_next.h"
#include "renderhal.h"

#define SURFACE_DW_UY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->UPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->UPlaneOffset.iYOffset : 0)

#define SURFACE_DW_VY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->VPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->VPlaneOffset.iYOffset : 0)


VeboxCopyStateXe_Lpm_Plus_Base::VeboxCopyStateXe_Lpm_Plus_Base(PMOS_INTERFACE osInterface, MhwInterfacesNext* mhwInterfaces) :
    VeboxCopyStateNext(osInterface, mhwInterfaces)
{
}

VeboxCopyStateXe_Lpm_Plus_Base::~VeboxCopyStateXe_Lpm_Plus_Base()
{
}

MOS_STATUS VeboxCopyStateXe_Lpm_Plus_Base::Initialize()
{
    MHW_VEBOX_GPUNODE_LIMIT     GpuNodeLimit;
    MOS_GPU_NODE                VeboxGpuNode;
    MOS_GPU_CONTEXT             VeboxGpuContext;

    VEBOX_COPY_CHK_NULL_RETURN(m_veboxItf);

    GpuNodeLimit.bCpEnabled = (m_osInterface->osCpInterface->IsCpEnabled())? true : false;

    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->FindVeboxGpuNodeToUse(&GpuNodeLimit));

    VeboxGpuNode = (MOS_GPU_NODE)(GpuNodeLimit.dwGpuNodeToUse);
    VeboxGpuContext = (VeboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;

    // Create VEBOX/VEBOX2 Context
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->CreateGpuContext(
        m_osInterface,
        VeboxGpuContext,
        VeboxGpuNode));

    // Register Vebox GPU context with the Batch Buffer completion event
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX));

    const MHW_VEBOX_HEAP* veboxHeap = nullptr;
    m_veboxItf->GetVeboxHeapInfo(&veboxHeap);

    if (veboxHeap == nullptr)
    {
        m_veboxItf->CreateHeap();
    }        
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VeboxCopyStateXe_Lpm_Plus_Base::CopyMainSurface(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  mhwVeboxSurfaceStateCmdParams;
    uint32_t                            streamID = 0;
    const MHW_VEBOX_HEAP                *veboxHeap = nullptr;
    MOS_SURFACE inputSurface, outputSurface;

    VEBOX_COPY_CHK_NULL_RETURN(src);
    VEBOX_COPY_CHK_NULL_RETURN(dst);
    VEBOX_COPY_CHK_NULL_RETURN(m_miItf);
    VEBOX_COPY_CHK_NULL_RETURN(m_veboxItf);

    // Get input resource info
    MOS_ZeroMemory(&inputSurface, sizeof(MOS_SURFACE));
    inputSurface.OsResource = *src;
    GetResourceInfo(&inputSurface);

    // Get output resource info
    MOS_ZeroMemory(&outputSurface, sizeof(MOS_SURFACE));
    outputSurface.OsResource = *dst;
    GetResourceInfo(&outputSurface);

    if (!IsFormatSupported(&inputSurface))
    {
        VEBOX_COPY_ASSERTMESSAGE("UnSupported Format.");
        return MOS_STATUS_UNIMPLEMENTED;
    }

    MOS_GPUCTX_CREATOPTIONS      createOption;

    // no gpucontext will be created if the gpu context has been created before.
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX,
        MOS_GPU_NODE_VE,
        &createOption));
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX));

    // Sync on Vebox Input Resource, Ensure the input is ready to be read
    // Currently, MOS RegisterResourcere cannot sync the 3d resource.
    // Temporaly, call sync resource to do the sync explicitly.
    // Sync need be done after switching context.
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        src,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&veboxHeap));

    VEBOX_COPY_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    //there is a new usage that input surface is clear and output surface is secure.
    //replace Huc Copy by DoubleBuffer resolve to update ccs data.
    //So need consolidate both input/output surface information to decide cp context.
     PMOS_RESOURCE surfaceArray[2];
     surfaceArray[0] = src;
     surfaceArray[1] = dst;

    // preprocess in cp first
    VEBOX_COPY_CHK_STATUS_RETURN(
        m_osInterface->osCpInterface->PrepareResources((void **)&surfaceArray, sizeof(surfaceArray) / sizeof(PMOS_RESOURCE), nullptr, 0));

    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    VEBOX_COPY_CHK_STATUS_RETURN(InitCommandBuffer(&cmdBuffer));

    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VEBOX_COPY_CHK_STATUS_RETURN(SetupVeboxSurfaceState(&mhwVeboxSurfaceStateCmdParams, &inputSurface, &outputSurface));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->AddVeboxSurfaces(
        &cmdBuffer,
        &mhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->AddVeboxTilingConvert(&cmdBuffer, &mhwVeboxSurfaceStateCmdParams.SurfInput, &mhwVeboxSurfaceStateCmdParams.SurfOutput));
    VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->SetPrologCmd(&cmdBuffer));

    auto& flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams = {};
    flushDwParams.pOsResource = (PMOS_RESOURCE)&veboxHeap->DriverResource;
    VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && veboxHeap)
    {
        flushDwParams = {};
        flushDwParams.pOsResource = (PMOS_RESOURCE)&veboxHeap->DriverResource;
        flushDwParams.dwResourceOffset = veboxHeap->uiOffsetSync;
        flushDwParams.dwDataDW1 = veboxHeap->dwNextTag;
        VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
    }

    VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    // Return unused command buffer space to OS
    m_osInterface->pfnReturnCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0);

    // Flush the command buffer
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        false));

    m_veboxItf->UpdateVeboxSync();
    
    return eStatus;
}

MOS_STATUS VeboxCopyStateXe_Lpm_Plus_Base::InitCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    uint32_t                    iRemaining;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams = {};
    PMOS_RESOURCE                           gpuStatusBuffer = nullptr;

    //---------------------------------------------
    VEBOX_COPY_CHK_NULL_RETURN(cmdBuffer);
    VEBOX_COPY_CHK_NULL_RETURN(m_osInterface);
    //---------------------------------------------

    eStatus = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;
    MOS_ZeroMemory(&GenericPrologParams, sizeof(RENDERHAL_GENERIC_PROLOG_PARAMS));

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

#ifndef EMUL
    if (pOsInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VEBOX_COPY_CHK_STATUS_RETURN(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, gpuStatusBuffer));
        VEBOX_COPY_CHK_NULL_RETURN(gpuStatusBuffer);
        // Register the buffer
        VEBOX_COPY_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(pOsInterface, gpuStatusBuffer, true, true));

        GenericPrologParams.bEnableMediaFrameTracking = true;
        GenericPrologParams.presMediaFrameTrackingSurface = gpuStatusBuffer;
        GenericPrologParams.dwMediaFrameTrackingTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
        GenericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
#endif

    if (GenericPrologParams.bEnableMediaFrameTracking)
    {
        VEBOX_COPY_CHK_NULL_RETURN(GenericPrologParams.presMediaFrameTrackingSurface);
        cmdBuffer->Attributes.bEnableMediaFrameTracking = GenericPrologParams.bEnableMediaFrameTracking;
        cmdBuffer->Attributes.dwMediaFrameTrackingTag = GenericPrologParams.dwMediaFrameTrackingTag;
        cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = GenericPrologParams.dwMediaFrameTrackingAddrOffset;
        cmdBuffer->Attributes.resMediaFrameTrackingSurface = GenericPrologParams.presMediaFrameTrackingSurface;
    }

    // initialize command buffer attributes
    cmdBuffer->Attributes.bTurboMode = false;
    cmdBuffer->Attributes.bMediaPreemptionEnabled = false;
    cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = 0;

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.bMmcEnabled = true;

    VEBOX_COPY_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(
        cmdBuffer,
        &genericPrologParams,
        m_miItf));

    return eStatus;
}
