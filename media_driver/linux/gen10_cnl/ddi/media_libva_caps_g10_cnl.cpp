/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     media_libva_caps_g10_cnl.cpp
//! \brief    This file implements the C++ class/interface for CNL media capbilities. 
//!

#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_caps_g10_cnl.h"
#include "media_libva_caps_factory.h"

MediaLibvaCapsG10Cnl::MediaLibvaCapsG10Cnl(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCapsG10(mediaCtx)
{
    // CNL supported Encode format
    static struct EncodeFormatTable encodeFormatTableCNL[] = 
    {
        {AVC, DualPipe, VA_RT_FORMAT_YUV420},
        {AVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444},
        {HEVC, DualPipe, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP},
        {HEVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
        {VP9, DualPipe, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP},
        {VP9, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 |VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
    };
    m_encodeFormatTable = (struct EncodeFormatTable*)(&encodeFormatTableCNL[0]);
    m_encodeFormatCount = sizeof(encodeFormatTableCNL)/sizeof(struct EncodeFormatTable);

    return;
}

VAStatus MediaLibvaCapsG10Cnl::GetMbProcessingRateEnc(
        MEDIA_FEATURE_TABLE *skuTable,
        uint32_t tuIdx,
        uint32_t codecMode,
        bool vdencActive,
        uint32_t *mbProcessingRatePerSec)
{
    DDI_CHK_NULL(skuTable, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mbProcessingRatePerSec, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t gtIdx = 0;

    if (vdencActive)            
    {
        // For AVC Vdenc
        if (codecMode == CODECHAL_ENCODE_MODE_AVC)
        {   
            if (MEDIA_IS_SKU(skuTable, FtrULX))
            {
                const uint32_t mbRate[7] =
                {
                    // TU7  |  TU6  |   TU5  |  TU4  |  TU3  | TU2  |  TU1
                    1200000, 1200000, 800000, 800000, 800000, 600000, 600000
                };

                *mbProcessingRatePerSec = mbRate[tuIdx];
            }
            else // same numbers for ULT/ classic
            {
                const uint32_t mbRate[7] =
                {
                    // TU7  |  TU6  |   TU5  |   TU4  |   TU3   |  TU2   |  TU1
                    2200000, 2200000, 1650000, 1650000, 1650000, 1100000, 1100000
                };

                *mbProcessingRatePerSec = mbRate[tuIdx];
            }
        }
        else if (codecMode == CODECHAL_ENCODE_MODE_HEVC)
        {
            if (MEDIA_IS_SKU(skuTable, FtrULX))
            {
                const uint32_t mbRate[7] =
                {
                    // TU7  |  TU6  |   TU5  |  TU4  |  TU3  | TU2  |  TU1
                    1200000, 1200000, 600000, 600000, 600000, 300000, 300000
                };

                *mbProcessingRatePerSec = mbRate[tuIdx];
            }
            else // same numbers for ULT/ classic
            {
                const uint32_t mbRate[7] =
                {
                    // TU7  |  TU6  |   TU5  |   TU4  |   TU3   |  TU2   |  TU1
                    2200000, 2200000, 1200000, 1200000, 1200000, 550000, 550000
                };

                *mbProcessingRatePerSec = mbRate[tuIdx];
            }
        }
    }
    else
    {
        DDI_CHK_NULL(m_mediaCtx->pGtSystemInfo, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
        //Dual pipe mode
        if (m_mediaCtx->pGtSystemInfo->EUCount == 16)    //FtrGT0_5
        {
            gtIdx = 5;
        }
        else if (m_mediaCtx->pGtSystemInfo->EUCount == 24)   //FtrGT1
        {
            gtIdx = 4;
        }
        else if (m_mediaCtx->pGtSystemInfo->EUCount == 32)   //FtrGT1_5
        {
            gtIdx = 3;
        }
        else if (m_mediaCtx->pGtSystemInfo->EUCount == 40)   //FtrGT2
        {
            gtIdx = 2;
        }
        else if (m_mediaCtx->pGtSystemInfo->EUCount == 56)   //FtrGT3
        {
            gtIdx = 1;
        }
        else if (m_mediaCtx->pGtSystemInfo->EUCount == 72)   //FtrGT4
        {
            gtIdx = 0;
        }
        else
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        // AVC Dual pipe mode. Data obtained from regular KBL 
        if (codecMode == CODECHAL_ENCODE_MODE_AVC)
        {
            if (MEDIA_IS_SKU(skuTable, FtrULX))
            {
                const uint32_t mbRate[7][6] =
                {
                    // GT4 | GT3 | GT2 | GT1.5 | GT1 | GT0.5
                    { 0, 0, 1029393, 1029393, 676280, 676280 },
                    { 0, 0, 975027, 975027, 661800, 661800 },
                    { 0, 0, 776921, 776921, 640000, 640000 },
                    { 0, 0, 776921, 776921, 640000, 640000 },
                    { 0, 0, 776921, 776921, 640000, 640000 },
                    { 0, 0, 416051, 416051, 317980, 317980 },
                    { 0, 0, 214438, 214438, 180655, 180655 }
                };
                if (gtIdx == 0 || gtIdx == 1)
                {
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }
                *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
            }
            else // same numbers for ULT/ classic
            {
                const uint32_t mbRate[7][6] =
                {
                    // GT4    | GT3   |   GT2  |  GT1.5  |  GT1  |  GT0.5
                    { 1544090, 1544090, 1544090, 1029393, 676280, 676280 },
                    { 1462540, 1462540, 1462540, 975027, 661800, 661800 },
                    { 1165381, 1165381, 1165381, 776921, 640000, 640000 },
                    { 1165381, 1165381, 1165381, 776921, 640000, 640000 },
                    { 1165381, 1165381, 1165381, 776921, 640000, 640000 },
                    { 624076, 624076, 624076, 416051, 317980, 317980 },
                    { 321657, 321657, 321657, 214438, 180655, 180655 }
                };
                *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
            }
        }
        else if (codecMode == CODECHAL_ENCODE_MODE_HEVC)
        {
            // HEVC dual pipe values temporarily set to 50000 for ULT/ULX all TUs and GT
            const uint32_t mbRate[7][6] =
            {
                // GT4    | GT3   |   GT2  | GT1.5  |  GT1  |  GT0.5
                { 500000, 500000, 500000, 500000, 500000, 500000 },
                { 500000, 500000, 500000, 500000, 500000, 500000 },
                { 250000, 250000, 250000, 250000, 250000, 250000 },
                { 250000, 250000, 250000, 250000, 250000, 250000 },
                { 250000, 250000, 250000, 250000, 250000, 250000 },
                { 125000, 125000, 125000, 125000, 125000, 250000 },
                { 125000, 125000, 125000, 125000, 125000, 250000 }
            };
            *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
        }
        else
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER; 
        }
    }
    return VA_STATUS_SUCCESS;
}

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

static bool cnlRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsG10Cnl>((uint32_t)IGFX_CANNONLAKE); 
