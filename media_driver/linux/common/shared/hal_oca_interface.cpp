/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     hal_oca_interface.cpp
//! \brief    Implementation of functions for Hal OCA Interface
//! \details  Implementation of functions for Hal OCA Interface
//!

#include "stdint.h"
#include "mos_os.h"
#include "hal_oca_interface.h"
#include "mhw_mmio.h"
#include "mos_interface.h"
#include "mos_oca_interface_specific.h"
#include "vphal.h"
#include "vphal_debug.h"


/****************************************************************************************************/
/*                                      HalOcaInterface                                             */
/****************************************************************************************************/
MOS_STATUS HalOcaInterface::MhwMiLoadRegisterImmCmd(
    MhwMiInterface                  *pMiInterface,
    PMOS_COMMAND_BUFFER              pCmdBuffer,
    MHW_MI_LOAD_REGISTER_IMM_PARAMS *params)
{
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Oca operation which should be called at the beginning of 1st level batch buffer start.
//! \param  [in/out] cmdBuffer
//!         Command buffer for current BB. hOcaBuf in cmdBuffer will be updated.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] gpuContextHandle
//!         Gpu context handle
//! \param  [in] mhwMiInterface
//!         Reference to MhwMiInterface.
//! \param  [in] mmioRegisters
//!         mmio registers for current engine.
//! \param  [in] offsetOf1stLevelBB
//!         Offset for current BB in cmdBuffer.
//! \param  [in] bUseSizeOfCmdBuf
//!         If true, use size of cmdBuffer for batch buffer, else use sizeOf1stLevelBB.
//! \param  [in] sizeOf1stLevelBB
//!         Size of BB. Ignore if bUseSizeOfResource == true.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterface::On1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext,
        uint32_t gpuContextHandle, MhwMiInterface &mhwMiInterface, MHW_MI_MMIOREGISTERS &mmioRegisters,
        uint32_t offsetOf1stLevelBB, bool bUseSizeOfCmdBuf, uint32_t sizeOf1stLevelBB)
{
    MosInterface::SetObjectCapture(&cmdBuffer.OsResource);
    MOS_STATUS status                   = MOS_STATUS_SUCCESS;
    MosOcaInterface *pOcaInterface      = &MosOcaInterfaceSpecific::GetInstance();
    MOS_OCA_BUFFER_HANDLE ocaBufHandle  = 0;
    uint64_t  ocaBase                   = 0;

    if (nullptr == pOcaInterface || !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled())
    {
        return;
    }
    PMOS_MUTEX mutex = pOcaInterface->GetMutex();
    if (mutex == nullptr)
    {
        OnOcaError(&mosContext, MOS_STATUS_NULL_POINTER, __FUNCTION__, __LINE__);
        return;
    }
    ocaBufHandle = GetOcaBufferHandle(cmdBuffer, mosContext);
    if (ocaBufHandle != MOS_OCA_INVALID_BUFFER_HANDLE)
    {
        // will come here when On1stLevelBBStart being called twice without On1stLevelBBEnd being called.
        OnOcaError(&mosContext, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return;
    }
    else
    {
        MosOcaAutoLock autoLock(mutex);
        ocaBufHandle = pOcaInterface->LockOcaBufAvailable(&mosContext, gpuContextHandle);
        if (MOS_OCA_INVALID_BUFFER_HANDLE == ocaBufHandle)
        {
            OnOcaError(&mosContext, MOS_STATUS_INVALID_HANDLE, __FUNCTION__, __LINE__);
            return;
        }
        auto success = s_hOcaMap.insert(std::make_pair(cmdBuffer.pCmdBase, ocaBufHandle));
        if (!success.second)
        {
            // Should never come to here.
            MOS_OS_ASSERTMESSAGE("ocaBufHandle has already been assigned to current cmdBuffer!");
            OnOcaError(&mosContext, MOS_STATUS_INVALID_HANDLE, __FUNCTION__, __LINE__);
            return;
        }
    }
    
    status = pOcaInterface->On1stLevelBBStart(ocaBase, ocaBufHandle, &mosContext, &cmdBuffer.OsResource, 0, true, 0);
    if (MOS_FAILED(status))
    {
        RemoveOcaBufferHandle(cmdBuffer, mosContext);
        OnOcaError(&mosContext, status, __FUNCTION__, __LINE__);
    }
}

//!
//! \brief  Oca operation which should be called at the beginning of 1st level batch buffer start.
//! \param  [in/out] cmdBuffer
//!         Command buffer for current BB. hOcaBuf in cmdBuffer will be updated.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] gpuContextHandle
//!         Gpu context handle
//! \param  [in] mhwMiInterface
//!         Reference to MhwMiInterface.
//! \param  [in] mmioRegisters
//!         mmio registers for current engine.
//! \param  [in] offsetOf1stLevelBB
//!         Offset for current BB in cmdBuffer.
//! \param  [in] bUseSizeOfCmdBuf
//!         If true, use size of cmdBuffer for batch buffer, else use sizeOf1stLevelBB.
//! \param  [in] sizeOf1stLevelBB
//!         Size of BB. Ignore if bUseSizeOfResource == true.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterface::On1stLevelBBStart(MOS_COMMAND_BUFFER& cmdBuffer, MOS_CONTEXT& mosContext,
    uint32_t gpuContextHandle, MhwMiInterface& mhwMiInterface, MmioRegistersMfx& mmioRegisters,
    uint32_t offsetOf1stLevelBB, bool bUseSizeOfCmdBuf, uint32_t sizeOf1stLevelBB)
{
    MosInterface::SetObjectCapture(&cmdBuffer.OsResource);
    MHW_MI_MMIOREGISTERS regs = {};
    regs.generalPurposeRegister0LoOffset = mmioRegisters.generalPurposeRegister0LoOffset;
    regs.generalPurposeRegister0HiOffset = mmioRegisters.generalPurposeRegister0HiOffset;
    regs.generalPurposeRegister4LoOffset = mmioRegisters.generalPurposeRegister4LoOffset;
    regs.generalPurposeRegister4HiOffset = mmioRegisters.generalPurposeRegister4HiOffset;
    regs.generalPurposeRegister11LoOffset = mmioRegisters.generalPurposeRegister11LoOffset;
    regs.generalPurposeRegister11HiOffset = mmioRegisters.generalPurposeRegister11HiOffset;
    regs.generalPurposeRegister12LoOffset = mmioRegisters.generalPurposeRegister12LoOffset;
    regs.generalPurposeRegister12HiOffset = mmioRegisters.generalPurposeRegister12HiOffset;

    On1stLevelBBStart(cmdBuffer, mosContext, gpuContextHandle, mhwMiInterface, regs,
        offsetOf1stLevelBB, bUseSizeOfCmdBuf, sizeOf1stLevelBB);
}

//!
//! \brief  Oca operation which should be called before adding dispatch states,
//!         e.g. VEB_DI_IECP_STATE and MEDIA_OBJECT_WALKER.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] mhwMiInterface
//!         Reference to MhwMiInterface.
//! \param  [in] mmioRegisters
//!         mmio registers for current engine.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterface::OnDispatch(MOS_COMMAND_BUFFER &_cmdBuffer, MOS_INTERFACE &osInterface, MhwMiInterface &mhwMiInterface, MHW_MI_MMIOREGISTERS &mmioRegisters)
{
}


/****************************************************************************************************/
/*                         Private functions to ensure class singleton.                             */
/****************************************************************************************************/
HalOcaInterface::HalOcaInterface()
{
}

HalOcaInterface::HalOcaInterface(HalOcaInterface &)
{
}

HalOcaInterface& HalOcaInterface::operator= (HalOcaInterface &)
{
    return *this;
}
