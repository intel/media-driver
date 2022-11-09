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
//! \file     decode_scalability_multipipe_next.h
//! \brief    Defines the common interface for decode scalability mulitpipe mode.
//! \details  The decode scalability mulitpipe interface is further sub-divided by codecs,
//!           this file is for the base interface which is shared by all codecs.
//!

#ifndef __DECODE_SCALABILITY_MULTIPIPE_NEXT_H__
#define __DECODE_SCALABILITY_MULTIPIPE_NEXT_H__
#include "mos_defs.h"
#include "mos_os.h"
#include "codec_hw_next.h"
#include "media_scalability_multipipe.h"
#include "decode_scalability_option.h"
#include "mos_os_virtualengine_next.h"
#include "decode_phase.h"

namespace decode
{
class DecodeScalabilityMultiPipeNext : public MediaScalabilityMultiPipe
{
public:
    //!
    //! \brief  Decode scalability mulitipipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to MediaContext
    //! \param  [in] componentType
    //!         Component type
    //!
    DecodeScalabilityMultiPipeNext(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  Decode scalability mulitipipe destructor
    //!
    ~DecodeScalabilityMultiPipeNext();

    //!
    //! \brief    Copy constructor
    //!
    DecodeScalabilityMultiPipeNext(const DecodeScalabilityMultiPipeNext &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    DecodeScalabilityMultiPipeNext &operator=(const DecodeScalabilityMultiPipeNext &) = delete;

    //!
    //! \brief   Initialize the media scalability
    //! \details It will prepare the resources needed in scalability
    //!          and initialize the state of scalability
    //! \param   [in] option
    //!          Input scalability option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize(const MediaScalabilityOption &option) override;

    //!
    //! \brief  Construct parameters for GPU context create.
    //! \param   [in, out] gpuCtxCreateOption
    //!          Pointer to the GPU Context Create Option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption) override;

    //!
    //! \brief  Destroy the media scalability
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

    //!
    //! \brief  Update the media scalability state
    //! \param  [in] statePars
    //!         parameters to update the state
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateState(void *statePars) override;

    //!
    //! \brief  Verify command buffer
    //! \param  [in] requestedSize
    //!         requested size for command buffer
    //! \param  [in] requestedPatchListSize
    //!         requested size for patched list
    //! \param  [out] singleTaskPhaseSupportedInPak
    //!         Inidcate if to use single task phase in pak.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak) override;

    //!
    //! \brief  Get command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested = true) override;

    //!
    //! \brief  Return command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    //!
    //! \brief  Submit command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    //!
    //! \brief  Add synchronization for pipes.
    //! \param  [in] syncType
    //!         type of pipe sync
    //! \param  [in] semaphoreId
    //!         Id of the semaphore used for this sync
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncPipe(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer) override;

    //!
    //! \brief  Reset semaphore
    //! \param  [in] syncType
    //!         type of pipe sync to find the related semaphore
    //! \param  [in] semaphoreId
    //!         Id of the semaphore for reset
    //! \param  [in, out] cmdBuffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResetSemaphore(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer) override;
    //!
    //! \brief  Create decode multi pipe
    //! \param  [in] hwInterface
    //!         void type hw interface
    //! \param  [in] mediaContext
    //!         required media context to create multi pipe
    //! \param  [in] componentType
    //!         Inidcate component.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS CreateDecodeMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);
protected:

    //!
    //! \brief    Resizes the cmd buffer and patch list with cmd buffer header
    //!
    //! \param    [in] requestedCommandBufferSize
    //!           Requested resize command buffer size
    //! \param    [in] requestedPatchListSize
    //!           Requested resize patchlist size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ResizeCommandBufferAndPatchList(
        uint32_t                    requestedCommandBufferSize,
        uint32_t                    requestedPatchListSize) override;

    //!
    //! \brief  Verify primary command buffer
    //! \param  [in] requestedSize
    //!         requested size for command buffer
    //! \param  [in] requestedPatchListSize
    //!         requested size for patched list
    //! \param  [out] singleTaskPhaseSupportedInPak
    //!         Inidcate if to use single task phase in pak.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak) override;

    //!
    //! \brief  Set hint parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetHintParams();

    //!
    //! \brief  Populate hint parameters
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PopulateHintParams(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Sync all pipes
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncAllPipes(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Sync pipes with first pipe wait for others
    //! \detials Only support to use this sync once per frame.
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \param  [in] pipeIdx
    //!         The index of pipeline which wait for others
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncOnePipeWaitOthers(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t pipeIdx);

    //! \brief  Allocate resources for semaphore
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSemaphore();

    //!
    //! \brief  Send Cmd buffer Attributes with frame tracking info
    //!
    //! \param  [in] cmdBuffer
    //!         Reference to command buffer
    //! \param  [in] frameTrackingRequested
    //!         Indicate if frame tracking is requested
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAttrWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested) override;

    static const uint8_t m_initSecondaryCmdBufNum = 8; //!< The initial secondary command buffer size
    static const uint8_t m_maxCmdBufferSetsNum = 16; //!< The max number of command buffer sets

    CodechalHwInterfaceNext        *m_hwInterface      = nullptr;
    MOS_COMMAND_BUFFER              m_primaryCmdBuffer = {};
    std::vector<MOS_COMMAND_BUFFER> m_secondaryCmdBuffers;

    std::vector<std::vector<MOS_RESOURCE>> m_resSemaphoreAllPipes;    //!< The sync semaphore between all pipes
    std::vector<std::vector<MOS_RESOURCE>> m_resSemaphoreOnePipeWait; //!< The sync semaphore between main pipe and other pipes
    uint8_t                                m_semaphoreIndex = 0;      //!< The index for semaphore using by current frame

    DecodePhase                    *m_phase = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodeScalabilityMultiPipeNext)
};
}  // namespace decode
#endif  // !__DECODE_SCALABILITY_MULTIPIPE_NEXT_H__
