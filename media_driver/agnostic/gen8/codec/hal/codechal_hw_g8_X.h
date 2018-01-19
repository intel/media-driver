/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file      codechal_hw_g8_X.h 
//! \brief         This modules implements HW interface layer to be used on gen8 platforms on all operating systems/DDIs, across CODECHAL components. 
//!
#ifndef __CODECHAL_HW_G8_X_H__
#define __CODECHAL_HW_G8_X_H__

#include "codechal_hw.h"
#include "mhw_mi_hwcmd_g8_X.h"
#include "mhw_render_hwcmd_g8_X.h"

//!  Codechal hw interface Gen8
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen8 platforms
*/
class CodechalHwInterfaceG8 : public CodechalHwInterface
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutG8[CODECHAL_NUM_MEDIA_STATES];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG8(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces)
        : CodechalHwInterface(osInterface, codecFunction, mhwInterfaces)
    {
        CODECHAL_HW_FUNCTION_ENTER;

        m_checkTargetCache = true;
        InitCacheabilityControlSettings(codecFunction);

        m_sizeOfCmdBatchBufferEnd = mhw_mi_g8_X::MI_BATCH_BUFFER_END_CMD::byteSize;

        // Slice Shutdown Threshold
        m_ssdResolutionThreshold = m_sliceShutdownAvcResolutionThreshold;
        m_ssdTargetUsageThreshold = m_sliceShutdownAvcTargetUsageThreshold;
        m_mpeg2SSDResolutionThreshold = m_sliceShutdownMpeg2ResolutionThreshold;

        m_noHuC = true;

        m_maxKernelLoadCmdSize =
            mhw_mi_g8_X::PIPE_CONTROL_CMD::byteSize +
            mhw_render_g8_X::PIPELINE_SELECT_CMD::byteSize +
            mhw_render_g8_X::MEDIA_OBJECT_CMD::byteSize +
            mhw_render_g8_X::STATE_BASE_ADDRESS_CMD::byteSize +
            mhw_render_g8_X::MEDIA_VFE_STATE_CMD::byteSize +
            mhw_render_g8_X::MEDIA_CURBE_LOAD_CMD::byteSize +
            mhw_render_g8_X::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD::byteSize +
            mhw_mi_g8_X::MI_BATCH_BUFFER_START_CMD::byteSize +
            mhw_render_g8_X::MEDIA_OBJECT_WALKER_CMD::byteSize +
            mhw_mi_g8_X::MI_STORE_DATA_IMM_CMD::byteSize;

        m_maxKernelLoadCmdSize += mhw_mi_g8_X::MI_STORE_DATA_IMM_CMD::byteSize +
            mhw_mi_g8_X::MEDIA_STATE_FLUSH_CMD::byteSize;

        m_sizeOfCmdMediaObject = mhw_render_g8_X::MEDIA_OBJECT_CMD::byteSize;
        m_sizeOfCmdMediaStateFlush = mhw_mi_g8_X::MEDIA_STATE_FLUSH_CMD::byteSize;

        m_noSeparateL3LlcCacheabilitySettings = true;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceG8() {}
};

#endif // __CODECHAL_HW_G8_X_H__
