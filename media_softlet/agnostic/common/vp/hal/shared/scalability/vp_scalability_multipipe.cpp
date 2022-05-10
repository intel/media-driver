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
//! \file     vp_scalability_multipipe.cpp
//! \brief    Defines the common interface for vp scalability multipipe mode.
//!

#include "vp_scalability_multipipe.h"

#include "media_context.h"
#include "media_status_report.h"
#include "mhw_utilities.h"
#include "mhw_mi_cmdpar.h"

namespace vp
{
VpScalabilityMultiPipe::VpScalabilityMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
    : VpScalabilityMultiPipeNext(hwInterface, mediaContext, componentType)
{
}

VpScalabilityMultiPipe::~VpScalabilityMultiPipe()
{

}

MOS_STATUS VpScalabilityMultiPipe::Initialize(const MediaScalabilityOption &option)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    m_miInterface = m_hwInterface->m_mhwMiInterface;

    VpScalabilityMultiPipeNext::Initialize(option);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Send hw semphore wait cmd
//! \details  Send hw semphore wait cmd for sync perpose
//!
//! \param    [in] semaMem
//!           Reource of Hw semphore
//! \param    [in] semaData
//!           Data of Hw semphore
//! \param    [in] opCode
//!           Operation code
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::SendHwSemaphoreWaitCmd(
    PMOS_RESOURCE                             semaMem,
    uint32_t                                  semaData,
    MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION opCode,
    PMOS_COMMAND_BUFFER                       cmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE            pMhwMiInterface;
    MHW_MI_SEMAPHORE_WAIT_PARAMS miSemaphoreWaitParams;

    MHW_CHK_NULL(m_hwInterface);
    MHW_CHK_NULL(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    if (m_miItf)
    {
        auto &params             = m_miItf->MHW_GETPAR_F(MI_SEMAPHORE_WAIT)();
        params                   = {};
        params.presSemaphoreMem = semaMem;
        params.bPollingWaitMode = true;
        params.dwSemaphoreData  = semaData;
        params.CompareOperation = (mhw::mi::MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION) opCode;
        eStatus                 = m_miItf->MHW_ADDCMD_F(MI_SEMAPHORE_WAIT)(cmdBuffer);
    }
    else
    {
        MOS_ZeroMemory((&miSemaphoreWaitParams), sizeof(miSemaphoreWaitParams));
        miSemaphoreWaitParams.presSemaphoreMem = semaMem;
        miSemaphoreWaitParams.bPollingWaitMode = true;
        miSemaphoreWaitParams.dwSemaphoreData  = semaData;
        miSemaphoreWaitParams.CompareOperation = opCode;
        eStatus                                = pMhwMiInterface->AddMiSemaphoreWaitCmd(cmdBuffer, &miSemaphoreWaitParams);
    }

finish:
    return eStatus;
}

//!
//! \brief    Send mi atomic dword cmd
//! \details  Send mi atomic dword cmd for sync perpose
//!
//! \param    [in] resource
//!           Reource used in mi atomic dword cmd
//! \param    [in] immData
//!           Immediate data
//! \param    [in] opCode
//!           Operation code
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::SendMiAtomicDwordCmd(
    PMOS_RESOURCE               resource,
    uint32_t                    immData,
    MHW_COMMON_MI_ATOMIC_OPCODE opCode,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE    pMhwMiInterface;
    MHW_MI_ATOMIC_PARAMS atomicParams;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    if (m_miItf)
    {
        auto &params             = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
        params                   = {};
        params.pOsResource       = resource;
        params.dwDataSize        = sizeof(uint32_t);
        params.Operation         = (mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE) opCode;
        params.bInlineData       = true;
        params.dwOperand1Data[0] = immData;
        eStatus                  = m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer);
    }
    else
    {
        MOS_ZeroMemory((&atomicParams), sizeof(atomicParams));
        atomicParams.pOsResource       = resource;
        atomicParams.dwDataSize        = sizeof(uint32_t);
        atomicParams.Operation         = opCode;
        atomicParams.bInlineData       = true;
        atomicParams.dwOperand1Data[0] = immData;
        eStatus = pMhwMiInterface->AddMiAtomicCmd(cmdBuffer, &atomicParams);
    }

    return eStatus;
}

//!
//! \brief    Send mi flush dword cmd
//! \details  Send mi flush dword cmd for sync perpose
//!
//! \param    [in] semMem
//!           Reource used in mi flush dword cmd
//! \param    [in] semaData
//!           Immediate data
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::AddMiFlushDwCmd(
    PMOS_RESOURCE                             semaMem,
    uint32_t                                  semaData,
    PMOS_COMMAND_BUFFER                       cmdBuffer)
{
    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE    pMhwMiInterface;
    MHW_MI_ATOMIC_PARAMS atomicParams;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    // Send MI_FLUSH command
    if (m_miItf)
    {
        auto& parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush = {};
        parFlush.bVideoPipelineCacheInvalidate = true;
        if (!Mos_ResourceIsNull(semaMem))
        {
            parFlush.pOsResource = semaMem;
            parFlush.dwDataDW1   = semaData + 1;
        }
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer);
    }
    else
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        if (!Mos_ResourceIsNull(semaMem))
        {
            flushDwParams.pOsResource = semaMem;
            flushDwParams.dwDataDW1   = semaData + 1;
        }
        SCALABILITY_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }

    return eStatus;
}

//!
//! \brief    Send mi store data dword cmd
//! \details  Send mi store dat dword cmd for sync perpose
//!
//! \param    [in] resource
//!           Reource used in mi store dat dword cmd
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::AddMiStoreDataImmCmd(
    PMOS_RESOURCE               resource,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE    pMhwMiInterface;
    MHW_MI_ATOMIC_PARAMS atomicParams;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    if (m_miItf)
    {
        auto &params             = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        params                   = {};
        params.pOsResource       = resource;
        params.dwResourceOffset  = 0;
        params.dwValue           = 0;
        eStatus                  = m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer);
    }
    else
    {
        MHW_MI_STORE_DATA_PARAMS dataParams = {};
        dataParams.pOsResource      = resource;
        dataParams.dwResourceOffset = 0;
        dataParams.dwValue          = 0;

        // Reset current pipe semaphore
        SCALABILITY_CHK_STATUS_RETURN(pMhwMiInterface->AddMiStoreDataImmCmd(
            cmdBuffer, &dataParams));
    }

    return eStatus;
}

}
