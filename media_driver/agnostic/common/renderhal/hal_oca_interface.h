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
//! \file     hal_oca_interface.h
//! \brief    Implementation of functions for HAL OCA Interface
//!

#ifndef __HAL_OCA_INTERFACE_H__
#define __HAL_OCA_INTERFACE_H__

#include "mhw_mi.h"
#include "mos_os_hw.h"
#include "hal_oca_interface_next.h"

/****************************************************************************************************/
/*                                      HalOcaInterface                                             */
/****************************************************************************************************/

class HalOcaInterface: public HalOcaInterfaceNext
{
public:
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
    static void On1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext,
        uint32_t gpuContextHandle, MhwMiInterface &mhwMiInterface, MHW_MI_MMIOREGISTERS &mmioRegisters,
        uint32_t offsetOf1stLevelBB = 0, bool bUseSizeOfCmdBuf = true, uint32_t sizeOf1stLevelBB = 0);

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
    static void On1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext,
        uint32_t gpuContextHandle, MhwMiInterface &mhwMiInterface, MmioRegistersMfx &mmioRegisters,
        uint32_t offsetOf1stLevelBB = 0, bool bUseSizeOfCmdBuf = true, uint32_t sizeOf1stLevelBB = 0);

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
    static void OnDispatch(MOS_COMMAND_BUFFER &cmdBuffer, MOS_INTERFACE &osInterface, MhwMiInterface &mhwMiInterface, MHW_MI_MMIOREGISTERS &mmioRegisters);

protected:
    static MOS_STATUS MhwMiLoadRegisterImmCmd(
        MhwMiInterface                   *pMiInterface,
        PMOS_COMMAND_BUFFER              pCmdBuffer,
        MHW_MI_LOAD_REGISTER_IMM_PARAMS  *params);

    // Private functions to ensure class singleton.
    HalOcaInterface();
    HalOcaInterface(HalOcaInterface &);
    HalOcaInterface& operator= (HalOcaInterface &);

};


#endif // __RHAL_OCA_INTERFACE_H__
