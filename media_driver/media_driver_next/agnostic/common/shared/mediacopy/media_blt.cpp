/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     meida_blt.cpp
//! \brief    Common interface used in Blitter Engine
//! \details  Common interface used in Blitter Engine which are platform independent
//!

#include "media_blt.h"

//!
//! \brief    BltState constructor
//! \details  Initialize the BltState members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltState::BltState(PMOS_INTERFACE    osInterface) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_miInterface(nullptr),
    m_bltInterface(nullptr),
    m_cpInterface(nullptr)
{
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_blt = 1;
    m_mhwInterfaces = MhwInterfaces::CreateFactory(params, osInterface);
    if (m_mhwInterfaces != nullptr)
    {
        m_bltInterface = m_mhwInterfaces->m_bltInterface;
        m_miInterface  = m_mhwInterfaces->m_miInterface;
    }
}

//!
//! \brief    BltState constructor
//! \details  Initialize the BltState members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltState::BltState(PMOS_INTERFACE    osInterface, MhwInterfaces* mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_miInterface(nullptr),
    m_bltInterface(nullptr),
    m_cpInterface(nullptr)
{
    m_bltInterface = mhwInterfaces->m_bltInterface;
    m_miInterface  = mhwInterfaces->m_miInterface;
    m_cpInterface  = mhwInterfaces->m_cpInterface;
}


BltState::~BltState()
{
    // component interface will be relesed in media copy.
    if (m_mhwInterfaces)
    {
        m_mhwInterfaces->Destroy();
        MOS_Delete(m_mhwInterfaces);
    }
}

//!
//! \brief    BltState initialize
//! \details  Initialize the BltState, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::Initialize()
{
    MOS_GPU_NODE            BltGpuNode;
    MOS_GPU_CONTEXT         BltGpuContext;
    MOS_GPUCTX_CREATOPTIONS createOption;

    BltGpuContext = MOS_GPU_CONTEXT_BLT;
    BltGpuNode    = MOS_GPU_NODE_BLT;

    BLT_CHK_NULL_RETURN(m_osInterface);
    BLT_CHK_NULL_RETURN(m_bltInterface);

    // Create BLT Context
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        BltGpuContext,
        BltGpuNode,
        &createOption));

    // Register context with the Batch Buffer completion event
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_BLT));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Copy main surface
//! \details  BLT engine will copy source surface to destination surface
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::CopyMainSurface(
    PMOS_SURFACE src,
    PMOS_SURFACE dst)
{
    BLT_CHK_STATUS_RETURN(CopyMainSurface(&src->OsResource, &dst->OsResource));
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Copy main surface
//! \details  BLT engine will copy source surface to destination surface
//! \param    src
//!           [in] Pointer to source resource
//! \param    dst
//!           [in] Pointer to destination resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::CopyMainSurface(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    BLT_STATE_PARAM bltStateParam;

    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_ZeroMemory(&bltStateParam, sizeof(BLT_STATE_PARAM));
    bltStateParam.bCopyMainSurface = true;
    bltStateParam.pSrcSurface      = src;
    bltStateParam.pDstSurface      = dst;

    BLT_CHK_STATUS_RETURN(SubmitCMD(&bltStateParam));

    // sync
    MOS_LOCK_PARAMS flag;
    flag.Value     = 0;
    flag.WriteOnly = 1;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnLockSyncRequest(m_osInterface, dst, &flag));
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return MOS_STATUS_SUCCESS;

}

//!
//! \brief    Setup fast copy parameters
//! \details  Setup fast copy parameters for BLT Engine
//! \param    mhwParams
//!           [in/out] Pointer to MHW_FAST_COPY_BLT_PARAM
//! \param    inputSurface
//!           [in] Pointer to input surface
//! \param    outputSurface
//!           [in] Pointer to output surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::SetupFastCopyBltParam(
    PMHW_FAST_COPY_BLT_PARAM pMhwBltParams,
    PMOS_RESOURCE            inputSurface,
    PMOS_RESOURCE            outputSurface)
{
    BLT_CHK_NULL_RETURN(pMhwBltParams);
    BLT_CHK_NULL_RETURN(inputSurface);
    BLT_CHK_NULL_RETURN(outputSurface);

    MOS_SURFACE       ResDetails;
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, inputSurface, &ResDetails));

    pMhwBltParams->dwSrcPitch  = ResDetails.dwPitch;
    pMhwBltParams->dwSrcTop    = 0;
    pMhwBltParams->dwSrcLeft   = 0;

    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, outputSurface, &ResDetails));
    pMhwBltParams->dwDstPitch  = ResDetails.dwPitch;
    pMhwBltParams->dwDstTop    = 0;
    pMhwBltParams->dwDstLeft   = 0;
    pMhwBltParams->dwDstBottom = (uint32_t)outputSurface->pGmmResInfo->GetSizeMainSurface() / ResDetails.dwPitch;
    pMhwBltParams->dwDstRight  = ResDetails.dwPitch;

    pMhwBltParams->dwColorDepth = 3;  //0:8bit 1:16bit 3:32bit 4:64bit

    pMhwBltParams->pSrcOsResource = inputSurface;
    pMhwBltParams->pDstOsResource = outputSurface;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Submit command2
//! \details  Submit BLT command2
//! \param    pBltStateParam
//!           [in] Pointer to BLT_STATE_PARAM
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::SubmitCMD(
    PBLT_STATE_PARAM pBltStateParam)
{
    MOS_STATUS                   eStatus;
    MOS_COMMAND_BUFFER           cmdBuffer;
    MHW_FAST_COPY_BLT_PARAM      fastCopyBltParam;

    // Set GPU context
    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_BLT);

    // Initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (pBltStateParam->bCopyMainSurface)
    {
        BLT_CHK_STATUS_RETURN(SetupFastCopyBltParam(
            &fastCopyBltParam,
            pBltStateParam->pSrcSurface,
            pBltStateParam->pDstSurface));
        BLT_CHK_STATUS_RETURN(m_bltInterface->AddFastCopyBlt(
            &cmdBuffer,
            &fastCopyBltParam));
    }

    // Add flush DW
    MHW_MI_FLUSH_DW_PARAMS FlushDwParams;
    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &FlushDwParams));

    // Add Batch Buffer end
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    // Flush the command buffer
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, false));

    return MOS_STATUS_SUCCESS;
}

