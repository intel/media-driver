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
    HalOcaInterfaceNext::On1stLevelBBStart(cmdBuffer, mosContext, gpuContextHandle);
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
    HalOcaInterfaceNext::On1stLevelBBStart(cmdBuffer, mosContext, gpuContextHandle);
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
    MOS_CONTEXT &mosContext = *osInterface.pOsContext;
    AddRTLogReource(_cmdBuffer, (MOS_CONTEXT_HANDLE)&mosContext, osInterface);
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
