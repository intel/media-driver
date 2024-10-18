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
//! \file     encode_hevc_vdenc_const_settings_xe2_hpm.cpp
//! \brief    Defines the common interface for xe2 hpm hevc vdenc const settings
//!

#include "codec_def_common.h"
#include "encode_hevc_vdenc_const_settings_xe2_hpm.h"
#include "encode_utils.h"
#include "mos_solo_generic.h"

namespace encode
{
    MOS_STATUS EncodeHevcVdencConstSettingsXe2_Hpm::SetTUSettings()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_featureSetting);
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
        ENCODE_CHK_NULL_RETURN(setting);

        setting->rdoqEnable = {true, true, true, true, true, true, true, false};
        setting->acqpEnable = {true, true, true, true, true, true, true, false};

        return eStatus;
    }

    MOS_STATUS EncodeHevcVdencConstSettingsXe2_Hpm::SetVdencStreaminStateSettings()
    {
        ENCODE_FUNC_CALL();

        auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
        ENCODE_CHK_NULL_RETURN(setting);

        setting->vdencStreaminStateSettings.emplace_back(
            VDENC_STREAMIN_STATE_LAMBDA(){
                static const std::array<
                    std::array<
                        uint8_t,
                        NUM_TARGET_USAGE_MODES + 1>,
                    4>
                    numMergeCandidates = {{
                        {3, 3, 3, 2, 2, 2, 1, 1},
                        {3, 3, 3, 2, 2, 2, 2, 2},
                        {4, 4, 3, 2, 2, 2, 2, 1},
                        {3, 3, 3, 2, 2, 2, 2, 2},
                    }};

                static const std::array<
                    uint8_t,
                    NUM_TARGET_USAGE_MODES + 1>
                    numImePredictors = {12, 12, 8, 8, 8, 8, 6, 3};

                par.maxTuSize                = 3;  //Maximum TU Size allowed, restriction to be set to 3
                par.maxCuSize                = (cu64Align) ? 3 : 2;
                par.numMergeCandidateCu64x64 = numMergeCandidates[3][m_hevcSeqParams->TargetUsage];
                par.numMergeCandidateCu32x32 = numMergeCandidates[2][m_hevcSeqParams->TargetUsage];
                par.numMergeCandidateCu16x16 = numMergeCandidates[1][m_hevcSeqParams->TargetUsage];
                par.numMergeCandidateCu8x8   = numMergeCandidates[0][m_hevcSeqParams->TargetUsage];
                par.numImePredictors         = numImePredictors[m_hevcSeqParams->TargetUsage];

                auto waTable = m_osItf == nullptr ? nullptr : m_osItf->pfnGetWaTable(m_osItf);
                if (waTable)
                {
                    if (MEDIA_IS_WA(waTable, WaHEVCVDEncROINumMergeCandidateSetting) && m_hevcSeqParams->TargetUsage == 4)
                    {
                        par.numMergeCandidateCu64x64 = 3;
                        par.numMergeCandidateCu32x32 = 3;
                        par.numMergeCandidateCu16x16 = 2;
                        par.numMergeCandidateCu8x8   = 1;
                    }

                    ENCODE_CHK_NULL_RETURN(m_osItf);
                    if (MEDIA_IS_WA(waTable, Wa_22011549751) && m_hevcPicParams->CodingType == I_TYPE && !m_osItf->bSimIsActive && !Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osItf->pOsContext) && !m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
                    {
                        par.numMergeCandidateCu64x64 = 0;
                        par.numMergeCandidateCu32x32 = 0;
                        par.numMergeCandidateCu16x16 = 0;
                        par.numMergeCandidateCu8x8   = 2;
                        par.numImePredictors         = 0;
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHevcVdencConstSettingsXe2_Hpm::SetVdencCmd1Settings()
    {
        ENCODE_FUNC_CALL();

        EncodeHevcVdencConstSettings::SetVdencCmd1Settings();

        auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
        ENCODE_CHK_NULL_RETURN(setting);

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 7)
                {
                    par.vdencCmd1Par2[6] = 143;
                    par.vdencCmd1Par2[7] = 143;
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
            {
                static const std::array<uint8_t, 12> data = {
                    3, 10, 16, 22, 29, 35, 42, 48, 54, 61, 67, 74};

                for (size_t i = 0; i < data.size(); i++)
                {
                    par.vdencCmd1Par4[i] = data[i];
                }
            }

            return MOS_STATUS_SUCCESS;
        });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    static const std::array<uint8_t, 16> data = {
                        11, 14, 15, 17, 
                        14, 17, 18, 19, 
                        11, 17, 18, 19, 
                         0, 23, 24, 25
                    };

                    for (size_t i = 0; i < 4; i++)
                    {
                        par.vdencCmd1Par8[i]  = data[i];
                        par.vdencCmd1Par9[i]  = data[i + 4];
                        par.vdencCmd1Par10[i] = data[i + 8];
                        par.vdencCmd1Par11[i] = data[i + 12];
                    }

                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        par.vdencCmd1Par8[0]  = 0;
                        par.vdencCmd1Par9[0]  = 0;
                        par.vdencCmd1Par10[0] = 0;
                        par.vdencCmd1Par11[0] = 0;
                    }
                }
                else
                {
                    static const std::array<uint8_t, 16> data = {
                        11, 14, 15, 17, 
                        14, 17, 18, 19, 
                        14, 17, 18, 19, 
                        20, 23, 24, 25,
                    };

                    for (size_t i = 0; i < 4; i++)
                    {
                        par.vdencCmd1Par8[i]  = data[i];
                        par.vdencCmd1Par9[i]  = data[i + 4];
                        par.vdencCmd1Par10[i] = data[i + 8];
                        par.vdencCmd1Par11[i] = data[i + 12];
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    static const std::array<uint8_t, 16> data = {
                        23, 26, 27, 29, 
                        26, 29, 30, 31, 
                        21, 29, 30, 31, 
                         0, 41, 42, 43};

                    for (size_t i = 0; i < 4; i++)
                    {
                        par.vdencCmd1Par12[i] = data[i];
                        par.vdencCmd1Par13[i] = data[i + 4];
                        par.vdencCmd1Par14[i] = data[i + 8];
                        par.vdencCmd1Par15[i] = data[i + 12];
                    }

                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        par.vdencCmd1Par12[0] = 0;
                        par.vdencCmd1Par13[0] = 0;
                        par.vdencCmd1Par14[0] = 0;
                        par.vdencCmd1Par15[0] = 0;
                    }
                }
                else
                {
                    static const std::array<uint8_t, 16> data = {
                        23, 26, 27, 29, 
                        26, 29, 30, 31, 
                        26, 29, 30, 31, 
                        38, 41, 42, 43,
                    };

                    for (size_t i = 0; i < 4; i++)
                    {
                        par.vdencCmd1Par12[i] = data[i];
                        par.vdencCmd1Par13[i] = data[i + 4];
                        par.vdencCmd1Par14[i] = data[i + 8];
                        par.vdencCmd1Par15[i] = data[i + 12];
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    if (m_hevcPicParams->CodingType == P_TYPE)
                    {
                        par.vdencCmd1Par16 = 82;
                        par.vdencCmd1Par17 = 20;
                        par.vdencCmd1Par18 = 83;
                        par.vdencCmd1Par19 = 17;
                        par.vdencCmd1Par20 = 15;
                        par.vdencCmd1Par21 = 0;
                    }
                    else if (m_hevcPicParams->CodingType == B_TYPE)
                    {
                        par.vdencCmd1Par16 = 99;
                        par.vdencCmd1Par17 = 23;
                        par.vdencCmd1Par18 = 99;
                        par.vdencCmd1Par19 = 19;
                        par.vdencCmd1Par20 = 17;
                        par.vdencCmd1Par21 = 0;
                    }
                }
                else
                {
                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        par.vdencCmd1Par16 = 92;
                        par.vdencCmd1Par17 = 23;
                        par.vdencCmd1Par18 = 92;
                        par.vdencCmd1Par19 = 21;
                        par.vdencCmd1Par20 = 23;
                        par.vdencCmd1Par21 = 0;
                    }
                    else if (m_hevcPicParams->CodingType == B_TYPE)
                    {
                        par.vdencCmd1Par16 = 110;
                        par.vdencCmd1Par17 = 26;
                        par.vdencCmd1Par18 = 110;
                        par.vdencCmd1Par19 = 24;
                        par.vdencCmd1Par20 = 26;
                        par.vdencCmd1Par21 = 0;
                    }
                    else if (m_hevcPicParams->CodingType == P_TYPE)
                    {
                        par.vdencCmd1Par16 = 110;
                        par.vdencCmd1Par17 = 26;
                        par.vdencCmd1Par18 = 110;
                        par.vdencCmd1Par19 = 24;
                        par.vdencCmd1Par20 = 26;
                        par.vdencCmd1Par21 = 0;
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcPicParams->CodingType == I_TYPE)
                {
                    par.vdencCmd1Par23 = 42;
                    if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                    {
                        par.vdencCmd1Par23 = 63;
                    }
                }
                else if (m_hevcPicParams->CodingType == B_TYPE)
                {
                    par.vdencCmd1Par23 = 54;
                }
                else if (m_hevcPicParams->CodingType == P_TYPE)
                {
                    par.vdencCmd1Par23 = 54;
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        par.vdencCmd1Par30 = 12;
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        par.vdencCmd1Par36 = 17;
                        par.vdencCmd1Par37 = 47;
                        par.vdencCmd1Par38 = 20;
                        par.vdencCmd1Par39 = 9;
                        par.vdencCmd1Par40 = 17;
                        par.vdencCmd1Par41 = m_hevcPicParams->NumROI ? 0 : 30;
                    }
                    else
                    {
                        par.vdencCmd1Par36 = 7;
                        par.vdencCmd1Par37 = 18;
                        par.vdencCmd1Par38 = 18;
                        par.vdencCmd1Par39 = 18;
                        par.vdencCmd1Par40 = 27;
                        par.vdencCmd1Par41 = m_hevcPicParams->NumROI ? 0 : 68;
                    }
                }
                else
                {
                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        par.vdencCmd1Par36 = 21;
                        par.vdencCmd1Par37 = 47;
                        par.vdencCmd1Par38 = 16;
                        par.vdencCmd1Par39 = 16;
                        par.vdencCmd1Par40 = 30;
                        par.vdencCmd1Par41 = m_hevcPicParams->NumROI ? 0 : 30;
                    }
                    else if (m_hevcPicParams->CodingType == B_TYPE)
                    {
                        par.vdencCmd1Par36 = 7;
                        par.vdencCmd1Par37 = 20;
                        par.vdencCmd1Par38 = 20;
                        par.vdencCmd1Par39 = 20;
                        par.vdencCmd1Par40 = 30;
                        par.vdencCmd1Par41 = m_hevcPicParams->NumROI ? 0 : 68;
                    }
                    else if (m_hevcPicParams->CodingType == P_TYPE)
                    {
                        par.vdencCmd1Par36 = 7;
                        par.vdencCmd1Par37 = 20;
                        par.vdencCmd1Par38 = 20;
                        par.vdencCmd1Par39 = 20;
                        par.vdencCmd1Par40 = 30;
                        par.vdencCmd1Par41 = m_hevcPicParams->NumROI ? 0 : 68;
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    if (m_hevcPicParams->CodingType == P_TYPE)
                    {
                        par.vdencCmd1Par48 = 0;
                        par.vdencCmd1Par49 = 32;
                        par.vdencCmd1Par50 = 68;
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    par.vdencCmd1Par55 = 0x0E;
                    par.vdencCmd1Par56 = 0x0E;
                    par.vdencCmd1Par57 = 0x0C;
                    par.vdencCmd1Par58 = 0x0B;

                    par.vdencCmd1Par59 = 0x10;
                    par.vdencCmd1Par60 = 0x10;
                    par.vdencCmd1Par61 = 0x0F;
                    par.vdencCmd1Par62 = 0x0F;

                    par.vdencCmd1Par63 = 0x10;
                    par.vdencCmd1Par64 = 0x10;
                    par.vdencCmd1Par65 = 0x10;
                    par.vdencCmd1Par66 = 0x10;

                    par.vdencCmd1Par67 = 0x14;
                    par.vdencCmd1Par68 = 0x10;
                    par.vdencCmd1Par69 = 0x10;
                    par.vdencCmd1Par70 = 0x10;

                    par.vdencCmd1Par71 = 0x0C;
                    par.vdencCmd1Par72 = 0x0C;
                    par.vdencCmd1Par73 = 0x0A;
                    par.vdencCmd1Par74 = 0x0A;

                    par.vdencCmd1Par75 = 0x10;
                    par.vdencCmd1Par76 = 0x10;
                    par.vdencCmd1Par77 = 0x10;
                    par.vdencCmd1Par78 = 0x10;

                    par.vdencCmd1Par79 = 0x10;
                    par.vdencCmd1Par80 = 0x10;
                    par.vdencCmd1Par81 = 0x10;
                    par.vdencCmd1Par82 = 0x10;

                    par.vdencCmd1Par83 = 0x10;
                    par.vdencCmd1Par84 = 0x10;
                    par.vdencCmd1Par85 = 0x0E;
                    par.vdencCmd1Par86 = 0x0F;
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                if (m_hevcSeqParams->TargetUsage == 2 || m_hevcSeqParams->TargetUsage == 6)
                {
                    static constexpr std::array<
                        std::array<uint8_t, 3>,
                        3>
                        data = {{{20, 35, 35},
                            {20, 35, 35},
                            {47, 16, 16}}};

                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        par.vdencCmd1Par87 = data[2][2];
                        par.vdencCmd1Par88 = data[2][1];
                        par.vdencCmd1Par89 = data[2][0];
                    }
                    else if (m_hevcPicParams->CodingType == P_TYPE)
                    {
                        par.vdencCmd1Par87 = data[1][2];
                        par.vdencCmd1Par88 = data[1][1];
                        par.vdencCmd1Par89 = data[1][0];
                    }
                    else if (m_hevcPicParams->CodingType == B_TYPE)
                    {
                        par.vdencCmd1Par87 = data[0][2];
                        par.vdencCmd1Par88 = data[0][1];
                        par.vdencCmd1Par89 = data[0][0];
                    }
                }

                return MOS_STATUS_SUCCESS;
            });

        setting->vdencCmd1Settings.emplace_back(
            VDENC_CMD1_LAMBDA() {
                par.vdencCmd1Par95 = 50;

                return MOS_STATUS_SUCCESS;
            });

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHevcVdencConstSettingsXe2_Hpm::SetVdencCmd2Settings()
    {
        ENCODE_FUNC_CALL();

        auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
        ENCODE_CHK_NULL_RETURN(setting);

#if _MEDIA_RESERVED
#define VDENC_CMD2_SETTINGS_EXT
#include "encode_hevc_vdenc_const_settings_xe2_hpm_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#else
#define VDENC_CMD2_SETTINGS_OPEN
#include "encode_hevc_vdenc_const_settings_xe2_hpm_open.h"
#undef VDENC_CMD2_SETTINGS_OPEN
#endif  // !(_MEDIA_RESERVED)

        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
