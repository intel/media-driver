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

#ifndef __ENCODE_SCALABILITY_SINGLEPIPE_H__
#define __ENCODE_SCALABILITY_SINGLEPIPE_H__
#include "mos_defs.h"
#include "mos_os.h"
#include "codec_hw_next.h"
#include "media_scalability_singlepipe_next.h"
#include "encode_scalability_option.h"

namespace encode
{

class EncodeScalabilitySinglePipe: public MediaScalabilitySinglePipeNext
{

public:
    //!
    //! \brief  Encode scalability singlepipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] componentType
    //!         Component type
    //!
    EncodeScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  Encode scalability singlepipe destructor
    //!
    virtual ~EncodeScalabilitySinglePipe(){};

    //!
    //! \brief    Copy constructor
    //!
    EncodeScalabilitySinglePipe(const EncodeScalabilitySinglePipe&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    EncodeScalabilitySinglePipe& operator=(const EncodeScalabilitySinglePipe&) = delete;

    //!
    //! \brief   Initialize the encode single scalability
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

    //void              InitMmioRegisters();
    //MmioRegistersMfx *GetMmioRegisters(MHW_VDBOX_NODE_IND index);
    //bool              ConvertToMiRegister(MHW_VDBOX_NODE_IND index, MHW_MI_MMIOREGISTERS &mmioRegister);

private:
    CodechalHwInterfaceNext   *m_hwInterface    = nullptr;
    MediaUserSettingSharedPtr  m_userSettingPtr = nullptr;

    ////! \brief Mmio registers address
    //MHW_MI_MMIOREGISTERS m_mmioRegisters = {};  //!< mfx mmio registers

MEDIA_CLASS_DEFINE_END(encode__EncodeScalabilitySinglePipe)
};

}
#endif // !__MEDIA_SCALABILITY_SINGLEPIPE_H__

