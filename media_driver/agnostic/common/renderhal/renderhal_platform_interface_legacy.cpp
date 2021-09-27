/*
* Copyright (c) 2021, Intel Corporation
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
//! \file      renderhal_platform_interface_legacy.cpp
//! \brief     abstract the platfrom specific APIs into one class
//!
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!

#include "renderhal_platform_interface_legacy.h"

//! \brief    Add Pipeline SelectCmd
//! \details  Add Pipeline SelectCmd
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddPipelineSelectCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    bool                        gpGpuPipe)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddPipelineSelectCmd(pCmdBuffer, gpGpuPipe));

finish:
    return eStatus;
}

//! \brief    Send StateBase Address
//! \details  Send StateBase Address
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendStateBaseAddress(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendStateBaseAddress(pRenderHal, pCmdBuffer));

finish:
    return eStatus;
}

//! \brief    Add Sip State Cmd
//! \details  Add Sip State Cmd
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddSipStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddSipStateCmd(pCmdBuffer, &pRenderHal->SipStateParams));

finish:
    return eStatus;
}

//! \brief    Add Cfe State Cmd
//! \details  Add Cfe State Cmd
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddCfeStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_VFE_PARAMS             params)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddCfeStateCmd(pCmdBuffer, params));

finish:
    return eStatus;
};

//! \brief    Send ChromaKey
//! \details  Send ChromaKey
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendChromaKey(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendChromaKey(pRenderHal, pCmdBuffer));

finish:
    return eStatus;
}

//! \brief    Send Palette
//! \details  Send Palette
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendPalette(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendPalette(pRenderHal, pCmdBuffer));

finish:
    return eStatus;
};

//! \brief    Set L3Cache
//! \details  Set L3Cache
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SetL3Cache(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->SetL3Cache(pCmdBuffer));

finish:
    return eStatus;
}

PMHW_MI_MMIOREGISTERS XRenderHal_Platform_Interface_Legacy::GetMmioRegisters(
    PRENDERHAL_INTERFACE        pRenderHal)
{
    PMHW_MI_MMIOREGISTERS     pMmioRegisters = nullptr;
    if (pRenderHal && pRenderHal->pMhwRenderInterface)
    {
        pMmioRegisters = pRenderHal->pMhwRenderInterface->GetMmioRegisters();
    }

    return pMmioRegisters;
};

MOS_STATUS XRenderHal_Platform_Interface_Legacy::EnablePreemption(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->EnablePreemption(pCmdBuffer));

finish:
    return eStatus;
}