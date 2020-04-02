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
//! \file     media_libva_caps_g9_skl.cpp
//! \brief    This file implements the C++ class/interface for SKL media capbilities. 
//!

#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_caps_g9_skl.h"
#include "media_libva_caps_factory.h"

MediaLibvaCapsG9Skl::MediaLibvaCapsG9Skl(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCapsG9(mediaCtx)
{
    // SKL supported Encode format
    static struct EncodeFormatTable encodeFormatTableSKL[] =
    {
        {AVC, DualPipe, VA_RT_FORMAT_YUV420},
        {AVC, Vdenc, VA_RT_FORMAT_YUV420},
        {HEVC, DualPipe, VA_RT_FORMAT_YUV420},
    };
    m_encodeFormatTable = (struct EncodeFormatTable*)(&encodeFormatTableSKL[0]);
    m_encodeFormatCount = sizeof(encodeFormatTableSKL)/sizeof(struct EncodeFormatTable);

    return;
}

VAStatus MediaLibvaCapsG9Skl::GetMbProcessingRateEnc(
        MEDIA_FEATURE_TABLE *skuTable,
        uint32_t tuIdx,
        uint32_t codecMode,
        bool vdencActive,
        uint32_t *mbProcessingRatePerSec)
{
    DDI_CHK_NULL(skuTable, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mbProcessingRatePerSec, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t gtIdx = 0;

    // Calculate the GT index based on GT type
    if (MEDIA_IS_SKU(skuTable, FtrGT1))
    {
        gtIdx = 4;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT1_5))
    {
        gtIdx = 3;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT2))
    {
        gtIdx = 2;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT3))
    {
        gtIdx = 1;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT4))
    {
        gtIdx = 0;
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (MEDIA_IS_SKU(skuTable, FtrULX))
    {
        const uint32_t mbRate[7][5] =
        {
            // GT4 | GT3 |  GT2   | GT1.5  |  GT1
            {    0,   0,   750000,  750000,   676280 },
            {    0,   0,   750000,  750000,  661800 },
            {    0,   0,   750000,  750000,  640000 },
            {    0,   0,   750000,  750000,  640000 },
            {    0,   0,   750000,  750000,  640000 },
            {    0,   0,   416051,  416051,  317980 },
            {    0,   0,   214438,  214438,  180655 }
        };

        if (gtIdx == 0 || gtIdx == 1)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
    }
    else if (MEDIA_IS_SKU(skuTable, FtrULT))
    {
        const uint32_t mbRate[7][5] =
        {
            // GT4    | GT3   |  GT2   | GT1.5   |  GT1
            { 1544090, 1544090, 1544090, 1029393, 676280 },
            { 1462540, 1462540, 1462540,  975027, 661800 },
            { 1165381, 1165381, 1165381,  776921, 640000 },
            { 1165381, 1165381, 1165381,  776921, 640000 },
            { 1165381, 1165381, 1165381,  776921, 640000 },
            {  624076,  624076,  624076,  416051, 317980 },
            {  321657,  321657,  321657,  214438, 180655 }
        };

        *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
    }
    else
    {
        const uint32_t mbRate[7][5] =
        {
            // GT4    | GT3   |   GT2  | GT1.5  |  GT1
            { 1544090, 1544090, 1544090, 1029393, 676280 },
            { 1462540, 1462540, 1462540,  975027, 661800 },
            { 1165381, 1165381, 1165381,  776921, 640000 },
            { 1165381, 1165381, 1165381,  776921, 640000 },
            { 1165381, 1165381, 1165381,  776921, 640000 },
            {  624076,  624076,  624076,  416051, 317980 },
            {  321657,  321657,  321657,  214438, 180655 }
        };

        *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
    }
    return VA_STATUS_SUCCESS;
}

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

static bool sklRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsG9Skl>((uint32_t)IGFX_SKYLAKE);
