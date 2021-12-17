/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     codechal_decode_vc1_xe_hpm.h
//! \brief    Implements the decode interface extension for Xe_HPM VC1.
//! \details  Implements all functions required by CodecHal for Xe_HPM VC1 decoding.
//!


#ifndef __CODECHAL_DECODE_VC1_XE_HPM_H__
#define __CODECHAL_DECODE_VC1_XE_HPM_H__

#include "codechal_decode_vc1_g12.h"
#include "codechal_kernel_base.h"
#include "codechal_kernel_olp_mdf_xe_hpm.h"

class CodechalKernelOlpMdfXe_Hpm;

//!
//! \class CodechalDecodeVc1Xe_Hpm
//! \brief This class defines the member fields, functions etc used by Xe_HPM VC1 decoder.
//!
class CodechalDecodeVc1Xe_Hpm : public CodechalDecodeVc1G12
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
       CodechalDecodeVc1Xe_Hpm(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeVc1Xe_Hpm(const CodechalDecodeVc1Xe_Hpm&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeVc1Xe_Hpm &operator=(const CodechalDecodeVc1Xe_Hpm &) = delete;

    //!
    //! \brief  Destructor
    //!
    ~CodechalDecodeVc1Xe_Hpm();

protected:
    MOS_STATUS AllocateStandard(
        CodechalSetting *          settings) override;

    MOS_STATUS PerformVc1Olp() override;

    MOS_STATUS HandleSkipFrame() override;

    CodechalKernelOlpMdfXe_Hpm *m_olpMdfKernel = nullptr;
    MOS_SURFACE           m_olpInYSurface;
    MOS_SURFACE           m_olpInUVSurface;
    MOS_SURFACE           m_olpOutYSurface;
    MOS_SURFACE           m_olpOutUVSurface;
};


#endif // __CODECHAL_DECODE_VC1_XE_HPM_H__
