/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file     codechal_decode_vc1_g11.h
//! \brief    Defines the decode interface extension for Gen11 VC1.
//! \details  Defines all types, macros, and functions required by CodecHal for Gen11 VC1 decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_VC1_G11_H__
#define __CODECHAL_DECODER_VC1_G11_H__

#include "codechal_decode_vc1.h"
#include "codechal_decode_singlepipe_virtualengine.h"

//!
//! \class CodechalDecodeVc1G11
//! \brief This class defines the member fields, functions etc used by Gen11 VC1 decoder.
//!
class CodechalDecodeVc1G11 : public CodechalDecodeVc1
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
    CodechalDecodeVc1G11(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeVc1G11(const CodechalDecodeVc1G11&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeVc1G11& operator=(const CodechalDecodeVc1G11&) = delete;

    ~CodechalDecodeVc1G11();

    MOS_STATUS AllocateStandard(
        CodechalSetting *          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for GEN11 VC1 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetFrameStates() override;

    MOS_STATUS DecodeStateLevel() override;

    MOS_STATUS DecodePrimitiveLevelVLD() override;

    MOS_STATUS DecodePrimitiveLevelIT() override;

protected:
    MOS_STATUS HandleSkipFrame() override;

    MOS_STATUS PerformVc1Olp();

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetGpuCtxCreatOption(CodechalSetting * settings) override;

    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE m_veState = nullptr;  //!< Sinlge pipe VirtualEngine
};

#endif  // __CODECHAL_DECODER_VC1_G11_H__
