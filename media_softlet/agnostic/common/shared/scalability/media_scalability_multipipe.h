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
//! \file     media_scalability_multipipe.h
//! \brief    Defines the common interface for media scalability mulitpipe mode.
//! \details  The media scalability mulitpipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_SCALABILITY_MULTIPIPE_H__
#define __MEDIA_SCALABILITY_MULTIPIPE_H__
#include "mos_defs.h"
#include "media_scalability.h"
class MediaContext;
class MhwMiInterface;

class MediaScalabilityMultiPipe: public MediaScalability
{

public:
    MediaScalabilityMultiPipe(MediaContext *mediaContext) : MediaScalability(mediaContext){};
    
    //!
    //! \brief  Media scalability mulitipipe destructor
    //!
    virtual ~MediaScalabilityMultiPipe(){};

    //!
    //! \brief  Destroy the media scalability multipipe
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

    //!
    //! \brief  Update the media scalability mulitipipe state mode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateState(void *statePars);

protected:
    //!
    //! \brief  Allocate GPU memory resources for NativeFence barrier counters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateNativeFenceCounters();

    //!
    //! \brief  Implement NativeFence barrier sync algorithm using counterChild and counterParent
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncPipesWithNativeFence(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Send MI_ATOMIC command (virtual function for derived class override)
    //! \param  [in] resource
    //!         Resource to perform atomic operation on
    //! \param  [in] offset
    //!         Offset within the resource
    //! \param  [in] immData
    //!         Immediate data for atomic operation
    //! \param  [in] opCode
    //!         Atomic operation code
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMiAtomicCmd(
        PMOS_RESOURCE               resource,
        uint32_t                    offset,
        uint32_t                    immData,
        MHW_COMMON_MI_ATOMIC_OPCODE opCode,
        PMOS_COMMAND_BUFFER         cmdBuffer) = 0;

    //!
    //! \brief  Send MI_SEMAPHORE_WAIT command (virtual function for derived class override)
    //! \param  [in] semaMem
    //!         Semaphore memory resource
    //! \param  [in] offset
    //!         Offset within the semaphore memory
    //! \param  [in] semaData
    //!         Semaphore data to compare
    //! \param  [in] opCode
    //!         Comparison operation code
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMiSemaphoreWaitCmd(
        PMOS_RESOURCE                             semaMem,
        uint32_t                                  offset,
        uint32_t                                  semaData,
        MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION opCode,
        PMOS_COMMAND_BUFFER                       cmdBuffer) = 0;

    //!
    //! \brief  Send MI_STORE_DATA_IMM command (virtual function for derived class override)
    //! \param  [in] resource
    //!         Resource to store data to
    //! \param  [in] offset
    //!         Offset within the resource
    //! \param  [in] value
    //!         Immediate value to store
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMiStoreDataImmCmd(
        PMOS_RESOURCE       resource,
        uint32_t            offset,
        uint32_t            value,
        PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    inline bool IsFirstPipe() { return (m_currentPipe == 0) ? true : false; }
    inline bool IsFirstPass() { return (m_currentPass == 0) ? true : false; }
    inline bool IsLastPipe() { return (m_currentPipe == (m_pipeNum - 1)) ? true : false; }

    inline bool IsPipeReadyToSubmit() { return (m_currentPipe == (m_pipeIndexForSubmit - 1)) ? true : false; }

    MOS_RESOURCE m_counterChild = {};  //!< NativeFence: Child-to-parent completion counter (N-1 children increment to signal)
    MOS_RESOURCE m_counterParent = {}; //!< NativeFence: Parent-to-child go signal counter (parent increments to 1 to release)

MEDIA_CLASS_DEFINE_END(MediaScalabilityMultiPipe)
};
#endif // !__MEDIA_SCALABILITY_MULTIPIPE_H__
