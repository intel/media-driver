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
                data[52] &= 0xfffffff1;
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
