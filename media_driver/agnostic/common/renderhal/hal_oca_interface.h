/*
* Copyright (c) 2019, Intel Corporation
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

/****************************************************************************************************/
/*                                      HalOcaInterface                                             */
/****************************************************************************************************/

class HalOcaInterface
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
    //! \brief  Oca operation which should be called before adding batch buffer end command for 1st
    //!         level batch buffer.
    //! \param  [in/out] cmdBuffer
    //!         Command buffer for current BB. hOcaBuf in cmdBuffer will be updated.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void On1stLevelBBEnd(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext);

    //!
    //! \brief  Oca operation which should be called before sending start sub level batch buffer command.
    //! \param  [in] cmdBuffer
    //!         Command buffer for current BB.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \param  [in] pMosResource
    //!         Pointer to the MOS_RESOURCE.
    //! \param  [in] offsetOfSubLevelBB
    //!         Offset for current BB in pMosResource.
    //! \param  [in] bUseSizeOfResource
    //!         If true, use size of pMosResource for batch buffer, else use sizeOfIndirectState.
    //! \param  [in] sizeOfSubLevelBB
    //!         Size of BB. Ignore if bUseSizeOfResource == true.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void OnSubLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, void *pMosResource, uint32_t offsetOfSubLevelBB, bool bUseSizeOfResource, uint32_t sizeOfSubLevelBB);

    //!
    //! \brief  Oca operation which should be called when indirect states being added.
    //! \param  [in] cmdBuffer
    //!         Command buffer for current BB.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \param  [in] pMosResource
    //!         Pointer to the MOS_RESOURCE.
    //! \param  [in] offsetOfIndirectState
    //!         Offset for current state in pMosResource.
    //! \param  [in] bUseSizeOfResource
    //!         If true, use size of pMosResource for indirect state, else use sizeOfIndirectState.
    //! \param  [in] sizeOfIndirectState
    //!         Size of indirect state. Ignore if bUseSizeOfResource == true.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void OnIndirectState(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, void *pMosResource, uint32_t offsetOfIndirectState, bool bUseSizeOfResource, uint32_t sizeOfIndirectState);

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
    static void OnDispatch(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, MhwMiInterface &mhwMiInterface, MHW_MI_MMIOREGISTERS &mmioRegisters);

    //!
    //! \brief  Add string to oca log section
    //! \param  [in] cmdBuffer
    //!         Command buffer for current BB.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \param  [in] str
    //!         string to be added.
    //! \param  [in] maxCount
    //!         size of the buffer pointed by str.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void TraceMessage(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, const char *str, uint32_t maxCount);

    //!
    //! \brief  Add vp kernel info to oca log section.
    //! \param  [in] cmdBuffer
    //!         Command buffer for current BB.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \param  [in] vpKernelID
    //!         Value of enum VpKernelID.
    //! \param  [in] fcKernelCount
    //!         If vpKernelID == kernelCombinedFc, fcKernelCount is the kernel count for fc, otherwise, it's not used.
    //! \param  [in] fcKernelList
    //!         If vpKernelID == kernelCombinedFc, fcKernelList is the kernel list for fc, otherwise, it's not used.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void DumpVpKernelInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, int vpKernelID, int fcKernelCount, int *fcKernelList);

    //!
    //! \brief  Add vphal parameters to oca log section.
    //! \param  [in] cmdBuffer
    //!         Command buffer for current BB.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \param  [in] pVphalDumper
    //!         Pointer to vphal dumper object.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void DumpVphalParam(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, void *pVphalDumper);

private:
    //!
    //! \brief  Error handle function.
    //! \param  [in] status
    //!         The MOS_STATUS for current error.
    //! \param  [in] funcName
    //!         The failure function name.
    //! \param  [in] lineNumber
    //!         The line number where OnOcaError being called in failure function.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void OnOcaError(MOS_STATUS status, const char *functionName, uint32_t lineNumber);

    //!
    //! \brief  Get OCA buffer handle from pool.
    //! \param  [in] cmdBuffer
    //!         Command buffer for current BB.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \return MOS_OCA_BUFFER_HANDLE
    //!         MOS_OCA_BUFFER_HANDLE.
    //!
    static MOS_OCA_BUFFER_HANDLE GetOcaBufferHandle(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext);

    //!
    //! \brief  Remove OCA buffer handle from pool.
    //! \param  [in] cmdBuffer
    //!         Command buffer for current BB.
    //! \param  [in] mosContext
    //!         Reference to MOS_CONTEXT.
    //! \return void
    //!         No return value. Handle all exception inside the function.
    //!
    static void RemoveOcaBufferHandle(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext);

    // Private functions to ensure class singleton.
    HalOcaInterface();
    HalOcaInterface(HalOcaInterface &);
    HalOcaInterface& operator= (HalOcaInterface &);

    static std::map<uint32_t*, MOS_OCA_BUFFER_HANDLE> s_hOcaMap;        //!< Oca buffer handle map to current command

};


#endif // __RHAL_OCA_INTERFACE_H__
