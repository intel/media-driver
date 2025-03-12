/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     vp_hdr_filter.h
//! \brief    Defines the common interface for Hdr
//!           this file is for the base interface which is shared by all Hdr in driver.
//!
#ifndef __VP_hdr_FILTER_H__
#define __VP_hdr_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp
{
#if (_DEBUG || _RELEASE_INTERNAL)
static bool sEnableQualityTuning = false;
static bool sEnableKernelDump    = false;
static bool sH2SBasedOnRGB       = false;
#endif

typedef struct
{
    uint32_t reserved : 2;
    uint32_t mantissa : 9;
    uint32_t exponent : 3;
    uint32_t sign : 1;
} CSC_COEFF_FORMAT;
//!
//! \brief hdr kernel eotf/oetf type definition enum
//!
typedef enum _VPHAL_HDR_KERNEL_EOTF_TYPE
{
    VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA = 0,
    VPHAL_HDR_KERNEL_SMPTE_ST2084           = 1
} VPHAL_HDR_KERNEL_EOTF_TYPE;

static const uint16_t HDRStageConfigTableForBDUHD[HDR_STAGES_CONFIG_TABLE_SIZE] =
{
    // Result:               CCM  PWLF  CCMExt1  GamutClamp1 CCMExt2  GamutClamp2 Invalid   Case id: OutputLinear OutputGamut OutputXDR InputGamut InputXDR
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          0),       //           0           0           0         0         0
    CONFIG_ENTRY_INITIALIZER(0,      1,      0,      0,       0,      0,          0),       //           0           0           0         0         1
    CONFIG_ENTRY_INITIALIZER(1,      0,      0,      0,       0,      0,          0),       //           0           0           0         1         0
    CONFIG_ENTRY_INITIALIZER(1,      1,      0,      0,       0,      0,          0),       //           0           0           0         1         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           0           0           1         0         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           0           0           1         0         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           0           0           1         1         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           0           0           1         1         1
    CONFIG_ENTRY_INITIALIZER(2,      0,      0,      0,       0,      0,          0),       //           0           1           0         0         0
    CONFIG_ENTRY_INITIALIZER(2,      1,      0,      0,       0,      0,          0),       //           0           1           0         0         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          0),       //           0           1           0         1         0
    CONFIG_ENTRY_INITIALIZER(0,      1,      0,      0,       0,      0,          0),       //           0           1           0         1         1
    CONFIG_ENTRY_INITIALIZER(2,      2,      0,      0,       0,      0,          0),       //           0           1           1         0         0
    CONFIG_ENTRY_INITIALIZER(2,      0,      0,      0,       0,      0,          0),       //           0           1           1         0         1
    CONFIG_ENTRY_INITIALIZER(0,      2,      0,      0,       0,      0,          0),       //           0           1           1         1         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          0),       //           0           1           1         1         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           0         0         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           0         0         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           0         1         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           0         1         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           1         0         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           1         0         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           1         1         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           0           1         1         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           1           0         0         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           1           0         0         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           1           0         1         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           1           0         1         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           1           1         0         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           1           1         0         1
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1),       //           1           1           1         1         0
    CONFIG_ENTRY_INITIALIZER(0,      0,      0,      0,       0,      0,          1)        //           1           1           1         1         1
};

const uint16_t HDRStageConfigTable[HDR_STAGES_CONFIG_TABLE_SIZE] =
{
    // CCM & CCMExt1 & CCMExt2 mode(should keep consistent with enum definition VPHAL_HDR_CCM_TYPE):
    // 0 - VPHAL_HDR_CCM_NONE
    // 1 - VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX
    // 2 - VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX
    // 3 - VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX
    // 4 - VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX
    // 5 - VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX
    //
    // PWLF mode(should keep consistent with enum definition VPHAL_HDR_MODE):
    // 0 - VPHAL_HDR_MODE_NONE
    // 1 - VPHAL_HDR_MODE_TONE_MAPPING
    // 2 - VPHAL_HDR_MODE_INVERSE_TONE_MAPPING
    // 3 - VPHAL_HDR_MODE_H2H
    // 4 - VPHAL_HDR_MODE_S2S
    //
    //               Result: CCM  PWLF  CCMExt1  GamutClamp1 CCMExt2  GamutClamp2 Invalid   Case id: OutputLinear OutputGamut OutputXDR InputGamut InputXDR
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 0),  //           0           0           0         0         0
    CONFIG_ENTRY_INITIALIZER(2, 1, 1, 0, 0, 0, 0),  //           0           0           0         0         1
    CONFIG_ENTRY_INITIALIZER(1, 0, 0, 0, 0, 0, 0),  //           0           0           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 1, 1, 0, 0, 0, 0),  //           0           0           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         0         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         1         1
    CONFIG_ENTRY_INITIALIZER(2, 0, 0, 0, 0, 0, 0),  //           0           1           0         0         0
    CONFIG_ENTRY_INITIALIZER(2, 1, 0, 0, 0, 0, 0),  //           0           1           0         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 0),  //           0           1           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 1, 0, 0, 0, 0, 0),  //           0           1           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 2, 0, 0, 0, 0),  //           0           1           1         0         0
    CONFIG_ENTRY_INITIALIZER(2, 3, 3, 1, 4, 0, 0),  //           0           1           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 3, 1, 4, 0, 0),  //           0           1           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 3, 0, 0, 0, 0, 0),  //           0           1           1         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 0),  //           1           0           0         0         0
    CONFIG_ENTRY_INITIALIZER(2, 1, 1, 1, 0, 0, 0),  //           1           0           0         0         1
    CONFIG_ENTRY_INITIALIZER(1, 0, 0, 0, 0, 0, 0),  //           1           0           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 1, 1, 1, 0, 0, 0),  //           1           0           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 0, 0, 0, 0, 0),  //           1           0           1         0         0
    CONFIG_ENTRY_INITIALIZER(2, 3, 3, 1, 5, 0, 0),  //           1           0           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 3, 1, 5, 0, 0),  //           1           0           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 3, 5, 0, 0, 0, 0),  //           1           0           1         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         0         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           1         0         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1)   //           1           1           1         1         1
};

#define VPHAL_HDR_MODE_3DLUT_MASK                   0x10
#define VPHAL_HDR_MODE_VEBOX_3DLUT_MASK             0x20
#define VPHAL_HDR_MODE_VEBOX_3DLUT33_MASK           0x30
#define VPHAL_HDR_MODE_VEBOX_1DLUT_MASK             0x40
#define VPHAL_HDR_MODE_VEBOX_1DLUT_3DLUT_MASK       0x50

#define HDR_DEFAULT_MAXCLL    4000
#define HDR_DEFAULT_MAXFALL   400

const uint32_t g_Hdr_ColorCorrect_OETF_sRGB_FP16[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER] =
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

static const uint32_t g_Hdr_ColorCorrect_OETF_BT709_FP16[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER] =
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
static const uint32_t g_Hdr_ColorCorrect_OETF_SMPTE_ST2084_3Segs_FP16[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER] =
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

typedef union _HDRCaseID
{
    uint32_t    index;
    struct {
        uint32_t    InputXDR : 1;  // SDR or HDR
        uint32_t    InputGamut : 1;  // 709 or 2020 primaries
        uint32_t    OutputXDR : 1;
        uint32_t    OutputGamut : 1;
        uint32_t    OutputLinear : 1;  // fp16 output
        uint32_t    Reserved : 27;
    };
} HDRCaseID;

class VpHdrFilter : public VpFilter
{
public:
    VpHdrFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    virtual ~VpHdrFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        SwFilterPipe    *executedPipe,
        VP_EXECUTE_CAPS vpExecuteCaps);

    MOS_STATUS        CalculateEngineParams(
        FeatureParamHdr &HdrParams,
        VP_EXECUTE_CAPS  vpExecuteCaps);
    PVEBOX_HDR_PARAMS GetVeboxParams()
    {
        return &m_veboxHdrParams;
    }
    PRENDER_HDR_3DLUT_CAL_PARAMS GetRenderHdr3DLutParams()
    {
        return &m_renderHdr3DLutParams;
    }

    PRENDER_HDR_PARAMS GetRenderParams()
    {
        return &m_renderHdrParams;
    }

protected:
    VEBOX_HDR_PARAMS m_veboxHdrParams = {};
    RENDER_HDR_3DLUT_CAL_PARAMS m_renderHdr3DLutParams = {};
    RENDER_HDR_PARAMS           m_renderHdrParams      = {};
    KERNEL_INDEX_ARG_MAP        m_renderHdr3DLutOclParams = {};
    SwFilterPipe                *m_executedPipe        = nullptr;

    SurfaceType m_surfType3DLut         = SurfaceType3DLut;
    SurfaceType m_surfType3DLutCoef     = SurfaceType3DLutCoef;
    uint32_t    m_3DLutSurfaceWidth     = 0;
    uint32_t    m_3DLutSurfaceHeight    = 0;
    PMOS_INTERFACE m_pOsInterface          = nullptr;
    MediaUserSettingSharedPtr m_userSettingPtr        = nullptr;  //!< usersettingInstance
    PVP_MHWINTERFACE          m_vpInterface           = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpHdrFilter)
};

struct HW_FILTER_HDR_PARAM : public HW_FILTER_PARAM
{
    FeatureParamHdr hdrParams;
};

class HwFilterHdrParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_HDR_PARAM &param, FeatureType featureType);
    HwFilterHdrParameter(FeatureType featureType);
    virtual ~HwFilterHdrParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_HDR_PARAM &param);

private:
    HW_FILTER_HDR_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterHdrParameter)
};

class VpVeboxHdrParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_HDR_PARAM &param);
    VpVeboxHdrParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxHdrParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_HDR_PARAM &params);

    VpHdrFilter m_HdrFilter;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxHdrParameter)
};

class PolicyVeboxHdrHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxHdrHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxHdrHandler();
    virtual bool               IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter * CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeHdrOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid Parameter for VEBOX Hdr!");
            return nullptr;
        }

        HW_FILTER_HDR_PARAM *HdrParam = (HW_FILTER_HDR_PARAM *)(&param);
        return VpVeboxHdrParameter::Create(*HdrParam);
    }

private:
    PacketParamFactory<VpVeboxHdrParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyVeboxHdrHandler)
};

/*****************HDR 3DLUT Calculate Kernel********************/
class VpRenderHdr3DLutCalParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_HDR_PARAM &param);
    VpRenderHdr3DLutCalParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpRenderHdr3DLutCalParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_HDR_PARAM &params);

    VpHdrFilter m_HdrFilter;

MEDIA_CLASS_DEFINE_END(vp__VpRenderHdr3DLutCalParameter)
};

class PolicyRenderHdr3DLutCalHandler : public PolicyFeatureHandler
{
public:
    PolicyRenderHdr3DLutCalHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyRenderHdr3DLutCalHandler();
    virtual bool               IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual MOS_STATUS         UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter * CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeHdr3DLutCalOnRender)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid Parameter for Render Hdr 3DLut Calculation!");
            return nullptr;
        }

        HW_FILTER_HDR_PARAM *HdrParam = (HW_FILTER_HDR_PARAM *)(&param);
        return VpRenderHdr3DLutCalParameter::Create(*HdrParam);
    }

private:
    PacketParamFactory<VpRenderHdr3DLutCalParameter> m_PacketParamFactory;

    MEDIA_CLASS_DEFINE_END(vp__PolicyRenderHdr3DLutCalHandler)
};

}  // namespace vp
#endif
