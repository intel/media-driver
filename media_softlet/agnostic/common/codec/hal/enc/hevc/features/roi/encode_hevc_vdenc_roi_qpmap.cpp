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
//! \file     encode_hevc_vdenc_roi_qpmap.cpp
//! \brief    implementation of qpmap ROI

#include "mos_defs.h"
#include "encode_hevc_vdenc_roi_qpmap.h"
namespace encode
{
    void QPMapROI::SetQpRoiCtrlPerLcu(
        StreamInParams* streaminParams,
        HevcVdencStreamInState* data)
    {
        data->DW7.QpEnable = 0xf;
        data->DW14.ForceQp_0 = streaminParams->forceQp[0];
        data->DW14.ForceQp_1 = streaminParams->forceQp[1];
        data->DW14.ForceQp_2 = streaminParams->forceQp[2];
        data->DW14.ForceQp_3 = streaminParams->forceQp[3];
    }

    void QPMapROI::SetRoiCtrlMode(
        uint32_t        lcuIndex,
        StreamInParams &streaminParams,
        uint32_t        w_in16,
        uint32_t        h_in16,
        uint32_t        Pitch,
        uint8_t *       QpData)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(QpData);
        // Calculate ForceQp
        bool     cu64Align      = false;
        uint32_t streamInWidth  = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32);
        uint32_t streamInHeight = (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(QpData);
        uint32_t x = 0, y = 0;
        //calculate offset in 32x32 cus in x/y direction by lcuIndex in zigzag order
        ZigZagToRaster(streamInWidth, lcuIndex, x, y);

        int8_t tmpQP = m_basicFeature->m_hevcPicParams->QpY +
                       m_basicFeature->m_hevcSliceParams->slice_qp_delta;
        streaminParams.setQpRoiCtrl = true;
        streaminParams.forceQp[0] = streaminParams.forceQp[1] = tmpQP;
        streaminParams.forceQp[2] = streaminParams.forceQp[3] = tmpQP;
        int8_t forceQP = tmpQP;

        int w_oflcu = (x == MOS_ROUNDUP_DIVIDE(w_in16, 2) && w_in16 % 2 != 0) ? 1 : 2;
        int h_oflcu = (y == MOS_ROUNDUP_DIVIDE(h_in16, 2) && h_in16 % 2 != 0) ? 1 : 2;
        if (x <= MOS_ROUNDUP_DIVIDE(w_in16, 2) && y <= MOS_ROUNDUP_DIVIDE(h_in16, 2))
        {
            for (int i = 0; i < w_oflcu; i++)
            {
                for (int j = 0; j < h_oflcu; j++)
                {
                    tmpQP = *(QpData + (y * 2 + i) * Pitch + x * 2 + j);
                    tmpQP = tmpQP ? tmpQP : forceQP;
                    streaminParams.forceQp[h_oflcu * i + j] = (int8_t)CodecHal_Clip3(10, 51, tmpQP);
                }
            }
        }
    }

    MOS_STATUS QPMapROI::WriteStreaminData(
        uint32_t                  lcuIndex,
        RoiOverlap::OverlapMarker marker,
        uint32_t                  roiRegionIndex,
        uint8_t *                 rawStreamIn)
    {
        ENCODE_CHK_NULL_RETURN(rawStreamIn);
        bool cu64Align = false;

        StreamInParams streaminDataParams;
        MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
        uint8_t *QpData = (uint8_t *)m_allocator->LockResourceForRead(&(m_basicFeature->m_mbQpDataSurface.OsResource));
        ENCODE_CHK_NULL_RETURN(QpData);

        uint32_t w_in16 = m_basicFeature->m_mbQpDataSurface.dwWidth;
        uint32_t h_in16 = m_basicFeature->m_mbQpDataSurface.dwHeight;
        uint32_t Pitch  = m_basicFeature->m_mbQpDataSurface.dwPitch;

        SetRoiCtrlMode(lcuIndex, streaminDataParams, w_in16, h_in16, Pitch, QpData);
        SetQpRoiCtrlPerLcu(&streaminDataParams, (HevcVdencStreamInState *)(rawStreamIn + (lcuIndex * 64)));

        m_allocator->UnLock(&(m_basicFeature->m_mbQpDataSurface.OsResource));
        HevcVdencStreamInState *data = (HevcVdencStreamInState *)(rawStreamIn + (lcuIndex * 64));

        if (lcuIndex % 4 == 3)
        {
            if ((data - 3)->DW14.ForceQp_0 == (data - 2)->DW14.ForceQp_0 &&
                (data - 2)->DW14.ForceQp_0 == (data - 1)->DW14.ForceQp_0 &&
                (data - 1)->DW14.ForceQp_0 == data->DW14.ForceQp_0)
            {
                cu64Align = true;
            }
            for (int i = 0; i < 4; i++)
            {
                MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
                SetStreaminParamByTU(cu64Align, streaminDataParams);
                SetStreaminDataPerLcu(&streaminDataParams, rawStreamIn + (lcuIndex-i) * 64);
            }
        }
        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
