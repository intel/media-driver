/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     media_scalability.h
//! \brief    Defines the common interface for media scalability
//! \details  The media scalability interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_SCALABILITY_H__
#define __MEDIA_SCALABILITY_H__
#include <stdint.h>
#include <memory>
#include "mos_defs.h"
#include "mos_os_specific.h"

#include "mos_os_virtualengine_specific.h"

#include "mos_os.h"

#include "mhw_mi_itf.h"
class MediaScalabilityOption;

struct ComponentState;
struct ScalabilityPars;
#define COMMAND_BUFFER_RESERVED_SPACE 0x80
class MediaStatusReport;
class MediaContext;
class MediaScalability : public mhw::mi::Itf::ParSetting
{
    friend class MediaContext;

public:
    //!
    //! \brief  Media scalability constructor
    //!
    MediaScalability(MediaContext *mediaContext);

    MediaScalability(){};
    //!
    //! \brief  Media scalability destructor
    //!
    virtual ~MediaScalability() {}

    //!
    //! \brief    Copy constructor
    //!
    MediaScalability(const MediaScalability &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    MediaScalability &operator=(const MediaScalability &) = delete;

    //!
    //! \brief  Check if the scalability mode decided by parameters matched with current
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return bool
    //!         true if mode decided by input parameters matched with current mode,
    //!         false if not matched.
    //!
    bool IsScalabilityModeMatched(ScalabilityPars *params);

    //!
    //! \brief  Check if the scalability mode matched with current
    //! \param  [in] scalabOption
    //!         Scalability option to match with current
    //! \return bool
    //!         true if mode matched with current mode,
    //!         false if not matched.
    //!
    bool IsScalabilityModeMatched(MediaScalabilityOption &scalabOption);

    //!
    //! \brief  Check if the Gpu ctx create option matched
    //! \param  [in] gpuCtxCreateOption1
    //!         Pointer to the input gpu ctx create option for compare
    //! \param  [in] gpuCtxCreateOption2
    //!         Pointer to the input gpu ctx create option for compare
    //! \return bool
    //!         true if the two option are matched and can share one gpu ctx,
    //!         false if not matched.
    //!
    bool IsGpuCtxCreateOptionMatched(PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption1, PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption2);

    //!
    //! \brief   Initialize the media scalability
    //! \details It will prepare the resources needed in scalability
    //!          and initialize the state of scalability
    //! \param   [in] option
    //!          Input scalability option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(const MediaScalabilityOption &option) = 0;

    //!
    //! \brief  Construct parameters for GPU context create.
    //! \param   [in, out] gpuCtxCreateOption
    //!          Pointer to the GPU Context Create Option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption) = 0;

    //!
    //! \brief  Destroy the media scalability
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy();

    //!
    //! \brief  Update the media scalability state
    //! \param  [in] statePars
    //!         parameters to update the state
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateState(void *statePars) = 0;

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
    virtual MOS_STATUS VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak) = 0;

    //!
    //! \brief  Get command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested = true) = 0;

    //!

    //! \brief  Return command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    //!
    //! \brief  Submit command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

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
    virtual MOS_STATUS SyncPipe(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    //!
    //! \brief  Reset semaphore
    //! \param  [in] syncType
    //!         type of pipe sync
    //! \param  [in] semaphoreId
    //!         Id of the semaphore used for this sync
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ResetSemaphore(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    //!
    //! \brief  Oca 1st Level BB Start
    //! \param  [in, out] cmdBuffer
    //!         Reference to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Oca1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer) { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Oca 1st Level BB End
    //! \param  [in, out] cmdBuffer
    //!         Reference to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Oca1stLevelBBEnd(MOS_COMMAND_BUFFER &cmdBuffer) { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Get pipe number
    //! \return Pipe number
    //!
    uint8_t GetPipeNumber() { return m_pipeNum; };

    //!
    //! \brief  Get current pipe
    //! \return Current pipe index
    //!
    uint8_t GetCurrentPipe() { return m_currentPipe; };

    //!
    //! \brief  Get pass number
    //! \return Pass number
    //!
    uint16_t GetPassNumber() { return m_passNum; };

    //!
    //! \brief  Get current pass
    //! \return Current pass index
    //!
    uint16_t GetCurrentPass() { return m_currentPass; };

    //!
    //! \brief  Set pass number
    //! \return void
    //!
    void SetPassNumber(uint16_t num) { m_passNum = num; };

    //!
    //! \brief  Set pass index
    //! \return void
    //!
    void SetCurrentPassIndex(uint8_t num) { m_currentPass = num; };

    //!
    //! \brief  Set pipe number
    //! \return void
    //!
    void SetPipeNumber(uint8_t num) { m_pipeNum = num; };

    //!
    //! \brief  Set pipe index
    //! \return void
    //!
    void SetCurrentPipeIndex(uint8_t num) { m_currentPipe = num; };

    uint8_t GetCurrentRow() { return m_currentRow; };

    uint8_t GetCurrentSubPass() { return m_currentSubPass; };

    //!
    //! \brief  Get component state
    //! \return point to component state
    //!
    ComponentState *GetComponentState() { return m_componentState; };

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
    virtual MOS_STATUS SendAttrWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested) = 0;

    //!
    //! \brief  Get if frame tracking is enabled from scalability
    //! \return bool
    //!         true if enabled, else false
    //!
    bool IsFrameTrackingEnabled() { return m_frameTrackingEnabled; };

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
        uint32_t requestedCommandBufferSize,
        uint32_t requestedPatchListSize)
    {
        return MOS_STATUS_SUCCESS;
    }

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
    virtual MOS_STATUS VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak);

    uint8_t m_currentPipe = 0;  //!< Current pipe index
    uint16_t m_currentPass = 0;  //!< Current pass index

    uint8_t m_pipeNum                  = 1;     //!< Pipe number
    uint16_t m_passNum                 = 1;     //!< Pass number
    bool    m_singleTaskPhaseSupported = true;  //!< Indicate if single task phase is supported
    uint8_t m_pipeIndexForSubmit       = 0;     //!< Pipe index to submit cmdbuffer

    uint8_t m_currentRow     = 0;  //!< Current row index when tile replay is enabled
    uint8_t m_currentSubPass = 0;  //!< Current tile row pass index when tile replay is enabled

    ComponentState *m_componentState = nullptr; //!< Component state

    uint8_t                  m_componentType        = 0;
    PMOS_INTERFACE           m_osInterface          = nullptr;  //!< OS interface
    MediaScalabilityOption * m_scalabilityOption    = nullptr;
    PMOS_GPUCTX_CREATOPTIONS m_gpuCtxCreateOption   = nullptr;  //!<For MultiPipe cases, it should be converted to PMOS_GPUCTX_CREATOPTIONS_ENHANCED;
    MediaStatusReport *      m_statusReport         = nullptr;  //!< Media status report ptr
    MediaContext *           m_mediaContext         = nullptr;  //!< Media context ptr
    bool                     m_attrReady            = false;    //!< Indicate if cmd buf attribute is ready
    bool                     m_frameTrackingEnabled = true;     //!< Indicate if frame tracking is enabled

    PMOS_VIRTUALENGINE_HINT_PARAMS m_veHitParams = nullptr;  //!< Virtual Engine hint parameters

    MOS_VE_HANDLE     m_veState = nullptr; //!< Virtual Engine State
    std::shared_ptr<mhw::mi::Itf> m_miItf = nullptr;
    PMOS_VIRTUALENGINE_INTERFACE  m_veInterface = nullptr;  //!< Virtual Engine Interface

MEDIA_CLASS_DEFINE_END(MediaScalability)
};
#endif  // !__MEDIA_SCALABILITY_H__
