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
//! \file     codechal_encode_hevc_g9_skl.h
//! \brief    HEVC dual-pipe encoder for GEN9 SKL.
//!

#ifndef __CODECHAL_ENCODE_HEVC_G9_SKL_H__
#define __CODECHAL_ENCODE_HEVC_G9_SKL_H__

#include "codechal_encode_hevc_g9.h"

//!  HEVC dual-pipe encoder class for GEN9 SKL
/*!
This class defines the member fields, functions for GEN9 SKL platform
*/
class CodechalEncHevcStateG9Skl : public CodechalEncHevcStateG9
{
public:
    //!
    //! \brief    Constructor
    //!     
    CodechalEncHevcStateG9Skl(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncHevcStateG9Skl() {};

    void UpdateSSDSliceCount();

private:

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
   
    // Inherited virtual function
    bool UsePlatformControlFlag()
    {
        return false;
    }

    MOS_STATUS Initialize(CodechalSetting * settings);
};

#endif  // __CODECHAL_ENCODE_HEVC_G9_SKL_H__
