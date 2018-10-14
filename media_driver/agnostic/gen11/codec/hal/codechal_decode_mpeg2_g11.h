/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_decode_mpeg2_g11.h
//! \brief    Defines the decode interface extension for MPEG2.
//! \details  Defines all types, macros, and functions required by CodecHal for MPEG2 decoding. Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_MPEG2_G11_H__
#define __CODECHAL_DECODER_MPEG2_G11_H__

#include "codechal_decode_mpeg2.h"
#include "codechal_decode_singlepipe_virtualengine.h"

//!
//! \class CodechalDecodeMpeg2G11
//! \brief This class defines the member fields, functions etc used by GEN11 MPEG2 decoder.
//!
class CodechalDecodeMpeg2G11 : public CodechalDecodeMpeg2
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
    CodechalDecodeMpeg2G11(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeMpeg2G11(const CodechalDecodeMpeg2G11&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeMpeg2G11& operator=(const CodechalDecodeMpeg2G11&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeMpeg2G11 ();

    //!
    //! \brief    Allocate and initialize MPEG2 decoder standard
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard (
        CodechalSetting *          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for GEN11 MPEG2 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates () override;

    //!
    //! \brief    MPEG2 decoder state level function
    //! \details  State level function for GEN11 MPEG2 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel () override;

    //!
    //! \brief    MPEG2 decoder slice level function 
    //! \details  Primitive level function in VLD mode for GEN specific MPEG2 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          SliceLevel() override;

protected:

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetGpuCtxCreatOption(CodechalSetting * settings) override;
    
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE m_veState = nullptr;  //!< single pipe virtual engine state
};

#endif  // __CODECHAL_DECODER_MPEG2_G11_H__
