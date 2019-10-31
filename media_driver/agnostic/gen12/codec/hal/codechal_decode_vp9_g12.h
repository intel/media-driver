/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_decode_vp9_g12.h
//! \brief    Defines the decode interface extension for Gen12 VP9.
//! \details  Defines all types, macros, and functions required by CodecHal for Gen12 VP9 decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODE_VP9_G12_H__
#define __CODECHAL_DECODE_VP9_G12_H__

#include "codechal_decode_vp9.h"
#include "codechal_decode_scalability_g12.h"
#include "codechal_decode_singlepipe_virtualengine.h"
#include "codechal_decode_sfc_vp9_g12.h"
#include "mhw_vdbox_g12_X.h"

//!
//! \class CodechalDecodeVp9G12
//! \brief This class defines the member fields, functions etc used by gen12 VP9 decoder.
//!
class CodechalDecodeVp9G12 : public CodechalDecodeVp9
{
public:
    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeVp9G12(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeVp9G12(const CodechalDecodeVp9G12&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeVp9G12& operator=(const CodechalDecodeVp9G12&) = delete;

    ~CodechalDecodeVp9G12();

    MOS_STATUS  AllocateStandard (
        CodechalSetting *          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for VP9 G12 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates() override;

    MOS_STATUS  DecodeStateLevel () override;

    MOS_STATUS  DecodePrimitiveLevel () override;

    MOS_STATUS  InitializeDecodeMode () override;

    MOS_STATUS AllocateResourcesVariableSizes() override;

    MOS_STATUS DetermineDecodePhase() override;

    MOS_STATUS InitSfcState() override;

    MOS_STATUS CalcDownsamplingParams(
        void                        *picParams,
        uint32_t                    *refSurfWidth,
        uint32_t                    *refSurfHeight,
        MOS_FORMAT                  *format,
        uint8_t                     *frameIdx) override;

    MOS_STATUS InitMmcState() override;

    //!
    //! \brief  Sets cenc decoder batch buffer
    //! \param    [in] cmdBuffer
    //!           Pointer of command buffer.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCencBatchBuffer( PMOS_COMMAND_BUFFER cmdBuffer) override;

protected:

    //!
    //! \brief    Set And Populate VE Hint parameters
    //! \details  Set Virtual Engine hint parameter and populate it to primary cmd buffer attributes
    //! \param    [in] primCmdBuf
    //!               Pointer to primary cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetAndPopulateVEHintParams(
        PMOS_COMMAND_BUFFER         primCmdBuf);

    //!
    //! \brief    Determin if need to send prolog with frame tracking
    //! \param    [in] sendPrologWithFrameTracking
    //!               Pointer to bool type flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DetermineSendProlgwithFrmTracking(
        bool                        *sendPrologWithFrameTracking);

    //!
    //! \brief    Add picture state mhw parameters
    //! \details  Add picture state mhw parameters to command buffer for VP9 decoder
    //! \param    [in] cmdBuffer
    //!           Command buffer to hold HW commands
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AddPicStateMhwCmds(
        PMOS_COMMAND_BUFFER       cmdBuffer) override;

    MOS_STATUS SetGpuCtxCreatOption(CodechalSetting *settings) override;

private:
    //!
    //! \brief  The virtual function for decode standard to override the requested space size
    //! \param    [in] requestedSize
    //!           The intial request size computed by picture level and slice level
    //! \return The final requested space size
    //!
    uint32_t RequestedSpaceSize(uint32_t requestedSize) override;

    //!
    //! \brief  The virtual function for decode standard to override the extra requested space size
    //! \param    [in] requestedSize
    //!           The intial request size computed by picture level and slice level
    //! \param    [in] additionalSizeNeeded
    //!           The additional request size for command buffer
    //! \return The extra requested space size
    //!
    virtual MOS_STATUS VerifyExtraSpace(
        uint32_t requestedSize,
        uint32_t additionalSizeNeeded) override;

    //!
    //! \brief  Utility function to allocate internal histogram surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateHistogramSurface();

protected:
    uint32_t m_frameSizeMaxAlloced;  //!< Max Frame size used for buffer allocation in past frames

    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE m_sinlgePipeVeState;  //!< single pipe virtual engine state
    PCODECHAL_DECODE_SCALABILITY_STATE_G12           m_scalabilityState;   //!< Scalability state
#ifdef _DECODE_PROCESSING_SUPPORTED
    CodechalVp9SfcStateG12 *m_sfcState = nullptr;  //!< VP9 SFC state
#endif
    PMOS_SURFACE m_histogramSurface = nullptr;  //!< Internal histogram buffer
};
#endif  // __CODECHAL_DECODE_VP9_G12_H__

