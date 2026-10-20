/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_scalability_multipipe.cpp
//! \brief    Defines the common interface for media scalability multipipe mode.
//! \details  The media scalability multipipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "media_scalability_multipipe.h"
#include "media_scalability_defs.h"
#include "mos_os.h"

MOS_STATUS MediaScalabilityMultiPipe::AllocateNativeFenceCounters()
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    // Check if FtrNativeFence is enabled
    // Check if FtrNativeFence is enabled
    if (!MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrNativeFence))
    {
        SCALABILITY_NORMALMESSAGE("NativeFence disabled, skipping counter allocation");
        return MOS_STATUS_SUCCESS;
    }

    SCALABILITY_NORMALMESSAGE("NativeFence enabled, allocating barrier counters");
    // Setup allocation parameters for GPU memory buffers
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.dwBytes  = sizeof(uint32_t);
    allocParams.pBufName = "NativeFence Child Counter";

    // Allocate m_counterChild
    SCALABILITY_CHK_STATUS_MESSAGE_RETURN(
        m_osInterface->pfnAllocateResource(m_osInterface, &allocParams, &m_counterChild),
        "Failed to allocate NativeFence child counter");

    // Initialize m_counterChild to 0
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    uint32_t *data = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &m_counterChild, &lockFlagsWriteOnly);
    SCALABILITY_CHK_NULL_RETURN(data);
    *data = 0;
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, &m_counterChild));

    SCALABILITY_NORMALMESSAGE("NativeFence child counter allocated and initialized");

    // Allocate m_counterParent
    allocParams.pBufName = "NativeFence Parent Counter";
    SCALABILITY_CHK_STATUS_MESSAGE_RETURN(
        m_osInterface->pfnAllocateResource(m_osInterface, &allocParams, &m_counterParent),
        "Failed to allocate NativeFence parent counter");

    // Initialize m_counterParent to 0
    data = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &m_counterParent, &lockFlagsWriteOnly);
    SCALABILITY_CHK_NULL_RETURN(data);
    *data = 0;
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, &m_counterParent));

    SCALABILITY_NORMALMESSAGE("NativeFence parent counter allocated and initialized");

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilityMultiPipe::SyncPipesWithNativeFence(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    if (m_pipeNum > 2)
    {
        SCALABILITY_ASSERTMESSAGE("Should not reach here. m_counterParent cleanup logic needs to be adjusted to cover the corner case of one parent with multiple children. m_pipeNum=%d", m_pipeNum);
    }

    bool isParent = (m_currentPipe == 0);

    if (isParent)
    {
        SCALABILITY_NORMALMESSAGE("NativeFence: Parent pipe (0) entering barrier sync, waiting for %d child signals", m_pipeNum - 1);

        // Parent: WAIT on m_counterChild until value equals (m_pipeNum - 1)
        SCALABILITY_CHK_STATUS_RETURN(SendMiSemaphoreWaitCmd(
            &m_counterChild,
            0,
            m_pipeNum - 1,
            MHW_MI_SAD_EQUAL_SDD,
            cmdBuffer));
        SCALABILITY_NORMALMESSAGE("NativeFence: Parent collected all %d child signals", m_pipeNum - 1);

        // Parent: STORE 0 to m_counterChild to reset accumulator
        SCALABILITY_CHK_STATUS_RETURN(SendMiStoreDataImmCmd(
            &m_counterChild,
            0,
            0,
            cmdBuffer));
        SCALABILITY_NORMALMESSAGE("NativeFence: Parent reset child counter to 0");

        // Parent: INCREMENT m_counterParent
        SCALABILITY_CHK_STATUS_RETURN(SendMiAtomicCmd(
            &m_counterParent,
            0,
            1,
            MHW_MI_ATOMIC_INC,
            cmdBuffer));
        SCALABILITY_NORMALMESSAGE("NativeFence: Parent signaled go to all children");
    }
    else
    {
        SCALABILITY_NORMALMESSAGE("NativeFence: Child pipe (%d) entering barrier sync", m_currentPipe);

        // Child: INCREMENT m_counterChild
        SCALABILITY_CHK_STATUS_RETURN(SendMiAtomicCmd(
            &m_counterChild,
            0,
            1,
            MHW_MI_ATOMIC_INC,
            cmdBuffer));
        SCALABILITY_NORMALMESSAGE("NativeFence: Child pipe (%d) signaled completion to parent", m_currentPipe);

        // Child: WAIT on m_counterParent until value equals 1
        SCALABILITY_CHK_STATUS_RETURN(SendMiSemaphoreWaitCmd(
            &m_counterParent,
            0,
            1,
            MHW_MI_SAD_EQUAL_SDD,
            cmdBuffer));
        SCALABILITY_NORMALMESSAGE("NativeFence: Child pipe (%d) received parent go signal", m_currentPipe);

        // Child #1 responsible for cleanup
        if (m_currentPipe == 1)
        {
            SCALABILITY_CHK_STATUS_RETURN(SendMiStoreDataImmCmd(
                &m_counterParent,
                0,
                0,
                cmdBuffer));
            SCALABILITY_NORMALMESSAGE("NativeFence: Child pipe 1 reset parent counter to 0");
        }
    }

    SCALABILITY_VERBOSEMESSAGE("NativeFence: Pipe (%d) exiting barrier sync", m_currentPipe);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilityMultiPipe::Destroy()
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_NORMALMESSAGE("MediaScalabilityMultiPipe::Destroy() - Cleaning up NativeFence resources");

    if (m_osInterface)
    {
        // Free m_counterChild if allocated
        if (!Mos_ResourceIsNull(&m_counterChild))
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_counterChild);
            SCALABILITY_NORMALMESSAGE("MediaScalabilityMultiPipe::Destroy() - Freed m_counterChild");
        }

        // Free m_counterParent if allocated
        if (!Mos_ResourceIsNull(&m_counterParent))
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_counterParent);
            SCALABILITY_NORMALMESSAGE("MediaScalabilityMultiPipe::Destroy() - Freed m_counterParent");
        }
    }

    // Call base class Destroy() to maintain cleanup chain
    SCALABILITY_CHK_STATUS_RETURN(MediaScalability::Destroy());

    SCALABILITY_NORMALMESSAGE("MediaScalabilityMultiPipe::Destroy() - Cleanup complete");
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilityMultiPipe::UpdateState(void *statePars)
{
    return MOS_STATUS_SUCCESS;
}

