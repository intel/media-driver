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
//! \file      codechal_hw_g9_X.h 
//! \brief         This modules implements HW interface layer to be used on gen9 platforms on all operating systems/DDIs, across CODECHAL components. 
//!
#ifndef __CODECHAL_HW_G9_X_H__
#define __CODECHAL_HW_G9_X_H__

#include "codechal_hw.h"
#include "mhw_mi_hwcmd_g9_X.h"
#include "mhw_render_hwcmd_g9_X.h"
#include "mhw_vdbox_mfx_hwcmd_g9_skl.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_skl.h"

//!  Codechal hw interface Gen9
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen9 platforms
*/
class CodechalHwInterfaceG9 : public CodechalHwInterface
{
protected:
    static const uint32_t m_sliceShutdownAvcTargetUsageThresholdG9Halo = 4;
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutG9[CODECHAL_NUM_MEDIA_STATES];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG9(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces)
        : CodechalHwInterface(osInterface, codecFunction, mhwInterfaces)
    {
        CODECHAL_HW_FUNCTION_ENTER;

        InitCacheabilityControlSettings(codecFunction);

        m_sizeOfCmdBatchBufferEnd = mhw_mi_g9_X::MI_BATCH_BUFFER_END_CMD::byteSize;
        m_vdencBrcImgStateBufferSize = mhw_vdbox_vdenc_g9_skl::VDENC_IMG_STATE_CMD::byteSize + mhw_vdbox_mfx_g9_skl::MFX_AVC_IMG_STATE_CMD::byteSize +
            mhw_mi_g9_X::MI_BATCH_BUFFER_END_CMD::byteSize;

        // Slice Shutdown Threshold
        m_ssdResolutionThreshold = m_sliceShutdownAvcResolutionThreshold;
        if (MEDIA_IS_SKU(m_skuTable, FtrGT4))
        {
            m_ssdTargetUsageThreshold = m_sliceShutdownAvcTargetUsageThresholdG9Halo;
        }
        else
        {
            m_ssdTargetUsageThreshold = m_sliceShutdownAvcTargetUsageThreshold;
        }
        m_mpeg2SSDResolutionThreshold = m_sliceShutdownMpeg2ResolutionThreshold;

        m_ssEuTable = m_defaultSsEuLutG9;

        m_maxKernelLoadCmdSize =
            mhw_mi_g9_X::PIPE_CONTROL_CMD::byteSize +
            mhw_render_g9_X::PIPELINE_SELECT_CMD::byteSize +
            mhw_render_g9_X::MEDIA_OBJECT_CMD::byteSize +
            mhw_render_g9_X::STATE_BASE_ADDRESS_CMD::byteSize +
            mhw_render_g9_X::MEDIA_VFE_STATE_CMD::byteSize +
            mhw_render_g9_X::MEDIA_CURBE_LOAD_CMD::byteSize +
            mhw_render_g9_X::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD::byteSize +
            mhw_mi_g9_X::MI_BATCH_BUFFER_START_CMD::byteSize +
            mhw_render_g9_X::MEDIA_OBJECT_WALKER_CMD::byteSize +
            mhw_mi_g9_X::MI_STORE_DATA_IMM_CMD::byteSize;

        m_sizeOfCmdMediaObject = mhw_render_g9_X::MEDIA_OBJECT_CMD::byteSize;
        m_sizeOfCmdMediaStateFlush = mhw_mi_g9_X::MEDIA_STATE_FLUSH_CMD::byteSize;

        if (osInterface->bEnableVdboxBalancing)
        {
           bEnableVdboxBalancingbyUMD = true;
           // Enabled VDbox Balancing, the HuC will be disabled.
           m_noHuC = true;
        }
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceG9() {}
};

#endif // __CODECHAL_HW_G9_X_H__
