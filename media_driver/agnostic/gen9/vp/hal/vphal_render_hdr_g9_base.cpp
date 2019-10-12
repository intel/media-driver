/*
* Copyright (c) 2010-2019, Intel Corporation
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
//! \file      vphal_render_hdr_g9.cpp
//! \brief         Rendering definitions for Unified VP HAL HDR processing
//!

#include "vphal_render_hdr_base.h"
#include "vphal_render_hdr_g9_base.h"
#include "vphal_render_composite.h"
#include "renderhal_platform_interface.h"

#include "vphal_renderer.h"
#include "vphal_debug.h"
#include <fstream>
#include <string>

#include "hal_oca_interface.h"
#include "vphal_render_ief.h"

enum HDR_TMMODE {
    PREPROCESS_TM_S2H,
    PREPROCESS_TM_H2S,
    PREPROCESS_TM_H2S_AUTO,
    PREPROCESS_TM_H2H,
    PREPROCESS_TM_MAX
};

MEDIA_WALKER_HDR_STATIC_DATA_G9 g_cInit_MEDIA_STATIC_HDR_g9 =
{
    // DWORD 0
    {
        { 0.0 }
    },

    // DWORD 1
    {
        { 0.0 }
    },

    // DWORD 2
    {
        { 0.0 }
    },

    // DWORD 3
    {
        { 0.0 }
    },

    // DWORD 4
    {
        { 0.0 }
    },

    // DWORD 5
    {
        { 0.0 }
    },

    // DWORD 6
    {
        { 0.0 }
    },

    // DWORD 7
    {
        { 0.0 }
    },

    // DWORD 8
    {
        { 0.0 }
    },

    // DWORD 9
    {
        { 0.0 }
    },

    // DWORD 10
    {
        { 0.0 }
    },

    // DWORD 11
    {
        { 0.0 }
    },

    // DWORD 12
    {
        { 0.0 }
    },

    // DWORD 13
    {
        { 0.0 }
    },

    // DWORD 14
    {
        { 0.0 }
    },

    // DWORD 15
    {
        { 0.0 }
    },

    // DWORD 16
    {
        { 0.0 }
    },

    // DWORD 17
    {
        { 0.0 }
    },

    // DWORD 18
    {
        { 0.0 }
    },

    // DWORD 19
    {
        { 0.0 }
    },

    // DWORD 20
    {
        { 0.0 }
    },

    // DWORD 21
    {
        { 0.0 }
    },

    // DWORD 22
    {
        { 0.0 }
    },

    // DWORD 23
    {
        { 0.0 }
    },

    // DWORD 24
    {
        { 0.0 }
    },

    // DWORD 25
    {
        { 0.0 }
    },

    // DWORD 26
    {
        { 0.0 }
    },

    // DWORD 27
    {
        { 0.0 }
    },

    // DWORD 28
    {
        { 0.0 }
    },

    // DWORD 29
    {
        { 0.0 }
    },

    // DWORD 30
    {
        { 0.0 }
    },

    // DWORD 31
    {
        { 0.0 }
    },

    // DWORD 32
    {
        { 0, 0 }
    },

    // DWORD 33
    {
        { 0, 0 }
    },

    // DWORD 34
    {
        { 0, 0 }
    },

    // DWORD 35
    {
        { 0, 0 }
    },

    // DWORD 36
    {
        { 0, 0 }
    },

    // DWORD 37
    {
        { 0, 0 }
    },

    // DWORD 38
    {
        { 0, 0 }
    },

    // DWORD 39
    {
        { 0, 0 }
    },

    // DWORD 40
    {
        { 0, 0 }
    },

    // DWORD 41
    {
        { 0, 0 }
    },

    // DWORD 42
    {
        { 0, 0 }
    },

    // DWORD 43
    {
        { 0, 0 }
    },

    // DWORD 44
    {
        { 0, 0 }
    },

    // DWORD 45
    {
        { 0, 0 }
    },

    // DWORD 46
    {
        { 0, 0 }
    },

    // DWORD 47
    {
        { 0, 0 }
    },

    // DWORD 48
    {
        {
            0,      // FormatDescriptorLayer0
            0,      // ChromaSittingLocationLayer0
            0,      // ChannelSwapEnablingFlagLayer0
            0,      // IEFBypassEnablingFlagLayer0
            0,      // RotationAngleMirrorDirectionLayer0
            0,      // SamplerIndexFirstPlaneLayer0
            0,      // SamplerIndexSecondThirdPlaneLayer0
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer0
            0,      // EOTF1DLUTEnablingFlagLayer0
            0,      // CCMEnablingFlagLayer0
            0,      // OETF1DLUTEnablingFlagLayer0
            0,      // PostCSCEnablingFlagLayer0
            0       // Enabling3DLUTFlagLayer0
        }
    },

    // DWORD 49
    {
        {
            0,      // FormatDescriptorLayer1
            0,      // ChromaSittingLocationLayer1
            0,      // ChannelSwapEnablingFlagLayer1
            0,      // IEFBypassEnablingFlagLayer1
            0,      // RotationAngleMirrorDirectionLayer1
            0,      // SamplerIndexFirstPlaneLayer1
            0,      // SamplerIndexSecondThirdPlaneLayer1
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer1
            0,      // EOTF1DLUTEnablingFlagLayer1
            0,      // CCMEnablingFlagLayer1
            0,      // OETF1DLUTEnablingFlagLayer1
            0,      // PostCSCEnablingFlagLayer1
            0       // Enabling3DLUTFlagLayer1
        }
    },

    // DWORD 50
    {
        {
            0,      // FormatDescriptorLayer2
            0,      // ChromaSittingLocationLayer2
            0,      // ChannelSwapEnablingFlagLayer2
            0,      // IEFBypassEnablingFlagLayer2
            0,      // RotationAngleMirrorDirectionLayer2
            0,      // SamplerIndexFirstPlaneLayer2
            0,      // SamplerIndexSecondThirdPlaneLayer2
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer2
            0,      // EOTF1DLUTEnablingFlagLayer2
            0,      // CCMEnablingFlagLayer2
            0,      // OETF1DLUTEnablingFlagLayer2
            0,      // PostCSCEnablingFlagLayer2
            0       // Enabling3DLUTFlagLayer2
        }
    },

    // DWORD 51
    {
        {
            0,      // FormatDescriptorLayer3
            0,      // ChromaSittingLocationLayer3
            0,      // ChannelSwapEnablingFlagLayer3
            0,      // IEFBypassEnablingFlagLayer3
            0,      // RotationAngleMirrorDirectionLayer3
            0,      // SamplerIndexFirstPlaneLayer3
            0,      // SamplerIndexSecondThirdPlaneLayer3
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer3
            0,      // EOTF1DLUTEnablingFlagLayer3
            0,      // CCMEnablingFlagLayer3
            0,      // OETF1DLUTEnablingFlagLayer3
            0,      // PostCSCEnablingFlagLayer3
            0       // Enabling3DLUTFlagLayer3
        }
    },

    // DWORD 52
    {
        {
            0,      // FormatDescriptorLayer4
            0,      // ChromaSittingLocationLayer4
            0,      // ChannelSwapEnablingFlagLayer4
            0,      // IEFBypassEnablingFlagLayer4
            0,      // RotationAngleMirrorDirectionLayer4
            0,      // SamplerIndexFirstPlaneLayer4
            0,      // SamplerIndexSecondThirdPlaneLayer4
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer4
            0,      // EOTF1DLUTEnablingFlagLayer4
            0,      // CCMEnablingFlagLayer4
            0,      // OETF1DLUTEnablingFlagLayer4
            0,      // PostCSCEnablingFlagLayer4
            0       // Enabling3DLUTFlagLayer4
        }
    },

    // DWORD 53
    {
        {
            0,      // FormatDescriptorLayer5
            0,      // ChromaSittingLocationLayer5
            0,      // ChannelSwapEnablingFlagLayer5
            0,      // IEFBypassEnablingFlagLayer5
            0,      // RotationAngleMirrorDirectionLayer5
            0,      // SamplerIndexFirstPlaneLayer5
            0,      // SamplerIndexSecondThirdPlaneLayer5
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer5
            0,      // EOTF1DLUTEnablingFlagLayer5
            0,      // CCMEnablingFlagLayer5
            0,      // OETF1DLUTEnablingFlagLayer5
            0,      // PostCSCEnablingFlagLayer5
            0       // Enabling3DLUTFlagLayer5
        }
    },

    // DWORD 54
    {
        {
            0,      // FormatDescriptorLayer6
            0,      // ChromaSittingLocationLayer6
            0,      // ChannelSwapEnablingFlagLayer6
            0,      // IEFBypassEnablingFlagLayer6
            0,      // RotationAngleMirrorDirectionLayer6
            0,      // SamplerIndexFirstPlaneLayer6
            0,      // SamplerIndexSecondThirdPlaneLayer6
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer6
            0,      // EOTF1DLUTEnablingFlagLayer6
            0,      // CCMEnablingFlagLayer6
            0,      // OETF1DLUTEnablingFlagLayer6
            0,      // PostCSCEnablingFlagLayer6
            0       // Enabling3DLUTFlagLayer6
        }
    },

    // DWORD 55
    {
        {
            0,      // FormatDescriptorLayer7
            0,      // ChromaSittingLocationLayer7
            0,      // ChannelSwapEnablingFlagLayer7
            0,      // IEFBypassEnablingFlagLayer7
            0,      // RotationAngleMirrorDirectionLayer7
            0,      // SamplerIndexFirstPlaneLayer7
            0,      // SamplerIndexSecondThirdPlaneLayer7
            0,      // Reserved
            0,      // PriorCSCEnablingFlagLayer7
            0,      // EOTF1DLUTEnablingFlagLayer7
            0,      // CCMEnablingFlagLayer7
            0,      // OETF1DLUTEnablingFlagLayer7
            0,      // PostCSCEnablingFlagLayer7
            0       // Enabling3DLUTFlagLayer7
        }
    },

    // DWORD 56
    {
        { 0, 0, 0, 0 }
    },

    // DWORD 57
    {
        { 0, 0, 0, 0 }
    },

    // DWORD 58
    {
        {
            0,      // TwoLayerOperationLayer0
            0,      // TwoLayerOperationLayer1
            0,      // TwoLayerOperationLayer2
            0       // TwoLayerOperationLayer3
        }
    },

    // DWORD 59
    {
        {
            0,      // TwoLayerOperationLayer4
            0,      // TwoLayerOperationLayer5
            0,      // TwoLayerOperationLayer6
            0       // TwoLayerOperationLayer7
        }
    },

    // DWORD 60
    {
        { 0, 0 }
    },

    // DWORD 61
    {
        { 0, 0 }
    },

    // DWORD 62
    {
        { 0, 0 }
    },

    // DWORD 63
    {
        { 0, 0, 0, 0, 0, 0, 0 }
    },
};

const uint32_t g_Hdr_ColorCorrect_OETF_sRGB_FP16_g9[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER] =
{
    0x2ab90000, 0x2f5b2daa, 0x30f7305e, 0x31fb317f, 0x32d5326c, 0x33953338, 0x342133ed, 0x346f3449, 0x3494346f, 0x34db34b8, 0x351d34fc, 0x355b353c, 0x35963579, 0x35ce35b2, 0x360435e9, 0x3637361e,
    0x36503637, 0x36813669, 0x36b03698, 0x36dd36c6, 0x370936f3, 0x3734371e, 0x375d3749, 0x37853771, 0x37993785, 0x37c037ad, 0x37e637d3, 0x380537f9, 0x3817380e, 0x38293820, 0x383a3832, 0x384b3843,
    0x3854384b, 0x3864385c, 0x3875386d, 0x3885387d, 0x3894388c, 0x38a4389c, 0x38b338ab, 0x38c238ba, 0x38c938c2, 0x38d838d0, 0x38e638df, 0x38f438ed, 0x390238fb, 0x39103909, 0x391d3917, 0x392b3924,
    0x3931392b, 0x393e3938, 0x394b3945, 0x39583952, 0x3965395f, 0x3971396b, 0x397e3978, 0x398a3984, 0x3990398a, 0x399c3996, 0x39a839a2, 0x39b439ae, 0x39bf39b9, 0x39cb39c5, 0x39d639d1, 0x39e239dc,
    0x39e739e2, 0x39f239ed, 0x39fd39f8, 0x3a083a03, 0x3a133a0e, 0x3a1e3a18, 0x3a283a23, 0x3a333a2e, 0x3a383a33, 0x3a433a3d, 0x3a4d3a48, 0x3a573a52, 0x3a613a5c, 0x3a6b3a66, 0x3a753a70, 0x3a7f3a7a,
    0x3a843a7f, 0x3a8e3a89, 0x3a983a93, 0x3aa13a9c, 0x3aab3aa6, 0x3ab43ab0, 0x3abe3ab9, 0x3ac73ac3, 0x3acc3ac7, 0x3ad53ad0, 0x3ade3ada, 0x3ae73ae3, 0x3af13aec, 0x3afa3af5, 0x3b033afe, 0x3b0b3b07,
    0x3b103b0b, 0x3b193b14, 0x3b213b1d, 0x3b2a3b26, 0x3b333b2e, 0x3b3b3b37, 0x3b443b40, 0x3b4c3b48, 0x3b513b4c, 0x3b593b55, 0x3b613b5d, 0x3b6a3b66, 0x3b723b6e, 0x3b7a3b76, 0x3b823b7e, 0x3b8b3b87,
    0x3b8f3b8b, 0x3b973b93, 0x3b9f3b9b, 0x3ba73ba3, 0x3baf3bab, 0x3bb73bb3, 0x3bbe3bba, 0x3bc63bc2, 0x3bca3bc6, 0x3bd23bce, 0x3bda3bd6, 0x3be13bdd, 0x3be93be5, 0x3bf03bed, 0x3bf83bf4, 0x3c003bfc
};

static const uint32_t g_Hdr_ColorCorrect_OETF_BT709_FP16_g9[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER] =
{
    0x24cc0000, 0x2b3328cc, 0x2dfb2ccc, 0x2fff2f09, 0x30db3071, 0x319d313f, 0x324d31f7, 0x32ee329f, 0x333a32ee, 0x33cb3384, 0x34293408, 0x346a344a, 0x34a73489, 0x34e234c5, 0x351a34fe, 0x35503535,
    0x356a3550, 0x359d3584, 0x35cf35b6, 0x35ff35e7, 0x362d3616, 0x365a3644, 0x36863671, 0x36b1369c, 0x36c636b1, 0x36f036db, 0x37183704, 0x3740372c, 0x37673753, 0x378d377a, 0x37b2379f, 0x37d637c4,
    0x37e837d6, 0x380637fa, 0x3817380e, 0x38283820, 0x38393831, 0x384a3842, 0x385a3852, 0x386a3862, 0x3872386a, 0x3882387a, 0x3892388a, 0x38a13899, 0x38b038a9, 0x38bf38b8, 0x38ce38c6, 0x38dc38d5,
    0x38e438dc, 0x38f238eb, 0x390038f9, 0x390e3907, 0x391c3915, 0x39293923, 0x39373930, 0x3944393e, 0x394b3944, 0x39583952, 0x3965395f, 0x3972396c, 0x397f3978, 0x398b3985, 0x39983992, 0x39a4399e,
    0x39ab39a4, 0x39b739b1, 0x39c339bd, 0x39cf39c9, 0x39db39d5, 0x39e739e1, 0x39f239ed, 0x39fe39f8, 0x3a0439fe, 0x3a0f3a0a, 0x3a1b3a15, 0x3a263a20, 0x3a313a2c, 0x3a3c3a37, 0x3a473a42, 0x3a523a4d,
    0x3a583a52, 0x3a633a5d, 0x3a6e3a68, 0x3a783a73, 0x3a833a7e, 0x3a8d3a88, 0x3a983a93, 0x3aa23a9d, 0x3aa73aa2, 0x3ab23aad, 0x3abc3ab7, 0x3ac63ac1, 0x3ad03acb, 0x3ada3ad5, 0x3ae43adf, 0x3aee3ae9,
    0x3af33aee, 0x3afd3af8, 0x3b073b02, 0x3b103b0c, 0x3b1a3b15, 0x3b243b1f, 0x3b2d3b28, 0x3b373b32, 0x3b3b3b37, 0x3b453b40, 0x3b4e3b4a, 0x3b583b53, 0x3b613b5c, 0x3b6a3b65, 0x3b733b6f, 0x3b7c3b78,
    0x3b813b7c, 0x3b8a3b85, 0x3b933b8e, 0x3b9c3b97, 0x3ba53ba0, 0x3bad3ba9, 0x3bb63bb2, 0x3bbf3bbb, 0x3bc33bbf, 0x3bcc3bc8, 0x3bd53bd0, 0x3bdd3bd9, 0x3be63be2, 0x3bef3bea, 0x3bf73bf3, 0x3c003bfb
};

// So far assume max luminance to be 2000 nit, may change in the future.
static const uint32_t g_Hdr_ColorCorrect_OETF_SMPTE_ST2084_3Segs_FP16_g9[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER] =
{
    0x2832000c, 0x2ae829c6, 0x2c4a2bd0, 0x2ced2ca0, 0x2d722d33, 0x2de42dad, 0x2e492e18, 0x2ea22e77, 0x2ecc2ea2, 0x2f1a2ef4, 0x2f622f3f, 0x2fa52f84, 0x2fe32fc5, 0x300f3000, 0x302b301d, 0x30453038,
    0x30523045, 0x306a305e, 0x30813076, 0x3098308d, 0x30ad30a3, 0x30c230b8, 0x31a730cc, 0x32c63247, 0x333132c6, 0x33dd338c, 0x34333413, 0x346d3451, 0x349f3487, 0x34cb34b6, 0x34f234df, 0x35163505,
    0x35273516, 0x35463537, 0x35633555, 0x357e3571, 0x3598358b, 0x35af35a4, 0x35c635bb, 0x35db35d1, 0x35e535db, 0x35f935ef, 0x360c3603, 0x361e3615, 0x362f3627, 0x36403638, 0x36503648, 0x365f3657,
    0x3666365f, 0x3675366e, 0x3683367c, 0x3691368a, 0x369e3697, 0x36ab36a4, 0x36b736b1, 0x36c336bd, 0x36c936c3, 0x36d436cf, 0x36e036da, 0x36eb36e5, 0x36f536f0, 0x370036fa, 0x370a3705, 0x3714370f,
    0x37193714, 0x3722371d, 0x372c3727, 0x379b3730, 0x381c37f1, 0x3856383b, 0x3884386f, 0x38ab3898, 0x38bb38ab, 0x38da38cb, 0x38f538e8, 0x390d3901, 0x39223918, 0x3936392c, 0x3948393f, 0x39593951,
    0x39613959, 0x39703969, 0x397e3977, 0x398b3985, 0x39983992, 0x39a4399e, 0x39af39aa, 0x39ba39b5, 0x39bf39ba, 0x39c939c4, 0x39d339ce, 0x39dc39d8, 0x39e539e1, 0x39ee39e9, 0x39f639f2, 0x39fe39fa,
    0x3a0239fe, 0x3a093a06, 0x3a113a0d, 0x3a183a14, 0x3a1f3a1b, 0x3a253a22, 0x3a2c3a28, 0x3a323a2f, 0x3a353a32, 0x3a3b3a38, 0x3a413a3e, 0x3a473a44, 0x3a4c3a49, 0x3a523a4f, 0x3a573a54, 0x3a5c3a59,
    0x3a5f3a5c, 0x3a643a61, 0x3a693a66, 0x3a6d3a6b, 0x3a723a70, 0x3a773a74, 0x3a7b3a79, 0x3a803a7d, 0x3a823a80, 0x3a863a84, 0x3a8a3a88, 0x3a8e3a8c, 0x3a923a90, 0x3a963a94, 0x3a9a3a98, 0x3a9e3a9c
};

inline void DumpDataToFile(const std::string &fileName, const void *data, size_t size)
{
    std::ofstream blob(fileName, std::ofstream::out | std::ifstream::binary);

    if (!blob.is_open())
    {
        VPHAL_RENDER_ASSERTMESSAGE("Error in opening raw data file: %s\n", fileName.c_str());
        return;
    }

    blob.write((const char *)data, size);
}

CSC_COEFF_FORMAT Convert_CSC_Coeff_To_Register_Format(double coeff)
{
    CSC_COEFF_FORMAT outVal = { 0 };
    uint32_t shift_factor = 0;

    if (coeff < 0)
    {
        outVal.sign = 1;
        coeff = -coeff;
    }

    // range check
    if (coeff > MAX_CSC_COEFF_VAL_ICL)
        coeff = MAX_CSC_COEFF_VAL_ICL;

    if (coeff < 0.125)                       //0.000bbbbbbbbb
    {
        outVal.exponent = 3;
        shift_factor = 12;
    }
    else if (coeff >= 0.125 && coeff < 0.25) //0.00bbbbbbbbb
    {
        outVal.exponent = 2;
        shift_factor = 11;
    }
    else if (coeff >= 0.25 && coeff < 0.5)  //0.0bbbbbbbbb
    {
        outVal.exponent = 1;
        shift_factor = 10;
    }
    else if (coeff >= 0.5 && coeff < 1.0)   // 0.bbbbbbbbb
    {
        outVal.exponent = 0;
        shift_factor = 9;
    }
    else if (coeff >= 1.0 && coeff < 2.0)    //b.bbbbbbbb
    {
        outVal.exponent = 7;
        shift_factor = 8;
    }
    else if (coeff >= 2.0)  // bb.bbbbbbb
    {
        outVal.exponent = 6;
        shift_factor = 7;
    }

    //Convert float to integer
    outVal.mantissa = static_cast<uint32_t>(round(coeff * (double)(1 << (int)shift_factor)));

    return outVal;
}

double Convert_CSC_Coeff_Register_Format_To_Double(CSC_COEFF_FORMAT regVal)
{
    double outVal = 0;

    switch (regVal.exponent)
    {
      case 0: outVal = (double)regVal.mantissa / 512.0; break;
      case 1: outVal = (double)regVal.mantissa / 1024.0; break;
      case 2: outVal = (double)regVal.mantissa / 2048.0; break;
      case 3: outVal = (double)regVal.mantissa / 4096.0; break;
      case 6: outVal = (double)regVal.mantissa / 128.0; break;
      case 7: outVal = (double)regVal.mantissa / 256.0; break;
    }

  if (regVal.sign)
  {
     outVal = -outVal;
  }

  return outVal;
}

float LimitFP32PrecisionToF3_9(float fp)
{
    double dbInput = static_cast<double>(fp);
    double dbOutput = Convert_CSC_Coeff_Register_Format_To_Double(Convert_CSC_Coeff_To_Register_Format(dbInput));
    return static_cast<float>(dbOutput);
}

void LimitFP32ArrayPrecisionToF3_9(float fps[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        fps[i] = LimitFP32PrecisionToF3_9(fps[i]);
    }
}

//! \brief    Get the HDR format descriptor of a format
//! \details  Get the HDR format descriptor of a format and return.
//! \param    MOS_FORMAT Format
//!           [in] MOS_FORMAT of a surface
//! \return   VPHAL_HDR_FORMAT_DESCRIPTOR_G9
//!           HDR format descriptor
//!
VPHAL_HDR_FORMAT_DESCRIPTOR_G9 VpHal_HdrGetFormatDescriptor_g9 (
    MOS_FORMAT      Format)
{
    VPHAL_HDR_FORMAT_DESCRIPTOR_G9 FormatDescriptor  = VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW_G9;

    switch (Format)
    {
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_R10G10B10A2_UNORM_G9;
            break;

        case Format_X8R8G8B8:
        case Format_A8R8G8B8:
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
        case Format_AYUV:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_R8G8B8A8_UNORM_G9;
            break;

        case Format_NV12:
        case Format_NV21:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_NV12_G9;
            break;

        case Format_YUY2:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_YUY2_G9;
            break;

        case Format_P010:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_P010_G9;
            break;

        case Format_P016:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_P016_G9;
            break;

        case Format_A16R16G16B16F:
        case Format_A16B16G16R16F:
            FormatDescriptor = VPHAL_HDR_FORMAT_R16G16B16A16_FLOAT_G9;
            break;

        default:
            VPHAL_PUBLIC_ASSERTMESSAGE("Unsupported input format.");
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW_G9;
            break;
    }

    return FormatDescriptor;
}
//! \brief    Get the HDR Chroma siting
//! \details  Get the HDR Chroma siting and return.
//! \param    uint32_t ChromaSiting
//!           [in] ChromaSiting of a surface
//! \return   VPHAL_HDR_CHROMA_SITING_G9
//!           HDR Chroma siting
//!
VPHAL_HDR_CHROMA_SITING_G9 VpHal_HdrGetHdrChromaSiting_g9 (
    uint32_t      ChromaSiting)
{
    VPHAL_HDR_CHROMA_SITING_G9 HdrChromaSiting = VPHAL_HDR_CHROMA_SITTING_A_G9;

    switch (ChromaSiting)
    {
    case CHROMA_SITING_HORZ_LEFT :
        HdrChromaSiting = VPHAL_HDR_CHROMA_SITTING_A_G9;
        break;
    default:
        HdrChromaSiting = VPHAL_HDR_CHROMA_SITTING_A_G9;
        break;
    }

    return HdrChromaSiting;
}

//! \brief    Get the HDR rotation
//! \details  Get the HDR rotation and return.
//! \param    VPHAL_ROTATION Rotation
//!           [in] Rotation of a surface
//! \return   VPHAL_HDR_ROTATION_G9
//!           HDR Chroma siting
//!
VPHAL_HDR_ROTATION_G9 VpHal_HdrGetHdrRotation_g9 (
    VPHAL_ROTATION      Rotation)
{
    VPHAL_HDR_ROTATION_G9 HdrRotation  = VPHAL_HDR_LAYER_ROTATION_0_G9;

    switch (Rotation)
    {
    case VPHAL_ROTATION_IDENTITY :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_0_G9;
        break;
    case VPHAL_ROTATION_90 :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_90_G9;
        break;
    case VPHAL_ROTATION_180 :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_180_G9;
        break;
    case VPHAL_ROTATION_270 :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_270_G9;
        break;
    case VPHAL_MIRROR_HORIZONTAL :
        HdrRotation = VPHAL_HDR_LAYER_MIRROR_H_G9;
        break;
    case VPHAL_MIRROR_VERTICAL :
        HdrRotation = VPHAL_HDR_LAYER_MIRROR_V_G9;
        break;
    case VPHAL_ROTATE_90_MIRROR_VERTICAL :
        HdrRotation = VPHAL_HDR_LAYER_ROT_90_MIR_V_G9;
        break;
    case VPHAL_ROTATE_90_MIRROR_HORIZONTAL :
        HdrRotation = VPHAL_HDR_LAYER_ROT_90_MIR_H_G9;
        break;
    default:
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_0_G9;
        break;
    }

    return HdrRotation;
}

//! \brief    Get the HDR Two Layer Option
//! \details  Get the HDR Two Layer Option and return.
//! \param    VPHAL_BLEND_TYPE BlendType
//!           [in] Blending type of a surface
//! \return   VPHAL_HDR_TWO_LAYER_OPTION_G9
//!           HDR Two Layer Option
//!
VPHAL_HDR_TWO_LAYER_OPTION_G9 VpHal_HdrGetHdrTwoLayerOption_g9(
    VPHAL_BLEND_TYPE BlendType)
{
    VPHAL_HDR_TWO_LAYER_OPTION_G9 HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND_G9;

    switch (BlendType)
    {
    case BLEND_NONE:
        HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_COMP_G9;
        break;
    case BLEND_SOURCE:
        HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND_G9;
        break;
    case BLEND_PARTIAL:
        HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_PBLEND_G9;
        break;
    case BLEND_CONSTANT:
        HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9;
        break;
    case BLEND_CONSTANT_SOURCE:
        HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9;
        break;
    case BLEND_CONSTANT_PARTIAL:
        HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9;
        break;
    default:
        HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND_G9;
        break;
    }

    return HdrTwoLayerOp;
}

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pSrcSurface
//!           [in] Pointer to source surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsInputFormatSupported_g9(
    PVPHAL_SURFACE              pSrcSurface,
    bool*                       pbSupported)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
 
    VPHAL_PUBLIC_CHK_NULL(pSrcSurface);
    VPHAL_PUBLIC_CHK_NULL(pbSupported);

    // HDR supported formats
    if (pSrcSurface->Format == Format_A8R8G8B8     ||
        pSrcSurface->Format == Format_X8R8G8B8     ||
        pSrcSurface->Format == Format_A8B8G8R8     ||
        pSrcSurface->Format == Format_X8B8G8R8     ||
        pSrcSurface->Format == Format_R10G10B10A2  ||
        pSrcSurface->Format == Format_B10G10R10A2  ||
        pSrcSurface->Format == Format_A16B16G16R16 ||
        pSrcSurface->Format == Format_A16R16G16B16 ||
        pSrcSurface->Format == Format_P016         ||
        pSrcSurface->Format == Format_NV12         ||
        pSrcSurface->Format == Format_P010         ||
        pSrcSurface->Format == Format_YUY2         ||
        pSrcSurface->Format == Format_AYUV)
    {
       *pbSupported = true;
        goto finish;
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE(
            "HDR Unsupported Source Format: '0x%08x'\n",
            pSrcSurface->Format);
        *pbSupported = false;
    }

finish:
    return eStatus;
}

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pTargetSurface
//!           [in] Pointer to target surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsOutputFormatSupported_g9(
    PVPHAL_SURFACE              pTargetSurface,
    bool*                       pbSupported)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
 
    VPHAL_PUBLIC_CHK_NULL(pTargetSurface);
    VPHAL_PUBLIC_CHK_NULL(pbSupported);

    // HDR supported formats
    if (pTargetSurface->Format == Format_A8R8G8B8     ||
        pTargetSurface->Format == Format_X8R8G8B8     ||
        pTargetSurface->Format == Format_A8B8G8R8     ||
        pTargetSurface->Format == Format_X8B8G8R8     ||
        pTargetSurface->Format == Format_R10G10B10A2  ||
        pTargetSurface->Format == Format_B10G10R10A2  ||
        pTargetSurface->Format == Format_A16B16G16R16 ||
        pTargetSurface->Format == Format_A16R16G16B16 ||
        pTargetSurface->Format == Format_YUY2         ||
        pTargetSurface->Format == Format_P016         ||
        pTargetSurface->Format == Format_NV12         ||
        pTargetSurface->Format == Format_P010         ||
        pTargetSurface->Format == Format_P016         ||
        pTargetSurface->Format == Format_A16R16G16B16F||
        pTargetSurface->Format == Format_A16B16G16R16F)
    {
       *pbSupported = true;
        goto finish;
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE(
            "HDR Unsupported Target Format: '0x%08x'\n", 
            pTargetSurface->Format);
        *pbSupported = false;
    }

finish:
    return eStatus;
}

//!
//! \brief    Recalculate Sampler Avs 8x8 Horizontal/Vertical scaling table
//! \details  Recalculate Sampler Avs 8x8 Horizontal/Vertical scaling table
//! \param    MOS_FORMAT SrcFormat
//!           [in] Source Format
//! \param    float fScale
//!           [in] Horizontal or Vertical Scale Factor
//! \param    bool bVertical
//!           [in] true if Vertical Scaling, else Horizontal Scaling
//! \param    uint32_t dwChromaSiting
//!           [in] Chroma Siting
//! \param    bool bBalancedFilter
//!           [in] true if Gen9+, balanced filter
//! \param    bool b8TapAdaptiveEnable
//!           [in] true if 8Tap Adaptive Enable
//! \param    PVPHAL_AVS_PARAMS pAvsParams
//!           [in/out] Pointer to AVS Params
//! \return   MOS_STATUS
//!
static MOS_STATUS VpHal_HdrSamplerAvsCalcScalingTable_g9(
    MOS_FORMAT                      SrcFormat,
    float                           fScale,
    bool                            bVertical,
    uint32_t                        dwChromaSiting,
    bool                            bBalancedFilter,
    bool                            b8TapAdaptiveEnable,
    PMHW_AVS_PARAMS                 pAvsParams)
{
    MOS_STATUS                      eStatus             = MOS_STATUS_SUCCESS;
    MHW_PLANE                       Plane               = MHW_GENERIC_PLANE;
    int32_t                         iUvPhaseOffset      = 0;
    uint32_t                        dwHwPhrase          = 0;
    uint32_t                        YCoefTableSize      = 0;
    uint32_t                        UVCoefTableSize     = 0;
    float                           fScaleParam         = 0.0f;
    int32_t*                        piYCoefsParam       = nullptr;
    int32_t*                        piUVCoefsParam      = nullptr;
    float                           fHPStrength         = 0.0f;

    VPHAL_RENDER_CHK_NULL(pAvsParams);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsX);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsX);

    if (bBalancedFilter)
    {
        YCoefTableSize      = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        UVCoefTableSize     = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;
        dwHwPhrase          = NUM_HW_POLYPHASE_TABLES_G9;
    }
    else
    {
        YCoefTableSize      = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G8;
        UVCoefTableSize     = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G8;
        dwHwPhrase          = MHW_NUM_HW_POLYPHASE_TABLES;
    }

    fHPStrength = 0.0F;
    piYCoefsParam   = bVertical ? pAvsParams->piYCoefsY : pAvsParams->piYCoefsX;
    piUVCoefsParam  = bVertical ? pAvsParams->piUVCoefsY : pAvsParams->piUVCoefsX;
    fScaleParam     = bVertical ? pAvsParams->fScaleY : pAvsParams->fScaleX;

    // Recalculate Horizontal or Vertical scaling table
    if (SrcFormat != pAvsParams->Format || fScale != fScaleParam)
    {
        MOS_ZeroMemory(piYCoefsParam, YCoefTableSize);
        MOS_ZeroMemory(piUVCoefsParam, UVCoefTableSize);

        // 4-tap filtering for RGB format G-channel if 8tap adaptive filter is not enabled.
        Plane = (IS_RGB32_FORMAT(SrcFormat) && !b8TapAdaptiveEnable) ? MHW_U_PLANE : MHW_Y_PLANE;
        if (bVertical)
        {
            pAvsParams->fScaleY = fScale;
        }
        else
        {
            pAvsParams->fScaleX = fScale;
        }

        // For 1x scaling in horizontal direction, use special coefficients for filtering
        // we don't do this when bForcePolyPhaseCoefs flag is set
        if (fScale == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
        {
            VPHAL_RENDER_CHK_STATUS(Mhw_SetNearestModeTable(
                piYCoefsParam,
                Plane,
                bBalancedFilter));
            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                VPHAL_RENDER_CHK_STATUS(Mhw_SetNearestModeTable(
                    piUVCoefsParam,
                    MHW_U_PLANE,
                    bBalancedFilter));
            }
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScale = MOS_MIN(1.0F, fScale);

            VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesY(
                piYCoefsParam,
                fScale,
                Plane,
                SrcFormat,
                fHPStrength,
                true,
                dwHwPhrase));

            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                if (!bBalancedFilter)
                {
                    VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesY(
                        piUVCoefsParam,
                        fScale,
                        MHW_U_PLANE,
                        SrcFormat,
                        fHPStrength,
                        true,
                        dwHwPhrase));
                }
                else
                {
                    // If Chroma Siting info is present
                    if (dwChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_TOP : MHW_CHROMA_SITING_HORZ_LEFT))
                    {
                        // No Chroma Siting
                        VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesUV(
                            piUVCoefsParam,
                            2.0F,
                            fScale));
                    }
                    else
                    {
                        // Chroma siting offset needs to be added
                        if (dwChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_CENTER : MHW_CHROMA_SITING_HORZ_CENTER))
                        {
                            iUvPhaseOffset = MOS_UF_ROUND(0.5F * 16.0F);   // U0.4
                        }
                        else //if (ChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_BOTTOM : MHW_CHROMA_SITING_HORZ_RIGHT))
                        {
                            iUvPhaseOffset = MOS_UF_ROUND(1.0F * 16.0F);   // U0.4
                        }

                        VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesUVOffset(
                            piUVCoefsParam,
                            3.0F,
                            fScale,
                            iUvPhaseOffset));
                    }
                }
            }
        }
    }

finish:
    return eStatus;
}

//!
//! \brief      Set Sampler8x8 Table for Gen9 Hdr AVS
//! \details    Set sampler8x8 table based on format, scale and chroma siting
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to RenderHal interface
//! \param      PMHW_SAMPLER_STATE_PARAM pSamplerStateParams,
//!             [in]    Pointer to sampler state params
//! \param      PVPHAL_AVS_PARAMS pAvsParams
//!             [in]    Pointer to avs parameters
//! \param      MOS_FORMAT SrcFormat
//!             [in]    source format
//! \param      float   fScaleX
//!             [in]    Scale X
//! \param      float   fScaleY
//!             [in]    Scale Y
//! \param      uint32_t   dwChromaSiting
//!             [in]    Chroma siting
//! \return     void
//!
MOS_STATUS VpHal_HdrSetSamplerAvsTableParam_g9(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting)
{
    MOS_STATUS                   eStatus                    = MOS_STATUS_SUCCESS;
    bool                         bBalancedFilter            = false;
    PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam   = nullptr;

    VPHAL_HW_CHK_NULL(pRenderHal);
    VPHAL_HW_CHK_NULL(pSamplerStateParams);
    VPHAL_HW_CHK_NULL(pAvsParams);
    VPHAL_HW_CHK_NULL(pAvsParams->piYCoefsY);
    VPHAL_HW_CHK_NULL(pAvsParams->piYCoefsX);
    VPHAL_HW_CHK_NULL(pAvsParams->piUVCoefsY);
    VPHAL_HW_CHK_NULL(pAvsParams->piUVCoefsX);

    pMhwSamplerAvsTableParam = pSamplerStateParams->Avs.pMhwSamplerAvsTableParam;

    pMhwSamplerAvsTableParam->b8TapAdaptiveEnable         = pSamplerStateParams->Avs.b8TapAdaptiveEnable;
    pMhwSamplerAvsTableParam->byteTransitionArea8Pixels   = MEDIASTATE_AVS_TRANSITION_AREA_8_PIXELS;
    pMhwSamplerAvsTableParam->byteTransitionArea4Pixels   = MEDIASTATE_AVS_TRANSITION_AREA_4_PIXELS;
    pMhwSamplerAvsTableParam->byteMaxDerivative8Pixels    = MEDIASTATE_AVS_MAX_DERIVATIVE_8_PIXELS;
    pMhwSamplerAvsTableParam->byteMaxDerivative4Pixels    = MEDIASTATE_AVS_MAX_DERIVATIVE_4_PIXELS;
    pMhwSamplerAvsTableParam->byteDefaultSharpnessLevel   = MEDIASTATE_AVS_SHARPNESS_LEVEL_SHARP;

    // Enable Adaptive Filtering, if it is being upscaled
    // in either direction. we must check for this before clamping the SF.
    if ((IS_YUV_FORMAT(SrcFormat) && (fScaleX > 1.0F || fScaleY > 1.0F)) ||
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable)
    {
        pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering = false;
        pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering = false;
        if (pMhwSamplerAvsTableParam->b8TapAdaptiveEnable)
        {
            pMhwSamplerAvsTableParam->bAdaptiveFilterAllChannels = true;

            if (IS_RGB_FORMAT(SrcFormat))
            {
                pMhwSamplerAvsTableParam->bEnableRGBAdaptive     = true;
            }
        }
    }
    else
    {
        pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering = true;
        pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering = true;
    }

    // No changes to AVS parameters -> skip
    if (SrcFormat == pAvsParams->Format &&
        fScaleX == pAvsParams->fScaleX &&
        fScaleY == pAvsParams->fScaleY)
    {
        goto finish;
    }

    // AVS Coefficients don't change for Scaling Factors > 1.0x
    // Hence recalculation is avoided
    if (fScaleX > 1.0F && pAvsParams->fScaleX > 1.0F)
    {
        pAvsParams->fScaleX = fScaleX;
    }

    // AVS Coefficients don't change for Scaling Factors > 1.0x
    // Hence recalculation is avoided
    if (fScaleY > 1.0F && pAvsParams->fScaleY > 1.0F)
    {
        pAvsParams->fScaleY = fScaleY;
    }

    bBalancedFilter = true;
    // Recalculate Horizontal scaling table
    VPHAL_HW_CHK_STATUS(VpHal_HdrSamplerAvsCalcScalingTable_g9(
        SrcFormat,
        fScaleX,
        false,
        dwChromaSiting,
        bBalancedFilter,
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
        pAvsParams));

    // Recalculate Vertical scaling table
    VPHAL_HW_CHK_STATUS(VpHal_HdrSamplerAvsCalcScalingTable_g9(
        SrcFormat,
        fScaleY,
        true,
        dwChromaSiting,
        bBalancedFilter,
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
        pAvsParams));

    pMhwSamplerAvsTableParam->bIsCoeffExtraEnabled = true;
    // Save format used to calculate AVS parameters
    pAvsParams->Format                             = SrcFormat;
    pMhwSamplerAvsTableParam->b4TapGY              = (IS_RGB32_FORMAT(SrcFormat) && !pMhwSamplerAvsTableParam->b8TapAdaptiveEnable);
    pMhwSamplerAvsTableParam->b4TapRBUV            = (!pMhwSamplerAvsTableParam->b8TapAdaptiveEnable);

    VpHal_RenderCommonSetAVSTableParam(pAvsParams, pMhwSamplerAvsTableParam);

finish:
    return eStatus;
}


typedef float (*pfnOETFFunc)(float radiance);

float OETF2084(float c)
{
    static const double C1 = 0.8359375;
    static const double C2 = 18.8515625;
    static const double C3 = 18.6875;
    static const double M1 = 0.1593017578125;
    static const double M2 = 78.84375;

    double tmp = c;
    double numerator = pow(tmp, M1);
    double denominator = numerator;

    denominator = 1.0 + C3 * denominator;
    numerator   = C1 + C2 * numerator;
    numerator   = numerator / denominator;

    return (float)pow(numerator, M2);
}

float OETFBT709(float c)
{
    static const double E0 = 0.45;
    static const double C1 = 0.099;
    static const double C2 = 4.5;
    static const double P0 = 0.018;

    double tmp = c;
    double result;

    if (tmp <= P0)
    {
        result = C2 * tmp;
    }
    else
    {
        result = (C1 + 1.0) * pow(tmp, E0) - C1;
    }
    return (float)result;
}

float OETFsRGB(float c)
{
    static const double E1 = 2.4;
    static const double C1 = 0.055;
    static const double C2 = 12.92;
    static const double P0 = 0.0031308;

    double tmp = c;
    double result;

    if (tmp <= P0)
    {
        result = C2 * tmp;
    }
    else
    {
        result = (C1 + 1.0) * pow(tmp, 1.0 / E1) - C1;
    }
    return (float)result;
}

// Non-uniform OETF LUT generator.
void VpHal_Generate2SegmentsOETFLUT(float fStretchFactor, pfnOETFFunc oetfFunc, uint16_t *lut)
{
    int i = 0, j = 0;

    for (i = 0; i < VPHAL_HDR_OETF_1DLUT_HEIGHT; ++i)
    {
        for (j = 0; j < VPHAL_HDR_OETF_1DLUT_WIDTH; ++j)
        {
            int idx = j + i * (VPHAL_HDR_OETF_1DLUT_WIDTH - 1);
            float a = (idx < 32) ? ((1.0f / 1024.0f) * idx) : ((1.0f / 32.0f) * (idx - 31));

            if (a > 1.0f)
                a = 1.0f;

            a *= fStretchFactor;
            lut[i * VPHAL_HDR_OETF_1DLUT_WIDTH + j] = VpHal_FloatToHalfFloat(oetfFunc(a));
        }
    }
}

typedef float Mat3[3][3];
typedef float Vec3[3];

static void Mat3MultiplyMat3(const Mat3 left, const Mat3 right, Mat3 output)
{
    output[0][0] = left[0][0] * right[0][0] + left[0][1] * right[1][0] + left[0][2] * right[2][0];
    output[0][1] = left[0][0] * right[0][1] + left[0][1] * right[1][1] + left[0][2] * right[2][1];
    output[0][2] = left[0][0] * right[0][2] + left[0][1] * right[1][2] + left[0][2] * right[2][2];
    output[1][0] = left[1][0] * right[0][0] + left[1][1] * right[1][0] + left[1][2] * right[2][0];
    output[1][1] = left[1][0] * right[0][1] + left[1][1] * right[1][1] + left[1][2] * right[2][1];
    output[1][2] = left[1][0] * right[0][2] + left[1][1] * right[1][2] + left[1][2] * right[2][2];
    output[2][0] = left[2][0] * right[0][0] + left[2][1] * right[1][0] + left[2][2] * right[2][0];
    output[2][1] = left[2][0] * right[0][1] + left[2][1] * right[1][1] + left[2][2] * right[2][1];
    output[2][2] = left[2][0] * right[0][2] + left[2][1] * right[1][2] + left[2][2] * right[2][2];
}

static void Mat3MultiplyVec3(const Mat3 input, const Vec3 vec, Vec3 output)
{
    output[0] = input[0][0] * vec[0] + input[0][1] * vec[1] + input[0][2] * vec[2];
    output[1] = input[1][0] * vec[0] + input[1][1] * vec[1] + input[1][2] * vec[2];
    output[2] = input[2][0] * vec[0] + input[2][1] * vec[1] + input[2][2] * vec[2];
}

static void Mat3Inverse(const Mat3 input, Mat3 output)
{
    const float a0 = input[0][0];
    const float a1 = input[0][1];
    const float a2 = input[0][2];

    const float b0 = input[1][0];
    const float b1 = input[1][1];
    const float b2 = input[1][2];

    const float c0 = input[2][0];
    const float c1 = input[2][1];
    const float c2 = input[2][2];

    float det = a0 * (b1 * c2 - b2 * c1) + a1 * (b2 * c0 - b0 * c2) + a2 * ( b0 * c1 - b1 * c0);

    if (det != 0.0f)
    {
        float det_recip = 1.0f / det;

        output[0][0] = (b1 * c2 - b2 * c1) * det_recip;
        output[0][1] = (a2 * c1 - a1 * c2) * det_recip;
        output[0][2] = (a1 * b2 - a2 * b1) * det_recip;

        output[1][0] = (b2 * c0 - b0 * c2) * det_recip;
        output[1][1] = (a0 * c2 - a2 * c0) * det_recip;
        output[1][2] = (a2 * b0 - a0 * b2) * det_recip;

        output[2][0] = (b0 * c1 - b1 * c0) * det_recip;
        output[2][1] = (a1 * c0 - a0 * c1) * det_recip;
        output[2][2] = (a0 * b1 - a1 * b0) * det_recip;
    }
    else
    {
        // irreversible
        output[0][0] = 1.0f;
        output[0][1] = 0.0f;
        output[0][2] = 0.0f;
        output[1][0] = 0.0f;
        output[1][1] = 1.0f;
        output[1][2] = 0.0f;
        output[2][0] = 0.0f;
        output[2][1] = 0.0f;
        output[2][2] = 1.0f;
    }
}

static void RGB2CIEXYZMatrix(
    const float xr, const float yr,
    const float xg, const float yg,
    const float xb, const float yb,
    const float xn, const float yn,
    Mat3   output)
{
    const float zr = 1.0f - xr - yr;
    const float zg = 1.0f - xg - yg;
    const float zb = 1.0f - xb - yb;
    const float zn = 1.0f - xn - yn;

    // m * [ar, ag, ab]T = [xn / yn, 1.0f, zn / yn]T;
    const Mat3 m =
    {
        xr, xg, xb,
        yr, yg, yb,
        zr, zg, zb
    };

    Mat3 inversed_m;

    Mat3Inverse(m, inversed_m);

    const Vec3 XYZWithUnityY = {xn / yn, 1.0f, zn / yn};
    float aragab[3];

    Mat3MultiplyVec3(inversed_m, XYZWithUnityY, aragab);

    output[0][0] = m[0][0] * aragab[0];
    output[1][0] = m[1][0] * aragab[0];
    output[2][0] = m[2][0] * aragab[0];
    output[0][1] = m[0][1] * aragab[1];
    output[1][1] = m[1][1] * aragab[1];
    output[2][1] = m[2][1] * aragab[1];
    output[0][2] = m[0][2] * aragab[2];
    output[1][2] = m[1][2] * aragab[2];
    output[2][2] = m[2][2] * aragab[2];
}

void VpHal_CalculateCCMWithMonitorGamut(
    VPHAL_HDR_CCM_TYPE  CCMType,
    PVPHAL_HDR_PARAMS   pTarget,
    float TempMatrix[12])
{
    float src_xr = 1.0f, src_yr = 1.0f;
    float src_xg = 1.0f, src_yg = 1.0f;
    float src_xb = 1.0f, src_yb = 1.0f;
    float src_xn = 1.0f, src_yn = 1.0f;

    float dst_xr = 1.0f, dst_yr = 1.0f;
    float dst_xg = 1.0f, dst_yg = 1.0f;
    float dst_xb = 1.0f, dst_yb = 1.0f;
    float dst_xn = 1.0f, dst_yn = 1.0f;

    Mat3 SrcMatrix = {1.0f};
    Mat3 DstMatrix = {1.0f};
    Mat3 DstMatrixInverse = {1.0f};
    Mat3 SrcToDstMatrix   = {1.0f};

    Mat3 BT709ToBT2020Matrix = {1.0f};
    Mat3 BT2020ToBT709Matrix = {1.0f};

    VPHAL_PUBLIC_CHK_NULL_NO_STATUS(pTarget);

    if (CCMType == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX)
    {
        src_xr = 0.708f;
        src_yr = 0.292f;
        src_xg = 0.170f;
        src_yg = 0.797f;
        src_xb = 0.131f;
        src_yb = 0.046f;
        src_xn = 0.3127f;
        src_yn = 0.3290f;

        dst_xr = pTarget->display_primaries_x[2] / 50000.0f;
        dst_yr = pTarget->display_primaries_y[2] / 50000.0f;
        dst_xg = pTarget->display_primaries_x[0] / 50000.0f;
        dst_yg = pTarget->display_primaries_y[0] / 50000.0f;
        dst_xb = pTarget->display_primaries_x[1] / 50000.0f;
        dst_yb = pTarget->display_primaries_y[1] / 50000.0f;
        dst_xn = pTarget->white_point_x / 50000.0f;
        dst_yn = pTarget->white_point_y / 50000.0f;
    }
    else if (CCMType == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX)
    {
        src_xr = pTarget->display_primaries_x[2] / 50000.0f;
        src_yr = pTarget->display_primaries_y[2] / 50000.0f;
        src_xg = pTarget->display_primaries_x[0] / 50000.0f;
        src_yg = pTarget->display_primaries_y[0] / 50000.0f;
        src_xb = pTarget->display_primaries_x[1] / 50000.0f;
        src_yb = pTarget->display_primaries_y[1] / 50000.0f;
        src_xn = pTarget->white_point_x / 50000.0f;
        src_yn = pTarget->white_point_y / 50000.0f;

        dst_xr = 0.708f;
        dst_yr = 0.292f;
        dst_xg = 0.170f;
        dst_yg = 0.797f;
        dst_xb = 0.131f;
        dst_yb = 0.046f;
        dst_xn = 0.3127f;
        dst_yn = 0.3290f;
    }
    else
    {
        // VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX
        src_xr = pTarget->display_primaries_x[2] / 50000.0f;
        src_yr = pTarget->display_primaries_y[2] / 50000.0f;
        src_xg = pTarget->display_primaries_x[0] / 50000.0f;
        src_yg = pTarget->display_primaries_y[0] / 50000.0f;
        src_xb = pTarget->display_primaries_x[1] / 50000.0f;
        src_yb = pTarget->display_primaries_y[1] / 50000.0f;
        src_xn = pTarget->white_point_x / 50000.0f;
        src_yn = pTarget->white_point_y / 50000.0f;

        dst_xr = 0.64f;
        dst_yr = 0.33f;
        dst_xg = 0.30f;
        dst_yg = 0.60f;
        dst_xb = 0.15f;
        dst_yb = 0.06f;
        dst_xn = 0.3127f;
        dst_yn = 0.3290f;
    }

    RGB2CIEXYZMatrix(
            src_xr, src_yr,
            src_xg, src_yg,
            src_xb, src_yb,
            src_xn, src_yn,
            SrcMatrix);

    RGB2CIEXYZMatrix(
            dst_xr, dst_yr,
            dst_xg, dst_yg,
            dst_xb, dst_yb,
            dst_xn, dst_yn,
            DstMatrix);

    Mat3Inverse(DstMatrix, DstMatrixInverse);
    Mat3MultiplyMat3(DstMatrixInverse, SrcMatrix, SrcToDstMatrix);

    TempMatrix[0]  = SrcToDstMatrix[0][0];
    TempMatrix[1]  = SrcToDstMatrix[0][1];
    TempMatrix[2]  = SrcToDstMatrix[0][2];
    TempMatrix[3]  = 0.0f;
    TempMatrix[4]  = SrcToDstMatrix[1][0];
    TempMatrix[5]  = SrcToDstMatrix[1][1];
    TempMatrix[6]  = SrcToDstMatrix[1][2];
    TempMatrix[7]  = 0.0f;
    TempMatrix[8]  = SrcToDstMatrix[2][0];
    TempMatrix[9]  = SrcToDstMatrix[2][1];
    TempMatrix[10] = SrcToDstMatrix[2][2];
    TempMatrix[11] = 0.0f;

finish:
    return;
}

//!
//! \brief    Initiate EOTF Surface for HDR
//! \details  Initiate EOTF Surface for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \param    PVPHAL_SURFACE pCoeffSurface
//!           [in] Pointer to CSC/CCM Surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrInitCoeff_g9 (
    PVPHAL_HDR_STATE pHdrState,
    PVPHAL_SURFACE   pCoeffSurface)
{
    MOS_STATUS       eStatus                = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE   pOsInterface           = nullptr;
    uint32_t         i                      = 0;
    float            *pFloat                = nullptr;
    float            *pCoeff                = nullptr;
    uint32_t         *pEOTFType             = nullptr;
    float            *pEOTFCoeff            = nullptr;
    float            *pPivotPoint           = nullptr;
    uint32_t         *pTMType               = nullptr;
    uint32_t         *pOETFNeqType          = nullptr;
    uint32_t         *pCCMEnable            = nullptr;
    float            *pPWLFStretch          = nullptr;
    float            *pCoeffR               = nullptr;
    float            *pCoeffG               = nullptr;
    float            *pCoeffB               = nullptr;
    uint16_t         *pSlopeIntercept       = nullptr;
    MOS_LOCK_PARAMS  LockFlags              = {};
    float            PriorCscMatrix[12]     = {};
    float            PostCscMatrix[12]      = {};
    float            CcmMatrix[12]          = {};
    float            TempMatrix[12]         = {};
    PVPHAL_SURFACE   pSrc                   = nullptr;

    VPHAL_PUBLIC_CHK_NULL(pHdrState);
    VPHAL_PUBLIC_CHK_NULL(pCoeffSurface);
    VPHAL_PUBLIC_CHK_NULL(pHdrState->pTargetSurf[0]);

    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = pHdrState->pOsInterface;

    VPHAL_PUBLIC_CHK_NULL(pOsInterface);

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    // Lock the surface for writing
    pFloat = (float *)pOsInterface->pfnLockResource(
        pOsInterface,
        &(pCoeffSurface->OsResource),
        &LockFlags);

    VPHAL_RENDER_CHK_NULL(pFloat);

    #define SET_MATRIX(_c0, _c1, _c2, _c3, _c4, _c5, _c6, _c7, _c8, _c9, _c10, _c11) \
    { \
        TempMatrix[0]          = _c0;              \
        TempMatrix[1]          = _c1;              \
        TempMatrix[2]          = _c2;              \
        TempMatrix[3]          = _c3;              \
        TempMatrix[4]          = _c4;              \
        TempMatrix[5]          = _c5;              \
        TempMatrix[6]          = _c6;              \
        TempMatrix[7]          = _c7;              \
        TempMatrix[8]          = _c8;              \
        TempMatrix[9]          = _c9;              \
        TempMatrix[10]         = _c10;             \
        TempMatrix[11]         = _c11;             \
    }

    #define SET_EOTF_COEFF(_c1, _c2, _c3, _c4, _c5) \
    { \
        *pEOTFCoeff          = _c1;                 \
        pEOTFCoeff += pCoeffSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff          = _c2;                 \
        pEOTFCoeff += pCoeffSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff          = _c3;                 \
        pEOTFCoeff += pCoeffSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff          = _c4;                 \
        pEOTFCoeff += pCoeffSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff          = _c5;                 \
    }

    #define WRITE_MATRIX(Matrix) \
    { \
        *pCoeff++          = Matrix[0];             \
        *pCoeff++          = Matrix[1];             \
        *pCoeff++          = Matrix[2];             \
        *pCoeff++          = Matrix[3];             \
        *pCoeff++          = Matrix[4];             \
        *pCoeff++          = Matrix[5];             \
         pCoeff+= pCoeffSurface->dwPitch / sizeof(float) - 6; \
        *pCoeff++          = Matrix[6];             \
        *pCoeff++          = Matrix[7];             \
        *pCoeff++          = Matrix[8];             \
        *pCoeff++          = Matrix[9];             \
        *pCoeff++          = Matrix[10];            \
        *pCoeff++          = Matrix[11];            \
         pCoeff+= pCoeffSurface->dwPitch / sizeof(float)  - 6; \
    }

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++, pFloat += VPHAL_HDR_COEF_LINES_PER_LAYER_BASIC_G9 * pCoeffSurface->dwPitch / (sizeof(float)))
    {
        if (pHdrState->pSrcSurf[i] == nullptr)
        {
            continue;
        }

        pSrc   = pHdrState->pSrcSurf[i];
        pCoeff = pFloat;

        if (pHdrState->pSrcSurf[i]->SurfType == SURF_IN_PRIMARY)
        {
            pHdrState->Reporting.HDRMode = pHdrState->HdrMode[i];
        }

        // EOTF/CCM/Tone Mapping/OETF require RGB input
        // So if prior CSC is needed, it will always be YUV to RGB conversion
        if (pHdrState->StageEnableFlags[i].PriorCSCEnable)
        {
            if (pHdrState->PriorCSC[i] == VPHAL_HDR_CSC_YUV_TO_RGB_BT601)
            {
                SET_MATRIX( 1.000000f,  0.000000f,  1.402000f,  0.000000f,
                            1.000000f, -0.344136f, -0.714136f,  0.000000f,
                            1.000000f,  1.772000f,  0.000000f,  0.000000f);

                VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT601, CSpace_sRGB, TempMatrix, PriorCscMatrix);
            }
            else if (pHdrState->PriorCSC[i] == VPHAL_HDR_CSC_YUV_TO_RGB_BT709)
            {
                SET_MATRIX( 1.000000f,  0.000000f,  1.574800f,  0.000000f,
                            1.000000f, -0.187324f, -0.468124f,  0.000000f,
                            1.000000f,  1.855600f,  0.000000f,  0.000000f);
                VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT709, CSpace_sRGB, TempMatrix, PriorCscMatrix);
            }
            else if (pHdrState->PriorCSC[i] == VPHAL_HDR_CSC_YUV_TO_RGB_BT2020)
            {
                SET_MATRIX( 1.000000f,  0.000000f,  1.474600f,  0.000000f,
                            1.000000f, -0.164550f, -0.571350f,  0.000000f,
                            1.000000f,  1.881400f,  0.000000f,  0.000000f);
                VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT2020, CSpace_sRGB, TempMatrix, PriorCscMatrix);
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Color Space Not found.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
            LimitFP32ArrayPrecisionToF3_9(PriorCscMatrix, ARRAY_SIZE(PriorCscMatrix));
            WRITE_MATRIX(PriorCscMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->dwPitch / sizeof(float) * 2;
        }

        if (pHdrState->StageEnableFlags[i].CCMEnable)
        {
            // BT709 to BT2020 CCM
            if (pHdrState->CCM[i] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
            {
                SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f,
                           0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f,
                           0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
            }
            // BT2020 to BT709 CCM
            else if (pHdrState->CCM[i] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
            {
                SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f,
                          -0.124550248621850f,  1.132898753013895f, -0.008347895599309f, 0.000000f,
                          -0.018151059958635f, -0.100578696221493f,  1.118729865913540f, 0.000000f);
            }
            else
            {
                SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f);
            }

            VpHal_HdrCalcCCMMatrix(TempMatrix, CcmMatrix);
            LimitFP32ArrayPrecisionToF3_9(CcmMatrix, ARRAY_SIZE(CcmMatrix));
            WRITE_MATRIX(CcmMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->dwPitch / sizeof(float) * 2;
        }

        // OETF will output RGB surface
        // So if post CSC is needed, it will always be RGB to YUV conversion
        if (pHdrState->StageEnableFlags[i].PostCSCEnable)
        {
            if (pHdrState->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT601)
            {
                SET_MATRIX( -0.331264f, -0.168736f,  0.500000f,  0.000000f,
                             0.587000f,  0.299000f,  0.114000f,  0.000000f,
                            -0.418688f,  0.500000f, -0.081312f,  0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT601, TempMatrix, PostCscMatrix);
            }
            else if (pHdrState->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT709)
            {
                SET_MATRIX( -0.385428f, -0.114572f,  0.500000f,  0.000000f,
                             0.715200f,  0.212600f,  0.072200f,  0.000000f,
                            -0.454153f,  0.500000f, -0.045847f,  0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT709, TempMatrix, PostCscMatrix);
            }
            else if (pHdrState->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT709_FULLRANGE)
            {
                SET_MATRIX( -0.385428f, -0.114572f,  0.500000f,  0.000000f,
                             0.715200f,  0.212600f,  0.072200f,  0.000000f,
                            -0.454153f,  0.500000f, -0.045847f, 0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT709_FullRange, TempMatrix, PostCscMatrix);
            }
            else if (pHdrState->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT2020)
            {
                SET_MATRIX( -0.360370f, -0.139630f,  0.500000f,  0.000000f,
                             0.678000f,  0.262700f,  0.059300f,  0.000000f,
                            -0.459786f,  0.500000f, -0.040214f,  0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT2020, TempMatrix, PostCscMatrix);
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Color Space Not found.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
            LimitFP32ArrayPrecisionToF3_9(PostCscMatrix, ARRAY_SIZE(PostCscMatrix));
            WRITE_MATRIX(PostCscMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->dwPitch / sizeof(float) * 2;
        }

        pEOTFType  = (uint32_t *)(pFloat + VPHAL_HDR_COEF_EOTF_OFFSET);
        pEOTFCoeff = pFloat + pCoeffSurface->dwPitch / sizeof(float) + VPHAL_HDR_COEF_EOTF_OFFSET;

        if (pHdrState->StageEnableFlags[i].EOTFEnable)
        {
            if (pHdrState->EOTFGamma[i] == VPHAL_GAMMA_TRADITIONAL_GAMMA)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA_G9;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_G9);
            }
            else if (pHdrState->EOTFGamma[i] == VPHAL_GAMMA_SMPTE_ST2084)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_SMPTE_ST2084_G9;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_SMPTE_ST2084_G9,
                               VPHAL_HDR_EOTF_COEFF2_SMPTE_ST2084_G9,
                               VPHAL_HDR_EOTF_COEFF3_SMPTE_ST2084_G9,
                               VPHAL_HDR_EOTF_COEFF4_SMPTE_ST2084_G9,
                               VPHAL_HDR_EOTF_COEFF5_SMPTE_ST2084_G9);
            }
            else if (pHdrState->EOTFGamma[i] == VPHAL_GAMMA_BT1886)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA_G9;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_BT1886_G9,
                               VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_BT1886_G9,
                               VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_BT1886_G9,
                               VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_BT1886_G9,
                               VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_BT1886_G9);
            }
            else if (pHdrState->EOTFGamma[i] == VPHAL_GAMMA_SRGB)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA_G9;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_SRGB_G9);
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }

        pEOTFType ++;
        pEOTFCoeff = pFloat + pCoeffSurface->dwPitch / sizeof(float) + VPHAL_HDR_COEF_EOTF_OFFSET + 1;

        if (pHdrState->StageEnableFlags[i].OETFEnable)
        {
            if (pHdrState->OETFGamma[i] == VPHAL_GAMMA_TRADITIONAL_GAMMA)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA_G9;
                SET_EOTF_COEFF(VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA_G9,
                               VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA_G9);
            }
            else if (pHdrState->OETFGamma[i] == VPHAL_GAMMA_SRGB)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA_G9;
                SET_EOTF_COEFF(VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA_SRGB_G9,
                               VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA_SRGB_G9);
            }
            else if (pHdrState->OETFGamma[i] == VPHAL_GAMMA_SMPTE_ST2084)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_SMPTE_ST2084_G9;
                SET_EOTF_COEFF(VPHAL_HDR_OETF_COEFF1_SMPTE_ST2084_G9,
                               VPHAL_HDR_OETF_COEFF2_SMPTE_ST2084_G9,
                               VPHAL_HDR_OETF_COEFF3_SMPTE_ST2084_G9,
                               VPHAL_HDR_OETF_COEFF4_SMPTE_ST2084_G9,
                               VPHAL_HDR_OETF_COEFF5_SMPTE_ST2084_G9);
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }

        // NOTE:
        // Pitch is not equal to width usually. So please be careful when using pointer addition.
        // Only do this when operands are in the same row.
        pPivotPoint     = pFloat + pCoeffSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_PIVOT_POINT_LINE_OFFSET;
        pSlopeIntercept = (uint16_t *)(pFloat + pCoeffSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_SLOPE_INTERCEPT_LINE_OFFSET);
        pPWLFStretch    = pFloat + pCoeffSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_PIVOT_POINT_LINE_OFFSET + 5;
        pTMType         = (uint32_t *)(pPWLFStretch);
        pCoeffR         = pPWLFStretch + 1;
        pCoeffG         = pCoeffR + 1;
        pCoeffB         = pFloat + pCoeffSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_SLOPE_INTERCEPT_LINE_OFFSET + 6;
        pOETFNeqType    = (uint32_t *)(pFloat + pCoeffSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_SLOPE_INTERCEPT_LINE_OFFSET + 7);

        if (pHdrState->HdrMode[i] == VPHAL_HDR_MODE_TONE_MAPPING)
        {
            *pTMType       = 1; // TMtype
            *pOETFNeqType  = 0 | (10000 << 16); // OETFNEQ
            *pCoeffR = 0.25f;
            *pCoeffG = 0.625f;
            *pCoeffB = 0.125f;
        }
        else if (pHdrState->HdrMode[i] == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
        {
            *pPWLFStretch = 0.01f; // Stretch
            *pOETFNeqType = 1 | ((uint32_t)100 << 16); // OETFNEQ
            *pCoeffR = 0.0f;
            *pCoeffG = 0.0f;
            *pCoeffB = 0.0f;
        }
        else if (pHdrState->HdrMode[i] == VPHAL_HDR_MODE_H2H ||
                 pHdrState->HdrMode[i] == VPHAL_HDR_MODE_H2H_AUTO_MODE)
        {
            PVPHAL_SURFACE  pTargetSurf  = (PVPHAL_SURFACE)pHdrState->pTargetSurf[0];

            *pTMType      = 1; // TMtype
            *pOETFNeqType = 2 | (((uint32_t)(pTargetSurf->pHDRParams->max_display_mastering_luminance)) << 16); // OETFNEQ
            *pCoeffR = 0.25f;
            *pCoeffG = 0.625f;
            *pCoeffB = 0.125f;
        }
        else
        {
            *pPivotPoint   = 0.0f;
            *pTMType       = 0; // TMtype
            *pOETFNeqType  = 0; // OETFNEQ
        }
    }

    // Skip the Dst CSC area
    pFloat += 2 * pCoeffSurface->dwPitch / sizeof(float);

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++, pFloat += VPHAL_HDR_COEF_LINES_PER_LAYER_EXT_G9 * pCoeffSurface->dwPitch / (sizeof(float)))
    {
        pCCMEnable = (uint32_t *)pFloat;
        *(pCCMEnable + VPHAL_HDR_COEF_CCMEXT_OFFSET) = pHdrState->StageEnableFlags[i].CCMExt1Enable;
        *(pCCMEnable + VPHAL_HDR_COEF_CLAMP_OFFSET ) = pHdrState->StageEnableFlags[i].GamutClamp1Enable;

        pCCMEnable += pCoeffSurface->dwPitch / sizeof(float) * 2;
        *(pCCMEnable + VPHAL_HDR_COEF_CCMEXT_OFFSET) = pHdrState->StageEnableFlags[i].CCMExt2Enable;
        *(pCCMEnable + VPHAL_HDR_COEF_CLAMP_OFFSET ) = pHdrState->StageEnableFlags[i].GamutClamp2Enable;

        if (pHdrState->pSrcSurf[i] == nullptr)
        {
            continue;
        }

        pCoeff = pFloat;
        if (pHdrState->StageEnableFlags[i].CCMExt1Enable)
        {
            // BT709 to BT2020 CCM
            if (pHdrState->CCMExt1[i] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
            {
                SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f,
                           0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f,
                           0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
            }
            // BT2020 to BT709 CCM
            else if (pHdrState->CCMExt1[i] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
            {
                SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f,
                          -0.124550248621850f,  1.132898753013895f, -0.008347895599309f, 0.000000f,
                          -0.018151059958635f, -0.100578696221493f,  1.118729865913540f, 0.000000f);
            }
            else if (pHdrState->CCMExt1[i] == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                     pHdrState->CCMExt1[i] == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                     pHdrState->CCMExt1[i] == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
            {
                PVPHAL_SURFACE  pTargetSurf = (PVPHAL_SURFACE)pHdrState->pTargetSurf[0];
                VpHal_CalculateCCMWithMonitorGamut(pHdrState->CCMExt1[i], pTargetSurf->pHDRParams, TempMatrix);
            }
            else
            {
                SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f);
            }

            VpHal_HdrCalcCCMMatrix(TempMatrix, CcmMatrix);
            LimitFP32ArrayPrecisionToF3_9(CcmMatrix, ARRAY_SIZE(CcmMatrix));
            WRITE_MATRIX(CcmMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->dwPitch / sizeof(float) * 2;
        }

        if (pHdrState->StageEnableFlags[i].CCMExt2Enable)
        {
            // BT709 to BT2020 CCM
            if (pHdrState->CCMExt2[i] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
            {
                SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f,
                           0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f,
                           0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
            }
            // BT2020 to BT709 CCM
            else if (pHdrState->CCMExt2[i] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
            {
                SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f,
                          -0.124550248621850f,  1.132898753013895f, -0.008347895599309f, 0.000000f,
                          -0.018151059958635f, -0.100578696221493f,  1.118729865913540f, 0.000000f);
            }
            else if (pHdrState->CCMExt2[i] == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                     pHdrState->CCMExt2[i] == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                     pHdrState->CCMExt2[i] == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
            {
                PVPHAL_SURFACE  pTargetSurf = (PVPHAL_SURFACE)pHdrState->pTargetSurf[0];
                VpHal_CalculateCCMWithMonitorGamut(pHdrState->CCMExt2[i], pTargetSurf->pHDRParams, TempMatrix);
            }
            else
            {
                SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f);
            }

            VpHal_HdrCalcCCMMatrix(TempMatrix, CcmMatrix);
            LimitFP32ArrayPrecisionToF3_9(CcmMatrix, ARRAY_SIZE(CcmMatrix));
            WRITE_MATRIX(CcmMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->dwPitch / sizeof(float) * 2;
        }
    }

    pOsInterface->pfnUnlockResource(
            pOsInterface,
            &(pCoeffSurface->OsResource));

    eStatus = MOS_STATUS_SUCCESS;

    #undef SET_MATRIX
    #undef SET_EOTF_COEFF
    #undef WRITE_MATRIX

finish:
    return eStatus;
}


//! \brief    Allocate Resources for HDR
//! \details  Allocate Resources for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrAllocateResources_g9(
    PVPHAL_HDR_STATE    pHdrState)
{
    MOS_STATUS              eStatus            = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE          pOsInterface       = nullptr;
    uint32_t                dwWidth            = 16;
    uint32_t                dwHeight           = 16;
    uint32_t                dwDepth            = 8;
    bool                    bAllocated         = false;
    uint32_t                dwUpdateMask       = 0;
    int32_t                 i                  = 0;
    VPHAL_GET_SURFACE_INFO  Info               = {};
    MOS_ALLOC_GFXRES_PARAMS AllocParams        = {};
    VPHAL_HDR_RENDER_DATA   RenderData         = {};

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pHdrState->pOsInterface);

    eStatus         = MOS_STATUS_UNKNOWN;
    pOsInterface    = pHdrState->pOsInterface;
    
    // Allocate CSC CCM Coeff Surface
    dwWidth  = VPHAL_HDR_COEF_SURFACE_WIDTH_G9;
    dwHeight = VPHAL_HDR_COEF_SURFACE_HEIGHT_G9;

    pHdrState->pHDRStageConfigTable = HDRStageConfigTable_g9;

    VPHAL_RENDER_CHK_STATUS(VpHal_HdrUpdatePerLayerPipelineStates(pHdrState, &dwUpdateMask));

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
        pOsInterface,
        &pHdrState->CoeffSurface,
        "CoeffSurface",
        Format_A8R8G8B8,
        MOS_GFXRES_2D,
        MOS_TILE_LINEAR,
        dwWidth,
        dwHeight,
        false,
        MOS_MMC_DISABLED,
        &bAllocated));

    // Initialize COEF Coeff Surface
    if (dwUpdateMask || bAllocated)
    {
        VPHAL_RENDER_CHK_STATUS(pHdrState->pfnInitCoeff(
            pHdrState,
            &pHdrState->CoeffSurface));
    }

    // Allocate OETF 1D LUT Surface
    dwWidth  = pHdrState->dwOetfSurfaceWidth;
    dwHeight = pHdrState->dwOetfSurfaceWidth;

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++)
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                pOsInterface,
                &pHdrState->OETF1DLUTSurface[i],
                "OETF1DLUTSurface",
                Format_R16F,
                MOS_GFXRES_2D,
                MOS_TILE_LINEAR,
                dwWidth,
                dwHeight,
                false,
                MOS_MMC_DISABLED,
                &bAllocated));

        if (bAllocated || (dwUpdateMask & (1 << i)) || (dwUpdateMask & (1 << VPHAL_MAX_HDR_INPUT_LAYER)))
        {
            // Initialize OETF 1D LUT Surface
            VPHAL_RENDER_CHK_STATUS(pHdrState->pfnInitOETF1DLUT(
                    pHdrState,
                    i,
                    &pHdrState->OETF1DLUTSurface[i]));
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Free Resources for HDR
//! \details  Free Resources for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrFreeResources_g9(
    PVPHAL_HDR_STATE    pHdrState)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE              pOsInterface     = nullptr;
    int32_t                     i                = 0;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pHdrState->pOsInterface);

    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = pHdrState->pOsInterface;

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++)
    {
        pOsInterface->pfnFreeResource(pOsInterface,
            &(pHdrState->OETF1DLUTSurface[i].OsResource));
    }

    pOsInterface->pfnFreeResource(pOsInterface,
            &(pHdrState->CoeffSurface.OsResource));
finish:
    return eStatus;
}

//!
//! \brief    HDR Surface State Setup
//! \details  Set up surface state used in HDR process, and bind the surface to pointed binding table entry.
//! \param    PVPHAL_HDR_STATE pHdrState
//            [in/out] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to hdr render data.
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetupSurfaceStates_g9(
    PVPHAL_HDR_STATE           pHdrState,
    PVPHAL_HDR_RENDER_DATA     pRenderData)
{
    PRENDERHAL_INTERFACE            pRenderHal                  = nullptr;
    PVPHAL_SURFACE                  pSource                     = nullptr;
    PVPHAL_SURFACE                  pTarget                     = nullptr;
    PRENDERHAL_SURFACE              pRenderHalSource            = nullptr;
    PRENDERHAL_SURFACE              pRenderHalTarget            = nullptr;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams               = {};
    MOS_STATUS                      eStatus                     = MOS_STATUS_UNKNOWN;
    uint32_t                        i                           = 0;
    int32_t                         iBTentry                    = 0;
    PVPHAL_SURFACE                  pSurfaceTemp                = nullptr;
    PRENDERHAL_SURFACE              pRenderHalSurfaceTemp       = nullptr;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pRenderData);

    pRenderHal   = pHdrState->pRenderHal;
    VPHAL_RENDER_CHK_NULL(pRenderHal);

    for (i = 0; i < pHdrState->uSourceCount; i++)
    {
        if (i >= VPHAL_MAX_HDR_INPUT_LAYER)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        pSource = pHdrState->pSrcSurf[i];
        pRenderHalSource = & pHdrState->RenderHalSrcSurf[i];
        VPHAL_RENDER_CHK_NULL(pSource);

        MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

        // Render target or private surface
        if (pSource->SurfType == SURF_OUT_RENDERTARGET)
        {
            // Disable AVS, IEF
            pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
            pSource->bIEF        = false;

            // Set flags for RT
            SurfaceParams.bRenderTarget    = true;
            SurfaceParams.bWidthInDword_Y  = true;
            SurfaceParams.bWidthInDword_UV = true;
            SurfaceParams.Boundary         = RENDERHAL_SS_BOUNDARY_DSTRECT;
        }
        // other surfaces
        else
        {
            SurfaceParams.bRenderTarget    = false;
            SurfaceParams.bWidthInDword_Y  = false;
            SurfaceParams.bWidthInDword_UV = false;
            SurfaceParams.Boundary         = RENDERHAL_SS_BOUNDARY_SRCRECT;
        }

        // Set surface type based on scaling mode
        if (pSource->ScalingMode == VPHAL_SCALING_AVS || pSource->bIEF)
        {
            SurfaceParams.Type = pRenderHal->SurfaceTypeAdvanced;
            SurfaceParams.bAVS = true;
        }
        else
        {
            SurfaceParams.Type = pRenderHal->SurfaceTypeDefault;
            SurfaceParams.bAVS = false;
        }

        SurfaceParams.MemObjCtl = pHdrState->SurfMemObjCtl.SourceSurfMemObjCtl;
        iBTentry = pHdrState->uSourceBindingTableIndex[i];

        if (!Mos_ResourceIsNull(&pSource->OsResource))
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                pSource,
                pRenderHalSource,
                &SurfaceParams,
                pRenderData->iBindingTable,
                iBTentry,
                false));
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("Null resource found");
            eStatus = MOS_STATUS_NULL_POINTER;
            goto finish;
        }

        if (pHdrState->LUTMode[i] == VPHAL_HDR_LUT_MODE_2D)
        {
            MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
            SurfaceParams.Type          = pRenderHal->SurfaceTypeDefault;
            SurfaceParams.bRenderTarget = false;
            SurfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
            SurfaceParams.bWidth16Align = false;
            SurfaceParams.MemObjCtl     = pHdrState->SurfMemObjCtl.Lut2DSurfMemObjCtl;

            if (!Mos_ResourceIsNull(&pHdrState->OETF1DLUTSurface[i].OsResource))
            {
                VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                    pRenderHal,
                    &pHdrState->OETF1DLUTSurface[i],
                    &pHdrState->RenderHalOETF1DLUTSurface[i],
                    &SurfaceParams,
                    pRenderData->iBindingTable,
                    iBTentry + VPHAL_HDR_BTINDEX_OETF1DLUT_OFFSET_G9,
                    false));
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Null resource found");
                eStatus = MOS_STATUS_NULL_POINTER;
                goto finish;
            }
        }
    }

    for (i = 0; i < pHdrState->uTargetCount; i++)
    {
        if (i >= VPHAL_MAX_HDR_OUTPUT_LAYER)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        pTarget = pHdrState->pTargetSurf[i];
        pRenderHalTarget = &pHdrState->RenderHalTargetSurf[i];
        VPHAL_RENDER_CHK_NULL(pTarget);

        MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

        // Render target or private surface
        if (pTarget->SurfType == SURF_OUT_RENDERTARGET)
        {
            // Disable AVS, IEF
            pTarget->ScalingMode = VPHAL_SCALING_BILINEAR;
            pTarget->bIEF        = false;

            // Set flags for RT
            SurfaceParams.bRenderTarget    = true;
            SurfaceParams.bWidthInDword_Y  = true;
            SurfaceParams.bWidthInDword_UV = true;
            SurfaceParams.Boundary         = RENDERHAL_SS_BOUNDARY_DSTRECT;
        }
        // other surfaces
        else
        {
            SurfaceParams.bRenderTarget    = false;
            SurfaceParams.bWidthInDword_Y  = false;
            SurfaceParams.bWidthInDword_UV = false;
            SurfaceParams.Boundary         = RENDERHAL_SS_BOUNDARY_SRCRECT;
        }

        // Set surface type based on scaling mode
        if (pTarget->ScalingMode == VPHAL_SCALING_AVS || pTarget->bIEF)
        {
            SurfaceParams.Type = pRenderHal->SurfaceTypeAdvanced;
            SurfaceParams.bAVS = true;
        }
        else
        {
            SurfaceParams.Type = pRenderHal->SurfaceTypeDefault;
            SurfaceParams.bAVS = false;
        }

        SurfaceParams.MemObjCtl = pHdrState->SurfMemObjCtl.TargetSurfMemObjCtl;
        iBTentry = pHdrState->uTargetBindingTableIndex[i];

        if (!Mos_ResourceIsNull(&pTarget->OsResource))
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                pTarget,
                pRenderHalTarget,
                &SurfaceParams,
                pRenderData->iBindingTable,
                iBTentry,
                true));
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("Null resource found");
            eStatus = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }

    pSurfaceTemp = &pHdrState->CoeffSurface;
    pRenderHalSurfaceTemp = &pHdrState->RenderHalCoeffSurface;
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    SurfaceParams.Type = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.bRenderTarget = false;
    SurfaceParams.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParams.bWidth16Align = false;
    SurfaceParams.MemObjCtl = pHdrState->SurfMemObjCtl.CoeffSurfMemObjCtl;
    iBTentry = VPHAL_HDR_BTINDEX_COEFF_G9;

    if (!Mos_ResourceIsNull(&pSurfaceTemp->OsResource))
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
            pRenderHal,
            pSurfaceTemp,
            pRenderHalSurfaceTemp,
            &SurfaceParams,
            pRenderData->iBindingTable,
            iBTentry,
            false));
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Null resource found");
        eStatus = MOS_STATUS_NULL_POINTER;
        goto finish;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Set the Sampler States
//! \details  Set the Sampler States
//! \param    VPHAL_HDR_PHASE pHdrState
//!           [in] pointer to HDR State
//! \param    PVPHAL_HDR_RENDER_DATA
//!           [in] HDR render data
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetSamplerStates_g9 (
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData)
{
    MOS_STATUS                  eStatus             = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE        pRenderHal          = nullptr;
    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams = nullptr;
    uint32_t                    i                   = 0;

    VPHAL_PUBLIC_CHK_NULL(pHdrState);
    VPHAL_PUBLIC_CHK_NULL(pRenderData);

    pRenderHal = pHdrState->pRenderHal;
    VPHAL_PUBLIC_CHK_NULL(pRenderHal);

    for (i = 0; i < VPHAL_HDR_SAMPLER_STATE_NUM; i++)
    {
        pSamplerStateParams = &pRenderData->SamplerStateParams[i];

        switch (i)
        {
        case VPHAL_HDR_SAMPLER_STATE_3D_NEAREST_INDEX_G9:
            pSamplerStateParams->bInUse = true;
            pSamplerStateParams->SamplerType = MHW_SAMPLER_TYPE_3D;
            pSamplerStateParams->Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_NEAREST;
            pSamplerStateParams->Unorm.AddressU = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            pSamplerStateParams->Unorm.AddressV = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            pSamplerStateParams->Unorm.AddressW = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            break;
        case VPHAL_HDR_SAMPLER_STATE_3D_BILINEAR_INDEX_G9:
            pSamplerStateParams->bInUse = true;
            pSamplerStateParams->SamplerType = MHW_SAMPLER_TYPE_3D;
            pSamplerStateParams->Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_BILINEAR;
            pSamplerStateParams->Unorm.AddressU = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            pSamplerStateParams->Unorm.AddressV = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            pSamplerStateParams->Unorm.AddressW = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            break;
        case VPHAL_HDR_SAMPLER_STATE_AVS_NEAREST_INDEX_G9:
        case VPHAL_HDR_SAMPLER_STATE_AVS_POLYPHASE_INDEX_G9:
            pSamplerStateParams->bInUse        = true;
            pSamplerStateParams->SamplerType   = MHW_SAMPLER_TYPE_AVS;
            pSamplerStateParams->Avs.bHdcDwEnable  = true;
            pSamplerStateParams->Avs.b8TapAdaptiveEnable = false;
            pSamplerStateParams->Avs.bEnableAVS         = true;
            pSamplerStateParams->Avs.WeakEdgeThr        = DETAIL_WEAK_EDGE_THRESHOLD;
            pSamplerStateParams->Avs.StrongEdgeThr      = DETAIL_STRONG_EDGE_THRESHOLD;
            pSamplerStateParams->Avs.StrongEdgeWght     = DETAIL_STRONG_EDGE_WEIGHT;
            pSamplerStateParams->Avs.RegularWght        = DETAIL_REGULAR_EDGE_WEIGHT;
            pSamplerStateParams->Avs.NonEdgeWght        = DETAIL_NON_EDGE_WEIGHT;

            if (pHdrState->pSrcSurf[0]             &&
                pHdrState->pSrcSurf[0]->pIEFParams &&
                pRenderData->pIEFParams            &&
                pRenderData->pIEFParams->bEnabled)
            {
                pHdrState->pfnSetIefStates(pHdrState, pRenderData, pSamplerStateParams);
            }

            if (i == VPHAL_HDR_SAMPLER_STATE_AVS_NEAREST_INDEX_G9)
            {
                pSamplerStateParams->Unorm.SamplerFilterMode      = MHW_SAMPLER_FILTER_NEAREST;
                pSamplerStateParams->Avs.pMhwSamplerAvsTableParam = &pHdrState->mhwSamplerAvsTableParam[0];

                pHdrState->pfnSetSamplerAvsTable(
                    pRenderHal,
                    pSamplerStateParams,
                    pRenderData->pAVSParameters[0],
                    pRenderData->PrimaryLayerFormat,
                    1.0f,
                    1.0f,
                    CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP);
            }

            if (i == VPHAL_HDR_SAMPLER_STATE_AVS_POLYPHASE_INDEX_G9)
            {
                pSamplerStateParams->Avs.pMhwSamplerAvsTableParam = &pHdrState->mhwSamplerAvsTableParam[0];

                pHdrState->pfnSetSamplerAvsTable(
                    pRenderHal,
                    pSamplerStateParams,
                    pRenderData->pAVSParameters[1],
                    pRenderData->PrimaryLayerFormat,
                    pRenderData->fPrimaryLayerScaleX,
                    pRenderData->fPrimaryLayerScaleY,
                    CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP);
            }
        default:
            break;
        }
    }

    eStatus = pRenderHal->pfnSetSamplerStates(
        pRenderHal,
        pRenderData->iMediaID,
        &pRenderData->SamplerStateParams[0],
        VPHAL_HDR_SAMPLER_STATE_NUM);

finish:
    return eStatus;
}

//!
//! \brief    Setup HDR CURBE data
//! \details  Setup HDR CURBE data
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Poniter to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Poniter to HDR render data
//! \param    int32_t* piCurbeOffsetOut
//!           [Out] Curbe offset
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise
//!
MOS_STATUS VpHal_HdrLoadStaticData_g9(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    int32_t*                    piCurbeOffsetOut)
{
    MOS_STATUS                      eStatus             = MOS_STATUS_SUCCESS;
    MEDIA_WALKER_HDR_STATIC_DATA_G9 HDRStatic           = g_cInit_MEDIA_STATIC_HDR_g9;
    PRENDERHAL_INTERFACE            pRenderHal          = nullptr;
    PVPHAL_SURFACE                  pSource             = nullptr;
    VPHAL_HDR_FORMAT_DESCRIPTOR_G9  FormatDescriptor    = VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW_G9;
    VPHAL_HDR_ROTATION_G9           HdrRotation         = VPHAL_HDR_LAYER_ROTATION_0_G9;
    VPHAL_HDR_TWO_LAYER_OPTION_G9   HdrTwoLayerOp       = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND_G9;
    uint32_t                        ChromaSiting        = 0;
    bool                            bChannelSwap        = false;
    bool                            bBypassIEF          = true;
    bool                            b3dLut              = false;
    uint32_t                        uiSamplerStateIndex = 0;
    uint32_t                        uiSamplerStateIndex2= 0;
    uint16_t                        wAlpha              = 0;
    uint32_t                        i                   = 0;
    uint32_t                        dwDestRectWidth     = 0;
    uint32_t                        dwDestRectHeight    = 0;  // Target rectangle Width Height

    float                           fScaleX = 0.0f, fScaleY = 0.0f;         // x,y scaling factor
    float                           fStepX = 0.0f, fStepY = 0.0f;           // x,y scaling steps
    float                           fOriginX = 0.0f, fOriginY = 0.0f;       // x,y layer origin
    float                           fShiftX = 0.0f, fShiftY = 0.0f;         // x,y shift

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pHdrState->pRenderHal);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(piCurbeOffsetOut);

    pRenderHal    = pHdrState->pRenderHal;
    HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND_G9;
    HdrRotation   = VPHAL_HDR_LAYER_ROTATION_0_G9;
    wAlpha        = 0x0ff;
    uiSamplerStateIndex = uiSamplerStateIndex2 = 0;

    for (i = 0; i < pHdrState->uSourceCount; i++)
    {
        if (i >= VPHAL_MAX_HDR_INPUT_LAYER)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        pSource = pHdrState->pSrcSurf[i];
        VPHAL_RENDER_CHK_NULL(pSource);

        bChannelSwap  = false;
        bBypassIEF    = true;

        fShiftX = 0;
        fShiftY = 0;

        // Source rectangle is pre-rotated, destination rectangle is post-rotated.
        if (pSource->Rotation == VPHAL_ROTATION_IDENTITY    ||
            pSource->Rotation == VPHAL_ROTATION_180         ||
            pSource->Rotation == VPHAL_MIRROR_HORIZONTAL    ||
            pSource->Rotation == VPHAL_MIRROR_VERTICAL)
        {
            fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                           (float)(pSource->rcSrc.right  - pSource->rcSrc.left);
            fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                           (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
        }
        else
        {
            // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
            // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
            fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                           (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
            fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                           (float)(pSource->rcSrc.right  - pSource->rcSrc.left);
        }

        if (fScaleX == 1.0f && fScaleY == 1.0f && pSource->ScalingMode == VPHAL_SCALING_BILINEAR)
        {
            pSource->ScalingMode = VPHAL_SCALING_NEAREST;
        }

        if (pSource->ScalingMode == VPHAL_SCALING_AVS)
        {
            uiSamplerStateIndex = VPHAL_HDR_AVS_SAMPLER_STATE_ADAPTIVE;

            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                uiSamplerStateIndex2 = VPHAL_HDR_AVS_SAMPLER_STATE_ADAPTIVE;
            }
        }
        else if (pSource->ScalingMode == VPHAL_SCALING_BILINEAR)
        {
            uiSamplerStateIndex = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;

            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                uiSamplerStateIndex2 = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;
            }

            fShiftX = VPHAL_HW_LINEAR_SHIFT;
            fShiftY = VPHAL_HW_LINEAR_SHIFT;
        }
        else
        {
            uiSamplerStateIndex = VPHAL_HDR_3D_SAMPLER_STATE_NEAREST;

            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                uiSamplerStateIndex2 = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;
            }

            fShiftX = VPHAL_HW_LINEAR_SHIFT;
            fShiftY = VPHAL_HW_LINEAR_SHIFT;
        }

        // Normalize source co-ordinates using the width and height programmed
        // in surface state. step X, Y pre-rotated
        // Source rectangle is pre-rotated, destination rectangle is post-rotated.
        if (pSource->Rotation == VPHAL_ROTATION_IDENTITY    ||
            pSource->Rotation == VPHAL_ROTATION_180         ||
            pSource->Rotation == VPHAL_MIRROR_HORIZONTAL    ||
            pSource->Rotation == VPHAL_MIRROR_VERTICAL)
        {
            fStepX = ((pSource->rcSrc.right - pSource->rcSrc.left) * 1.0f) / 
                      ((pSource->rcDst.right - pSource->rcDst.left) > 0 ? 
                       (pSource->rcDst.right - pSource->rcDst.left) : 1);
            fStepY = ((float)(pSource->rcSrc.bottom - pSource->rcSrc.top)) / 
                      ((pSource->rcDst.bottom - pSource->rcDst.top) > 0 ? 
                       (float)(pSource->rcDst.bottom - pSource->rcDst.top) : 1.0f);
        }
        else
        {
            // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 || 
            // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
            fStepX = ((pSource->rcSrc.right - pSource->rcSrc.left) * 1.0f) / 
                      ((pSource->rcDst.bottom - pSource->rcDst.top) > 0 ? 
                       (pSource->rcDst.bottom - pSource->rcDst.top) : 1);
            fStepY = ((float)(pSource->rcSrc.bottom - pSource->rcSrc.top)) / 
                      ((pSource->rcDst.right - pSource->rcDst.left) > 0 ? 
                       (float)(pSource->rcDst.right - pSource->rcDst.left) : 1.0f);
        }

        dwDestRectWidth  = pHdrState->pTargetSurf[0]->dwWidth;
        dwDestRectHeight = pHdrState->pTargetSurf[0]->dwHeight;

        switch (pSource->Rotation)
        {
            case VPHAL_ROTATION_IDENTITY:
                // Coordinate adjustment for render target coordinates (0,0)
                fShiftX  -= pSource->rcDst.left;
                fShiftY  -= pSource->rcDst.top;
                break;
            case VPHAL_ROTATION_90:
                // Coordinate adjustment for 90 degree rotation
                fShiftX  -= (float)pSource->rcDst.top;
                fShiftY  -= (float)dwDestRectWidth - 
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleX - 
                            (float)pSource->rcDst.left;
                break;
            case VPHAL_ROTATION_180:
                // Coordinate adjustment for 180 degree rotation
                fShiftX  -= (float)dwDestRectWidth - 
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleX - 
                            (float)pSource->rcDst.left;
                fShiftY  -= (float)dwDestRectHeight - 
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleY - 
                            (float)pSource->rcDst.top;
                break;
            case VPHAL_ROTATION_270:
                // Coordinate adjustment for 270 degree rotation
                fShiftX  -= (float)dwDestRectHeight - 
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleY - 
                            (float)pSource->rcDst.top;
                fShiftY  -= (float)pSource->rcDst.left;
                break;
            case VPHAL_MIRROR_HORIZONTAL:
                // Coordinate adjustment for horizontal mirroring
                fShiftX  -= (float)dwDestRectWidth - 
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleX - 
                            (float)pSource->rcDst.left;
                fShiftY  -= pSource->rcDst.top;
                break;
            case VPHAL_MIRROR_VERTICAL:
                // Coordinate adjustment for vertical mirroring
                fShiftX  -= pSource->rcDst.left;
                fShiftY  -= (float)dwDestRectHeight - 
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleY - 
                            (float)pSource->rcDst.top;
                break;
            case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
                // Coordinate adjustment for rotating 90 and horizontal mirroring
                fShiftX  -= (float)pSource->rcDst.top;
                fShiftY  -= (float)pSource->rcDst.left;
                break;
            case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            default:
                // Coordinate adjustment for rotating 90 and vertical mirroring
                fShiftX  -= (float)dwDestRectHeight - 
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleY - 
                            (float)pSource->rcDst.top;
                fShiftY  -= (float)dwDestRectWidth - 
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleX - 
                            (float)pSource->rcDst.left;
                break;
        } // switch

        fOriginX = ((float) pSource->rcSrc.left + fShiftX * fStepX) / (float)MOS_MIN(pSource->dwWidth, (uint32_t)pSource->rcSrc.right);
        fOriginY = ((float) pSource->rcSrc.top + fShiftY * fStepY) / (float)MOS_MIN(pSource->dwHeight, (uint32_t)pSource->rcSrc.bottom);

        fStepX /= MOS_MIN(pSource->dwWidth, (uint32_t)pSource->rcSrc.right);
        fStepY /= MOS_MIN(pSource->dwHeight, (uint32_t)pSource->rcSrc.bottom);

        FormatDescriptor = VpHal_HdrGetFormatDescriptor_g9(pSource->Format);

        if (FormatDescriptor == VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW_G9)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Unsupported hdr input format");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        // Chroma siting setting in curbe data will only take effect in 1x scaling case when 3D sampler is being used
        // For other cases, Chroma siting will be performed by AVS coefficients, and the here will be ignored by kernel
        ChromaSiting = VpHal_HdrGetHdrChromaSiting_g9(pSource->ChromaSiting);

        if (pSource->Format == Format_B10G10R10A2 ||
            pSource->Format == Format_A8R8G8B8    ||
            pSource->Format == Format_X8R8G8B8    ||
            pSource->Format == Format_A16R16G16B16F)
        {
            bChannelSwap = true;
        }

        if (pSource->pIEFParams)
        {
            if (pSource->pIEFParams->bEnabled)
            {
                bBypassIEF = false;
            }
        }

        HdrRotation = VpHal_HdrGetHdrRotation_g9(pSource->Rotation);

        b3dLut = (pHdrState->LUTMode[i] == VPHAL_HDR_LUT_MODE_3D ? true : false);

        if (pSource->SurfType == SURF_IN_PRIMARY)
        {
            if (b3dLut)
            {
                pHdrState->Reporting.HDRMode = (VPHAL_HDR_MODE)(((uint32_t)pHdrState->Reporting.HDRMode) | VPHAL_HDR_MODE_3DLUT_MASK);
            }
        }

        switch (i)
        {
        case 0:
            HDRStatic.DW0.HorizontalFrameOriginLayer0       = fOriginX;
            HDRStatic.DW8.VerticalFrameOriginLayer0         = fOriginY;
            HDRStatic.DW16.HorizontalScalingStepRatioLayer0 = fStepX;
            HDRStatic.DW24.VerticalScalingStepRatioLayer0   = fStepY;
            HDRStatic.DW32.LeftCoordinateRectangleLayer0    = pSource->rcDst.left;
            HDRStatic.DW32.TopCoordinateRectangleLayer0     = pSource->rcDst.top;
            HDRStatic.DW40.RightCoordinateRectangleLayer0   = pSource->rcDst.right - 1;
            HDRStatic.DW40.BottomCoordinateRectangleLayer0  = pSource->rcDst.bottom - 1;
            HDRStatic.DW48.FormatDescriptorLayer0           = FormatDescriptor;
            HDRStatic.DW48.ChromaSittingLocationLayer0      = ChromaSiting;
            HDRStatic.DW48.ChannelSwapEnablingFlagLayer0    = bChannelSwap;
            HDRStatic.DW48.IEFBypassEnablingFlagLayer0      = bBypassIEF;
            HDRStatic.DW48.RotationAngleMirrorDirectionLayer0 = HdrRotation;
            HDRStatic.DW48.SamplerIndexFirstPlaneLayer0       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW48.SamplerIndexSecondThirdPlaneLayer0 = uiSamplerStateIndex2;
            }
            HDRStatic.DW48.CCMExtensionEnablingFlagLayer0     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW48.ToneMappingEnablingFlagLayer0      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW48.PriorCSCEnablingFlagLayer0         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW48.EOTF1DLUTEnablingFlagLayer0        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW48.CCMEnablingFlagLayer0              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW48.OETF1DLUTEnablingFlagLayer0        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW48.PostCSCEnablingFlagLayer0          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW48.Enabling3DLUTFlagLayer0            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW56.ConstantBlendingAlphaFillColorLayer0 = wAlpha;
            }
            HDRStatic.DW58.TwoLayerOperationLayer0              = VPHAL_HDR_TWO_LAYER_OPTION_COMP_G9;
            if (pSource->SurfType == SURF_IN_PRIMARY                &&
                pSource->pBlendingParams                            &&
                pSource->pBlendingParams->BlendType == BLEND_SOURCE &&
                (IS_RGB_CSPACE(pSource->ColorSpace) || IS_COLOR_SPACE_BT2020_RGB(pSource->ColorSpace)))
            {
                // For PDVD alpha blending issue:
                // If first frame is sRGB/stRGB/BT2020RGB format delivered as primary layer
                // (which means main video content is in the first layer)
                // and blend-type set as source blending,
                // do source blending w/ bg, instead of setting as composite.
                HDRStatic.DW58.TwoLayerOperationLayer0 = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND_G9;
            }
            break;
        case 1:
            HDRStatic.DW1.HorizontalFrameOriginLayer1       = fOriginX;
            HDRStatic.DW9.VerticalFrameOriginLayer1         = fOriginY;
            HDRStatic.DW17.HorizontalScalingStepRatioLayer1 = fStepX;
            HDRStatic.DW25.VerticalScalingStepRatioLayer1   = fStepY;
            HDRStatic.DW33.LeftCoordinateRectangleLayer1    = pSource->rcDst.left;
            HDRStatic.DW33.TopCoordinateRectangleLayer1     = pSource->rcDst.top;
            HDRStatic.DW41.RightCoordinateRectangleLayer1   = pSource->rcDst.right - 1;
            HDRStatic.DW41.BottomCoordinateRectangleLayer1  = pSource->rcDst.bottom - 1;
            HDRStatic.DW49.FormatDescriptorLayer1           = FormatDescriptor;
            HDRStatic.DW49.ChromaSittingLocationLayer1      = ChromaSiting;
            HDRStatic.DW49.ChannelSwapEnablingFlagLayer1    = bChannelSwap;
            HDRStatic.DW49.IEFBypassEnablingFlagLayer1      = bBypassIEF;
            HDRStatic.DW49.RotationAngleMirrorDirectionLayer1 = HdrRotation;
            HDRStatic.DW49.SamplerIndexFirstPlaneLayer1       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW49.SamplerIndexSecondThirdPlaneLayer1 = uiSamplerStateIndex2;
            }
            HDRStatic.DW49.CCMExtensionEnablingFlagLayer1     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW49.ToneMappingEnablingFlagLayer1      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW49.PriorCSCEnablingFlagLayer1         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW49.EOTF1DLUTEnablingFlagLayer1        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW49.CCMEnablingFlagLayer1              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW49.OETF1DLUTEnablingFlagLayer1        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW49.PostCSCEnablingFlagLayer1          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW49.Enabling3DLUTFlagLayer1            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW56.ConstantBlendingAlphaFillColorLayer1 = wAlpha;
            }
            HDRStatic.DW58.TwoLayerOperationLayer1              = HdrTwoLayerOp;
            break;
        case 2:
            HDRStatic.DW2.HorizontalFrameOriginLayer2       = fOriginX;
            HDRStatic.DW10.VerticalFrameOriginLayer2        = fOriginY;
            HDRStatic.DW18.HorizontalScalingStepRatioLayer2 = fStepX;
            HDRStatic.DW26.VerticalScalingStepRatioLayer2   = fStepY;
            HDRStatic.DW34.LeftCoordinateRectangleLayer2    = pSource->rcDst.left;
            HDRStatic.DW34.TopCoordinateRectangleLayer2     = pSource->rcDst.top;
            HDRStatic.DW42.RightCoordinateRectangleLayer2   = pSource->rcDst.right - 1;
            HDRStatic.DW42.BottomCoordinateRectangleLayer2  = pSource->rcDst.bottom - 1;
            HDRStatic.DW50.FormatDescriptorLayer2           = FormatDescriptor;
            HDRStatic.DW50.ChromaSittingLocationLayer2      = ChromaSiting;
            HDRStatic.DW50.ChannelSwapEnablingFlagLayer2    = bChannelSwap;
            HDRStatic.DW50.IEFBypassEnablingFlagLayer2      = bBypassIEF;
            HDRStatic.DW50.RotationAngleMirrorDirectionLayer2 = HdrRotation;
            HDRStatic.DW50.SamplerIndexFirstPlaneLayer2       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW50.SamplerIndexSecondThirdPlaneLayer2 = uiSamplerStateIndex2;
            }
            HDRStatic.DW50.CCMExtensionEnablingFlagLayer2     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW50.ToneMappingEnablingFlagLayer2      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW50.PriorCSCEnablingFlagLayer2         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW50.EOTF1DLUTEnablingFlagLayer2        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW50.CCMEnablingFlagLayer2              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW50.OETF1DLUTEnablingFlagLayer2        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW50.PostCSCEnablingFlagLayer2          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW50.Enabling3DLUTFlagLayer2            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW56.ConstantBlendingAlphaFillColorLayer2 = wAlpha;
            }
            HDRStatic.DW58.TwoLayerOperationLayer2              = HdrTwoLayerOp;
            break;
        case 3:
            HDRStatic.DW3.HorizontalFrameOriginLayer3       = fOriginX;
            HDRStatic.DW11.VerticalFrameOriginLayer3        = fOriginY;
            HDRStatic.DW19.HorizontalScalingStepRatioLayer3 = fStepX;
            HDRStatic.DW27.VerticalScalingStepRatioLayer3   = fStepY;
            HDRStatic.DW35.LeftCoordinateRectangleLayer3    = pSource->rcDst.left;
            HDRStatic.DW35.TopCoordinateRectangleLayer3     = pSource->rcDst.top;
            HDRStatic.DW43.RightCoordinateRectangleLayer3   = pSource->rcDst.right - 1;
            HDRStatic.DW43.BottomCoordinateRectangleLayer3  = pSource->rcDst.bottom - 1;
            HDRStatic.DW51.FormatDescriptorLayer3           = FormatDescriptor;
            HDRStatic.DW51.ChromaSittingLocationLayer3      = ChromaSiting;
            HDRStatic.DW51.ChannelSwapEnablingFlagLayer3    = bChannelSwap;
            HDRStatic.DW51.IEFBypassEnablingFlagLayer3      = bBypassIEF;
            HDRStatic.DW51.RotationAngleMirrorDirectionLayer3 = HdrRotation;
            HDRStatic.DW51.SamplerIndexFirstPlaneLayer3       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW51.SamplerIndexSecondThirdPlaneLayer3 = uiSamplerStateIndex2;
            }
            HDRStatic.DW51.CCMExtensionEnablingFlagLayer3     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW51.ToneMappingEnablingFlagLayer3      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW51.PriorCSCEnablingFlagLayer3         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW51.EOTF1DLUTEnablingFlagLayer3        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW51.CCMEnablingFlagLayer3              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW51.OETF1DLUTEnablingFlagLayer3        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW51.PostCSCEnablingFlagLayer3          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW51.Enabling3DLUTFlagLayer3            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW56.ConstantBlendingAlphaFillColorLayer3 = wAlpha;
            }
            HDRStatic.DW58.TwoLayerOperationLayer3              = HdrTwoLayerOp;
            break;
        case 4:
            HDRStatic.DW4.HorizontalFrameOriginLayer4       = fOriginX;
            HDRStatic.DW12.VerticalFrameOriginLayer4        = fOriginY;
            HDRStatic.DW20.HorizontalScalingStepRatioLayer4 = fStepX;
            HDRStatic.DW28.VerticalScalingStepRatioLayer4   = fStepY;
            HDRStatic.DW36.LeftCoordinateRectangleLayer4    = pSource->rcDst.left;
            HDRStatic.DW36.TopCoordinateRectangleLayer4     = pSource->rcDst.top;
            HDRStatic.DW44.RightCoordinateRectangleLayer4   = pSource->rcDst.right - 1;
            HDRStatic.DW44.BottomCoordinateRectangleLayer4  = pSource->rcDst.bottom - 1;
            HDRStatic.DW52.FormatDescriptorLayer4           = FormatDescriptor;
            HDRStatic.DW52.ChromaSittingLocationLayer4      = ChromaSiting;
            HDRStatic.DW52.ChannelSwapEnablingFlagLayer4    = bChannelSwap;
            HDRStatic.DW52.IEFBypassEnablingFlagLayer4      = bBypassIEF;
            HDRStatic.DW52.RotationAngleMirrorDirectionLayer4 = HdrRotation;
            HDRStatic.DW52.SamplerIndexFirstPlaneLayer4       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW52.SamplerIndexSecondThirdPlaneLayer4 = uiSamplerStateIndex2;
            }
            HDRStatic.DW52.CCMExtensionEnablingFlagLayer4     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW52.ToneMappingEnablingFlagLayer4      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW52.PriorCSCEnablingFlagLayer4         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW52.EOTF1DLUTEnablingFlagLayer4        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW52.CCMEnablingFlagLayer4              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW52.OETF1DLUTEnablingFlagLayer4        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW52.PostCSCEnablingFlagLayer4          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW52.Enabling3DLUTFlagLayer4            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW57.ConstantBlendingAlphaFillColorLayer4 = wAlpha;
            }
            HDRStatic.DW59.TwoLayerOperationLayer4              = HdrTwoLayerOp;
            break;
        case 5:
            HDRStatic.DW5.HorizontalFrameOriginLayer5       = fOriginX;
            HDRStatic.DW13.VerticalFrameOriginLayer5        = fOriginY;
            HDRStatic.DW21.HorizontalScalingStepRatioLayer5 = fStepX;
            HDRStatic.DW29.VerticalScalingStepRatioLayer5   = fStepY;
            HDRStatic.DW37.LeftCoordinateRectangleLayer5    = pSource->rcDst.left;
            HDRStatic.DW37.TopCoordinateRectangleLayer5     = pSource->rcDst.top;
            HDRStatic.DW45.RightCoordinateRectangleLayer5   = pSource->rcDst.right - 1;
            HDRStatic.DW45.BottomCoordinateRectangleLayer5  = pSource->rcDst.bottom - 1;
            HDRStatic.DW53.FormatDescriptorLayer5           = FormatDescriptor;
            HDRStatic.DW53.ChromaSittingLocationLayer5      = ChromaSiting;
            HDRStatic.DW53.ChannelSwapEnablingFlagLayer5    = bChannelSwap;
            HDRStatic.DW53.IEFBypassEnablingFlagLayer5      = bBypassIEF;
            HDRStatic.DW53.RotationAngleMirrorDirectionLayer5 = HdrRotation;
            HDRStatic.DW53.SamplerIndexFirstPlaneLayer5       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW53.SamplerIndexSecondThirdPlaneLayer5 = uiSamplerStateIndex2;
            }
            HDRStatic.DW53.CCMExtensionEnablingFlagLayer5     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW53.ToneMappingEnablingFlagLayer5      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW53.PriorCSCEnablingFlagLayer5         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW53.EOTF1DLUTEnablingFlagLayer5        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW53.CCMEnablingFlagLayer5              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW53.OETF1DLUTEnablingFlagLayer5        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW53.PostCSCEnablingFlagLayer5          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW53.Enabling3DLUTFlagLayer5            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW57.ConstantBlendingAlphaFillColorLayer5 = wAlpha;
            }
            HDRStatic.DW59.TwoLayerOperationLayer5              = HdrTwoLayerOp;
            break;
        case 6:
            HDRStatic.DW6.HorizontalFrameOriginLayer6       = fOriginX;
            HDRStatic.DW14.VerticalFrameOriginLayer6        = fOriginY;
            HDRStatic.DW22.HorizontalScalingStepRatioLayer6 = fStepX;
            HDRStatic.DW30.VerticalScalingStepRatioLayer6   = fStepY;
            HDRStatic.DW38.LeftCoordinateRectangleLayer6    = pSource->rcDst.left;
            HDRStatic.DW38.TopCoordinateRectangleLayer6     = pSource->rcDst.top;
            HDRStatic.DW46.RightCoordinateRectangleLayer6   = pSource->rcDst.right - 1;
            HDRStatic.DW46.BottomCoordinateRectangleLayer6  = pSource->rcDst.bottom - 1;
            HDRStatic.DW54.FormatDescriptorLayer6           = FormatDescriptor;
            HDRStatic.DW54.ChromaSittingLocationLayer6      = ChromaSiting;
            HDRStatic.DW54.ChannelSwapEnablingFlagLayer6    = bChannelSwap;
            HDRStatic.DW54.IEFBypassEnablingFlagLayer6      = bBypassIEF;
            HDRStatic.DW54.RotationAngleMirrorDirectionLayer6 = HdrRotation;
            HDRStatic.DW54.SamplerIndexFirstPlaneLayer6       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW54.SamplerIndexSecondThirdPlaneLayer6 = uiSamplerStateIndex2;
            }
            HDRStatic.DW54.CCMExtensionEnablingFlagLayer6     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW54.ToneMappingEnablingFlagLayer6      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW54.PriorCSCEnablingFlagLayer6         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW54.EOTF1DLUTEnablingFlagLayer6        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW54.CCMEnablingFlagLayer6              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW54.OETF1DLUTEnablingFlagLayer6        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW54.PostCSCEnablingFlagLayer6          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW54.Enabling3DLUTFlagLayer6            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW57.ConstantBlendingAlphaFillColorLayer6 = wAlpha;
            }
            HDRStatic.DW59.TwoLayerOperationLayer6              = HdrTwoLayerOp;
            break;
        case 7:
            HDRStatic.DW7.HorizontalFrameOriginLayer7       = fOriginX;
            HDRStatic.DW15.VerticalFrameOriginLayer7        = fOriginY;
            HDRStatic.DW23.HorizontalScalingStepRatioLayer7 = fStepX;
            HDRStatic.DW31.VerticalScalingStepRatioLayer7   = fStepY;
            HDRStatic.DW39.LeftCoordinateRectangleLayer7    = pSource->rcDst.left;
            HDRStatic.DW39.TopCoordinateRectangleLayer7     = pSource->rcDst.top;
            HDRStatic.DW47.RightCoordinateRectangleLayer7   = pSource->rcDst.right - 1;
            HDRStatic.DW47.BottomCoordinateRectangleLayer7  = pSource->rcDst.bottom - 1;
            HDRStatic.DW55.FormatDescriptorLayer7           = FormatDescriptor;
            HDRStatic.DW55.ChromaSittingLocationLayer7      = ChromaSiting;
            HDRStatic.DW55.ChannelSwapEnablingFlagLayer7    = bChannelSwap;
            HDRStatic.DW55.IEFBypassEnablingFlagLayer7      = bBypassIEF;
            HDRStatic.DW55.RotationAngleMirrorDirectionLayer7 = HdrRotation;
            HDRStatic.DW55.SamplerIndexFirstPlaneLayer7       = uiSamplerStateIndex;
            if (pSource->Format == Format_P010 ||
                pSource->Format == Format_P016)
            {
                HDRStatic.DW55.SamplerIndexSecondThirdPlaneLayer7 = uiSamplerStateIndex2;
            }
            HDRStatic.DW55.CCMExtensionEnablingFlagLayer7     = pHdrState->StageEnableFlags[i].CCMExt1Enable ||
                                                                pHdrState->StageEnableFlags[i].CCMExt2Enable;
            HDRStatic.DW55.ToneMappingEnablingFlagLayer7      = pHdrState->StageEnableFlags[i].PWLFEnable;
            HDRStatic.DW55.PriorCSCEnablingFlagLayer7         = pHdrState->StageEnableFlags[i].PriorCSCEnable;
            HDRStatic.DW55.EOTF1DLUTEnablingFlagLayer7        = pHdrState->StageEnableFlags[i].EOTFEnable;
            HDRStatic.DW55.CCMEnablingFlagLayer7              = pHdrState->StageEnableFlags[i].CCMEnable;
            HDRStatic.DW55.OETF1DLUTEnablingFlagLayer7        = pHdrState->StageEnableFlags[i].OETFEnable;
            HDRStatic.DW55.PostCSCEnablingFlagLayer7          = pHdrState->StageEnableFlags[i].PostCSCEnable;
            HDRStatic.DW55.Enabling3DLUTFlagLayer7            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9  ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9 ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9)
            {
                HDRStatic.DW57.ConstantBlendingAlphaFillColorLayer7 = wAlpha;
            }
            HDRStatic.DW59.TwoLayerOperationLayer7              = HdrTwoLayerOp;
            break;
        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid input layer number.");
            break;
        }
    }

    FormatDescriptor = VpHal_HdrGetFormatDescriptor_g9(pHdrState->pTargetSurf[0]->Format);
    ChromaSiting     = VpHal_HdrGetHdrChromaSiting_g9(pHdrState->pTargetSurf[0]->ChromaSiting);

    if (pHdrState->pTargetSurf[0]->Format == Format_B10G10R10A2 ||
        pHdrState->pTargetSurf[0]->Format == Format_A8R8G8B8    ||
        pHdrState->pTargetSurf[0]->Format == Format_X8R8G8B8    ||
        pHdrState->pTargetSurf[0]->Format == Format_A16R16G16B16F)
    {
        bChannelSwap = true;
    }
    else
    {
        bChannelSwap = false;
    }

    HDRStatic.DW62.DestinationWidth                 = pHdrState->pTargetSurf[0]->dwWidth;
    HDRStatic.DW62.DestinationHeight                = pHdrState->pTargetSurf[0]->dwHeight;
    HDRStatic.DW63.TotalNumberInputLayers           = pHdrState->uSourceCount;

    if (0 == pHdrState->uSourceCount)
    {
        HDRStatic.DW32.LeftCoordinateRectangleLayer0    = pHdrState->pTargetSurf[0]->dwWidth  + 16;
        HDRStatic.DW32.TopCoordinateRectangleLayer0     = pHdrState->pTargetSurf[0]->dwHeight + 16;
        HDRStatic.DW40.RightCoordinateRectangleLayer0   = pHdrState->pTargetSurf[0]->dwWidth  + 16;
        HDRStatic.DW40.BottomCoordinateRectangleLayer0  = pHdrState->pTargetSurf[0]->dwHeight + 16;
        HDRStatic.DW58.TwoLayerOperationLayer0          = VPHAL_HDR_TWO_LAYER_OPTION_COMP_G9;
    }

    HDRStatic.DW63.FormatDescriptorDestination        = FormatDescriptor;
    HDRStatic.DW63.ChromaSittingLocationDestination   = ChromaSiting;
    HDRStatic.DW63.ChannelSwapEnablingFlagDestination = bChannelSwap;

    // Set Background color (use cspace of first layer)
    if (pHdrState->pColorFillParams)
    {
        VPHAL_COLOR_SAMPLE_8 Src, Dst;
        VPHAL_CSPACE         src_cspace, dst_cspace;

        Src.dwValue = pHdrState->pColorFillParams->Color;

        // get src and dst colorspaces
        src_cspace = pHdrState->pColorFillParams->CSpace;
        dst_cspace = pHdrState->pTargetSurf[0]->ColorSpace;

        // Convert BG color only if not done so before. CSC is expensive!
        if (VpHal_CSC_8(&Dst, &Src, src_cspace, dst_cspace))
        {
            HDRStatic.DW60.FixedPointFillColorRVChannel     = Dst.R << 8;
            HDRStatic.DW60.FixedPointFillColorGYChannel     = Dst.G << 8;
            HDRStatic.DW61.FixedPointFillColorBUChannel     = Dst.B << 8;
            HDRStatic.DW61.FixedPointFillColorAlphaChannel  = Dst.A << 8;
        }
        else
        {
            HDRStatic.DW60.FixedPointFillColorRVChannel     = Src.R << 8;
            HDRStatic.DW60.FixedPointFillColorGYChannel     = Src.G << 8;
            HDRStatic.DW61.FixedPointFillColorBUChannel     = Src.B << 8;
            HDRStatic.DW61.FixedPointFillColorAlphaChannel  = Src.A << 8;
        }
    }

    *piCurbeOffsetOut = pRenderHal->pfnLoadCurbeData(
        pRenderHal,
        pRenderData->pMediaState,
        &HDRStatic,
        sizeof(MEDIA_WALKER_HDR_STATIC_DATA_G9));

    if (*piCurbeOffsetOut < 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }
    pRenderData->iCurbeOffset = *piCurbeOffsetOut;

finish:
    return eStatus;
}

//!
//! \brief    Get Hdr iTouch Split Frame Portion number
//! \details  Get Hdr iTouch Split Frame Portion number is Gen9 platform specific
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in/out] Pointer to HDR state
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrGetSplitFramePortion_g9(
    PVPHAL_HDR_STATE        pHdrState)
{
    MOS_STATUS eStatus;
    uint32_t   dwPixels;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pHdrState->pTargetSurf[0]);

    dwPixels = pHdrState->pTargetSurf[0]->dwWidth * pHdrState->pTargetSurf[0]->dwHeight;
    //pHdrState->uiPortions = MOS_ROUNDUP_DIVIDE(dwPixels, g_Hdr_iTouc_Pixel_Throughput_g9[pHdrState->uSourceCount - 1]);
   
    if (dwPixels <= 1920 * 1080)
    {
        pHdrState->uiSplitFramePortions = 2;
    }
    else if (dwPixels <= 3840 * 2160)
    {
        pHdrState->uiSplitFramePortions = 4;
    }
    else
    {
        pHdrState->uiSplitFramePortions = 8;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Initializes interface for HDR
//! \details  Initializes interface for HDR which is Gen9 platform specific
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrInitInterface_g9(
    PVPHAL_HDR_STATE        pHdrState)
{
    MOS_STATUS eStatus      = MOS_STATUS_SUCCESS;
    int32_t    i            = 0;

    VPHAL_RENDER_CHK_NULL(pHdrState);

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++)
    {
        pHdrState->uSourceBindingTableIndex[i] = VPHAL_HDR_BTINDEX_LAYER0_G9 + i * VPHAL_HDR_BTINDEX_PER_LAYER0_G9;
    }

    for (i = 0; i < VPHAL_MAX_HDR_OUTPUT_LAYER; i++)
    {
        pHdrState->uTargetBindingTableIndex[i] = VPHAL_HDR_BTINDEX_RENDERTARGET_G9 + i * VPHAL_HDR_BTINDEX_PER_TARGET_G9;
    }

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++)
    {
        pHdrState->LUTMode[i]   = VPHAL_HDR_LUT_MODE_NONE;
        pHdrState->EOTFGamma[i] = VPHAL_GAMMA_NONE;
        pHdrState->OETFGamma[i] = VPHAL_GAMMA_NONE;
        pHdrState->CCM[i]       = VPHAL_HDR_CCM_NONE;
        pHdrState->CCMExt1[i]   = VPHAL_HDR_CCM_NONE;
        pHdrState->CCMExt2[i]   = VPHAL_HDR_CCM_NONE;
        pHdrState->HdrMode[i]   = VPHAL_HDR_MODE_NONE;
        pHdrState->PriorCSC[i]  = VPHAL_HDR_CSC_NONE;
        pHdrState->PostCSC[i]   = VPHAL_HDR_CSC_NONE;
    }

    // Allocate AVS parameters
    VpHal_RenderInitAVSParams(&pHdrState->AVSParameters[0],
                              POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9,
                              POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9);

    VpHal_RenderInitAVSParams(&pHdrState->AVSParameters[1],
                              POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9,
                              POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9);

    pHdrState->dwOetfSurfaceWidth         = VPHAL_HDR_OETF_1DLUT_WIDTH;
    pHdrState->dwOetfSurfaceWidth         = VPHAL_HDR_OETF_1DLUT_HEIGHT;
    pHdrState->pKernelParamTable          = (PRENDERHAL_KERNEL_PARAM)g_Hdr_KernelParam_g9;
    pHdrState->pfnAllocateResources       = VpHal_HdrAllocateResources_g9;
    pHdrState->pfnSetupSurfaceStates      = VpHal_HdrSetupSurfaceStates_g9;
    pHdrState->pfnIsInputFormatSupported  = VpHal_HdrIsInputFormatSupported_g9;
    pHdrState->pfnIsOutputFormatSupported = VpHal_HdrIsOutputFormatSupported_g9;
    pHdrState->pfnLoadStaticData          = VpHal_HdrLoadStaticData_g9;
    pHdrState->pfnGetKernelParam          = VpHal_HdrGetKernelParam_g9;
    pHdrState->pfnInitOETF1DLUT           = VpHal_HdrInitOETF1DLUT_g9;
    pHdrState->pfnInitCoeff               = VpHal_HdrInitCoeff_g9;
    pHdrState->pfnSetSamplerStates        = VpHal_HdrSetSamplerStates_g9;
    pHdrState->pfnSetIefStates            = VpHal_HdrSetIefStates_g9;
    pHdrState->pfnSetSamplerAvsTable      = VpHal_HdrSetSamplerAvsTableParam_g9;
    pHdrState->pfnFreeResources           = VpHal_HdrFreeResources_g9;
    pHdrState->pfnGetSplitFramePortion    = VpHal_HdrGetSplitFramePortion_g9;

    pHdrState->pfnSetupPreSurfaceStates = VpHal_HdrSetupPreProcessSurfaceStates_g9;
    pHdrState->pfnLoadPreStaticData     = VpHal_HdrPreprocessLoadStaticData_g9;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Destroy interface for HDR
//! \details  Destroy interface for HDR which is Gen9 platform specific
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \return   MOS_STATUS
//!

MOS_STATUS VpHal_HdrDestroyInterface_g9(
    PVPHAL_HDR_STATE        pHdrState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VPHAL_RENDER_CHK_NULL(pHdrState);

    VpHal_RenderDestroyAVSParams(&pHdrState->AVSParameters[0]);
    VpHal_RenderDestroyAVSParams(&pHdrState->AVSParameters[1]);

finish:
    return eStatus;
}

//!
//! \brief    Set HDR Ief State
//! \details  Set HDR Ief State
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to render data
//! \param    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams
//!           [in] Pointer to Sampler State Parameters
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrSetIefStates_g9(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pSamplerStateParams);
    VPHAL_RENDER_CHK_NULL(pRenderData);

    MOS_UNUSED(pRenderData);
    {
        Ief ief(pHdrState->pSrcSurf[0]);
        VPHAL_RENDER_CHK_STATUS(ief.SetHwState(pSamplerStateParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Initiate EOTF Surface for HDR
//! \details  Initiate EOTF Surface for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \param    int32_t iIndex
//!           [in] input surface index
//! \param    PVPHAL_SURFACE pOETF1DLUTSurface
//!           [in] Pointer to OETF 1D LUT Surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrInitOETF1DLUT_g9(
    PVPHAL_HDR_STATE pHdrState,
    int32_t               iIndex,
    PVPHAL_SURFACE        pOETF1DLUTSurface)
{
    MOS_STATUS       eStatus            = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE   pOsInterface       = nullptr;
    uint32_t         i                  = 0;
    uint16_t         *pSrcOetfLut       = nullptr;
    uint8_t          *pDstOetfLut       = nullptr;
    MOS_LOCK_PARAMS  LockFlags          = {};
    PVPHAL_SURFACE   pTargetSurf        = (PVPHAL_SURFACE)pHdrState->pTargetSurf[0];

    VPHAL_PUBLIC_CHK_NULL(pHdrState);
    VPHAL_PUBLIC_CHK_NULL(pOETF1DLUTSurface);
    pOsInterface = pHdrState->pOsInterface;

    VPHAL_PUBLIC_CHK_NULL(pOsInterface);

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    // Lock the surface for writing
    pDstOetfLut = (uint8_t *)pOsInterface->pfnLockResource(
        pOsInterface,
        &(pOETF1DLUTSurface->OsResource),
        &LockFlags);

    VPHAL_RENDER_CHK_NULL(pDstOetfLut);
    VPHAL_RENDER_CHK_NULL(pHdrState->pTargetSurf[0]);
    VPHAL_RENDER_CHK_NULL(pHdrState->pTargetSurf[0]);

    // Hdr kernel require 0 to 1 floating point color value
    // To transfer the value of 16bit integer OETF table to 0 to 1 floating point
    // We need to divide the table with 2^16 - 1 
    if ((pTargetSurf->pHDRParams &&
        (pTargetSurf->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
            pTargetSurf->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)) ||
        !pTargetSurf->pHDRParams)
    {
        if (pHdrState->OETFGamma[iIndex] == VPHAL_GAMMA_SRGB)
        {
            pSrcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_sRGB_FP16_g9;
        }
        else
        {
            pSrcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_BT709_FP16_g9;
        }

        for (i = 0;
            i < pOETF1DLUTSurface->dwHeight;
            i++, pDstOetfLut += pOETF1DLUTSurface->dwPitch, pSrcOetfLut += pOETF1DLUTSurface->dwWidth)
        {
            MOS_SecureMemcpy(pDstOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->dwWidth,
                pSrcOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->dwWidth);
        }
    }
    else if (pTargetSurf->pHDRParams &&
        pTargetSurf->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
    {
        if (pHdrState->HdrMode[iIndex] == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
        {
            const float fStretchFactor = 0.01f;
            VpHal_Generate2SegmentsOETFLUT(fStretchFactor, OETF2084, pHdrState->OetfSmpteSt2084);
            pSrcOetfLut = pHdrState->OetfSmpteSt2084;
        }
        else // pHdrState->HdrMode[iIndex] == VPHAL_HDR_MODE_H2H
        {
            pSrcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_SMPTE_ST2084_3Segs_FP16_g9;
        }

        for (i = 0;
            i < pOETF1DLUTSurface->dwHeight;
            i++, pDstOetfLut += pOETF1DLUTSurface->dwPitch, pSrcOetfLut += pOETF1DLUTSurface->dwWidth)
        {
            MOS_SecureMemcpy(pDstOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->dwWidth,
                pSrcOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->dwWidth);
        }
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    pOsInterface->pfnUnlockResource(
        pOsInterface,
        &(pOETF1DLUTSurface->OsResource));

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Get the Kernel Params
//! \details  Get the Kernel Params, including kernel unique ID, KDT Index
//! \param    VPHAL_HDR_KERNELID HdrKernelID
//!           [in] HDR Kernel ID
//! \param    int32_t* pKUIDOut
//!           [out] Kernel unique ID
//! \param    int32_t* pKDTIndexOut
//!           [out] KDT index
//! \param    PVPHAL_PERFTAG pPerfTag
//!           [out] Performance tag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrGetKernelParam_g9(
    uint32_t                    HdrKernelID,
    int32_t*                    pKUIDOut,
    int32_t*                    pKDTIndexOut)
{
    MOS_STATUS                  eStatus;

    VPHAL_PUBLIC_CHK_NULL(pKUIDOut);
    VPHAL_PUBLIC_CHK_NULL(pKDTIndexOut);

    eStatus = MOS_STATUS_SUCCESS;

    if (HdrKernelID == KERNEL_HDR_MANDATORY)
    {
        *pKUIDOut = IDR_VP_HDR_mandatory;
        *pKDTIndexOut = KERNEL_HDR_MANDATORY_G9;
    }
    else if (HdrKernelID == KERNEL_HDR_PREPROCESS)
    {
        *pKUIDOut = IDR_VP_HDR_preprocess;
        *pKDTIndexOut = KERNEL_HDR_PREPROCESS_G9;
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Kernel Not found.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;        
    }

finish:
    return eStatus;
}

//!
//! \brief    HDR PreProcess Surface State Setup
//! \details  Set up surface state used in HDR PreProcess, and bind the surface to pointed binding table entry.
//! \param    PVPHAL_HDR_STATE pHdrState
//            [in/out] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to hdr render data.
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetupPreProcessSurfaceStates_g9(
    PVPHAL_HDR_STATE           pHdrState,
    PVPHAL_HDR_RENDER_DATA     pRenderData)
{
    PVPHAL_SURFACE                  pSurfaceTemp                = nullptr;
    PRENDERHAL_SURFACE              pRenderHalSurfaceTemp       = nullptr;
    PRENDERHAL_INTERFACE            pRenderHal                  = nullptr;
    MOS_STATUS                      eStatus                     = MOS_STATUS_UNKNOWN;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams               = {};
    int32_t                         iBTentry                    = 0;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pRenderData);

    eStatus         = MOS_STATUS_UNKNOWN;
    pRenderHal      = pHdrState->pRenderHal;
    VPHAL_RENDER_CHK_NULL(pRenderHal);

    pSurfaceTemp            = &pHdrState->CoeffSurface;
    pRenderHalSurfaceTemp   = &pHdrState->RenderHalCoeffSurface;

    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.bRenderTarget     = false;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParams.bWidth16Align     = false;
    SurfaceParams.MemObjCtl         = pHdrState->SurfMemObjCtl.CoeffSurfMemObjCtl;
    iBTentry                        = 16;

    if (!Mos_ResourceIsNull(&pSurfaceTemp->OsResource))
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
            pRenderHal,
            pSurfaceTemp,
            pRenderHalSurfaceTemp,
            &SurfaceParams,
            pRenderData->iBindingTable,
            iBTentry,
            false));
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Null resource found");
        eStatus = MOS_STATUS_NULL_POINTER;
        goto finish;
    }

finish:
    return eStatus;
}

//!
//! \brief    Setup HDR PreProcess CURBE data
//! \details  Setup HDR PreProcess CURBE data
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Poniter to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Poniter to HDR render data
//! \param    int32_t* piCurbeOffsetOut
//!           [Out] Curbe offset
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise 
//!
MOS_STATUS VpHal_HdrPreprocessLoadStaticData_g9(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    int32_t*                    piCurbeOffsetOut)
{
    MOS_STATUS                                  eStatus     = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE                        pRenderHal  = nullptr;
    PVPHAL_SURFACE                              pSource     = nullptr;
    PVPHAL_SURFACE                              pTarget     = nullptr;

    MEDIA_WALKER_HDR_PREPROCESS_STATIC_DATA_G9 HDRStatic;
    MOS_ZeroMemory(&HDRStatic, sizeof(MEDIA_WALKER_HDR_PREPROCESS_STATIC_DATA_G9));

    uint32_t uiMaxCLL[VPHAL_MAX_HDR_INPUT_LAYER] = { 0 };
    uint32_t uiMaxDLL[VPHAL_MAX_HDR_INPUT_LAYER] = { 0 };
    HDR_TMMODE tmMode[VPHAL_MAX_HDR_INPUT_LAYER] = { PREPROCESS_TM_MAX };

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pHdrState->pRenderHal);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(piCurbeOffsetOut);

    pRenderHal = pHdrState->pRenderHal;

    for (uint32_t i = 0; i < pHdrState->uSourceCount; i++)
    {
        if (i >= VPHAL_MAX_HDR_INPUT_LAYER)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        pSource = pHdrState->pSrcSurf[i];   
        if (pSource)
        {
            uiMaxCLL[i] = (pSource->pHDRParams) ? pSource->pHDRParams->MaxCLL : 0;
        }

        switch (pHdrState->HdrMode[i])
        {
        case VPHAL_HDR_MODE_TONE_MAPPING:           // H2S
            tmMode[i] = PREPROCESS_TM_H2S;
            break;
        case VPHAL_HDR_MODE_H2H:                    // H2S
            tmMode[i] = PREPROCESS_TM_H2H;
            break;
        case VPHAL_HDR_MODE_INVERSE_TONE_MAPPING:   // S2H
            tmMode[i] = PREPROCESS_TM_S2H;
            break;
        default:
            break;
        }

        pTarget = pHdrState->pTargetSurf[0];        
        if (pTarget)
        {
            uiMaxDLL[0] = (pTarget->pHDRParams) ? pTarget->pHDRParams->max_display_mastering_luminance : 0;
        }

        HDRStatic.uiMaxCLL[i] = uiMaxCLL[i];
        HDRStatic.uiMaxDLL[i] = uiMaxDLL[0];
        HDRStatic.uiTMMode[i] = tmMode[i];
        VPHAL_RENDER_NORMALMESSAGE("StreamIndex: %d, maxCLL: %d, maxDLL: %d, TMMode: %d", i, HDRStatic.uiMaxCLL[i], HDRStatic.uiMaxDLL[i], HDRStatic.uiTMMode[i]);
    }
    HDRStatic.OutputCoeffIndex = 16; 

    *piCurbeOffsetOut = pRenderHal->pfnLoadCurbeData(
        pRenderHal,
        pRenderData->pMediaState,
        &HDRStatic,
        sizeof(MEDIA_WALKER_HDR_PREPROCESS_STATIC_DATA_G9));

    if (*piCurbeOffsetOut < 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }
    pRenderData->iCurbeOffset = *piCurbeOffsetOut;

finish:
    return eStatus;
}
