/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     encode_av1_scc_xe3_lpm_base.cpp
//! \brief    SCC feature
//!

#include "encode_av1_scc_xe3_lpm_base.h"

namespace encode
{

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1SccXe3_Lpm_Base)
    {
        ENCODE_CHK_STATUS_RETURN(Av1Scc::MHW_SETPAR_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)(params));

        if (m_enablePalette)
        {
            params.VdencHEVCVP9TileSlicePar22 = 88;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1SccXe3_Lpm_Base)
    {
        ENCODE_CHK_STATUS_RETURN(Av1Scc::MHW_SETPAR_F(VDENC_CMD2)(params));

#if _MEDIA_RESERVED   
        if (m_enablePalette && (m_basicFeature->m_targetUsage == 7 || m_basicFeature->m_targetUsage == 6))
        {
            params.vdencCmd2Par134[1]          = 1;
            params.vdencCmd2Par138             = 2;
            params.vdencCmd2Par139             = 2;
        }

        if (m_enablePalette && m_basicFeature->m_targetUsage == 7)
        {
            params.vdencCmd2Par89 = 1;
        }

        if ((m_basicFeature->m_targetUsage == 7 || m_basicFeature->m_targetUsage == 6) && (m_enablePalette || m_enableIBC))
        {
            params.vdencCmd2Par100 = 0;
        }

        // Palette ON + IBC OFF tuning for TU6/TU7 using vdencCmd2Par settings
        if (m_enablePalette && !m_enableIBC && (m_basicFeature->m_targetUsage == 6 || m_basicFeature->m_targetUsage == 7))
        {
            const auto frame_type = static_cast<Av1FrameType>(m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type);

            if (AV1_KEY_OR_INRA_FRAME(frame_type))
            {
                // Key frame vdencCmd2Par settings
                params.vdencCmd2Par149 = 3;
                params.vdencCmd2Par138 = 3;
                params.vdencCmd2Par150 = 3;
                params.vdencCmd2Par153 = 4;
                params.vdencCmd2Par139 = 3;
                params.vdencCmd2Par147 = 1;
            }
            else
            {
                // Non-key frame vdencCmd2Par settings
                params.vdencCmd2Par153 = 0;
            }

            // All frames vdencCmd2Par settings
            params.vdencCmd2Par146 = 1;
            params.vdencCmd2Par100 = 3;
        }

        if (m_IBCEnabledForCurrentTile)
        {
            if ((m_basicFeature->m_targetUsage == 2 || m_basicFeature->m_targetUsage == 1) && (params.vdencCmd2Par135[1] == vdencCmd2Par135Value1))
            {
                params.vdencCmd2Par135[1] = vdencCmd2Par135Value2;
            }
            params.vdencCmd2Par140 = 0;
            params.vdencCmd2Par141 = 0;
        }
#else
        params.extSettings.emplace_back([this](uint32_t *data){
            if (m_enablePalette && (m_basicFeature->m_targetUsage == 7 || m_basicFeature->m_targetUsage == 6))
            {
                data[51] = (data[51] & 0xffffff3f) | 0x40;
                data[62] = (data[62] & 0xfffff0ff) | 0x200;
                data[62] = (data[62] & 0xf0ffffff) | 0x2000000;
            }

            if (m_enablePalette && m_basicFeature->m_targetUsage == 7)
            {
                data[52] = (data[52] & 0xf7ffffff) | 0x8000000;
            }

            if ((m_basicFeature->m_targetUsage == 7 || m_basicFeature->m_targetUsage == 6) && (m_enablePalette || m_enableIBC))
            {
                data[54] &= 0xffffffE3;
            }

            // Palette ON + IBC OFF tuning for TU6/TU7 using data array bit manipulation
            if (m_enablePalette && !m_enableIBC && (m_basicFeature->m_targetUsage == 6 || m_basicFeature->m_targetUsage == 7))
            {
                const auto frame_type = static_cast<Av1FrameType>(m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type);

                if (AV1_KEY_OR_INRA_FRAME(frame_type))
                {
                    // Key frame vdencCmd2Par settings in DW62 and DW63
                    data[62] = (data[62] & 0xF00F000F) | 0x03403330;
                    data[63] = (data[63] & 0xF3FFFFFF) | 0x04000000;
                }
                else
                {
                    // Non-key frame vdencCmd2Par settings in DW62
                    data[62] = (data[62] & 0xFF0FFFFF);
                }

                // All frames vdencCmd2Par settings in DW63 and DW54
                data[63] = (data[63] & 0xCFFFFFFF) | 0x10000000;
                data[54] = (data[54] & 0xFFFFFFE3) | 0xC;
            }

            if (m_IBCEnabledForCurrentTile)
            {
                if ((m_basicFeature->m_targetUsage == 2 || m_basicFeature->m_targetUsage == 1) && (((data[51] >> 10) & 0b11) == vdencCmd2Par135Value1))
                {
                    data[51] = (data[51] & 0xfffff3ff) | 0x800;
                }
                data[64] &= 0xfffeffff;
                data[64] &= 0xfffdffff;
            }
            return MOS_STATUS_SUCCESS;
        });
#endif
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD1, Av1SccXe3_Lpm_Base)
    {
        ENCODE_CHK_STATUS_RETURN(Av1Scc::MHW_SETPAR_F(VDENC_CMD1)(params));

        if (m_IBCEnabledForCurrentTile)
        {
            std::fill_n(&params.vdencCmd1Par15[0], 4, (uint8_t)0);
            std::fill_n(&params.vdencCmd1Par14[0], 4, (uint8_t)0);
            std::fill_n(&params.vdencCmd1Par13[0], 4, (uint8_t)0);
            std::fill_n(&params.vdencCmd1Par12[0], 4, (uint8_t)0);

            std::fill_n(&params.vdencCmd1Par11[0], 4, (uint8_t)0);
            std::fill_n(&params.vdencCmd1Par10[0], 4, (uint8_t)0);
            std::fill_n(&params.vdencCmd1Par9[0], 4, (uint8_t)0);
            std::fill_n(&params.vdencCmd1Par8[0], 4, (uint8_t)0);

            params.vdencCmd1Par20 = 0;
            params.vdencCmd1Par16 = 0;
            params.vdencCmd1Par17 = 0;
            params.vdencCmd1Par18 = 0;
            params.vdencCmd1Par19 = 0;

            params.vdencCmd1Par6 = 4;
            params.vdencCmd1Par5 = 8;
            params.vdencCmd1Par7 = 12;
        }

        return MOS_STATUS_SUCCESS;
    }


}  // namespace encode
