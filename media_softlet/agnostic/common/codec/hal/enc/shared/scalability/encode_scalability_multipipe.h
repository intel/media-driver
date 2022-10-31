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
//! \file     encode_scalability_multipipe.h
//! \brief    Defines the common interface for media scalability mulitpipe mode.
//! \details  The media scalability mulitpipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_SCALABILITY_MULTIPIPE_H__
#define __ENCODE_SCALABILITY_MULTIPIPE_H__
#include "mos_defs.h"
#include "mos_os.h"
#include "codec_hw_next.h"
#include "media_scalability_multipipe.h"
#include "encode_scalability_option.h"
#include "mos_os_virtualengine_scalability_next.h"

namespace encode
{
class EncodeScalabilityMultiPipe : public MediaScalabilityMultiPipe
{
public:
    //!
    //! \brief  Encode scalability mulitipipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] componentType
    //!         Component type
    //!
    EncodeScalabilityMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  Encode scalability mulitipipe destructor
    //!
    ~EncodeScalabilityMultiPipe();

    //!
    //! \brief    Copy constructor
    //!
    EncodeScalabilityMultiPipe(const EncodeScalabilityMultiPipe &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    EncodeScalabilityMultiPipe &operator=(const EncodeScalabilityMultiPipe &) = delete;

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
    //! \brief  Oca 1st Level BB Start
    //! \param  [in, out] cmdBuffer
    //!         Reference to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Oca1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer) override;

    //!
    //! \brief  Oca 1st Level BB End
    //! \param  [in, out] cmdBuffer
    //!         Reference to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Oca1stLevelBBEnd(MOS_COMMAND_BUFFER &cmdBuffer) override;

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
    //! \param  [in] semaphoreId
    //!         Id of the semaphore for this sync
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncAllPipes(uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer);
    //!
    //! \brief  Sync pipes with first pipe wait for others
    //! \detials Only support to use this sync once per frame.
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncOnePipeWaitOthers(PMOS_COMMAND_BUFFER cmdBuffer);
    //!
    //! \brief  Sync pipes with second pipe wait for first pipe
    //! \detials Only support to use this sync once per frame.
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncOnePipeForAnother(PMOS_COMMAND_BUFFER cmdBuffer);
    //!
    //! \brief  Sync pipes with other pipes wait for first pipe
    //! \detials Only support to use this sync once per frame.
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncOtherPipesForOne(PMOS_COMMAND_BUFFER cmdBuffer);
    //! \brief  Allocate resources for semaphore
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSemaphore();

    virtual MOS_STATUS SendAttrWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested) override;

    static const uint8_t m_maxPassNum      = 4;
    static const uint8_t m_maxNumBRCPasses = 4;

    static const uint8_t m_maxPipeNum      = 4;
    static const uint8_t m_maxSemaphoreNum = 16;

    CodechalHwInterfaceNext *      m_hwInterface = nullptr;
    MOS_COMMAND_BUFFER             m_primaryCmdBuffer = {};
    MOS_COMMAND_BUFFER             m_secondaryCmdBuffer[m_maxPipeNum * m_maxPassNum] = {};
    MOS_RESOURCE                   m_resSemaphoreAllPipes[m_maxSemaphoreNum] = {};
    MOS_RESOURCE                   m_resSemaphoreOnePipeWait[m_maxPipeNum] = {};
    MOS_RESOURCE                   m_resSemaphoreOnePipeForAnother = {};
    MOS_RESOURCE                   m_resSemaphoreOtherPipesForOne = {};
    uint32_t                       m_numDelay = 15;
    MOS_RESOURCE                   m_resDelayMinus = {0};
    MediaUserSettingSharedPtr      m_userSettingPtr = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeScalabilityMultiPipe)
};
}  // namespace encode
#endif  // !__ENCODE_SCALABILITY_MULTIPIPE_H__
