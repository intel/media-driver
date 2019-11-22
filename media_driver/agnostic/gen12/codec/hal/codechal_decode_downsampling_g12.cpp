/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_decode_downsampling_g12.cpp
//! \brief    Implements the decode interface extension for downsampling on Gen12.
//!
#include "codechal_decoder.h"

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif

#include "codechal_decode_downsampling_g12.h"
#include "mhw_render_g12_X.h"
#include "codechal_mmc_g12.h"
#include "mos_util_user_interface_g12.h"

FieldScalingInterfaceG12::FieldScalingInterfaceG12(CodechalHwInterface *hwInterface) :
    FieldScalingInterface(hwInterface)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase = (uint8_t*)IGCODECKRN_G12;
#else
    m_kernelBase = NULL;
#endif

    InitInterfaceStateHeapSetting(hwInterface);
}

MOS_STATUS FieldScalingInterfaceG12::SetupMediaVfe(
    PMOS_COMMAND_BUFFER  cmdBuffer,
    MHW_KERNEL_STATE     *kernelState)
{
    MHW_VFE_PARAMS_G12 vfeParams = {};
    vfeParams.pKernelState = kernelState;
    
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FieldScalingInterfaceG12::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    if (m_mmcState == nullptr)
    {
        m_mmcState = MOS_New(CodecHalMmcStateG12, m_hwInterface);
        CODECHAL_DECODE_CHK_NULL_RETURN(m_mmcState);
    }
#endif
    return MOS_STATUS_SUCCESS;
}