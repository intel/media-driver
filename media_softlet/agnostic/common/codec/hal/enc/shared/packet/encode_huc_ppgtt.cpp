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
//! \file     encode_huc_ppgtt.cpp
//! \brief    Defines the common interface for encode huc implementation
//! \details  The encode huc interface is further sub-divided by different huc usage,
//!           this file is for the base interface which is shared by all.
//!

#include "encode_huc_ppgtt.h"

namespace encode
{
    MHW_SETPAR_DECL_SRC(HUC_IMEM_ADDR, EncodeHucPPGTTPkt)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_kernelBinBuffer);

        params.kernelbinOffset = 0;
        params.kernelBinBuffer = m_kernelBinBuffer;
        params.kernelBinSize   = m_kernelBinBuffer->iSize;
        params.integrityEnable = false;

        return MOS_STATUS_SUCCESS;
    }
}
