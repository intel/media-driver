/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_av1_scc_xe2_hpm.cpp
//! \brief    SCC feature
//!

#include "encode_av1_scc_xe2_hpm.h"
#include "encode_av1_vdenc_const_settings.h"
#include "encode_av1_tile.h"
#include "encode_av1_segmentation.h"

namespace encode
{
    Av1SccXe2_Hpm::~Av1SccXe2_Hpm()
    {
        ENCODE_FUNC_CALL();
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1SccXe2_Hpm)
    {
        ENCODE_CHK_STATUS_RETURN(Av1Scc::MHW_SETPAR_F(VDENC_CMD2)(params));

#if _MEDIA_RESERVED        
        if (params.vdencCmd2Par133 == true)
        {
            params.vdencCmd2Par102 = false;
            params.vdencCmd2Par101 = false;
        }

        if (m_IBCEnabledForCurrentTile)
        {
            params.vdencCmd2Par3 = vdencCmd2Par3Value3;

            const auto frame_type = static_cast<Av1FrameType>(m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type);
            if (AV1_KEY_OR_INRA_FRAME(frame_type))
            {
                if (m_basicFeature->m_targetUsage == 7 || m_basicFeature->m_targetUsage == 6 || m_basicFeature->m_targetUsage == 4)
                {
                    params.vdencCmd2Par25 = 0;
                    params.vdencCmd2Par26 = 0;
                    params.vdencCmd2Par27 = 0;
                    params.vdencCmd2Par28 = 0;
                    params.vdencCmd2Par29 = 0;
                    params.vdencCmd2Par30 = 0;
                    params.vdencCmd2Par31 = 0x55;
                    params.vdencCmd2Par32 = 0x55;
                    params.vdencCmd2Par33 = 0;
                    params.vdencCmd2Par34 = 0;
                    params.vdencCmd2Par35 = 0;
                    params.vdencCmd2Par36 = 0;
                    params.vdencCmd2Par37 = 0;
                }
                else if (m_basicFeature->m_targetUsage == 2)
                {
                    params.vdencCmd2Par25 = 0;
                    params.vdencCmd2Par26 = 0x24;
                    params.vdencCmd2Par27 = 0;
                    params.vdencCmd2Par28 = 0;
                    params.vdencCmd2Par29 = 0;
                    params.vdencCmd2Par30 = 2;
                    params.vdencCmd2Par31 = 0x90;
                    params.vdencCmd2Par32 = 0x90;
                    params.vdencCmd2Par33 = 0xe4;
                    params.vdencCmd2Par34 = 1;
                    params.vdencCmd2Par35 = 0;
                    params.vdencCmd2Par36 = 2;
                    params.vdencCmd2Par37 = 1;
                }
            }
        }
#else
        params.extSettings.emplace_back(
            [this](uint32_t *data) {
                if (((data[2] >> 6) & 0b1) == true)
                {
                    data[54] &= 0xFFFFFF3F;
                }

                if (m_IBCEnabledForCurrentTile)
                {
                    data[2] |= 0x3;

                    const auto frame_type = static_cast<Av1FrameType>(m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type);
                    if (AV1_KEY_OR_INRA_FRAME(frame_type))
                    {
                        if (m_basicFeature->m_targetUsage == 7 || m_basicFeature->m_targetUsage == 6 || m_basicFeature->m_targetUsage == 4)
                        {
                            data[8]  = 0x55550000;
                            data[9] &= 0xFFFF0000;
                        }
                        else if (m_basicFeature->m_targetUsage == 2)
                        {
                            data[8] = 0x90908090;
                            data[9] = data[9] & 0xFFFF0000 | 0x61e4;
                        }
                    }
                }

                return MOS_STATUS_SUCCESS;
            });
#endif  // _MEDIA_RESERVED

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1SccXe2_Hpm)
    {
        ENCODE_CHK_STATUS_RETURN(Av1Scc::MHW_SETPAR_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)(params));

        if (m_enablePalette)
        {
            params.VdencHEVCVP9TileSlicePar24= 1;
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
