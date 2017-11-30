/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file      codec_ddi.c
//! \brief         This modules implements structures, arrays and macros common     to all of codec.
//!

#include "codec_ddi.h"
#include "codec_def_common.h"

// ---------------------------
// Tables
// ---------------------------


bool GetMbProcessingRateEnc(
    PLATFORM                            platform,
    MEDIA_FEATURE_TABLE                 *skuTable,
    MEDIA_SYSTEM_INFO                   *gtSystemInfo,
    uint32_t                            tuIdx,
    uint32_t                            codechalMode,
    bool                                vdencActive,
    uint32_t*                           mbProcessingRatePerSec
    )
{
    MOS_STATUS  eStatus;
    bool        res = false;
    uint32_t    gtIdx = 0;

    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, skuTable);
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, mbProcessingRatePerSec);

    if (GFX_GET_CURRENT_PRODUCT(platform) == IGFX_BROADWELL)
    {
        // Calculate the GT index based on GT type
        if (MEDIA_IS_SKU(skuTable, FtrGT1))
        {
            gtIdx = 3;
        }
        else if (MEDIA_IS_SKU(skuTable, FtrGT1_5))
        {
            gtIdx = 2;
        }
        else if (MEDIA_IS_SKU(skuTable, FtrGT2))
        {
            gtIdx = 1;
        }
        else if (MEDIA_IS_SKU(skuTable, FtrGT3))
        {
            gtIdx = 0;
        }
        else
        {
            goto finish;
        }

        if (MEDIA_IS_SKU(skuTable, FtrULX))
        {
            static const uint32_t BDWULX_MB_RATE[7][4] =
            {
                // GT3 |  GT2   | GT1.5  |  GT1 
                { 0, 750000, 750000, 676280 },
                { 0, 750000, 750000, 661800 },
                { 0, 750000, 750000, 640000 },
                { 0, 750000, 750000, 640000 },
                { 0, 750000, 750000, 640000 },
                { 0, 416051, 416051, 317980 },
                { 0, 214438, 214438, 180655 }
            };

            if (gtIdx == 0)
            {
                goto finish;
            }
            *mbProcessingRatePerSec = BDWULX_MB_RATE[tuIdx][gtIdx];
        }
        else if (MEDIA_IS_SKU(skuTable, FtrULT))
        {
            static const uint32_t BDWULT_MB_RATE[7][4] =
            {
                // GT3   |  GT2   | GT1.5  |  GT1 
                { 1544090, 1544090, 1029393, 676280 },
                { 1462540, 1462540, 975027, 661800 },
                { 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 776921, 640000 },
                { 624076, 624076, 416051, 317980 },
                { 321657, 321657, 214438, 180655 }
            };

            *mbProcessingRatePerSec = BDWULT_MB_RATE[tuIdx][gtIdx];
        }
        else
        {
            // regular BDW
            static const uint32_t BDW_MB_RATE[7][4] =
            {
                // GT3   |   GT2  | GT1.5  |  GT1
                { 1544090, 1544090, 1029393, 676280 },
                { 1462540, 1462540, 975027, 661800 },
                { 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 776921, 640000 },
                { 624076, 624076, 416051, 317980 },
                { 321657, 321657, 214438, 180655 }
            };

            *mbProcessingRatePerSec = BDW_MB_RATE[tuIdx][gtIdx];
        }
    }
    else
        if (GFX_GET_CURRENT_PRODUCT(platform) == IGFX_SKYLAKE)
        {
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
                goto finish;
            }

            if (MEDIA_IS_SKU(skuTable, FtrULX))
            {
                static const uint32_t SKLULX_MB_RATE[7][5] =
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
                    goto finish;
                }
                *mbProcessingRatePerSec = SKLULX_MB_RATE[tuIdx][gtIdx];
            }
            else if (MEDIA_IS_SKU(skuTable, FtrULT))
            {
                static const uint32_t SKLULT_MB_RATE[7][5] =
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

                *mbProcessingRatePerSec = SKLULT_MB_RATE[tuIdx][gtIdx];
            }
            else
            {
                // regular SKL
                static const uint32_t SKL_MB_RATE[7][5] =
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

                *mbProcessingRatePerSec = SKL_MB_RATE[tuIdx][gtIdx];
            }
        }
    else
    if (GFX_GET_CURRENT_PRODUCT(platform) == IGFX_KABYLAKE ||
        GFX_GET_CURRENT_PRODUCT(platform) == IGFX_COFFEELAKE)
    {
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
            goto finish;
        }

        if (codechalMode == CODECHAL_ENCODE_MODE_AVC)
        {
            if (MEDIA_IS_SKU(skuTable, FtrULX))
            {
                static const uint32_t KBLULX_MB_RATE[7][5] =
                {
                    // GT4 | GT3 |  GT2   | GT1.5  |  GT1 
                    { 0, 0, 1029393, 1029393, 676280 },
                    { 0, 0, 975027, 975027, 661800 },
                    { 0, 0, 776921, 776921, 640000 },
                    { 0, 0, 776921, 776921, 640000 },
                    { 0, 0, 776921, 776921, 640000 },
                    { 0, 0, 416051, 416051, 317980 },
                    { 0, 0, 214438, 214438, 180655 }
                };

                if (gtIdx == 0 || gtIdx == 1)
                {
                    goto finish;
                }
                *mbProcessingRatePerSec = KBLULX_MB_RATE[tuIdx][gtIdx];
            }
            else if (MEDIA_IS_SKU(skuTable, FtrULT))
            {
                static const uint32_t KBLULT_MB_RATE[7][5] =
                {
                    // GT4    | GT3   |  GT2   | GT1.5   |  GT1 
                    { 1544090, 1544090, 1544090, 1029393, 676280 },
                    { 1462540, 1462540, 1462540, 975027, 661800 },
                    { 1165381, 1165381, 1165381, 776921, 640000 },
                    { 1165381, 1165381, 1165381, 776921, 640000 },
                    { 1165381, 1165381, 1165381, 776921, 640000 },
                    { 624076, 624076, 624076, 416051, 317980 },
                    { 321657, 321657, 321657, 214438, 180655 }
                };

                *mbProcessingRatePerSec = KBLULT_MB_RATE[tuIdx][gtIdx];
            }
            else
            {
                // regular KBL
                static const uint32_t KBL_MB_RATE[7][5] =
                {
                    // GT4    | GT3   |   GT2  | GT1.5  |  GT1
                    { 1544090, 1544090, 1544090, 1029393, 676280 },
                    { 1462540, 1462540, 1462540, 975027, 661800 },
                    { 1165381, 1165381, 1165381, 776921, 640000 },
                    { 1165381, 1165381, 1165381, 776921, 640000 },
                    { 1165381, 1165381, 1165381, 776921, 640000 },
                    { 624076, 624076, 624076, 416051, 317980 },
                    { 321657, 321657, 321657, 214438, 180655 }
                };

                *mbProcessingRatePerSec = KBL_MB_RATE[tuIdx][gtIdx];
            }
        }
        else if (codechalMode == CODECHAL_ENCODE_MODE_HEVC)
        {
            static const uint32_t KBL_MB_RATE[7][5] =
            {
                // GT4    | GT3   |   GT2  | GT1.5  |  GT1
                { 500000, 500000, 500000, 500000, 500000 },
                { 500000, 500000, 500000, 500000, 500000 },
                { 250000, 250000, 250000, 250000, 250000 },
                { 250000, 250000, 250000, 250000, 250000 },
                { 250000, 250000, 250000, 250000, 250000 },
                { 125000, 125000, 125000, 125000, 125000 },
                { 125000, 125000, 125000, 125000, 125000 }
            };

            *mbProcessingRatePerSec = KBL_MB_RATE[tuIdx][gtIdx];
        }
    }
    else
    if (GFX_GET_CURRENT_PRODUCT(platform) == IGFX_BROXTON ||
        GFX_GET_CURRENT_PRODUCT(platform) == IGFX_GEMINILAKE)
    {
        static const uint32_t BXT_MB_RATE[7] = { 991254, 885321, 839852, 838299, 838471, 704420, 703934 };
        *mbProcessingRatePerSec = BXT_MB_RATE[tuIdx];
    } 
    else
    if (GFX_GET_CURRENT_PRODUCT(platform) == IGFX_CANNONLAKE)
    {
        if (vdencActive)            
        {
	        // For AVC Vdenc
            if (codechalMode == CODECHAL_ENCODE_MODE_AVC)
            {   
                if (MEDIA_IS_SKU(skuTable, FtrULX))
                {
                    static const uint32_t CNLULX_MB_RATE[7] =
                    {
                        // TU7  |  TU6  |   TU5  |  TU4  |  TU3  | TU2  |  TU1
                        1200000, 1200000, 800000, 800000, 800000, 600000, 600000
                    };

                    *mbProcessingRatePerSec = CNLULX_MB_RATE[tuIdx];
                }
                else // same numbers for ULT/ classic
                {
                    static const uint32_t CNL_MB_RATE[7] =
                    {
                        // TU7  |  TU6  |   TU5  |   TU4  |   TU3   |  TU2   |  TU1
                        2200000, 2200000, 1650000, 1650000, 1650000, 1100000, 1100000
                    };

                    *mbProcessingRatePerSec = CNL_MB_RATE[tuIdx];
                }
            }
            else if (codechalMode == CODECHAL_ENCODE_MODE_HEVC)
            {
                if (MEDIA_IS_SKU(skuTable, FtrULX))
                {
                    static const uint32_t CNLULX_MB_RATE[7] =
                    {
                        // TU7  |  TU6  |   TU5  |  TU4  |  TU3  | TU2  |  TU1
                        1200000, 1200000, 600000, 600000, 600000, 300000, 300000
                    };

                    *mbProcessingRatePerSec = CNLULX_MB_RATE[tuIdx];
                }
                else // same numbers for ULT/ classic
                {
                    static const uint32_t CNL_MB_RATE[7] =
                    {
                        // TU7  |  TU6  |   TU5  |   TU4  |   TU3   |  TU2   |  TU1
                        2200000, 2200000, 1200000, 1200000, 1200000, 550000, 550000
                    };

                    *mbProcessingRatePerSec = CNL_MB_RATE[tuIdx];
                }
            }
        }
        else
        {  
            //Dual pipe mode
            if (gtSystemInfo->EUCount == 16)    //FtrGT0_5
            {
                gtIdx = 5;
            }
            else if (gtSystemInfo->EUCount == 24)   //FtrGT1
            {
                gtIdx = 4;
            }
            else if (gtSystemInfo->EUCount == 32)   //FtrGT1_5
            {
                gtIdx = 3;
            }
            else if (gtSystemInfo->EUCount == 40)   //FtrGT2
            {
                gtIdx = 2;
            }
            else if (gtSystemInfo->EUCount == 56)   //FtrGT3
            {
                gtIdx = 1;
            }
            else if (gtSystemInfo->EUCount == 72)   //FtrGT4
            {
                gtIdx = 0;
            }
            else
            {
                goto finish;
            }
            // AVC Dual pipe mode. Data obtained from regular KBL 
            if (codechalMode == CODECHAL_ENCODE_MODE_AVC)
            {
                if (MEDIA_IS_SKU(skuTable, FtrULX))
                {
                    static const uint32_t CNLULX_MB_RATE[7][6] =
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
                        goto finish;
                    }
                    *mbProcessingRatePerSec = CNLULX_MB_RATE[tuIdx][gtIdx];
                }
                else // same numbers for ULT/ classic
                {
                    static const uint32_t CNL_MB_RATE[7][6] =
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
                    *mbProcessingRatePerSec = CNL_MB_RATE[tuIdx][gtIdx];
                }
            }
            else if (codechalMode == CODECHAL_ENCODE_MODE_HEVC)
            {
                // HEVC dual pipe values temporarily set to 50000 for ULT/ULX all TUs and GT
                static const uint32_t CNL_MB_RATE[7][6] =
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
                *mbProcessingRatePerSec = CNL_MB_RATE[tuIdx][gtIdx];
            }
            else
            {
                goto finish;
            }
        }
    }
    else //default case
    {
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
            goto finish;
        }

        if (MEDIA_IS_SKU(skuTable, FtrULX))
        {
            static const uint32_t DEFAULTULX_MB_RATE[7][5] =
            {
                // GT4 | GT3 |  GT2   | GT1.5  |  GT1 
                { 0, 0, 1029393, 1029393, 676280 },
                { 0, 0, 975027, 975027, 661800 },
                { 0, 0, 776921, 776921, 640000 },
                { 0, 0, 776921, 776921, 640000 },
                { 0, 0, 776921, 776921, 640000 },
                { 0, 0, 416051, 416051, 317980 },
                { 0, 0, 214438, 214438, 180655 }
            };

            if (gtIdx == 0 || gtIdx == 1)
            {
                goto finish;
            }
            *mbProcessingRatePerSec = DEFAULTULX_MB_RATE[tuIdx][gtIdx];
        }
        else if (MEDIA_IS_SKU(skuTable, FtrULT))
        {
            static const uint32_t DEFAULTULT_MB_RATE[7][5] =
            {
                // GT4    | GT3   |  GT2   | GT1.5   |  GT1 
                { 1544090, 1544090, 1544090, 1029393, 676280 },
                { 1462540, 1462540, 1462540, 975027, 661800 },
                { 1165381, 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 1165381, 776921, 640000 },
                { 624076, 624076, 624076, 416051, 317980 },
                { 321657, 321657, 321657, 214438, 180655 }
            };

            *mbProcessingRatePerSec = DEFAULTULT_MB_RATE[tuIdx][gtIdx];
        }
        else
        {
            // regular
            static const uint32_t DEFAULT_MB_RATE[7][5] =
            {
                // GT4    | GT3   |   GT2  | GT1.5  |  GT1
                { 1544090, 1544090, 1544090, 1029393, 676280 },
                { 1462540, 1462540, 1462540, 975027, 661800 },
                { 1165381, 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 1165381, 776921, 640000 },
                { 1165381, 1165381, 1165381, 776921, 640000 },
                { 624076, 624076, 624076, 416051, 317980 },
                { 321657, 321657, 321657, 214438, 180655 }
            };

            *mbProcessingRatePerSec = DEFAULT_MB_RATE[tuIdx][gtIdx];
        }
    }

    res = true;

finish:
    return res;
}

bool GetMbProcessingRateDec(
	PLATFORM                            platform,
	MEDIA_FEATURE_TABLE                 *skuTable,
	uint32_t*                           mbProcessingRatePerSec
	)
{
	uint32_t	idx = 0;
	MOS_STATUS	eStatus = MOS_STATUS_SUCCESS;
    MOS_UNUSED(platform);

	MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, skuTable);
	MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, mbProcessingRatePerSec);

	static const uint32_t MB_RATE[2] =
	{
		// non-ULX, ULX/Atom
		4800000, 3600000
	};

	if (MEDIA_IS_SKU(skuTable, FtrLCIA) || //Atom
		MEDIA_IS_SKU(skuTable, FtrULX)) // ULX
	{
		idx = 1;
	}
	else
	{
		// Default is non-ULX
		idx = 0;
	}

	*mbProcessingRatePerSec = MB_RATE[idx];

finish:
	return (eStatus == MOS_STATUS_SUCCESS);
}
