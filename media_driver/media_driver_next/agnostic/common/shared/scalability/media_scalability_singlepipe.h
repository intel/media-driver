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
//! \file     media_scalability_singlepipe.h
//! \brief    Defines the common interface for media scalability singlepipe mode.
//! \details  The media scalability singlepipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_SCALABILITY_SINGLEPIPE_H__
#define __MEDIA_SCALABILITY_SINGLEPIPE_H__
#include "mos_defs.h"
#include "mos_os.h"
#include "media_scalability.h"

class MediaScalabilitySinglePipe: public MediaScalability
{

public:
    //!
    //! \brief  Media scalability singlepipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] componentType
    //!         Component type
    //!
    MediaScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  Media scalability singlepipe destructor
    //!
    virtual ~MediaScalabilitySinglePipe(){};

    //!
    //! \brief    Copy constructor
    //!
    MediaScalabilitySinglePipe(const MediaScalabilitySinglePipe&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    MediaScalabilitySinglePipe& operator=(const MediaScalabilitySinglePipe&) = delete;
 
    //!
    //! \brief   Initialize the media single pipe scalability
    //! \details It will prepare the resources needed in scalability
    //!          and initialize the state of scalability
    //! \param   [in] option
    //!          Input scalability option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(const MediaScalabilityOption &option) override;

    //!
    //! \brief  Construct parameters for GPU context create.
    //! \param   [in, out] gpuCtxCreateOption
    //!          Pointer to the GPU Context Create Option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption) override;

    //!
    //! \brief  Destroy the media scalability
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

    //!
    //! \brief  Update the media scalability singlepipe mode state
    //! \param  [in] statePars
    //!         parameters to update the state
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateState(void *statePars) override { return MOS_STATUS_SUCCESS; }

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
    virtual MOS_STATUS VerifyCmdBuffer(uint32_t requestedSize, 
                uint32_t requestedPatchListSize, 
                bool &singleTaskPhaseSupportedInPak)  override;

    //!
    //! \brief  Get command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    //!
    //! \brief  Return command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    //!
    //! \brief  Submit command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

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
    virtual MOS_STATUS SyncPipe(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer) override { return MOS_STATUS_SUCCESS;};

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
    virtual MOS_STATUS ResetSemaphore(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer) override { return MOS_STATUS_SUCCESS;};

protected:
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
};

#endif // !__MEDIA_SCALABILITY_SINGLEPIPE_H__

