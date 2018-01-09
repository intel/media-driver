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
//! \file     codechal_encode_mpeg2_g9.h
//! \brief    MPEG2 dual-pipe encoder for GEN9 platform.
//!

#ifndef __CODECHAL_ENCODE_MPEG2_G9_H__
#define __CODECHAL_ENCODE_MPEG2_G9_H__

#include "codechal_encode_mpeg2.h"

//!
//! \class   CodechalEncodeMpeg2G9
//! \brief   MPEG2 dual-pipe encoder base class for GEN9
//! \details This class defines the member fields, functions for GEN9 platform
//!
class CodechalEncodeMpeg2G9 : public CodechalEncodeMpeg2
{
public:
    //!
    //! \brief    Constructor
    //!                                                                                         
    CodechalEncodeMpeg2G9(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeMpeg2G9() {};

protected:
    MOS_STATUS Initialize(CodechalSetting * codecHalSettings);

    MOS_STATUS InitKernelState();

    MOS_STATUS InitKernelStateMe();

    MOS_STATUS InitKernelStateMbEnc();
        
    MOS_STATUS SetCurbeMe();
    
    MOS_STATUS SendMeSurfaces(PMOS_COMMAND_BUFFER cmdBuffer);
    
    MOS_STATUS SetCurbeMbEnc(
        bool mbEncIFrameDistEnabled,
        bool mbQpDataEnabled);

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] binary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //! \param    [out] krnHeader
    //!           Pointer to kernel header
    //! \param    [out] krnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void                           *binary,
        EncOperation                   operation,
		uint32_t                       krnStateIdx,
        void                           *krnHeader,
		uint32_t                       *krnSize);

};

#endif  // __CODECHAL_ENCODE_MPEG2_G9_H__
