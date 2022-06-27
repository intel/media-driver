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
//! \file     encode_const_settings.h
//! \brief
//! \details
//!

#ifndef __ENCODE_CONST_SETTINGS_H__
#define __ENCODE_CONST_SETTINGS_H__

#include <array>
#include <vector>
#include <functional>
#include "media_feature_const_settings.h"
#include "encode_utils.h"
#include "mhw_vdbox_vdenc_cmdpar.h"

struct VdencFeatureSettings: MediaFeatureSettings
{
    virtual ~VdencFeatureSettings(){};

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_STREAMIN_STATE) & par, bool cu64Align)> >
        vdencStreaminStateSettings;

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD1) & par, bool isLowDelay)> >
        vdencCmd1Settings;

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD2) & par, bool isLowDelay)> >
        vdencCmd2Settings;

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD3) & par)> >
        vdencCmd3Settings;

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_AVC_IMG_STATE) & par)> >
        vdencAvcImgStateSettings;
};

class VdencConstSettings : public MediaFeatureConstSettings
{
public:
    VdencConstSettings(PMOS_INTERFACE osInterface) : MediaFeatureConstSettings(osInterface){};
    VdencConstSettings() = default;
    //!
    //! \brief  Frame level update
    //! \param  [in] params
    //!         Pointer to EncoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) = 0;

    //!
    //! \brief  Set OS interface
    //! \param  [in] osItf
    //!         Pointer to OS interface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetOsInterface(PMOS_INTERFACE osItf)
    {
        ENCODE_CHK_NULL_RETURN(osItf);
        m_osItf = osItf;

        return MOS_STATUS_SUCCESS;
    }

protected:
    //!
    //! \brief  Prepare VDENC STREAMIN STATE related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencStreaminStateSettings() { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Prepare VDENC CMD1 related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencCmd1Settings() { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Prepare VDENC CMD2 related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencCmd2Settings() = 0;

    //!
    //! \brief  Prepare BRC related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBrcSettings() = 0;

protected:
    PMOS_INTERFACE m_osItf = nullptr;

MEDIA_CLASS_DEFINE_END(VdencConstSettings)
};

#define VDENC_CMD1_LAMBDA() [&](mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD1) & par, bool isLowDelay) -> MOS_STATUS
#define VDENC_CMD2_LAMBDA() [&](mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD2) & par, bool isLowDelay) -> MOS_STATUS
#define VDENC_CMD3_LAMBDA() [&](mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD3) & par) -> MOS_STATUS
#define VDENC_AVC_IMG_STATE_LAMBDA() [&](mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_AVC_IMG_STATE) & par) -> MOS_STATUS
#define VDENC_STREAMIN_STATE_LAMBDA() [&](mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_STREAMIN_STATE) & par, bool cu64Align) -> MOS_STATUS

#endif  // __ENCODE_CONST_SETTINGS_H__
