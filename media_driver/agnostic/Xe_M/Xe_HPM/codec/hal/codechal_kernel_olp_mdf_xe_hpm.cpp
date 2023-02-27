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
//! \file     codechal_kernel_olp_mdf_xe_hpm.cpp
//! \brief    Implements the MDF OLP kernel for Xe_HPM VC1.
//! \details  Implements the MDF OLP kernel for Xe_HPM VC1.
//!
#include "codechal_kernel_olp_mdf_xe_hpm.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "XE_HPM_VC1_OLP.h"
#endif

MOS_STATUS CodechalKernelOlpMdfXe_Hpm::Init(PMOS_INTERFACE osInterface)
{
    CODECHAL_DECODE_FUNCTION_ENTER;
    CODECHAL_DECODE_CHK_NULL_RETURN(osInterface);
    m_osInterface = osInterface;
    if (m_cmDevice)
    {
        return MOS_STATUS_SUCCESS;
    }

    osInterface->pfnNotifyStreamIndexSharing(osInterface);

    uint32_t devCreateOption = CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE;
    CODECHAL_DECODE_CHK_STATUS_RETURN(osInterface->pfnCreateCmDevice(
        osInterface->pOsContext,
        m_cmDevice,
        devCreateOption,
        CM_DEVICE_CREATE_PRIORITY_DEFAULT));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateQueue(m_cmQueue));
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->LoadProgram(
        (void *)XE_HPM_VC1_OLP,
        XE_HPM_VC1_OLP_SIZE,
        m_cmProgram,
        "-nojitter"));
#endif
    for (int i = 0; i < 2; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateKernel(
            m_cmProgram,
            "VC1_OLP_NV12",
            m_cmKernels[i]));
    }
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateTask(m_cmTask));

    return MOS_STATUS_SUCCESS;
}

