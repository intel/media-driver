/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     decode_scalability_singlepipe.h
//! \brief    Defines the common interface for decode scalability singlepipe mode.
//! \details  The decode scalability singlepipe interface is further sub-divided by codecs,
//!           this file is for the base interface which is shared by all codecs.
//!

#ifndef __DECODE_SCALABILITY_SINGLEPIPE_H__
#define __DECODE_SCALABILITY_SINGLEPIPE_H__
#include "mos_defs.h"
#include "mos_os.h"
#include "codechal_hw.h"
#include "decode_scalability_singlepipe_next.h"
#include "decode_scalability_option.h"

namespace decode
{

class DecodeScalabilitySinglePipe: public DecodeScalabilitySinglePipeNext
{
public:
    //!
    //! \brief  decode scalability singlepipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to MediaContext
    //! \param  [in] componentType
    //!         Component type
    //!
    DecodeScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  decode scalability singlepipe destructor
    //!
    virtual ~DecodeScalabilitySinglePipe() {};

    //!
    //! \brief    Copy constructor
    //!
    DecodeScalabilitySinglePipe(const DecodeScalabilitySinglePipe&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    DecodeScalabilitySinglePipe& operator=(const DecodeScalabilitySinglePipe&) = delete;

    //!
    //! \brief   Initialize the decode single scalability
    //! \details It will prepare the resources needed in scalability
    //!          and initialize the state of scalability
    //! \param   [in] option
    //!          Input scalability option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(const MediaScalabilityOption &option) override;

    //!
    //! \brief  Update the media scalability singlepipe mode state
    //! \param  [in] statePars
    //!         parameters to update the state
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateState(void *statePars) override;

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
    //! \brief  Submit command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;
    //!
    //! \brief  Create decode single pipe
    //! \param  [in] hwInterface
    //!         void type hw interface
    //! \param  [in] mediaContext
    //!         required media context to create single pipe
    //! \param  [in] componentType
    //!         Inidcate component.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS CreateDecodeSinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

protected:
    //!
    //! \brief  Verify command buffer size and patch list size, reallocate if required
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS VerifySpaceAvailable(uint32_t requestedSize, 
                uint32_t requestedPatchListSize, 
                bool &singleTaskPhaseSupportedInPak) override;

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

    virtual MOS_STATUS SendAttrWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested) override;

private:
    CodechalHwInterface *m_hwInterface = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodeScalabilitySinglePipe)
};

}
#endif // !__MEDIA_SCALABILITY_SINGLEPIPE_H__

