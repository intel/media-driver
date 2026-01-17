/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_huc_ppgtt.h
//! \brief    Defines the common interface for media huc usage
//! \details  The media huc interface is further sub-divided by different huc usages,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_HUC_PPGTT_H__
#define __ENCODE_HUC_PPGTT_H__

#include "mhw_vdbox_huc_ppgtt_itf.h"
#include "mhw_vdbox_huc_ppgtt_cmdpar.h"

namespace encode
{
    class EncodeHucPPGTTPkt : public mhw::vdbox::huc::ItfPPGTT::ParSetting
    {
    public:
        EncodeHucPPGTTPkt(CodechalHwInterfaceNext *hwInterface)
        {
        }
        virtual ~EncodeHucPPGTTPkt() {}

    protected:
        MHW_SETPAR_DECL_HDR(HUC_IMEM_ADDR);

        PMOS_RESOURCE m_kernelBinBuffer = nullptr;

        MEDIA_CLASS_DEFINE_END(encode__EncodeHucPPGTTPkt)
    };
}
#endif  // !__ENCODE_HUC_PPGTT_H__
