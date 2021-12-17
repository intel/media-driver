/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     codechal_kernel_olp_mdf_xe_hpm.h
//! \brief    Implements the MDF OLP kernel for Xe_HPM VC1.
//! \details  Implements the MDF OLP kernel for Xe_HPM VC1.
//!

#ifndef __CODECHAL_KERNEL_OLP_MDF_XE_HPM_H__
#define __CODECHAL_KERNEL_OLP_MDF_XE_HPM_H__

#include "codechal_kernel_olp_mdf_xe_xpm.h"


//!
//! \class CodechalKernelOlpMdf
//! \brief This class defines the member fields, functions etc used by MDF OLP kernel.
//!
class CodechalKernelOlpMdfXe_Hpm : public CodechalKernelOlpMdf
{
public:
    CodechalKernelOlpMdfXe_Hpm(){};
    virtual ~CodechalKernelOlpMdfXe_Hpm() {}
    MOS_STATUS Init(PMOS_INTERFACE osInterface) override;
};

#endif // __CODECHAL_KERNEL_OLP_MDF_XE_HPM_H__
