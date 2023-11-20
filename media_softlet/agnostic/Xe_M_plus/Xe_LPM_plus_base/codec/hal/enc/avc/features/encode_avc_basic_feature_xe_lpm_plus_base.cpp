/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_avc_basic_feature_xe_lpm_plus_base.cpp
//! \brief    Defines the common interface for encode avc Xe_LPM_plus+ parameter
//!

#include "encode_avc_basic_feature_xe_lpm_plus_base.h"
#include "mhw_vdbox_vdenc_hwcmd_xe_lpm_plus.h"
#include "mos_os_cp_interface_specific.h"

namespace encode
{

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcBasicFeatureXe_Lpm_Plus_Base)
{
    AvcBasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params);

    if (m_seqParam->EnableStreamingBufferLLC || m_seqParam->EnableStreamingBufferDDR)
    {
        params.streamingBufferConfig = mhw::vdbox::vdenc::xe_lpm_plus_base::v0::Cmd::VDENC_PIPE_MODE_SELECT_CMD::STREAMING_BUFFER_64;
        params.captureMode           = mhw::vdbox::vdenc::xe_lpm_plus_base::v0::Cmd::VDENC_PIPE_MODE_SELECT_CMD::CAPTURE_MODE_CAMERA;
    }

    return MOS_STATUS_SUCCESS;
}

bool AvcBasicFeatureXe_Lpm_Plus_Base::InputSurfaceNeedsExtraCopy(const MOS_SURFACE &input)
{
#if _DEBUG || _RELEASE_INTERNAL
    static int8_t supported = -1;

    if (supported == -1)
    {
        MediaUserSetting::Value outValue{};

        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "DisableInputSurfaceCopy",
            MediaUserSetting::Group::Sequence);

        supported = !outValue.Get<bool>();
    }

    if (!supported)
    {
        return false;
    }
#endif

    if (m_osInterface->osCpInterface && m_osInterface->osCpInterface->IsCpEnabled())
    {
        return false;
    }

    uint32_t alignedSize = 0;
    switch (input.Format)
    {
    case Format_NV12:
        alignedSize = MOS_MAX((uint32_t)m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH, (uint32_t)input.dwPitch) *
            (m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT) * 3 / 2;
        break;
    case Format_A8R8G8B8:
        alignedSize = MOS_MAX((uint32_t)m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH * 4, (uint32_t)input.dwPitch) *
            (m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT);
        break;
    default:
        alignedSize = 0;
        break;
    }

    return input.dwSize < alignedSize;
}

}  // namespace encode
