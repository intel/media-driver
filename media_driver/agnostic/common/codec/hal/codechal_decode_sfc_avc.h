/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_decode_sfc_avc.h
//! \brief    Defines the SFC interface extension for AVC decode.
//! \details  Defines all types, macros, and functions required by CodecHal SFC for AVC decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODE_SFC_AVC_H__
#define __CODECHAL_DECODE_SFC_AVC_H__

#include "codechal_decode_sfc.h"

//!
//! \class    CodechalAvcSfcState
//! \brief    Codechal AVC SFC state
//!
class CodechalAvcSfcState : public CodechalSfcState
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalAvcSfcState() { CODECHAL_HW_FUNCTION_ENTER; };
    //!
    //! \brief    Destructor
    //!
    ~CodechalAvcSfcState() { CODECHAL_HW_FUNCTION_ENTER; };

    //!
    //! \brief    Check if SFC output is supported and Initialize SFC
    //! \param    [in] decProcessingParams
    //!           Pointer to decode processing params
    //! \param    [in] picParams
    //!           Pointer to Avc Pic Params
    //! \param    [in] width
    //!           Input Frame width and height
    //! \param    [in] height
    //!           Input Frame width and height
    //! \param    [in] deblockingEnabled
    //!           Enable deblocking
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckAndInitialize(
        PCODECHAL_DECODE_PROCESSING_PARAMS  decProcessingParams,
        PCODEC_AVC_PIC_PARAMS               picParams,
        uint32_t                            width,
        uint32_t                            height,
        bool                                deblockingEnabled);

    //!
    //! \brief    Update Input Info for SfcStateParams
    //! \param    [in] sfcStateParams
    //!           Pointer to Sfc State Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateInputInfo(
        PMHW_SFC_STATE_PARAMS               sfcStateParams);
};

#endif
