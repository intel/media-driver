/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_hevc_vdenc_scc_xe2_hpm.cpp
//! \brief    Defines the Xe2Hpm interface for hevc encode scc features
//!

#include "encode_hevc_vdenc_scc_xe2_hpm.h"
#include "encode_hevc_basic_feature.h"
namespace encode
{

HevcVdencSccXe2_Hpm::HevcVdencSccXe2_Hpm(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) : HevcVdencScc(featureManager, allocator, hwInterface, constSettings)
{
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcVdencSccXe2_Hpm)
{
    ENCODE_CHK_STATUS_RETURN(HevcVdencScc::MHW_SETPAR_F(VDENC_CMD2)(params));

    if (m_enableSCC)
    {
#if _MEDIA_RESERVED
        params.vdencCmd2Par102 = false;
        params.vdencCmd2Par101 = false;
#else
        params.extSettings.emplace_back(
                [this](uint32_t *data) {
                    data[54] &= 0xFFFFFF3F;
                    return MOS_STATUS_SUCCESS;
                });
#endif  // _MEDIA_RESERVED

        if (m_basicFeature->m_targetUsage == 1)
        {
#if _MEDIA_RESERVED
            params.vdencCmd2Par87[2]     = 3;
            params.vdencCmd2Par135[1]    = 1;
            params.vdencCmd2Par88[1][0]  = 3;
            params.vdencCmd2Par93        = 65535;
            params.vdencCmd2Par135[0]    = 1;
#else
            params.extSettings.emplace_back(
                [this](uint32_t *data) {
                    data[51] = data[51] & 0xFF0FC3FF | 0x301400;
                    data[52] |= 0xC0;
                    data[53] |= 0xFFFF0000;
                    return MOS_STATUS_SUCCESS;
                });
#endif  // _MEDIA_RESERVED 
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HevcVdencSccXe2_Hpm)
{
    ENCODE_CHK_STATUS_RETURN(HevcVdencScc::MHW_SETPAR_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)(params));

    auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(hevcFeature);

    uint32_t IbcControl = 0;
    if (hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
    {
        IbcControl = m_enableLBCOnly ? 1 : 3;
    }
    else
    {
        IbcControl = 0;
    }
    uint32_t PaletteModeEnable  = (hevcFeature->m_hevcSeqParams->palette_mode_enabled_flag != 0) ? 1 : 0;
    uint32_t SliceQP            = hevcFeature->m_hevcPicParams->QpY + hevcFeature->m_hevcSliceParams->slice_qp_delta;
    uint32_t BitDepthLumaMinus8 = hevcFeature->m_hevcSeqParams->bit_depth_luma_minus8;
    uint8_t  TargetUsage        = hevcFeature->m_hevcSeqParams->TargetUsage;

    // For IBC
    params.VdencHEVCVP9TileSlicePar0 = 0;
    params.ibcControl                = IbcControl;

    // For palette mode
    params.paletteModeEnable         = PaletteModeEnable;
    params.VdencHEVCVP9TileSlicePar1 = 1;

    uint32_t tableIdx;
    if (SliceQP <= 12)
    {
        tableIdx = 0;
    }
    else if (SliceQP > 12 && SliceQP <= 17)
    {
        tableIdx = 1;
    }
    else if (SliceQP > 17 && SliceQP <= 22)
    {
        tableIdx = 2;
    }
    else if (SliceQP > 22 && SliceQP <= 27)
    {
        tableIdx = 3;
    }
    else if (SliceQP > 27 && SliceQP <= 32)
    {
        tableIdx = 4;
    }
    else if (SliceQP > 32 && SliceQP <= 37)
    {
        tableIdx = 5;
    }
    else if (SliceQP > 37 && SliceQP <= 42)
    {
        tableIdx = 6;
    }
    else if (SliceQP > 42 && SliceQP <= 47)
    {
        tableIdx = 7;
    }
    else if (SliceQP > 47 && SliceQP <= 49)
    {
        tableIdx = 8;
    }
    else
    {
        tableIdx = 9;
    }

    static const uint32_t table[3][10][11] =
        {
            {
                {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 0},
                {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 0},
                {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 0},
                {16, 16, 4, 8, 10, 12, 128, 2, 1, 1, 0},
                {32, 32, 8, 4, 10, 4, 128, 2, 2, 1, 0},
                {48, 32, 12, 6, 16, 4, 128, 2, 2, 1, 0},
                {64, 63, 12, 6, 24, 1, 128, 2, 2, 1, 0},
                {96, 63, 12, 6, 24, 1, 128, 2, 3, 1, 0},
                {128, 63, 16, 12, 32, 1, 128, 2, 6, 1, 0},
                {256, 48, 24, 6, 48, 1, 128, 3, 8, 1, 0},
            },
            {
                {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 0},
                {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 0},
                {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 0},
                {16, 16, 4, 8, 10, 12, 128, 2, 1, 1, 0},
                {32, 32, 8, 4, 10, 4, 128, 2, 2, 1, 0},
                {48, 32, 12, 6, 16, 4, 128, 2, 2, 1, 0},
                {64, 63, 12, 6, 24, 1, 128, 2, 2, 1, 0},
                {96, 63, 12, 6, 24, 1, 128, 2, 3, 1, 0},
                {128, 63, 16, 12, 32, 1, 128, 2, 6, 1, 0},
                {256, 48, 24, 6, 48, 1, 128, 3, 8, 1, 0},
            },
            {
                {256, 24, 4, 4, 12, 8, 128, 2, 1, 1, 0},
                {256, 32, 4, 4, 12, 8, 128, 2, 1, 1, 0},
                {256, 32, 4, 4, 16, 8, 128, 2, 1, 1, 0},
                {256, 32, 8, 4, 16, 8, 128, 2, 1, 1, 0},
                {256, 32, 8, 4, 32, 4, 128, 3, 1, 1, 0},
                {768, 32, 8, 4, 32, 4, 128, 3, 1, 1, 0},
                {768, 63, 32, 8, 64, 1, 128, 3, 4, 1, 0},
                {768, 63, 48, 8, 128, 1, 128, 3, 12, 1, 0},
                {768, 63, 48, 8, 128, 1, 128, 3, 24, 1, 0},
                {768, 63, 64, 8, 128, 1, 128, 4, 32, 0, 0},
            },
        };

    params.VdencHEVCVP9TileSlicePar14 = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][0];
    params.VdencHEVCVP9TileSlicePar8  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][1];
    params.VdencHEVCVP9TileSlicePar6  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][2];
    params.VdencHEVCVP9TileSlicePar9  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][3];
    params.VdencHEVCVP9TileSlicePar7  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][4];
    params.VdencHEVCVP9TileSlicePar10 = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][5];

    params.VdencHEVCVP9TileSlicePar5  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][7];
    params.VdencHEVCVP9TileSlicePar2  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][8];
    params.VdencHEVCVP9TileSlicePar3  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][9];
    params.VdencHEVCVP9TileSlicePar15 = 0;

    if (BitDepthLumaMinus8 > 0 && PaletteModeEnable)
    {
        uint32_t shift = BitDepthLumaMinus8;
        params.VdencHEVCVP9TileSlicePar5 += shift;
        params.VdencHEVCVP9TileSlicePar6 <<= shift;
        params.VdencHEVCVP9TileSlicePar7 <<= shift;
        if (params.VdencHEVCVP9TileSlicePar14 >= 256)
        {
            params.VdencHEVCVP9TileSlicePar14 = 255;
        }
        params.VdencHEVCVP9TileSlicePar14 <<= shift;
    }

    params.VdencHEVCVP9TileSlicePar4  = 6;
    params.VdencHEVCVP9TileSlicePar11 = 1;
    params.VdencHEVCVP9TileSlicePar12 = 72;
    params.VdencHEVCVP9TileSlicePar13 = 2;

    params.VdencHEVCVP9TileSlicePar16[2] = 1;
    params.VdencHEVCVP9TileSlicePar16[1] = 0;
    params.VdencHEVCVP9TileSlicePar16[0] = 1;
    params.VdencHEVCVP9TileSlicePar23    = 6;

    if (TargetUsage == 7 || TargetUsage == 6)
    {
        params.VdencHEVCVP9TileSlicePar17[2] = 49;
        params.VdencHEVCVP9TileSlicePar17[1] = 49;
        params.VdencHEVCVP9TileSlicePar17[0] = 49;
    }
    else
    {
        params.VdencHEVCVP9TileSlicePar17[2] = 63;
        params.VdencHEVCVP9TileSlicePar17[1] = 63;
        params.VdencHEVCVP9TileSlicePar17[0] = 63;
    }
    //simplified PLT
    if (PaletteModeEnable)
    {
        params.VdencHEVCVP9TileSlicePar5 = 4;

        if (BitDepthLumaMinus8 > 0)
        {
            uint32_t shift = hevcFeature->m_hevcSeqParams->bit_depth_luma_minus8;
            params.VdencHEVCVP9TileSlicePar5 += shift;
        }
    }
    return MOS_STATUS_SUCCESS;
}

bool HevcVdencSccXe2_Hpm::IsCompressFlagNeeded()
{
    auto skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrXe2Compression))
    {
        return false;
    }
    return true;
}

}  // namespace encode

