/*
* Copyright (c) 2012-2017, Intel Corporation
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
//! \file     codechal_decode_hevc_g11.h
//! \brief    Defines the decode interface extension for HEVC.
//! \details  Defines all types, macros, and functions required by CodecHal for GEN11 HEVC decoding. Definitions are not externally facing.
//!            This file should not be included except in codechal_decode_hevc_*.c/h.
//!

#ifndef __CODECHAL_DECODER_HEVC__G11_H__
#define __CODECHAL_DECODER_HEVC__G11_H__

#include "codechal_decode_hevc.h"
#include "codechal_decode_scalability.h"
#include "codechal_decode_singlepipe_virtualengine.h"

class CodechalDecodeHevcG11 : public CodechalDecodeHevc
{
public:

    //!
    //! \brief    Allocate and initialize HEVC decoder standard
    //! \param    [in] settings
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard (
        CodechalSetting *  settings) override;

    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeHevcG11(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeHevcG11(const CodechalDecodeHevcG11&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeHevcG11& operator=(const CodechalDecodeHevcG11&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeHevcG11 ();

    //!
    //! \brief  Set states for each frame to prepare for GEN11 HEVC decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates () override;

    //!
    //! \brief    HEVC decoder state level function
    //! \details  State level function for GEN11 HEVC decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel () override;

    //!
    //! \brief    HEVC decoder primitive level function
    //! \details  Primitive level function for GEN specific HEVC decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodePrimitiveLevel () override;

    //!
    //! \brief    Allocate variable sized resources
    //! \details  Allocate variable sized resources in HEVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateResourcesVariableSizes ();

    //!
    //! \brief    Set S2L picture level Dmem parameters
    //! \details  Set S2L HuC Dmem picture level paramters
    //!
    //! \param    [out] hucHevcS2LPicBss
    //!           Pointer to S2L Dmem picture Bss paramters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetHucDmemS2LPictureBss(
        PHUC_HEVC_S2L_PIC_BSS       hucHevcS2LPicBss) override;

    //!
    //! \brief    Determine Decode Phase
    //! \details  Determine decode phase in hevc decode
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DetermineDecodePhase ();

    //!
    //! \brief    Send S2L picture level commands
    //! \details  Send S2L picture level commands in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SendPictureS2L () override;

    //!
    //! \brief    Initialize the picture level MHW parameters for long format
    //! \details  Initialize the picture level MHW parameters for long format in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  InitPicLongFormatMhwParams() override;

    //!
    //! \brief    Send long format picture level commands
    //! \details  Send long format picture level commands in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SendPictureLongFormat () override;

    //!
    //! \brief    Send long format Slice level commands
    //! \details  Send long format Slice level commands in HEVC decode driver
    //!
    //! \param    [out] cmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] hevcSliceState
    //!           Pointer to HEVC Slice State
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SendSliceLongFormat (
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE hevcSliceState);

    PCODEC_HEVC_EXT_PIC_PARAMS   m_hevcExtPicParams;    //!< Extended pic params for Rext
    PCODEC_HEVC_EXT_SLICE_PARAMS m_hevcExtSliceParams;  //!< Extended slice params for Rext

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        PCODEC_HEVC_PIC_PARAMS     picParams,
        PCODEC_HEVC_EXT_PIC_PARAMS extPicParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_HEVC_SLICE_PARAMS     sliceParams,
        PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
        uint32_t                     numSlices,
        bool                         shortFormatInUse);
#endif

protected:

    //!
    //! \brief  End status report for GEN specific decoder
    //! \param  [in] decodeStatusReport
    //!         Decode status
    //! \param  [out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EndStatusReport(
        CodechalDecodeStatusReport &decodeStatusReport,
        PMOS_COMMAND_BUFFER cmdBuffer) override;

    //!
    //! \brief    End status report for GEN specific decoder
    //  \details  End status report for GEN specific decoder to read HCP status report at end of FE
    //! \param    [out] cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EndStatusReportForFE(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Set And Populate VE Hint parameters
    //! \details  Set Virtual Engine hint parameter and populate it to primary cmd buffer attributes
    //! \param    [in] primCmdBuf
    //!               Pointer to primary cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetAndPopulateVEHintParams(
        PMOS_COMMAND_BUFFER        primCmdBuf);

    //!
    //! \brief    Determine if need to send prolog with frame tracking
    //! \param    [in] sendPrologWithFrameTracking
    //!               Pointer to bool type flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DetermineSendProlgwithFrmTracking(
        bool                        *sendPrologWithFrameTracking);

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetGpuCtxCreatOption(CodechalSetting * settings) override;

    //!
    //! \brief    Check LCU Size
    //! \details  Check LCU Size for HEVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  CheckLCUSize();

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

protected:
    uint32_t m_frameSizeMaxAlloced;  //!< Max Frame size used for buffer allocation in past frames

    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE m_sinlgePipeVeState;  //!< single pipe virtual engine state
    PCODECHAL_DECODE_SCALABILITY_STATE              m_scalabilityState;   //!< Scalability state
};

#endif  // __CODECHAL_DECODER_HEVC__G11_H__
