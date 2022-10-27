/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     encode_preenc_const_settings.h
//! \brief    Defines the common interface for preenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_PREENC_CONST_SETTINGS_H__
#define __ENCODE_PREENC_CONST_SETTINGS_H__

#include <array>
#include <vector>
#include <functional>
#include "mos_defs.h"
#include "media_class_trace.h"
#include "mhw_vdbox_vdenc_cmdpar.h"
#include "codec_def_encode_hevc.h"
#include "encode_preenc_defs.h"

namespace encode
{
struct PreEncFeatureSettings
{
    std::array<
        std::array<
            std::array<
                std::array<
                    std::array<uint8_t,
                        2>,
                    2>,
                2>,
            2>,
        4>
        transformSkipCoeffsTable{};

    std::array<uint16_t, 52> transformSkipLambdaTable{};

    std::array<
        std::array<
            std::array<
                std::array<uint16_t,
                    52>,
                2>,
            2>,
        2>
        rdoqLamdas8bits{};

    std::array<
        std::array<
            std::array<
                std::array<uint16_t,
                    64>,
                2>,
            2>,
        2>
        rdoqLamdas10bits{};

    std::array<
        std::array<
            std::array<
                std::array<uint16_t,
                    76>,
                2>,
            2>,
        2>
        rdoqLamdas12bits{};

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD1) & par, bool isLowDelay, CODEC_PRE_ENC_PARAMS preEncConfig)> >
        vdencCmd1Settings;

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD2) & par, bool isLowDelay, CODEC_PRE_ENC_PARAMS preEncConfig)> >
        vdencCmd2Settings;
};


class EncodePreEncConstSettings
{
public:

    //!
    //! \brief  EncodePreEncConstSettings constructor
    //!
    EncodePreEncConstSettings();

    //!
    //! \brief  EncodePreEncConstSettings destructor
    //!
    virtual ~EncodePreEncConstSettings();

    MOS_STATUS PrepareConstSettings();

    MOS_STATUS SetVdencCmd1Settings();

    virtual MOS_STATUS SetVdencCmd2Settings();

    MOS_STATUS SetCommonSettings();

    void *GetConstSettings() { return m_featureSetting; };

protected:
    PreEncFeatureSettings *m_featureSetting = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodePreEncConstSettings)
};

#define PREENC_VDENC_CMD1_LAMBDA() [&](mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD1) & par, bool isLowDelay, CODEC_PRE_ENC_PARAMS preEncConfig) -> MOS_STATUS
#define PREENC_VDENC_CMD2_LAMBDA() [&](mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD2) & par, bool isLowDelay, CODEC_PRE_ENC_PARAMS preEncConfig) -> MOS_STATUS

}
#endif // !__ENCODE_PREENC_CONST_SETTINGS_H__

