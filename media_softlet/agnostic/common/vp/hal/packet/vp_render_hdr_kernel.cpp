/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     vp_render_hdr_kernel.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!
#include "vp_render_hdr_kernel.h"
#include "vp_render_kernel_obj.h"
#include "hal_kerneldll_next.h"
#include "hal_oca_interface_next.h"
#include "vp_user_feature_control.h"
#include "vp_hal_ddi_utils.h"

using namespace vp;

#define VP_COMP_SOURCE_DEPTH          16
#define VP_COMP_P010_DEPTH            0

// Compositing surface binding table index
#define VP_COMP_BTINDEX_LAYER0          0
#define VP_COMP_BTINDEX_LAYER0_FIELD0   0
#define VP_COMP_BTINDEX_LAYER1          3
#define VP_COMP_BTINDEX_LAYER2          6
#define VP_COMP_BTINDEX_LAYER3          9
#define VP_COMP_BTINDEX_LAYER4         12
#define VP_COMP_BTINDEX_LAYER5         15
#define VP_COMP_BTINDEX_LAYER6         18
#define VP_COMP_BTINDEX_LAYER7         21
#define VP_COMP_BTINDEX_RENDERTARGET   24
#define VP_COMP_BTINDEX_RT_SECOND      27    // Pre-SKL
#define VP_COMP_BTINDEX_L0_FIELD1_DUAL 48    // Pre-SKL
#define VP_HDR_SAMPLER_STATE_NUM       16

// CMFC macro
#define VP_COMP_BTINDEX_CSC_COEFF      34

#define VP_MAX_HDR_INPUT_LAYER 8
#define VP_MAX_HDR_OUTPUT_LAYER 1

#define DETAIL_STRONG_EDGE_WEIGHT 7
#define DETAIL_NON_EDGE_WEIGHT 1
#define DETAIL_REGULAR_EDGE_WEIGHT 2
#define DETAIL_WEAK_EDGE_THRESHOLD 1
#define DETAIL_STRONG_EDGE_THRESHOLD 8

#define VP_SAMPLER_INDEX_Y_NEAREST 1
#define VP_SAMPLER_INDEX_U_NEAREST 2
#define VP_SAMPLER_INDEX_V_NEAREST 3

#define VP_SAMPLER_INDEX_Y_BILINEAR 4
#define VP_SAMPLER_INDEX_U_BILINEAR 5
#define VP_SAMPLER_INDEX_V_BILINEAR 6

#define VP_HDR_DUMP_INPUT_LAYER0                         "InputLayer0"
#define VP_HDR_DUMP_OETF1DLUT_SURFACE0                   "OETF1DLUTSurfacce0"
#define VP_HDR_DUMP_COEFF_SURFACE                        "CoeffSurfacce"
#define VP_HDR_DUMP_TARGET_SURFACE0                      "TargetSurface0"
#define VP_HDR_DUMP_RENDER_INPUT                         "RenderInput"

MEDIA_WALKER_HDR_STATIC_DATA g_cInit_MEDIA_STATIC_HDR =
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


const int32_t VpRenderHdrKernel::s_bindingTableIndex[] =
{
    VP_COMP_BTINDEX_LAYER0,
    VP_COMP_BTINDEX_LAYER1,
    VP_COMP_BTINDEX_LAYER2,
    VP_COMP_BTINDEX_LAYER3,
    VP_COMP_BTINDEX_LAYER4,
    VP_COMP_BTINDEX_LAYER5,
    VP_COMP_BTINDEX_LAYER6,
    VP_COMP_BTINDEX_LAYER7
};

const int32_t VpRenderHdrKernel::s_bindingTableIndexField[] =
{
    VP_COMP_BTINDEX_L0_FIELD1_DUAL,
    VP_COMP_BTINDEX_L0_FIELD1_DUAL + 1,
    VP_COMP_BTINDEX_L0_FIELD1_DUAL + 2,
    VP_COMP_BTINDEX_L0_FIELD1_DUAL + 3,
    VP_COMP_BTINDEX_L0_FIELD1_DUAL + 4,
    VP_COMP_BTINDEX_L0_FIELD1_DUAL + 5,
    VP_COMP_BTINDEX_L0_FIELD1_DUAL + 6,
    VP_COMP_BTINDEX_L0_FIELD1_DUAL + 7
};

VpRenderHdrKernel::VpRenderHdrKernel(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    VpRenderKernelObj(hwInterface, (VpKernelID)kernelHdrMandatory, 0, VpRenderKernel::s_kernelNameNonAdvKernels, allocator)
{
    VP_FUNC_CALL();

    renderHal = hwInterface ? hwInterface->m_renderHal : nullptr;

    m_kernelBinaryID = IDR_VP_HDR_mandatory;

    m_renderHal = m_hwInterface ? m_hwInterface->m_renderHal : nullptr;
    if (m_renderHal)
    {
        m_renderHal->bEnableP010SinglePass = false;
        VP_RENDER_NORMALMESSAGE("m_renderHal->bEnableP010SinglePass %d", m_renderHal->bEnableP010SinglePass);
    }
}

static inline RENDERHAL_SURFACE_TYPE ConvertVpSurfaceTypeToRenderSurfType(VPHAL_SURFACE_TYPE vpSurfType)
{
    switch (vpSurfType)
    {
        case SURF_IN_BACKGROUND:
            return RENDERHAL_SURF_IN_BACKGROUND;

        case SURF_IN_PRIMARY:
            return RENDERHAL_SURF_IN_PRIMARY;

        case SURF_IN_SUBSTREAM:
            return RENDERHAL_SURF_IN_SUBSTREAM;

        case SURF_IN_REFERENCE:
            return RENDERHAL_SURF_IN_REFERENCE;

        case SURF_OUT_RENDERTARGET:
            return RENDERHAL_SURF_OUT_RENDERTARGET;

        case SURF_NONE:
        default:
            return RENDERHAL_SURF_NONE;
    }
}

static inline RENDERHAL_SCALING_MODE ConvertVpScalingModeToRenderScalingMode(VPHAL_SCALING_MODE vpScalingMode)
{
    switch (vpScalingMode)
    {
        case VPHAL_SCALING_NEAREST:
            return RENDERHAL_SCALING_NEAREST;

        case VPHAL_SCALING_BILINEAR:
            return RENDERHAL_SCALING_BILINEAR;

        case VPHAL_SCALING_AVS:
            return RENDERHAL_SCALING_AVS;

        default:
            VP_RENDER_VERBOSEMESSAGE("Invalid VPHAL_SCALING_MODE %d, force to nearest mode.", vpScalingMode);
            return RENDERHAL_SCALING_NEAREST;
    }
}

static inline RENDERHAL_SAMPLE_TYPE ConvertVpSampleTypeToRenderSampleType(VPHAL_SAMPLE_TYPE SampleType)
{
    switch (SampleType)
    {
        case SAMPLE_PROGRESSIVE:
            return RENDERHAL_SAMPLE_PROGRESSIVE;

        case SAMPLE_SINGLE_TOP_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_TOP_FIELD;

        case SAMPLE_SINGLE_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;

        case SAMPLE_INVALID:
        default:
            VP_RENDER_VERBOSEMESSAGE("Invalid VPHAL_SAMPLE_TYPE %d.\n", SampleType);
            return RENDERHAL_SAMPLE_INVALID;
    }
}

static inline MHW_ROTATION VpRotationModeToRenderRotationMode(VPHAL_ROTATION Rotation)
{
    MHW_ROTATION    Mode = MHW_ROTATION_IDENTITY;

    switch (Rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            Mode = MHW_ROTATION_IDENTITY;
            break;

        case VPHAL_ROTATION_90:
            Mode = MHW_ROTATION_90;
            break;

        case VPHAL_ROTATION_180:
            Mode = MHW_ROTATION_180;
            break;

        case VPHAL_ROTATION_270:
            Mode = MHW_ROTATION_270;
            break;

        case VPHAL_MIRROR_HORIZONTAL:
            Mode = MHW_MIRROR_HORIZONTAL;
            break;

        case VPHAL_MIRROR_VERTICAL:
            Mode = MHW_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            Mode = MHW_ROTATE_90_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
            Mode = MHW_ROTATE_90_MIRROR_HORIZONTAL;
            break;

        default:
            VP_RENDER_VERBOSEMESSAGE("Invalid Rotation Angle.");
            break;
    }

    return Mode;
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

    float det = a0 * (b1 * c2 - b2 * c1) + a1 * (b2 * c0 - b0 * c2) + a2 * (b0 * c1 - b1 * c0);

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
    const float xr, const float yr, const float xg, const float yg, const float xb, const float yb, const float xn, const float yn, Mat3 output)
{
    const float zr = 1.0f - xr - yr;
    const float zg = 1.0f - xg - yg;
    const float zb = 1.0f - xb - yb;
    const float zn = 1.0f - xn - yn;

    // m * [ar, ag, ab]T = [xn / yn, 1.0f, zn / yn]T;
    const Mat3 m =
        {
            xr, xg, xb, yr, yg, yb, zr, zg, zb};

    Mat3 inversed_m;

    Mat3Inverse(m, inversed_m);

    const Vec3 XYZWithUnityY = {xn / yn, 1.0f, zn / yn};
    float      aragab[3];

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

//!
//! \brief    Calculate Yuv To Rgb Matrix
//! \details  Calculate Yuv To Rgb Matrix
//! \param    VPHAL_CSPACE src
//!           [in] Source color space
//! \param    VPHAL_CSPACE dst
//!           [in] Dest color space
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::VpHal_HdrCalcYuvToRgbMatrix(
    VPHAL_CSPACE src,
    VPHAL_CSPACE dst,
    float       *pTransferMatrix,
    float       *pOutMatrix)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    float      Y_o = 0.0f, Y_e = 0.0f, C_z = 0.0f, C_e = 0.0f;
    float      R_o = 0.0f, R_e = 0.0f;

    VP_PUBLIC_CHK_NULL_RETURN(pTransferMatrix);
    VP_PUBLIC_CHK_NULL_RETURN(pOutMatrix);

    VpHal_HdrGetRgbRangeAndOffset(dst, &R_o, &R_e);
    VpHal_HdrGetYuvRangeAndOffset(src, &Y_o, &Y_e, &C_z, &C_e);

    // after + (3x3)(3x3)
    pOutMatrix[0]  = pTransferMatrix[0] * R_e / Y_e;
    pOutMatrix[4]  = pTransferMatrix[4] * R_e / Y_e;
    pOutMatrix[8]  = pTransferMatrix[8] * R_e / Y_e;
    pOutMatrix[1]  = pTransferMatrix[1] * R_e / C_e;
    pOutMatrix[5]  = pTransferMatrix[5] * R_e / C_e;
    pOutMatrix[9]  = pTransferMatrix[9] * R_e / C_e;
    pOutMatrix[2]  = pTransferMatrix[2] * R_e / C_e;
    pOutMatrix[6]  = pTransferMatrix[6] * R_e / C_e;
    pOutMatrix[10] = pTransferMatrix[10] * R_e / C_e;

    // (3x1) - (3x3)(3x3)(3x1)
    pOutMatrix[3]  = R_o - (pOutMatrix[0] * Y_o + pOutMatrix[1] * C_z + pOutMatrix[2] * C_z);
    pOutMatrix[7]  = R_o - (pOutMatrix[4] * Y_o + pOutMatrix[5] * C_z + pOutMatrix[6] * C_z);
    pOutMatrix[11] = R_o - (pOutMatrix[8] * Y_o + pOutMatrix[9] * C_z + pOutMatrix[10] * C_z);

    return eStatus;
}

//!
//! \brief    Calculate Yuv Range and Offest
//! \details  Calculate Yuv Range and Offest
//! \param    VPHAL_CSPACE cspace
//!           [in] Source color space
//! \param    float* pLumaOffset
//!           [out] Pointer to Luma Offset
//! \param    float* pLumaExcursion
//!           [out] Pointer to Luma Excursion
//! \param    float* pChromaZero
//!           [out] Pointer to Chroma Offset
//! \param    float* pChromaExcursion
//!           [out] Pointer to Chroma Excursion
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::VpHal_HdrGetYuvRangeAndOffset(
    VPHAL_CSPACE cspace,
    float       *pLumaOffset,
    float       *pLumaExcursion,
    float       *pChromaZero,
    float       *pChromaExcursion)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(pLumaOffset);
    VP_PUBLIC_CHK_NULL_RETURN(pLumaExcursion);
    VP_PUBLIC_CHK_NULL_RETURN(pChromaZero);
    VP_PUBLIC_CHK_NULL_RETURN(pChromaExcursion);

    switch (cspace)
    {
    case CSpace_BT601_FullRange:
    case CSpace_BT709_FullRange:
    case CSpace_BT601Gray_FullRange:
    case CSpace_BT2020_FullRange:
        *pLumaOffset      = 0.0f;
        *pLumaExcursion   = 255.0f;
        *pChromaZero      = 128.0f;
        *pChromaExcursion = 255.0f;
        break;

    case CSpace_BT601:
    case CSpace_BT709:
    case CSpace_xvYCC601:  // since matrix is the same as 601, use the same range
    case CSpace_xvYCC709:  // since matrix is the same as 709, use the same range
    case CSpace_BT601Gray:
    case CSpace_BT2020:
        *pLumaOffset      = 16.0f;
        *pLumaExcursion   = 219.0f;
        *pChromaZero      = 128.0f;
        *pChromaExcursion = 224.0f;
        break;

    default:
        *pLumaOffset      = 0.0f;
        *pLumaExcursion   = 255.0f;
        *pChromaZero      = 128.0f;
        *pChromaExcursion = 255.0f;
        break;
    }

    *pLumaOffset /= 255.0f;
    *pLumaExcursion /= 255.0f;
    *pChromaZero /= 255.0f;
    *pChromaExcursion /= 255.0f;

    return eStatus;
}

//!
//! \brief    Calculate Rgb Range and Offest
//! \details  Calculate Rgb Range and Offest
//! \param    VPHAL_CSPACE cspace
//!           [in] Source color space
//! \param    float* pLumaOffset
//!           [out] Pointer to Rgb Offset
//! \param    float* pLumaExcursion
//!           [out] Pointer to Rgb Excursion
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::VpHal_HdrGetRgbRangeAndOffset(
    VPHAL_CSPACE cspace,
    float       *pRgbOffset,
    float       *pRgbExcursion)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(pRgbOffset);
    VP_PUBLIC_CHK_NULL_RETURN(pRgbExcursion);

    switch (cspace)
    {
    case CSpace_sRGB:
    case CSpace_BT2020_RGB:
        *pRgbOffset    = 0.0f;
        *pRgbExcursion = 255.0f;
        break;

    case CSpace_stRGB:
    case CSpace_BT2020_stRGB:
        *pRgbOffset    = 16.0f;
        *pRgbExcursion = 219.0f;
        break;

    default:
        *pRgbOffset    = 0.0f;
        *pRgbExcursion = 255.0f;
        break;
    }

    *pRgbOffset /= 255.0f;
    *pRgbExcursion /= 255.0f;

    return eStatus;
}

//!
//! \brief    Calculate Rgb To Yuv Matrix
//! \details  Calculate Rgb To Yuv Matrix
//! \param    VPHAL_CSPACE src
//!           [in] Source color space
//! \param    VPHAL_CSPACE dst
//!           [in] Dest color space
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::VpHal_HdrCalcRgbToYuvMatrix(
    VPHAL_CSPACE src,
    VPHAL_CSPACE dst,
    float       *pTransferMatrix,
    float       *pOutMatrix)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    float      Y_o = 0.0f, Y_e = 0.0f, C_z = 0.0f, C_e = 0.0f;
    float      R_o = 0.0f, R_e = 0.0f;
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(pTransferMatrix);
    VP_PUBLIC_CHK_NULL_RETURN(pOutMatrix);

    VpHal_HdrGetRgbRangeAndOffset(src, &R_o, &R_e);
    VpHal_HdrGetYuvRangeAndOffset(dst, &Y_o, &Y_e, &C_z, &C_e);

    // multiplication of + onwards
    pOutMatrix[0]  = pTransferMatrix[0] * Y_e / R_e;
    pOutMatrix[1]  = pTransferMatrix[1] * Y_e / R_e;
    pOutMatrix[2]  = pTransferMatrix[2] * Y_e / R_e;
    pOutMatrix[4]  = pTransferMatrix[4] * C_e / R_e;
    pOutMatrix[5]  = pTransferMatrix[5] * C_e / R_e;
    pOutMatrix[6]  = pTransferMatrix[6] * C_e / R_e;
    pOutMatrix[8]  = pTransferMatrix[8] * C_e / R_e;
    pOutMatrix[9]  = pTransferMatrix[9] * C_e / R_e;
    pOutMatrix[10] = pTransferMatrix[10] * C_e / R_e;

    pOutMatrix[7]  = Y_o - Y_e * R_o / R_e;
    pOutMatrix[3]  = C_z;
    pOutMatrix[11] = C_z;

    return eStatus;
}

//!
//! \brief    Calculate CCM Matrix
//! \details  Calculate CCM Matrix
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::VpHal_HdrCalcCCMMatrix(
    float *pTransferMatrix,
    float *pOutMatrix)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(pTransferMatrix);
    VP_PUBLIC_CHK_NULL_RETURN(pOutMatrix);

    // multiplication of + onwards
    pOutMatrix[0]  = pTransferMatrix[1];
    pOutMatrix[1]  = pTransferMatrix[2];
    pOutMatrix[2]  = pTransferMatrix[0];
    pOutMatrix[4]  = pTransferMatrix[5];
    pOutMatrix[5]  = pTransferMatrix[6];
    pOutMatrix[6]  = pTransferMatrix[4];
    pOutMatrix[8]  = pTransferMatrix[9];
    pOutMatrix[9]  = pTransferMatrix[10];
    pOutMatrix[10] = pTransferMatrix[8];

    pOutMatrix[3]  = pTransferMatrix[11];
    pOutMatrix[7]  = pTransferMatrix[3];
    pOutMatrix[11] = pTransferMatrix[7];

    return eStatus;
}

void VpRenderHdrKernel::HdrCalculateCCMWithMonitorGamut(
    VPHAL_HDR_CCM_TYPE CCMType,
    HDR_PARAMS         Target,
    float              TempMatrix[12])
{
    float src_xr = 1.0f, src_yr = 1.0f;
    float src_xg = 1.0f, src_yg = 1.0f;
    float src_xb = 1.0f, src_yb = 1.0f;
    float src_xn = 1.0f, src_yn = 1.0f;

    float dst_xr = 1.0f, dst_yr = 1.0f;
    float dst_xg = 1.0f, dst_yg = 1.0f;
    float dst_xb = 1.0f, dst_yb = 1.0f;
    float dst_xn = 1.0f, dst_yn = 1.0f;

    Mat3 SrcMatrix        = {1.0f};
    Mat3 DstMatrix        = {1.0f};
    Mat3 DstMatrixInverse = {1.0f};
    Mat3 SrcToDstMatrix   = {1.0f};

    Mat3 BT709ToBT2020Matrix = {1.0f};
    Mat3 BT2020ToBT709Matrix = {1.0f};

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

        dst_xr = Target.display_primaries_x[2] / 50000.0f;
        dst_yr = Target.display_primaries_y[2] / 50000.0f;
        dst_xg = Target.display_primaries_x[0] / 50000.0f;
        dst_yg = Target.display_primaries_y[0] / 50000.0f;
        dst_xb = Target.display_primaries_x[1] / 50000.0f;
        dst_yb = Target.display_primaries_y[1] / 50000.0f;
        dst_xn = Target.white_point_x / 50000.0f;
        dst_yn = Target.white_point_y / 50000.0f;
    }
    else if (CCMType == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX)
    {
        src_xr = Target.display_primaries_x[2] / 50000.0f;
        src_yr = Target.display_primaries_y[2] / 50000.0f;
        src_xg = Target.display_primaries_x[0] / 50000.0f;
        src_yg = Target.display_primaries_y[0] / 50000.0f;
        src_xb = Target.display_primaries_x[1] / 50000.0f;
        src_yb = Target.display_primaries_y[1] / 50000.0f;
        src_xn = Target.white_point_x / 50000.0f;
        src_yn = Target.white_point_y / 50000.0f;

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
        src_xr = Target.display_primaries_x[2] / 50000.0f;
        src_yr = Target.display_primaries_y[2] / 50000.0f;
        src_xg = Target.display_primaries_x[0] / 50000.0f;
        src_yg = Target.display_primaries_y[0] / 50000.0f;
        src_xb = Target.display_primaries_x[1] / 50000.0f;
        src_yb = Target.display_primaries_y[1] / 50000.0f;
        src_xn = Target.white_point_x / 50000.0f;
        src_yn = Target.white_point_y / 50000.0f;

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
        src_xr, src_yr, src_xg, src_yg, src_xb, src_yb, src_xn, src_yn, SrcMatrix);

    RGB2CIEXYZMatrix(
        dst_xr, dst_yr, dst_xg, dst_yg, dst_xb, dst_yb, dst_xn, dst_yn, DstMatrix);

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

    return;
}

CSC_COEFF_FORMAT HdrConvert_CSC_Coeff_To_Register_Format(double coeff)
{
    VP_FUNC_CALL();

    CSC_COEFF_FORMAT outVal       = {0};
    uint32_t         shift_factor = 0;

    if (coeff < 0)
    {
        outVal.sign = 1;
        coeff       = -coeff;
    }

    // range check
    if (coeff > MAX_CSC_COEFF_VAL_ICL)
        coeff = MAX_CSC_COEFF_VAL_ICL;

    if (coeff < 0.125)  //0.000bbbbbbbbb
    {
        outVal.exponent = 3;
        shift_factor    = 12;
    }
    else if (coeff >= 0.125 && coeff < 0.25)  //0.00bbbbbbbbb
    {
        outVal.exponent = 2;
        shift_factor    = 11;
    }
    else if (coeff >= 0.25 && coeff < 0.5)  //0.0bbbbbbbbb
    {
        outVal.exponent = 1;
        shift_factor    = 10;
    }
    else if (coeff >= 0.5 && coeff < 1.0)  // 0.bbbbbbbbb
    {
        outVal.exponent = 0;
        shift_factor    = 9;
    }
    else if (coeff >= 1.0 && coeff < 2.0)  //b.bbbbbbbb
    {
        outVal.exponent = 7;
        shift_factor    = 8;
    }
    else if (coeff >= 2.0)  // bb.bbbbbbb
    {
        outVal.exponent = 6;
        shift_factor    = 7;
    }

    //Convert float to integer
    outVal.mantissa = static_cast<uint32_t>(round(coeff * (double)(1 << (int)shift_factor)));

    return outVal;
}

double HdrConvert_CSC_Coeff_Register_Format_To_Double(CSC_COEFF_FORMAT regVal)
{
    VP_FUNC_CALL();

    double outVal = 0;

    switch (regVal.exponent)
    {
    case 0:
        outVal = (double)regVal.mantissa / 512.0;
        break;
    case 1:
        outVal = (double)regVal.mantissa / 1024.0;
        break;
    case 2:
        outVal = (double)regVal.mantissa / 2048.0;
        break;
    case 3:
        outVal = (double)regVal.mantissa / 4096.0;
        break;
    case 6:
        outVal = (double)regVal.mantissa / 128.0;
        break;
    case 7:
        outVal = (double)regVal.mantissa / 256.0;
        break;
    }

    if (regVal.sign)
    {
        outVal = -outVal;
    }

    return outVal;
}

float HdrLimitFP32PrecisionToF3_9(float fp)
{
    VP_FUNC_CALL();

    double dbInput  = static_cast<double>(fp);
    double dbOutput = HdrConvert_CSC_Coeff_Register_Format_To_Double(HdrConvert_CSC_Coeff_To_Register_Format(dbInput));
    return static_cast<float>(dbOutput);
}

void VpRenderHdrKernel::HdrLimitFP32ArrayPrecisionToF3_9(float fps[], size_t size)
{
    VP_FUNC_CALL();

    for (size_t i = 0; i < size; i++)
    {
        fps[i] = HdrLimitFP32PrecisionToF3_9(fps[i]);
    }
}

typedef float (*pfnOETFFunc)(float radiance);

float HdrOETF2084(float c)
{
    static const double C1 = 0.8359375;
    static const double C2 = 18.8515625;
    static const double C3 = 18.6875;
    static const double M1 = 0.1593017578125;
    static const double M2 = 78.84375;

    double tmp         = c;
    double numerator   = pow(tmp, M1);
    double denominator = numerator;

    denominator = 1.0 + C3 * denominator;
    numerator   = C1 + C2 * numerator;
    numerator   = numerator / denominator;

    return (float)pow(numerator, M2);
}

float HdrOETFBT709(float c)
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

float HdrOETFsRGB(float c)
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
void HdrGenerate2SegmentsOETFLUT(float fStretchFactor, pfnOETFFunc oetfFunc, uint16_t *lut)
{
    int i = 0, j = 0;

    for (i = 0; i < VPHAL_HDR_OETF_1DLUT_HEIGHT; ++i)
    {
        for (j = 0; j < VPHAL_HDR_OETF_1DLUT_WIDTH; ++j)
        {
            int   idx = j + i * (VPHAL_HDR_OETF_1DLUT_WIDTH - 1);
            float a   = (idx < 32) ? ((1.0f / 1024.0f) * idx) : ((1.0f / 32.0f) * (idx - 31));

            if (a > 1.0f)
                a = 1.0f;

            a *= fStretchFactor;
            lut[i * VPHAL_HDR_OETF_1DLUT_WIDTH + j] = VpHal_FloatToHalfFloat(oetfFunc(a));
        }
    }
}

//!
//! \brief    Color Transfer for Hdr 3d Lut
//! \details  Color Transfer for Hdr 3d Lut
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to Hdr State
//! \param    int32_t iIndex
//!           [in] Input Surface index
//! \param    float fInputX
//!           [in] Input color for x axis of 3D Lut
//! \param    float fInputY
//!           [in] Input color for y axis of 3D Lut
//! \param    float fInputZ
//!           [in] Input color for z axis of 3D Lut
//! \param    uint16_t *puOutputX
//!           [in] Output color for x axis of 3D Lut
//! \param    uint16_t *puOutputX
//!           [in] Output color for y axis of 3D Lut
//! \param    uint16_t *puOutputX
//!           [in] Output color for z axis of 3D Lut
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::VpHal_HdrColorTransfer3dLut(
    PRENDER_HDR_PARAMS params,
    int32_t          iIndex,
    float            fInputX,
    float            fInputY,
    float            fInputZ,
    uint16_t        *puOutputX,
    uint16_t        *puOutputY,
    uint16_t        *puOutputZ)
{
    VP_FUNC_CALL();

    float                PriorCscMatrix[12] = {}, PostCscMatrix[12] = {};
    float                TempMatrix[12] = {};
    double               fTempX = 0, fTempY = 0, fTempZ = 0;
    double               fTemp1X = 0, fTemp1Y = 0, fTemp1Z = 0;
    double               fTemp = 0;
    double               m1        = 0.1593017578125;  // SMPTE ST2084 EOTF parameters
    double               m2        = 78.84375;         // SMPTE ST2084 EOTF parameters
    double               c2        = 18.8515625;       // SMPTE ST2084 EOTF parameters
    double               c3        = 18.6875;          // SMPTE ST2084 EOTF parameters
    double               c1        = c3 - c2 + 1;      // SMPTE ST2084 EOTF parameters

    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(puOutputX);
    VP_PUBLIC_CHK_NULL_RETURN(puOutputX);
    VP_PUBLIC_CHK_NULL_RETURN(puOutputX);

    fTempX = (double)fInputX;
    fTempY = (double)fInputY;
    fTempZ = (double)fInputZ;

#define SET_MATRIX(_c0, _c1, _c2, _c3, _c4, _c5, _c6, _c7, _c8, _c9, _c10, _c11) \
    {                                                                            \
        TempMatrix[0]  = _c0;                                                    \
        TempMatrix[1]  = _c1;                                                    \
        TempMatrix[2]  = _c2;                                                    \
        TempMatrix[3]  = _c3;                                                    \
        TempMatrix[4]  = _c4;                                                    \
        TempMatrix[5]  = _c5;                                                    \
        TempMatrix[6]  = _c6;                                                    \
        TempMatrix[7]  = _c7;                                                    \
        TempMatrix[8]  = _c8;                                                    \
        TempMatrix[9]  = _c9;                                                    \
        TempMatrix[10] = _c10;                                                   \
        TempMatrix[11] = _c11;                                                   \
    }

#define CLAMP_MIN_MAX(_a, _min, _max) \
    {                                 \
        if (_a < _min)                \
        {                             \
            _a = _min;                \
        }                             \
        if (_a > _max)                \
        {                             \
            _a = _max;                \
        }                             \
    }

    auto        inputSurface = m_surfaceGroup->find(SurfaceType(SurfaceTypeHdrInputLayer0 + iIndex));
    VP_SURFACE *input        = (m_surfaceGroup->end() != inputSurface) ? inputSurface->second : nullptr;
    // EOTF/CCM/Tone Mapping/OETF require RGB input
    // So if prior CSC is needed, it will always be YUV to RGB conversion
    if (params->StageEnableFlags[iIndex].PriorCSCEnable)
    {
        if (params->PriorCSC[iIndex] == VPHAL_HDR_CSC_YUV_TO_RGB_BT601)
        {
            SET_MATRIX(1.000000f, 0.000000f, 1.402000f, 0.000000f, 1.000000f, -0.344136f, -0.714136f, 0.000000f, 1.000000f, 1.772000f, 0.000000f, 0.000000f);
            VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT601, CSpace_sRGB, TempMatrix, PriorCscMatrix);
        }
        else if (params->PriorCSC[iIndex] == VPHAL_HDR_CSC_YUV_TO_RGB_BT709)
        {
            SET_MATRIX(1.000000f, 0.000000f, 1.574800f, 0.000000f, 1.000000f, -0.187324f, -0.468124f, 0.000000f, 1.000000f, 1.855600f, 0.000000f, 0.000000f);
            VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT709, CSpace_sRGB, TempMatrix, PriorCscMatrix);
        }
        else if (params->PriorCSC[iIndex] == VPHAL_HDR_CSC_YUV_TO_RGB_BT2020)
        {
            SET_MATRIX(1.000000f, 0.000000f, 1.474600f, 0.000000f, 1.000000f, -0.164550f, -0.571350f, 0.000000f, 1.000000f, 1.881400f, 0.000000f, 0.000000f);
            VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT2020, CSpace_sRGB, TempMatrix, PriorCscMatrix);
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Invalid Prior CSC parameter.");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        fTemp1X = fTempX;
        fTemp1Y = fTempY;
        fTemp1Z = fTempZ;

        if (input && (input->osSurface->Format == Format_AYUV))
        {
            fTempX = PriorCscMatrix[0] * fTemp1Y + PriorCscMatrix[1] * fTemp1Z + PriorCscMatrix[2] * fTemp1X + PriorCscMatrix[3];
            fTempY = PriorCscMatrix[4] * fTemp1Y + PriorCscMatrix[5] * fTemp1Z + PriorCscMatrix[6] * fTemp1X + PriorCscMatrix[7];
            fTempZ = PriorCscMatrix[8] * fTemp1Y + PriorCscMatrix[9] * fTemp1Z + PriorCscMatrix[10] * fTemp1X + PriorCscMatrix[11];
        }
        else
        {
            fTempX = PriorCscMatrix[0] * fTemp1Z + PriorCscMatrix[1] * fTemp1Y + PriorCscMatrix[2] * fTemp1X + PriorCscMatrix[3];
            fTempY = PriorCscMatrix[4] * fTemp1Z + PriorCscMatrix[5] * fTemp1Y + PriorCscMatrix[6] * fTemp1X + PriorCscMatrix[7];
            fTempZ = PriorCscMatrix[8] * fTemp1Z + PriorCscMatrix[9] * fTemp1Y + PriorCscMatrix[10] * fTemp1X + PriorCscMatrix[11];
        }

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

    if (params->StageEnableFlags[iIndex].EOTFEnable)
    {
        if (params->EOTFGamma[iIndex] == VPHAL_GAMMA_TRADITIONAL_GAMMA)
        {
            if (fTempX < 0.081)
            {
                fTempX = fTempX / 4.5;
            }
            else
            {
                fTempX = (fTempX + 0.099) / 1.099;
                fTempX = pow(fTempX, 1.0 / 0.45);
            }

            if (fTempY < 0.081)
            {
                fTempY = fTempY / 4.5;
            }
            else
            {
                fTempY = (fTempY + 0.099) / 1.099;
                fTempY = pow(fTempY, 1.0 / 0.45);
            }

            if (fTempZ < 0.081)
            {
                fTempZ = fTempZ / 4.5;
            }
            else
            {
                fTempZ = (fTempZ + 0.099) / 1.099;
                fTempZ = pow(fTempZ, 1.0 / 0.45);
            }
        }
        else if (params->EOTFGamma[iIndex] == VPHAL_GAMMA_SMPTE_ST2084)
        {
            fTempX = pow(fTempX, 1.0f / m2);
            fTemp  = c2 - c3 * fTempX;
            fTempX = fTempX > c1 ? fTempX - c1 : 0;
            fTempX = fTempX / fTemp;
            fTempX = pow(fTempX, 1.0f / m1);

            fTempY = pow(fTempY, 1.0f / m2);
            fTemp  = c2 - c3 * fTempY;
            fTempY = fTempY > c1 ? fTempY - c1 : 0;
            fTempY = fTempY / fTemp;
            fTempY = pow(fTempY, 1.0f / m1);

            fTempZ = pow(fTempZ, 1.0f / m2);
            fTemp  = c2 - c3 * fTempZ;
            fTempZ = fTempZ > c1 ? fTempZ - c1 : 0;
            fTempZ = fTempZ / fTemp;
            fTempZ = pow(fTempZ, 1.0f / m1);
        }
        else if (params->EOTFGamma[iIndex] == VPHAL_GAMMA_BT1886)
        {
            if (fTempX < -0.0f)
            {
                fTempX = 0;
            }
            else
            {
                fTempX = pow(fTempX, 2.4);
            }

            if (fTempY < -0.0f)
            {
                fTempY = 0;
            }
            else
            {
                fTempY = pow(fTempY, 2.4);
            }

            if (fTempZ < -0.0f)
            {
                fTempZ = 0;
            }
            else
            {
                fTempZ = pow(fTempZ, 2.4);
            }
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

    if (params->StageEnableFlags[iIndex].CCMEnable)
    {
        // BT709 to BT2020 CCM
        if (params->CCM[iIndex] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
        }
        // BT2020 to BT709 CCM
        else if (params->CCM[iIndex] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f);
        }
        else
        {
            SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        }

        fTemp1X = fTempX;
        fTemp1Y = fTempY;
        fTemp1Z = fTempZ;

        fTempX = TempMatrix[0] * fTemp1X + TempMatrix[1] * fTemp1Y + TempMatrix[2] * fTemp1Z + TempMatrix[3];
        fTempY = TempMatrix[4] * fTemp1X + TempMatrix[5] * fTemp1Y + TempMatrix[6] * fTemp1Z + TempMatrix[7];
        fTempZ = TempMatrix[8] * fTemp1X + TempMatrix[9] * fTemp1Y + TempMatrix[10] * fTemp1Z + TempMatrix[11];

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

    if (params->StageEnableFlags[iIndex].PWLFEnable)
    {
        VpHal_HdrToneMapping3dLut(params->HdrMode[iIndex], fTempX, fTempY, fTempZ, &fTempX, &fTempY, &fTempZ);

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

    if (params->StageEnableFlags[iIndex].CCMExt1Enable)
    {
        // BT709 to BT2020 CCM
        if (params->CCMExt1[iIndex] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
        }
        // BT2020 to BT709 CCM
        else if (params->CCMExt1[iIndex] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f);
        }
        else if (params->CCMExt1[iIndex] == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                 params->CCMExt1[iIndex] == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                 params->CCMExt1[iIndex] == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
        {
            HdrCalculateCCMWithMonitorGamut(params->CCMExt1[iIndex], params->targetHDRParams[0], TempMatrix);
        }
        else
        {
            SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        }

        fTemp1X = fTempX;
        fTemp1Y = fTempY;
        fTemp1Z = fTempZ;

        fTempX = TempMatrix[0] * fTemp1X + TempMatrix[1] * fTemp1Y + TempMatrix[2] * fTemp1Z + TempMatrix[3];
        fTempY = TempMatrix[4] * fTemp1X + TempMatrix[5] * fTemp1Y + TempMatrix[6] * fTemp1Z + TempMatrix[7];
        fTempZ = TempMatrix[8] * fTemp1X + TempMatrix[9] * fTemp1Y + TempMatrix[10] * fTemp1Z + TempMatrix[11];

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

    if (params->StageEnableFlags[iIndex].CCMExt2Enable)
    {
        // BT709 to BT2020 CCM
        if (params->CCMExt2[iIndex] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
        }
        // BT2020 to BT709 CCM
        else if (params->CCMExt2[iIndex] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f);
        }
        else if (params->CCMExt2[iIndex] == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                 params->CCMExt2[iIndex] == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                 params->CCMExt2[iIndex] == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
        {
            HdrCalculateCCMWithMonitorGamut(params->CCMExt2[iIndex], params->targetHDRParams[0], TempMatrix);
        }
        else
        {
            SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        }

        fTemp1X = fTempX;
        fTemp1Y = fTempY;
        fTemp1Z = fTempZ;

        fTempX = TempMatrix[0] * fTemp1X + TempMatrix[1] * fTemp1Y + TempMatrix[2] * fTemp1Z + TempMatrix[3];
        fTempY = TempMatrix[4] * fTemp1X + TempMatrix[5] * fTemp1Y + TempMatrix[6] * fTemp1Z + TempMatrix[7];
        fTempZ = TempMatrix[8] * fTemp1X + TempMatrix[9] * fTemp1Y + TempMatrix[10] * fTemp1Z + TempMatrix[11];

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

    if (params->OETFGamma[iIndex] != VPHAL_GAMMA_NONE)
    {
        if (params->OETFGamma[iIndex] == VPHAL_GAMMA_TRADITIONAL_GAMMA)
        {
            if (fTempX < 0.018)
            {
                fTempX = 4.5 * fTempX;
            }
            else
            {
                fTempX = pow(fTempX, 0.45);
                fTempX = 1.099 * fTempX - 0.099;
            }

            if (fTempY < 0.018)
            {
                fTempY = 4.5 * fTempY;
            }
            else
            {
                fTempY = pow(fTempY, 0.45);
                fTempY = 1.099 * fTempY - 0.099;
            }

            if (fTempZ < 0.018)
            {
                fTempZ = 4.5 * fTempZ;
            }
            else
            {
                fTempZ = pow(fTempZ, 0.45);
                fTempZ = 1.099 * fTempZ - 0.099;
            }
        }
        else if (params->OETFGamma[iIndex] == VPHAL_GAMMA_SMPTE_ST2084)
        {
            fTempX = pow(fTempX, m1);
            fTempX = (c1 + c2 * fTempX) / (1 + c3 * fTempX);
            fTempX = pow(fTempX, m2);

            fTempY = pow(fTempY, m1);
            fTempY = (c1 + c2 * fTempY) / (1 + c3 * fTempY);
            fTempY = pow(fTempY, m2);

            fTempZ = pow(fTempZ, m1);
            fTempZ = (c1 + c2 * fTempZ) / (1 + c3 * fTempZ);
            fTempZ = pow(fTempZ, m2);
        }
        else if (params->OETFGamma[iIndex] == VPHAL_GAMMA_SRGB)
        {
            if (fTempX < 0.0031308f)
            {
                fTempX = 12.92 * fTempX;
            }
            else
            {
                fTempX = pow(fTempX, (double)(1.0f / 2.4f));
                fTempX = 1.055 * fTempX - 0.055;
            }

            if (fTempY < 0.0031308f)
            {
                fTempY = 12.92 * fTempY;
            }
            else
            {
                fTempY = pow(fTempY, (double)(1.0f / 2.4f));
                fTempY = 1.055 * fTempY - 0.055;
            }

            if (fTempZ < 0.0031308f)
            {
                fTempZ = 12.92 * fTempZ;
            }
            else
            {
                fTempZ = pow(fTempZ, (double)(1.0f / 2.4f));
                fTempZ = 1.055 * fTempZ - 0.055;
            }
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

    // OETF will output RGB surface
    // So if post CSC is needed, it will always be RGB to YUV conversion
    if (params->PostCSC[iIndex] != VPHAL_HDR_CSC_NONE)
    {
        if (params->PostCSC[iIndex] == VPHAL_HDR_CSC_RGB_TO_YUV_BT601)
        {
            SET_MATRIX(-0.331264f, -0.168736f, 0.500000f, 0.000000f, 0.587000f, 0.299000f, 0.114000f, 0.000000f, -0.418688f, 0.500000f, -0.081312f, 0.000000f);

            SET_MATRIX(0.500000f, -0.418688f, -0.081312f, 0.000000f, 0.299000f, 0.587000f, 0.114000f, 0.000000f, -0.168736f, -0.331264f, 0.500000f, 0.000000f);

            VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT601, TempMatrix, PostCscMatrix);
        }
        else if (params->PostCSC[iIndex] == VPHAL_HDR_CSC_RGB_TO_YUV_BT709)
        {
            SET_MATRIX(-0.385428f, -0.114572f, 0.500000f, 0.000000f, 0.715200f, 0.212600f, 0.072200f, 0.000000f, -0.454153f, 0.500000f, -0.045847f, 0.000000f);

            SET_MATRIX(0.500000f, -0.454153f, -0.045847f, 0.000000f, 0.212600f, 0.715200f, 0.072200f, 0.000000f, -0.114572f, -0.385428f, 0.500000f, 0.000000f);

            VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT709, TempMatrix, PostCscMatrix);
        }
        else if (params->PostCSC[iIndex] == VPHAL_HDR_CSC_RGB_TO_YUV_BT2020)
        {
            SET_MATRIX(-0.360370f, -0.139630f, 0.500000f, 0.000000f, 0.678000f, 0.262700f, 0.059300f, 0.000000f, -0.459786f, 0.500000f, -0.040214f, 0.000000f);

            SET_MATRIX(0.500000f, -0.459786f, -0.040214f, 0.000000f, 0.262700f, 0.678000f, 0.059300f, 0.000000f, -0.139630f, -0.360370f, 0.500000f, 0.000000f);

            VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT2020, TempMatrix, PostCscMatrix);
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Color Space Not found.");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        fTemp1X = fTempX;
        fTemp1Y = fTempY;
        fTemp1Z = fTempZ;

        fTempX = PostCscMatrix[0] * fTemp1X + PostCscMatrix[1] * fTemp1Y + PostCscMatrix[2] * fTemp1Z + PostCscMatrix[3];
        fTempY = PostCscMatrix[4] * fTemp1X + PostCscMatrix[5] * fTemp1Y + PostCscMatrix[6] * fTemp1Z + PostCscMatrix[7];
        fTempZ = PostCscMatrix[8] * fTemp1X + PostCscMatrix[9] * fTemp1Y + PostCscMatrix[10] * fTemp1Z + PostCscMatrix[11];

        CLAMP_MIN_MAX(fTempX, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempY, 0.0f, 1.0f);
        CLAMP_MIN_MAX(fTempZ, 0.0f, 1.0f);
    }

#undef SET_MATRIX
#undef CLAMP_MIN_MAX

    if (params->bGpuGenerate3DLUT)
    {
        params->f3DLUTNormalizationFactor = 1023.0f;
    }
    else
    {
        params->f3DLUTNormalizationFactor = 65535.0f;
    }

    // Convert and round up the [0, 1] float color value to 16 bit integer value
    *puOutputX = (uint16_t)(fTempX * params->f3DLUTNormalizationFactor + 0.5f);
    *puOutputY = (uint16_t)(fTempY * params->f3DLUTNormalizationFactor + 0.5f);
    *puOutputZ = (uint16_t)(fTempZ * params->f3DLUTNormalizationFactor + 0.5f);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Color Transfer for Hdr 3d Lut
//! \details  Color Transfer for Hdr 3d Lut
//! \param    VPHAL_HDR_MODE HdrMode
//!           [in] Hdr mode
//! \param    double fInputX
//!           [in] Input color for Tone Mapping
//! \param    double fInputY
//!           [in] Input color for Tone Mapping
//! \param    double fInputZ
//!           [in] Input color for Tone Mapping
//! \param    double * pfOutputX
//!           [in/out] Output color for Tone Mapping
//! \param    double * pfOutputY
//!           [in/out] Output color for Tone Mapping
//! \param    double * pfOutputZ
//!           [in/out] Output color for Tone Mapping
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::VpHal_HdrToneMapping3dLut(
    VPHAL_HDR_MODE HdrMode,
    double         fInputX,
    double         fInputY,
    double         fInputZ,
    double        *pfOutputX,
    double        *pfOutputY,
    double        *pfOutputZ)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    double     fPivot[5] = {};
    double     fSlope[6] = {};
    double     fIntercept[6] = {};

    VP_PUBLIC_CHK_NULL_RETURN(pfOutputX);
    VP_PUBLIC_CHK_NULL_RETURN(pfOutputX);
    VP_PUBLIC_CHK_NULL_RETURN(pfOutputX);

#define TONE_MAPPING(Input, Output)                     \
    {                                                   \
        if (Input < fPivot[0])                          \
        {                                               \
            Output = Input * fSlope[0] + fIntercept[0]; \
        }                                               \
        else if (Input < fPivot[1])                     \
        {                                               \
            Output = Input * fSlope[1] + fIntercept[1]; \
        }                                               \
        else if (Input < fPivot[2])                     \
        {                                               \
            Output = Input * fSlope[2] + fIntercept[2]; \
        }                                               \
        else if (Input < fPivot[3])                     \
        {                                               \
            Output = Input * fSlope[3] + fIntercept[3]; \
        }                                               \
        else if (Input < fPivot[4])                     \
        {                                               \
            Output = Input * fSlope[4] + fIntercept[4]; \
        }                                               \
        else                                            \
        {                                               \
            Output = Input * fSlope[5] + fIntercept[5]; \
        }                                               \
        if (Output > 1.0f)                              \
        {                                               \
            Output = 1.0f;                              \
        }                                               \
    }

    if (HdrMode == VPHAL_HDR_MODE_TONE_MAPPING || HdrMode == VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE)
    {
        // TODO: add auto mode support, i.e. read back the coeffs from audo mode output surface.
        fPivot[0]     = VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X1;
        fPivot[1]     = VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X2;
        fPivot[2]     = VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X3;
        fPivot[3]     = VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X4;
        fPivot[4]     = VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X5;
        fSlope[0]     = VPHAL_HDR_TONE_MAPPING_SLOPE0;
        fSlope[1]     = VPHAL_HDR_TONE_MAPPING_SLOPE1;
        fSlope[2]     = VPHAL_HDR_TONE_MAPPING_SLOPE2;
        fSlope[3]     = VPHAL_HDR_TONE_MAPPING_SLOPE3;
        fSlope[4]     = VPHAL_HDR_TONE_MAPPING_SLOPE4;
        fSlope[5]     = VPHAL_HDR_TONE_MAPPING_SLOPE5;
        fIntercept[0] = VPHAL_HDR_TONE_MAPPING_INTERCEPT0;
        fIntercept[1] = VPHAL_HDR_TONE_MAPPING_INTERCEPT1;
        fIntercept[2] = VPHAL_HDR_TONE_MAPPING_INTERCEPT2;
        fIntercept[3] = VPHAL_HDR_TONE_MAPPING_INTERCEPT3;
        fIntercept[4] = VPHAL_HDR_TONE_MAPPING_INTERCEPT4;
        fIntercept[5] = VPHAL_HDR_TONE_MAPPING_INTERCEPT5;

        TONE_MAPPING(fInputX, *pfOutputX);
        TONE_MAPPING(fInputY, *pfOutputY);
        TONE_MAPPING(fInputZ, *pfOutputZ);
    }
    else if (HdrMode == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
    {
        // For inverse tone mapping in 3D LUT
        // PWLF function should not be applied
        // A simple / 100 devision should be performed
        *pfOutputX = fInputX / 100.0f;
        *pfOutputY = fInputY / 100.0f;
        *pfOutputZ = fInputZ / 100.0f;
    }
    else if (HdrMode == VPHAL_HDR_MODE_H2H)
    {
        // TODO: port the H2H mapping algorithm
        *pfOutputX = fInputX;
        *pfOutputY = fInputY;
        *pfOutputZ = fInputZ;
    }

#undef TONE_MAPPING

    return eStatus;
}


//!
//! \brief    Initiate EOTF Surface for HDR
//! \details  Initiate EOTF Surface for HDR
//! \param    PVPHAL_HDR_STATE paramse
//!           [in] Pointer to HDR state
//! \param    int32_t iIndex
//!           [in] input surface index
//! \param    PVPHAL_SURFACE pOETF1DLUTSurface
//!           [in] Pointer to OETF 1D LUT Surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::InitOETF1DLUT(
    PRENDER_HDR_PARAMS params,
    int32_t            iIndex,
    VP_SURFACE         *pOETF1DLUTSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus     = MOS_STATUS_SUCCESS;
    uint32_t        i           = 0;
    uint16_t       *pSrcOetfLut = nullptr;
    uint8_t        *pDstOetfLut = nullptr;
    MOS_LOCK_PARAMS LockFlags   = {};

    VP_PUBLIC_CHK_NULL_RETURN(pOETF1DLUTSurface);
    VP_PUBLIC_CHK_NULL_RETURN(pOETF1DLUTSurface->osSurface);

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    // Lock the surface for writing
    pDstOetfLut = (uint8_t *)m_allocator->Lock(
        &(pOETF1DLUTSurface->osSurface->OsResource),
        &LockFlags);

    VP_PUBLIC_CHK_NULL_RETURN(pDstOetfLut);

    // Hdr kernel require 0 to 1 floating point color value
    // To transfer the value of 16bit integer OETF table to 0 to 1 floating point
    // We need to divide the table with 2^16 - 1
    if ((params->targetHDRParams[0].EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
            params->targetHDRParams[0].EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR))
    {
        if (params->OETFGamma[iIndex] == VPHAL_GAMMA_SRGB)
        {
            pSrcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_sRGB_FP16;
        }
        else
        {
            pSrcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_BT709_FP16;
        }

        for (i = 0;
             i < pOETF1DLUTSurface->osSurface->dwHeight;
             i++, pDstOetfLut += pOETF1DLUTSurface->osSurface->dwPitch, pSrcOetfLut += pOETF1DLUTSurface->osSurface->dwWidth)
        {
            MOS_SecureMemcpy(pDstOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->osSurface->dwWidth, pSrcOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->osSurface->dwWidth);
        }
    }
    else if (params->targetHDRParams[0].EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
    {
        if (params->HdrMode[iIndex] == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
        {
            const float fStretchFactor = 0.01f;
            HdrGenerate2SegmentsOETFLUT(fStretchFactor, HdrOETF2084, params->OetfSmpteSt2084);
            pSrcOetfLut = params->OetfSmpteSt2084;
        }
        else  // params->HdrMode[iIndex] == VPHAL_HDR_MODE_H2H
        {
            pSrcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_SMPTE_ST2084_3Segs_FP16;
        }

        for (i = 0;
             i < pOETF1DLUTSurface->osSurface->dwHeight;
             i++, pDstOetfLut += pOETF1DLUTSurface->osSurface->dwPitch, pSrcOetfLut += pOETF1DLUTSurface->osSurface->dwWidth)
        {
            MOS_SecureMemcpy(pDstOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->osSurface->dwWidth, pSrcOetfLut, sizeof(uint16_t) * pOETF1DLUTSurface->osSurface->dwWidth);
        }
    }
    else
    {
        VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&pOETF1DLUTSurface->osSurface->OsResource));

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

//!
//! \brief    Initiate Cri 3D Lut Surface for HDR
//! \details  Initiate Cri 3D Lut Surface for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \param    int32_t iIndex
//!           [in] input surface index
//! \param    PVPHAL_SURFACE pCRI3DLUTSurface
//!           [in/out] Pointer to Cri 3D Lut Surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::InitCri3DLUT(
    PRENDER_HDR_PARAMS params,
    int32_t          iIndex,
    VP_SURFACE       *pCRI3DLUTSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;
    uint32_t        i = 0, j = 0, k = 0;
    uint16_t        u3dLutOutputX = 0, u3dLutOutputY = 0, u3dLutOutputZ = 0;
    uint16_t       *pwDst3dLut = nullptr;
    uint32_t       *puiDst3dLut = nullptr;
    uint8_t        *pByte = nullptr;
    MOS_LOCK_PARAMS LockFlags     = {};
    uint8_t         bBytePerPixel = 0;

    VP_PUBLIC_CHK_NULL_RETURN(pCRI3DLUTSurface);
    VP_PUBLIC_CHK_NULL_RETURN(pCRI3DLUTSurface->osSurface);

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    // Lock the surface for writing
    pByte = (uint8_t *)m_allocator->Lock(
        &(pCRI3DLUTSurface->osSurface->OsResource),
        &LockFlags);

    VP_PUBLIC_CHK_NULL_RETURN(pByte);

    if (pCRI3DLUTSurface->osSurface->Format == Format_A16B16G16R16)
    {
        bBytePerPixel = 8;

        for (i = 0; i < params->Cri3DLUTSize; i++)
        {
            for (j = 0; j < params->Cri3DLUTSize; j++)
            {
                for (k = 0; k < params->Cri3DLUTSize; k++)
                {
                    pwDst3dLut = (uint16_t *)(pByte +
                                              i * params->Cri3DLUTSize * pCRI3DLUTSurface->osSurface->dwPitch +
                                              j * pCRI3DLUTSurface->osSurface->dwPitch +
                                              k * bBytePerPixel);

                    u3dLutOutputX = u3dLutOutputY = u3dLutOutputZ = 0;

                    VpHal_HdrColorTransfer3dLut(params,
                        iIndex,
                        (float)k / (float)(params->Cri3DLUTSize - 1),
                        (float)j / (float)(params->Cri3DLUTSize - 1),
                        (float)i / (float)(params->Cri3DLUTSize - 1),
                        &u3dLutOutputX,
                        &u3dLutOutputY,
                        &u3dLutOutputZ);

                    *pwDst3dLut++ = u3dLutOutputX;
                    *pwDst3dLut++ = u3dLutOutputY;
                    *pwDst3dLut++ = u3dLutOutputZ;
                }
            }
        }

        eStatus = MOS_STATUS_SUCCESS;
    }
    else if (pCRI3DLUTSurface->osSurface->Format == Format_R10G10B10A2)
    {
        bBytePerPixel = 4;

        for (i = 0; i < params->Cri3DLUTSize; i++)
        {
            for (j = 0; j < params->Cri3DLUTSize; j++)
            {
                for (k = 0; k < params->Cri3DLUTSize; k++)
                {
                    puiDst3dLut = (uint32_t *)(pByte +
                                               i * params->Cri3DLUTSize * pCRI3DLUTSurface->osSurface->dwPitch +
                                               j * pCRI3DLUTSurface->osSurface->dwPitch +
                                               k * bBytePerPixel);

                    u3dLutOutputX = u3dLutOutputY = u3dLutOutputZ = 0;

                    VpHal_HdrColorTransfer3dLut(params,
                        iIndex,
                        (float)k / (float)(params->Cri3DLUTSize - 1),
                        (float)j / (float)(params->Cri3DLUTSize - 1),
                        (float)i / (float)(params->Cri3DLUTSize - 1),
                        &u3dLutOutputX,
                        &u3dLutOutputY,
                        &u3dLutOutputZ);

                    *puiDst3dLut = (uint32_t)u3dLutOutputX +
                                   ((uint32_t)u3dLutOutputY << 10) +
                                   ((uint32_t)u3dLutOutputZ << 20);
                }
            }
        }

        eStatus = MOS_STATUS_SUCCESS;
    }
    else
    {
        VP_RENDER_ASSERTMESSAGE("Unexpected HDR 3DLUT format.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&pCRI3DLUTSurface->osSurface->OsResource));

    return eStatus;
}

//!
//! \brief    Calculate H2H PWLF Coefficients
//! \details  Calculate H2H PWLF Coefficients
//! \param    PVPHAL_HDR_PARAMS pSource
//!           [in] Pointer to source surface HDR params
//! \param    PVPHAL_HDR_PARAMS pTarget
//!           [in] Pointer to target surface HDR params
//! \param    FLOAT* pTarget
//!           [in] Pointer to target surface
//! \param    float* pPivotPoint
//!           [out] Pointer to output pivot positions.
//! \param    uint16_t* pSlopeIntercept
//!           [out] Pointer to output slope and intercepts.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to MOS interface for HDR key report.
//! \return   void
//!
void VpRenderHdrKernel::CalculateH2HPWLFCoefficients(
    HDR_PARAMS    *pSource,
    HDR_PARAMS    *pTarget,
    float         *pPivotPoint,
    uint16_t      *pSlopeIntercept,
    PMOS_INTERFACE pOsInterface)
{
    VP_FUNC_CALL();

    float pivot0_x;
    float pivot1_x;
    float pivot2_x;
    float pivot3_x;
    float pivot4_x;
    float pivot5_x;

    float pivot0_y;
    float pivot1_y;
    float pivot2_y;
    float pivot3_y;
    float pivot4_y;
    float pivot5_y;

    float sf1;
    float sf2;
    float sf3;
    float sf4;
    float sf5;

    float in1f;
    float in2f;
    float in3f;
    float in4f;
    float in5f;

    const float minLumDisp = pTarget->min_display_mastering_luminance / 10000.0f / 10000.0f;
    const float maxLumDisp = pTarget->max_display_mastering_luminance / 10000.0f;
    const bool  align33Lut = true;
    const int   lutEntries = 32;
    const float lutStep    = 1.0f / lutEntries;
    const float split      = 0.7f;

    pivot0_x = 0.0f;
    pivot1_x = 0.0313f;
    pivot5_x = pSource->MaxCLL / 10000.0f;

    //pivot0_y = minLumDisp;
    pivot0_y = 0.0f;  // Force Y0 to zero to workaround a green bar issue.
    pivot1_y = 0.0313f;
    pivot5_y = maxLumDisp;

    if (pTarget->max_display_mastering_luminance >= pSource->MaxCLL)
    {
        pivot2_x = pivot3_x = pivot4_x = pivot5_x = pivot2_y = pivot3_y = pivot4_y = maxLumDisp;
    }
    else
    {
        if (align33Lut)
        {
            pivot5_x = ceil(pivot5_x / lutStep) * lutStep;
        }

        pivot2_x = pivot1_x + (pivot5_x - pivot1_x) / 5.0f;
        pivot3_x = pivot1_x + (pivot5_x - pivot1_x) * 2.0f / 5.0f;
        pivot4_x = pivot1_x + (pivot5_x - pivot1_x) * 3.0f / 5.0f;

        if (align33Lut)
        {
            pivot2_x = floor(pivot2_x / lutStep) * lutStep;
            pivot3_x = floor(pivot3_x / lutStep) * lutStep;
            pivot4_x = floor(pivot4_x / lutStep) * lutStep;
        }

        pivot4_y = pivot5_y * 0.95f;
        if (pivot4_y > pivot4_x)
        {
            pivot4_y = pivot4_x;
        }

        pivot2_y = pivot1_y + (pivot4_y - pivot1_y) * split;
        if (pivot2_y > pivot2_x)
        {
            pivot2_y = pivot2_x;
        }

        pivot3_y = pivot2_y + (pivot4_y - pivot2_y) * split;
        if (pivot3_y > pivot3_x)
        {
            pivot3_y = pivot3_x;
        }
    }


    // Calculate Gradient and Intercepts
    sf1      = (pivot1_x - pivot0_x) > 0.0f ? (float)(pivot1_y - pivot0_y) / (pivot1_x - pivot0_x) : 0.0f;
    pivot1_y = sf1 * (pivot1_x - pivot0_x) + pivot0_y;

    sf2      = (pivot2_x - pivot1_x) > 0.0f ? (float)(pivot2_y - pivot1_y) / (pivot2_x - pivot1_x) : 0.0f;
    pivot2_y = sf2 * (pivot2_x - pivot1_x) + pivot1_y;

    sf3      = (pivot3_x - pivot2_x) > 0.0f ? (float)(pivot3_y - pivot2_y) / (pivot3_x - pivot2_x) : 0.0f;
    pivot3_y = sf3 * (pivot3_x - pivot2_x) + pivot2_y;

    sf4      = (pivot4_x - pivot3_x) > 0.0f ? (float)(pivot4_y - pivot3_y) / (pivot4_x - pivot3_x) : 0.0f;
    pivot4_y = sf4 * (pivot4_x - pivot3_x) + pivot3_y;

    sf5      = (pivot5_x - pivot4_x) > 0.0f ? (float)(pivot5_y - pivot4_y) / (pivot5_x - pivot4_x) : 0.0f;
    pivot5_y = sf5 * (pivot5_x - pivot4_x) + pivot4_y;

    // Calculating Intercepts
    in1f = pivot0_y;
    in2f = pivot1_y - (sf2 * pivot1_x);
    in3f = pivot2_y - (sf3 * pivot2_x);
    in4f = pivot3_y - (sf4 * pivot3_x);
    in5f = pivot4_y - (sf5 * pivot4_x);

    pPivotPoint[0] = pivot1_x;
    pPivotPoint[1] = pivot2_x;
    pPivotPoint[2] = pivot3_x;
    pPivotPoint[3] = pivot4_x;
    pPivotPoint[4] = pivot5_x;

    pSlopeIntercept[0]  = VpHal_FloatToHalfFloat(sf1);
    pSlopeIntercept[1]  = VpHal_FloatToHalfFloat(in1f);
    pSlopeIntercept[2]  = VpHal_FloatToHalfFloat(sf2);
    pSlopeIntercept[3]  = VpHal_FloatToHalfFloat(in2f);
    pSlopeIntercept[4]  = VpHal_FloatToHalfFloat(sf3);
    pSlopeIntercept[5]  = VpHal_FloatToHalfFloat(in3f);
    pSlopeIntercept[6]  = VpHal_FloatToHalfFloat(sf4);
    pSlopeIntercept[7]  = VpHal_FloatToHalfFloat(in4f);
    pSlopeIntercept[8]  = VpHal_FloatToHalfFloat(sf5);
    pSlopeIntercept[9]  = VpHal_FloatToHalfFloat(in5f);
    pSlopeIntercept[10] = VpHal_FloatToHalfFloat(0.0f);  // Saturation
    pSlopeIntercept[11] = VpHal_FloatToHalfFloat(pivot5_y);
}
//!

//!
//! \brief    Initiate EOTF Surface for HDR
//! \details  Initiate EOTF Surface for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \param    PVPHAL_SURFACE pCoeffSurface
//!           [in] Pointer to CSC/CCM Surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::HdrInitCoeff(
    PRENDER_HDR_PARAMS         params,
    VP_SURFACE                *pCoeffSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus            = MOS_STATUS_SUCCESS;
    MOS_INTERFACE  *pOsInterface       = nullptr;
    uint32_t        i                  = 0;
    float          *pFloat             = nullptr;
    float          *pCoeff             = nullptr;
    uint32_t       *pEOTFType          = nullptr;
    float          *pEOTFCoeff         = nullptr;
    float          *pPivotPoint        = nullptr;
    uint32_t       *pTMType            = nullptr;
    float          *pFP16Normalizer    = nullptr;
    uint32_t       *pOETFNeqType       = nullptr;
    uint32_t       *pCCMEnable         = nullptr;
    float          *pPWLFStretch       = nullptr;
    float          *pCoeffR            = nullptr;
    float          *pCoeffG            = nullptr;
    float          *pCoeffB            = nullptr;
    uint16_t       *pSlopeIntercept    = nullptr;
    MOS_LOCK_PARAMS LockFlags          = {};
    float           PriorCscMatrix[12] = {};
    float           PostCscMatrix[12]  = {};
    float           CcmMatrix[12]      = {};
    float           TempMatrix[12]     = {};
    uint8_t        *pByte              = nullptr;

    MediaUserSettingSharedPtr userSettingPtr = nullptr;
    uint32_t                  coeffR = 0, coeffG = 0, coeffB = 0;
    uint32_t                  pivot0x = 0, pivot1x = 0, pivot2x = 0, pivot3x = 0, pivot4x = 0, pivot5x = 0;
    uint32_t                  pivot0y = 0, pivot1y = 0, pivot2y = 0, pivot3y = 0, pivot4y = 0, pivot5y = 0;

    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(pCoeffSurface);
    VP_PUBLIC_CHK_NULL_RETURN(pCoeffSurface->osSurface);

    eStatus = MOS_STATUS_SUCCESS;
    pOsInterface = m_hwInterface->m_osInterface;
    VP_PUBLIC_CHK_NULL_RETURN(pOsInterface);

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    pFloat = (float *)m_allocator->Lock(
        &pCoeffSurface->osSurface->OsResource,
        &LockFlags);
    VP_PUBLIC_CHK_NULL_RETURN(pFloat);

#define SET_MATRIX(_c0, _c1, _c2, _c3, _c4, _c5, _c6, _c7, _c8, _c9, _c10, _c11) \
    {                                                                            \
        TempMatrix[0]  = _c0;                                                    \
        TempMatrix[1]  = _c1;                                                    \
        TempMatrix[2]  = _c2;                                                    \
        TempMatrix[3]  = _c3;                                                    \
        TempMatrix[4]  = _c4;                                                    \
        TempMatrix[5]  = _c5;                                                    \
        TempMatrix[6]  = _c6;                                                    \
        TempMatrix[7]  = _c7;                                                    \
        TempMatrix[8]  = _c8;                                                    \
        TempMatrix[9]  = _c9;                                                    \
        TempMatrix[10] = _c10;                                                   \
        TempMatrix[11] = _c11;                                                   \
    }

#define SET_EOTF_COEFF(_c1, _c2, _c3, _c4, _c5)                          \
    {                                                                    \
        *pEOTFCoeff = _c1;                                               \
        pEOTFCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff = _c2;                                               \
        pEOTFCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff = _c3;                                               \
        pEOTFCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff = _c4;                                               \
        pEOTFCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float); \
        *pEOTFCoeff = _c5;                                               \
    }

#define WRITE_MATRIX(Matrix)                                             \
    {                                                                    \
        *pCoeff++ = Matrix[0];                                           \
        *pCoeff++ = Matrix[1];                                           \
        *pCoeff++ = Matrix[2];                                           \
        *pCoeff++ = Matrix[3];                                           \
        *pCoeff++ = Matrix[4];                                           \
        *pCoeff++ = Matrix[5];                                           \
        pCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float) - 6; \
        *pCoeff++ = Matrix[6];                                           \
        *pCoeff++ = Matrix[7];                                           \
        *pCoeff++ = Matrix[8];                                           \
        *pCoeff++ = Matrix[9];                                           \
        *pCoeff++ = Matrix[10];                                          \
        *pCoeff++ = Matrix[11];                                          \
        pCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float) - 6; \
    }

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++, pFloat += VPHAL_HDR_COEF_LINES_PER_LAYER_BASIC * pCoeffSurface->osSurface->dwPitch / (sizeof(float)))
    {
        if (params->InputSrc[i] == false)
        {
            continue;
        }

        pCoeff = pFloat;

        // EOTF/CCM/Tone Mapping/OETF require RGB input
        // So if prior CSC is needed, it will always be YUV to RGB conversion
        if (params->StageEnableFlags[i].PriorCSCEnable)
        {
            if (params->PriorCSC[i] == VPHAL_HDR_CSC_YUV_TO_RGB_BT601)
            {
                SET_MATRIX(1.000000f, 0.000000f, 1.402000f, 0.000000f, 1.000000f, -0.344136f, -0.714136f, 0.000000f, 1.000000f, 1.772000f, 0.000000f, 0.000000f);

                VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT601, CSpace_sRGB, TempMatrix, PriorCscMatrix);
            }
            else if (params->PriorCSC[i] == VPHAL_HDR_CSC_YUV_TO_RGB_BT709)
            {
                SET_MATRIX(1.000000f, 0.000000f, 1.574800f, 0.000000f, 1.000000f, -0.187324f, -0.468124f, 0.000000f, 1.000000f, 1.855600f, 0.000000f, 0.000000f);
                VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT709, CSpace_sRGB, TempMatrix, PriorCscMatrix);
            }
            else if (params->PriorCSC[i] == VPHAL_HDR_CSC_YUV_TO_RGB_BT2020)
            {
                SET_MATRIX(1.000000f, 0.000000f, 1.474600f, 0.000000f, 1.000000f, -0.164550f, -0.571350f, 0.000000f, 1.000000f, 1.881400f, 0.000000f, 0.000000f);
                VpHal_HdrCalcYuvToRgbMatrix(CSpace_BT2020, CSpace_sRGB, TempMatrix, PriorCscMatrix);
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Color Space Not found.");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            HdrLimitFP32ArrayPrecisionToF3_9(PriorCscMatrix, ARRAY_SIZE(PriorCscMatrix));
            WRITE_MATRIX(PriorCscMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float) * 2;
        }

        if (params->StageEnableFlags[i].CCMEnable)
        {
            // BT709 to BT2020 CCM
            if (params->CCM[i] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
            {
                SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
            }
            // BT2020 to BT709 CCM
            else if (params->CCM[i] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
            {
                SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f);
            }
            else
            {
                SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
            }

            VpHal_HdrCalcCCMMatrix(TempMatrix, CcmMatrix);
            HdrLimitFP32ArrayPrecisionToF3_9(CcmMatrix, ARRAY_SIZE(CcmMatrix));
            WRITE_MATRIX(CcmMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float) * 2;
        }

        // OETF will output RGB surface
        // So if post CSC is needed, it will always be RGB to YUV conversion
        if (params->StageEnableFlags[i].PostCSCEnable)
        {
            if (params->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT601)
            {
                SET_MATRIX(-0.331264f, -0.168736f, 0.500000f, 0.000000f, 0.587000f, 0.299000f, 0.114000f, 0.000000f, -0.418688f, 0.500000f, -0.081312f, 0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT601, TempMatrix, PostCscMatrix);
            }
            else if (params->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT709)
            {
                SET_MATRIX(-0.385428f, -0.114572f, 0.500000f, 0.000000f, 0.715200f, 0.212600f, 0.072200f, 0.000000f, -0.454153f, 0.500000f, -0.045847f, 0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT709, TempMatrix, PostCscMatrix);
            }
            else if (params->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT709_FULLRANGE)
            {
                SET_MATRIX(-0.385428f, -0.114572f, 0.500000f, 0.000000f, 0.715200f, 0.212600f, 0.072200f, 0.000000f, -0.454153f, 0.500000f, -0.045847f, 0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT709_FullRange, TempMatrix, PostCscMatrix);
            }
            else if (params->PostCSC[i] == VPHAL_HDR_CSC_RGB_TO_YUV_BT2020)
            {
                SET_MATRIX(-0.360370f, -0.139630f, 0.500000f, 0.000000f, 0.678000f, 0.262700f, 0.059300f, 0.000000f, -0.459786f, 0.500000f, -0.040214f, 0.000000f);
                VpHal_HdrCalcRgbToYuvMatrix(CSpace_sRGB, CSpace_BT2020, TempMatrix, PostCscMatrix);
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Color Space Not found.");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            HdrLimitFP32ArrayPrecisionToF3_9(PostCscMatrix, ARRAY_SIZE(PostCscMatrix));
            WRITE_MATRIX(PostCscMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float) * 2;
        }

        pEOTFType  = (uint32_t *)(pFloat + VPHAL_HDR_COEF_EOTF_OFFSET);
        pEOTFCoeff = pFloat + pCoeffSurface->osSurface->dwPitch / sizeof(float) + VPHAL_HDR_COEF_EOTF_OFFSET;

        if (params->StageEnableFlags[i].EOTFEnable)
        {
            if (params->EOTFGamma[i] == VPHAL_GAMMA_TRADITIONAL_GAMMA)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA);
            }
            else if (params->EOTFGamma[i] == VPHAL_GAMMA_SMPTE_ST2084)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_SMPTE_ST2084;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_SMPTE_ST2084,
                    VPHAL_HDR_EOTF_COEFF2_SMPTE_ST2084,
                    VPHAL_HDR_EOTF_COEFF3_SMPTE_ST2084,
                    VPHAL_HDR_EOTF_COEFF4_SMPTE_ST2084,
                    VPHAL_HDR_EOTF_COEFF5_SMPTE_ST2084);
            }
            else if (params->EOTFGamma[i] == VPHAL_GAMMA_BT1886)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_BT1886,
                    VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_BT1886,
                    VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_BT1886,
                    VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_BT1886,
                    VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_BT1886);
            }
            else if (params->EOTFGamma[i] == VPHAL_GAMMA_SRGB)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
                SET_EOTF_COEFF(VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_SRGB);
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        pEOTFType++;
        pEOTFCoeff = pFloat + pCoeffSurface->osSurface->dwPitch / sizeof(float) + VPHAL_HDR_COEF_EOTF_OFFSET + 1;

        if (params->StageEnableFlags[i].OETFEnable)
        {
            if (params->OETFGamma[i] == VPHAL_GAMMA_TRADITIONAL_GAMMA)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
                SET_EOTF_COEFF(VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA,
                    VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA);
            }
            else if (params->OETFGamma[i] == VPHAL_GAMMA_SRGB)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
                SET_EOTF_COEFF(VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA_SRGB,
                    VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA_SRGB);
            }
            else if (params->OETFGamma[i] == VPHAL_GAMMA_SMPTE_ST2084)
            {
                *pEOTFType = VPHAL_HDR_KERNEL_SMPTE_ST2084;
                SET_EOTF_COEFF(VPHAL_HDR_OETF_COEFF1_SMPTE_ST2084,
                    VPHAL_HDR_OETF_COEFF2_SMPTE_ST2084,
                    VPHAL_HDR_OETF_COEFF3_SMPTE_ST2084,
                    VPHAL_HDR_OETF_COEFF4_SMPTE_ST2084,
                    VPHAL_HDR_OETF_COEFF5_SMPTE_ST2084);
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        // NOTE:
        // Pitch is not equal to width usually. So please be careful when using pointer addition.
        // Only do this when operands are in the same row.
        pPivotPoint     = pFloat + pCoeffSurface->osSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_PIVOT_POINT_LINE_OFFSET;
        pSlopeIntercept = (uint16_t *)(pFloat + pCoeffSurface->osSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_SLOPE_INTERCEPT_LINE_OFFSET);
        pPWLFStretch    = pFloat + pCoeffSurface->osSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_PIVOT_POINT_LINE_OFFSET + 5;
        pTMType         = (uint32_t *)(pPWLFStretch);
        pCoeffR         = pPWLFStretch + 1;
        pCoeffG         = pCoeffR + 1;
        pCoeffB         = pFloat + pCoeffSurface->osSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_SLOPE_INTERCEPT_LINE_OFFSET + 6;
        pOETFNeqType    = (uint32_t *)(pFloat + pCoeffSurface->osSurface->dwPitch / sizeof(float) * VPHAL_HDR_COEF_SLOPE_INTERCEPT_LINE_OFFSET + 7);

        if (params->HdrMode[i] == VPHAL_HDR_MODE_TONE_MAPPING)
        {
            *pTMType      = 1;                  // TMtype
            *pOETFNeqType = 0 | (10000 << 16);  // OETFNEQ
            *pCoeffR      = 0.25f;
            *pCoeffG      = 0.625f;
            *pCoeffB      = 0.125f;

            float pivot0_x = 0.0f, pivot1_x = 0.03125f, pivot2_x = 0.09375f, pivot3_x = 0.125f, pivot4_x = 0.21875f, pivot5_x = 0.40625f;
            float pivot0_y = 0.0f, pivot1_y = 0.7f, pivot2_y = 0.9f, pivot3_y = 0.95f, pivot4_y = 0.99f, pivot5_y = 1.0f;
            float sf1, sf2, sf3, sf4, sf5;
            float in1f, in2f, in3f, in4f, in5f;

            pivot0_x = 0.0f;
            pivot0_y = 0.0f;

            // Calculate Gradient and Intercepts
            sf1      = (pivot1_x - pivot0_x) > 0.0f ? (float)(pivot1_y - pivot0_y) / (pivot1_x - pivot0_x) : 0.0f;
            pivot1_y = sf1 * (pivot1_x - pivot0_x) + pivot0_y;

            sf2      = (pivot2_x - pivot1_x) > 0.0f ? (float)(pivot2_y - pivot1_y) / (pivot2_x - pivot1_x) : 0.0f;
            pivot2_y = sf2 * (pivot2_x - pivot1_x) + pivot1_y;

            sf3      = (pivot3_x - pivot2_x) > 0.0f ? (float)(pivot3_y - pivot2_y) / (pivot3_x - pivot2_x) : 0.0f;
            pivot3_y = sf3 * (pivot3_x - pivot2_x) + pivot2_y;

            sf4      = (pivot4_x - pivot3_x) > 0.0f ? (float)(pivot4_y - pivot3_y) / (pivot4_x - pivot3_x) : 0.0f;
            pivot4_y = sf4 * (pivot4_x - pivot3_x) + pivot3_y;

            sf5      = (pivot5_x - pivot4_x) > 0.0f ? (float)(pivot5_y - pivot4_y) / (pivot5_x - pivot4_x) : 0.0f;
            pivot5_y = sf5 * (pivot5_x - pivot4_x) + pivot4_y;

            // Calculating Intercepts
            in1f = pivot0_y;
            in2f = pivot1_y - (sf2 * pivot1_x);
            in3f = pivot2_y - (sf3 * pivot2_x);
            in4f = pivot3_y - (sf4 * pivot3_x);
            in5f = pivot4_y - (sf5 * pivot4_x);

            // Pivot Point
            *pPivotPoint++ = pivot1_x;
            *pPivotPoint++ = pivot2_x;
            *pPivotPoint++ = pivot3_x;
            *pPivotPoint++ = pivot4_x;
            *pPivotPoint++ = pivot5_x;

            // Slope and Intercept
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(sf1);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(in1f);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(sf2);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(in2f);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(sf3);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(in3f);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(sf4);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(in4f);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(sf5);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(in5f);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(0.0f);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(pivot5_y);
        }
        else if (params->HdrMode[i] == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
        {
            *pPWLFStretch = 0.01f;                      // Stretch
            *pOETFNeqType = 1 | ((uint32_t)100 << 16);  // OETFNEQ
            *pCoeffR      = 0.0f;
            *pCoeffG      = 0.0f;
            *pCoeffB      = 0.0f;

            // Pivot Point
            *pPivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X1;
            *pPivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X2;
            *pPivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X3;
            *pPivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X4;
            *pPivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X5;

            // Slope and Intercept
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE0);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT0);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE1);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT1);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE2);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT2);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE3);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT3);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE4);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT4);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE5);
            *pSlopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT5);
        }
        else if (params->HdrMode[i] == VPHAL_HDR_MODE_H2H ||
                 params->HdrMode[i] == VPHAL_HDR_MODE_H2H_AUTO_MODE)
        {
            *pTMType      = 1;                                                 // TMtype
            *pOETFNeqType = 2 | (((uint32_t)(params->uiMaxDisplayLum)) << 16);  // OETFNEQ
            *pCoeffR      = 0.25f;
            *pCoeffG      = 0.625f;
            *pCoeffB      = 0.125f;

            CalculateH2HPWLFCoefficients(&params->srcHDRParams[0], &params->targetHDRParams[0], pPivotPoint, pSlopeIntercept, pOsInterface);

        }
        else
        {
            *pPivotPoint  = 0.0f;
            *pTMType      = 0;  // TMtype
            *pOETFNeqType = 0;  // OETFNEQ
        }
    }

    // Coef[64][7] is to normalize the fp16 input value
    pFP16Normalizer  = pFloat + 7;
    *pFP16Normalizer = 1.0f / 125.0f;

    // Skip the Dst CSC area
    pFloat += 2 * pCoeffSurface->osSurface->dwPitch / sizeof(float);

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++, pFloat += VPHAL_HDR_COEF_LINES_PER_LAYER_EXT * pCoeffSurface->osSurface->dwPitch / (sizeof(float)))
    {
        pCCMEnable                                   = (uint32_t *)pFloat;
        *(pCCMEnable + VPHAL_HDR_COEF_CCMEXT_OFFSET) = params->StageEnableFlags[i].CCMExt1Enable;
        *(pCCMEnable + VPHAL_HDR_COEF_CLAMP_OFFSET)  = params->StageEnableFlags[i].GamutClamp1Enable;

        pCCMEnable += pCoeffSurface->osSurface->dwPitch / sizeof(float) * 2;
        *(pCCMEnable + VPHAL_HDR_COEF_CCMEXT_OFFSET) = params->StageEnableFlags[i].CCMExt2Enable;
        *(pCCMEnable + VPHAL_HDR_COEF_CLAMP_OFFSET)  = params->StageEnableFlags[i].GamutClamp2Enable;

        if (params->InputSrc[i] == false)
        {
            continue;
        }

        pCoeff = pFloat;
        if (params->StageEnableFlags[i].CCMExt1Enable)
        {
            // BT709 to BT2020 CCM
            if (params->CCMExt1[i] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
            {
                SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
            }
            // BT2020 to BT709 CCM
            else if (params->CCMExt1[i] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
            {
                SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f);
            }
            else if (params->CCMExt1[i] == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                     params->CCMExt1[i] == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                     params->CCMExt1[i] == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
            {
                HdrCalculateCCMWithMonitorGamut(params->CCMExt1[i], params->targetHDRParams[0], TempMatrix);
            }
            else
            {
                SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
            }

            VpHal_HdrCalcCCMMatrix(TempMatrix, CcmMatrix);
            HdrLimitFP32ArrayPrecisionToF3_9(CcmMatrix, ARRAY_SIZE(CcmMatrix));
            WRITE_MATRIX(CcmMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float) * 2;
        }

        if (params->StageEnableFlags[i].CCMExt2Enable)
        {
            // BT709 to BT2020 CCM
            if (params->CCMExt2[i] == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
            {
                SET_MATRIX(0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f);
            }
            // BT2020 to BT709 CCM
            else if (params->CCMExt2[i] == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
            {
                SET_MATRIX(1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f);
            }
            else if (params->CCMExt2[i] == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                     params->CCMExt2[i] == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                     params->CCMExt2[i] == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
            {
                HdrCalculateCCMWithMonitorGamut(params->CCMExt2[i], params->targetHDRParams[0], TempMatrix);
            }
            else
            {
                SET_MATRIX(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
            }

            VpHal_HdrCalcCCMMatrix(TempMatrix, CcmMatrix);
            HdrLimitFP32ArrayPrecisionToF3_9(CcmMatrix, ARRAY_SIZE(CcmMatrix));
            WRITE_MATRIX(CcmMatrix);
        }
        else
        {
            pCoeff += pCoeffSurface->osSurface->dwPitch / sizeof(float) * 2;
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&pCoeffSurface->osSurface->OsResource));

    eStatus = MOS_STATUS_SUCCESS;

#undef SET_MATRIX
#undef SET_EOTF_COEFF
#undef WRITE_MATRIX

    return eStatus;
}

//!
//! \brief    Assemble the HDR kernel per layer stages
//! \details  Contruct a case id from the input information, and look up the configuration entry in the table.
//! \param    HDRStageConfigEntry *pConfigEntry
//!           [out] Pointer to configuration entry
//! \return   bool
//!           True if find proper configuration entry, otherwise false
//!
bool VpRenderHdrKernel::ToneMappingStagesAssemble(
    HDR_PARAMS          *srcHDRParams,
    HDR_PARAMS          *targetHDRParams,
    HDRStageConfigEntry *pConfigEntry,
    uint32_t index)
{
    HDRCaseID id = {0};
    VP_FUNC_CALL();
    VP_RENDER_ASSERT(pConfigEntry);
    VP_RENDER_ASSERT(m_hdrParams);
    VP_RENDER_ASSERT(srcHDRParams);
    VP_RENDER_ASSERT(targetHDRParams);

    auto        inputSurface = m_surfaceGroup->find(SurfaceType(SurfaceTypeHdrInputLayer0 + index));
    VP_SURFACE *input = (m_surfaceGroup->end() != inputSurface) ? inputSurface->second : nullptr;
    if (input == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("input surface creat failed, skip process");
        return false;
    }
    if (input->osSurface == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("input surface creat failed, skip process");
        return false;
    }

    auto        targetSurface = m_surfaceGroup->find(SurfaceTypeHdrTarget0);
    VP_SURFACE *target = (m_surfaceGroup->end() != targetSurface) ? targetSurface->second : nullptr;
    if (target == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("target surface creat failed, skip process");
        return false;
    }
    if (target->osSurface == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("input surface creat failed, skip process");
        return false;
    }

    // Because FP16 format can represent both SDR or HDR, we need do judgement here.
    // We need this information because we dont have unified tone mapping algorithm for various scenarios(H2S/H2H).
    // To do this, we make two assumptions:
    // 1. This colorspace will be set to BT709/Gamma1.0 from APP, so such information can NOT be used to check HDR.
    // 2. If APP pass any HDR metadata, it indicates this is HDR.
    id.InputXDR     = (srcHDRParams &&
                      ((srcHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084) || IS_RGB64_FLOAT_FORMAT(input->osSurface->Format)))
                          ? 1
                          : 0;
    id.InputGamut   = IS_COLOR_SPACE_BT2020(input->ColorSpace);
    id.OutputXDR    = (targetHDRParams &&
                       ((targetHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084) || IS_RGB64_FLOAT_FORMAT(target->osSurface->Format)))
                          ? 1
                          : 0;
    id.OutputGamut  = IS_COLOR_SPACE_BT2020(target->ColorSpace);
    id.OutputLinear = IS_RGB64_FLOAT_FORMAT(target->osSurface->Format) ? 1 : 0;

    if (m_hdrParams->pHDRStageConfigTable)
    {
        pConfigEntry->value = m_hdrParams->pHDRStageConfigTable[id.index];
    }
    else
    {
        pConfigEntry->Invalid = 1;
    }

    if (pConfigEntry->Invalid == 1)
    {
        VP_RENDER_ASSERTMESSAGE(
            "Tone mapping stages assembling failed, please reexamine the usage case(case id %d)! "
            "If it is definitely a correct usage, please add an entry in HDRStageEnableTable.",
            id.index);
    }

    return (pConfigEntry->Invalid != 1);
}


//!
//! \brief    Update per layer pipeline states and return update mask for each layer
//! \details  Update per layer pipeline states and return update mask for each layer
//! \param    uint32_t* pdwUpdateMask
//!           [out] Pointer to update mask
//! \return   MOS_STATUS
//!
MOS_STATUS VpRenderHdrKernel::UpdatePerLayerPipelineStates(
    uint32_t           *pdwUpdateMask)
{
    MOS_STATUS           eStatus              = MOS_STATUS_UNKNOWN;
    uint32_t             i                    = 0;
    PVPHAL_SURFACE       pSrc                 = nullptr;
    PVPHAL_SURFACE       pTarget              = nullptr;
    VPHAL_HDR_LUT_MODE   CurrentLUTMode       = VPHAL_HDR_LUT_MODE_NONE;
    VPHAL_GAMMA_TYPE     CurrentEOTF          = VPHAL_GAMMA_NONE;            //!< EOTF
    VPHAL_GAMMA_TYPE     CurrentOETF          = VPHAL_GAMMA_NONE;            //!< OETF
    VPHAL_HDR_MODE       CurrentHdrMode       = VPHAL_HDR_MODE_NONE;      //!< Hdr Mode
    VPHAL_HDR_CCM_TYPE   CurrentCCM           = VPHAL_HDR_CCM_NONE;           //!< CCM Mode
    VPHAL_HDR_CCM_TYPE   CurrentCCMExt1       = VPHAL_HDR_CCM_NONE;       //!< CCM Ext1 Mode
    VPHAL_HDR_CCM_TYPE   CurrentCCMExt2       = VPHAL_HDR_CCM_NONE;       //!< CCM Ext2 Mode
    VPHAL_HDR_CSC_TYPE   CurrentPriorCSC      = VPHAL_HDR_CSC_NONE;      //!< Prior CSC Mode
    VPHAL_HDR_CSC_TYPE   CurrentPostCSC       = VPHAL_HDR_CSC_NONE;       //!< Post CSC Mode
    HDRStageConfigEntry  ConfigEntry          = { 0 };
    HDRStageEnables      StageEnables         = { 0 };

    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(pdwUpdateMask);
    VP_RENDER_CHK_NULL_RETURN(m_hdrParams);

    //VP_PUBLIC_CHK_NULL(m_hdrParams->pTarget[0]);

    *pdwUpdateMask = 0;

    auto        targetSurface = m_surfaceGroup->find(SurfaceTypeHdrTarget0);
    VP_SURFACE *target = (m_surfaceGroup->end() != targetSurface) ? targetSurface->second : nullptr;
    if (target == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("target surface creat failed, skip process");
        return MOS_STATUS_NULL_POINTER;
    }
    VP_RENDER_CHK_NULL_RETURN(target->osSurface);

    for (i = 0; i < m_hdrParams->uSourceCount; i++)
    {
        auto        inputSurface = m_surfaceGroup->find(SurfaceType(SurfaceTypeHdrInputLayer0 + i));
        VP_SURFACE *input        = (m_surfaceGroup->end() != inputSurface) ? inputSurface->second : nullptr;
        if (input == nullptr)
        {
            VP_RENDER_ASSERTMESSAGE("input surface creat failed, skip process");
            return MOS_STATUS_NULL_POINTER;
        }
        VP_RENDER_CHK_NULL_RETURN(input->osSurface);
        if (m_hdrParams->InputSrc[i] == false)
        {
            m_hdrParams->LUTMode[i]        = VPHAL_HDR_LUT_MODE_NONE;
            m_hdrParams->EOTFGamma[i] = VPHAL_GAMMA_NONE;
            m_hdrParams->OETFGamma[i] = VPHAL_GAMMA_NONE;
            m_hdrParams->CCM[i]       = VPHAL_HDR_CCM_NONE;
            m_hdrParams->CCMExt1[i]   = VPHAL_HDR_CCM_NONE;
            m_hdrParams->CCMExt2[i]   = VPHAL_HDR_CCM_NONE;
            m_hdrParams->HdrMode[i]   = VPHAL_HDR_MODE_NONE;
            m_hdrParams->PriorCSC[i]  = VPHAL_HDR_CSC_NONE;
            m_hdrParams->PostCSC[i]   = VPHAL_HDR_CSC_NONE;

            m_hdrParams->StageEnableFlags[i].value = 0;
            MOS_ZeroMemory(&m_hdrParams->HDRLastFrameSourceParams[i], sizeof(VPHAL_HDR_PARAMS));

            continue;
        }

        //pSrc = (PVPHAL_SURFACE)m_hdrParams->pSrc[i];

        CurrentLUTMode  = VPHAL_HDR_LUT_MODE_NONE;
        CurrentEOTF     = VPHAL_GAMMA_NONE;
        CurrentOETF     = VPHAL_GAMMA_NONE;
        CurrentHdrMode  = VPHAL_HDR_MODE_NONE;
        CurrentCCM      = VPHAL_HDR_CCM_NONE;
        CurrentCCMExt1  = VPHAL_HDR_CCM_NONE;
        CurrentCCMExt2  = VPHAL_HDR_CCM_NONE;
        CurrentPriorCSC = VPHAL_HDR_CSC_NONE;
        CurrentPostCSC  = VPHAL_HDR_CSC_NONE;

        m_hdrParams->Cri3DLUTSize = VPHAL_HDR_CRI_3DLUT_SIZE;

        if (!ToneMappingStagesAssemble(&m_hdrParams->srcHDRParams[i], &m_hdrParams->targetHDRParams[0], &ConfigEntry, i))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        CurrentHdrMode = (VPHAL_HDR_MODE)ConfigEntry.PWLF;
        CurrentCCM     = (VPHAL_HDR_CCM_TYPE)ConfigEntry.CCM;
        CurrentCCMExt1 = (VPHAL_HDR_CCM_TYPE)ConfigEntry.CCMExt1;
        CurrentCCMExt2 = (VPHAL_HDR_CCM_TYPE)ConfigEntry.CCMExt2;

        // So far only enable auto mode in H2S cases.
        if (CurrentHdrMode == VPHAL_HDR_MODE_TONE_MAPPING &&
            m_hdrParams->srcHDRParams[i].bAutoMode                   &&
            input->SurfType == SURF_IN_PRIMARY)
        {
            CurrentHdrMode = VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE;
        }

        StageEnables.value             = 0;
        StageEnables.CCMEnable         = (CurrentCCM     != VPHAL_HDR_CCM_NONE ) ? 1 : 0;
        StageEnables.PWLFEnable        = (CurrentHdrMode != VPHAL_HDR_MODE_NONE) ? 1 : 0;
        StageEnables.CCMExt1Enable     = (CurrentCCMExt1 != VPHAL_HDR_CCM_NONE ) ? 1 : 0;
        StageEnables.CCMExt2Enable     = (CurrentCCMExt2 != VPHAL_HDR_CCM_NONE ) ? 1 : 0;
        StageEnables.GamutClamp1Enable = ConfigEntry.GamutClamp1;
        StageEnables.GamutClamp2Enable = ConfigEntry.GamutClamp2;

        if (IS_YUV_FORMAT(input->osSurface->Format) || IS_ALPHA_YUV_FORMAT(input->osSurface->Format))
        {
            StageEnables.PriorCSCEnable = 1;
        }

        if (!IS_RGB64_FLOAT_FORMAT(input->osSurface->Format) &&
            (StageEnables.CCMEnable || StageEnables.PWLFEnable || StageEnables.CCMExt1Enable || StageEnables.CCMExt2Enable))
        {
            StageEnables.EOTFEnable = 1;
        }

        if (!IS_RGB64_FLOAT_FORMAT(target->osSurface->Format) && (StageEnables.EOTFEnable || IS_RGB64_FLOAT_FORMAT(input->osSurface->Format)))
        {
            StageEnables.OETFEnable = 1;
        }

        if (IS_YUV_FORMAT(target->osSurface->Format))
        {
            StageEnables.PostCSCEnable = 1;
        }

        if (input->SurfType == SURF_IN_PRIMARY && m_hdrParams->GlobalLutMode != VPHAL_HDR_LUT_MODE_3D)
        {
            CurrentLUTMode = VPHAL_HDR_LUT_MODE_2D;
        }
        else
        {
            CurrentLUTMode = VPHAL_HDR_LUT_MODE_3D;
        }

        // Neither 1D nor 3D LUT is needed in linear output case.
        if (IS_RGB64_FLOAT_FORMAT(target->osSurface->Format))
        {
            CurrentLUTMode = VPHAL_HDR_LUT_MODE_NONE;
        }

        // EOTF/CCM/Tone Mapping/OETF require RGB input
        // So if prior CSC is needed, it will always be YUV to RGB conversion
        if (StageEnables.PriorCSCEnable)
        {
            if (input->ColorSpace == CSpace_BT601)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT601;
            }
            else if (input->ColorSpace == CSpace_BT709)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT709;
            }
            else if (input->ColorSpace == CSpace_BT2020)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT2020;
            }
            else if (input->ColorSpace == CSpace_BT2020_FullRange)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT2020;
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Color Space %d Not found.", input->ColorSpace);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }

        if (StageEnables.EOTFEnable)
        {
            if (m_hdrParams->srcHDRParams[i].EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
                m_hdrParams->srcHDRParams[i].EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)
            {
                // Mark tranditional HDR/SDR gamma as the same type
                CurrentEOTF = VPHAL_GAMMA_TRADITIONAL_GAMMA;
            }
            else if (m_hdrParams->srcHDRParams[i].EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
            {
                CurrentEOTF = VPHAL_GAMMA_SMPTE_ST2084;
            }
            else if (m_hdrParams->srcHDRParams[i].EOTF == VPHAL_HDR_EOTF_BT1886)
            {
                CurrentEOTF = VPHAL_GAMMA_BT1886;
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }

        if (StageEnables.OETFEnable)
        {
            if (m_hdrParams->targetHDRParams[0].EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
                m_hdrParams->targetHDRParams[0].EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)
            {
                CurrentOETF = VPHAL_GAMMA_SRGB;
            }
            else if (m_hdrParams->targetHDRParams[0].EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
            {
                CurrentOETF = VPHAL_GAMMA_SMPTE_ST2084;
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }

        // OETF will output RGB surface
        // So if post CSC is needed, it will always be RGB to YUV conversion
        if (StageEnables.PostCSCEnable)
        {
            if (target->ColorSpace == CSpace_BT601)
            {
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT601;
            }
            else if (target->ColorSpace == CSpace_BT709)
            {
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT709;
            }
            else if (target->ColorSpace == CSpace_BT709_FullRange)
            {
                // CSC for target BT709_FULLRANGE is only exposed to Vebox Preprocessed HDR cases.
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT709_FULLRANGE;
            }
            else if (target->ColorSpace == CSpace_BT2020 ||
                     target->ColorSpace == CSpace_BT2020_FullRange)
            {
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT2020;
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("Color Space %d Not found.", target->ColorSpace);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }

        if (m_hdrParams->LUTMode[i]   != CurrentLUTMode ||
            m_hdrParams->EOTFGamma[i] != CurrentEOTF    ||
            m_hdrParams->OETFGamma[i] != CurrentOETF    ||
            m_hdrParams->CCM[i]       != CurrentCCM     ||
            m_hdrParams->CCMExt1[i]  != CurrentCCMExt1  ||
            m_hdrParams->CCMExt2[i]  != CurrentCCMExt2  ||
            m_hdrParams->HdrMode[i]  != CurrentHdrMode  ||
            m_hdrParams->PriorCSC[i] != CurrentPriorCSC ||
            m_hdrParams->PostCSC[i]  != CurrentPostCSC)
        {
            *pdwUpdateMask |= (1 << i);
        }

        if (memcmp(&m_hdrParams->srcHDRParams[i], &m_hdrParams->HDRLastFrameSourceParams[i], sizeof(HDR_PARAMS)))
        {
            *pdwUpdateMask |= (1 << i);
            m_hdrParams->HDRLastFrameSourceParams[i] = m_hdrParams->srcHDRParams[i];
        }

        m_hdrParams->LUTMode[i]         = CurrentLUTMode;
        m_hdrParams->EOTFGamma[i]       = CurrentEOTF;
        m_hdrParams->OETFGamma[i]       = CurrentOETF;
        m_hdrParams->CCM[i]             = CurrentCCM;
        m_hdrParams->CCMExt1[i]         = CurrentCCMExt1;
        m_hdrParams->CCMExt2[i]         = CurrentCCMExt2;
        m_hdrParams->HdrMode[i]         = CurrentHdrMode;
        m_hdrParams->PriorCSC[i]        = CurrentPriorCSC;
        m_hdrParams->PostCSC[i]        = CurrentPostCSC;
        m_hdrParams->StageEnableFlags[i] = StageEnables;
    }

    if (memcmp(&m_hdrParams->targetHDRParams[0], &m_hdrParams->HDRLastFrameTargetParams, sizeof(HDR_PARAMS)))
    {
        *pdwUpdateMask |= (1 << VPHAL_MAX_HDR_INPUT_LAYER);
        m_hdrParams->HDRLastFrameTargetParams = m_hdrParams->targetHDRParams[0];
    }

    m_hdrParams->dwUpdateMask = *pdwUpdateMask;
    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS VpRenderHdrKernel::SetCacheCntl(PVP_RENDER_CACHE_CNTL surfMemCacheCtl)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(surfMemCacheCtl);

    if (!surfMemCacheCtl->bHdr)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    m_surfMemCacheCtl = surfMemCacheCtl->Hdr;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdrKernel::InitRenderHalSurface(
    VP_SURFACE               *surf,
    PRENDERHAL_SURFACE      renderHalSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    //---------------------------------------
    VP_RENDER_CHK_NULL_RETURN(surf);
    VP_RENDER_CHK_NULL_RETURN(surf->osSurface);
    VP_RENDER_CHK_NULL_RETURN(renderHalSurface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    //---------------------------------------

    auto osInterface = m_hwInterface->m_osInterface;

    VP_RENDER_CHK_NULL_RETURN(osInterface);
    VP_RENDER_CHK_NULL_RETURN(osInterface->pfnGetMemoryCompressionMode);
    VP_RENDER_CHK_NULL_RETURN(osInterface->pfnGetMemoryCompressionFormat);

    MOS_ZeroMemory(renderHalSurface, sizeof(*renderHalSurface));

    renderHalSurface->OsSurface  = *surf->osSurface;

    if (0 == renderHalSurface->OsSurface.dwQPitch)
    {
        renderHalSurface->OsSurface.dwQPitch = renderHalSurface->OsSurface.dwHeight;
    }

    VP_RENDER_CHK_STATUS_RETURN(osInterface->pfnGetMemoryCompressionMode(osInterface,
        &surf->osSurface->OsResource, &renderHalSurface->OsSurface.MmcState));

    if (m_hwInterface->m_waTable && MEDIA_IS_WA(m_hwInterface->m_waTable, Wa_16023363837))
    {
        VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurfaceCMF(surf->osSurface, renderHalSurface));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(osInterface->pfnGetMemoryCompressionFormat(osInterface,
            &surf->osSurface->OsResource, &renderHalSurface->OsSurface.CompressionFormat));
    }

    renderHalSurface->rcSrc                        = surf->rcSrc;
    renderHalSurface->rcDst                        = surf->rcDst;
    renderHalSurface->rcMaxSrc                     = surf->rcMaxSrc;
    renderHalSurface->SurfType                     =
                    ConvertVpSurfaceTypeToRenderSurfType(surf->SurfType);
    renderHalSurface->ScalingMode                  =
        ConvertVpScalingModeToRenderScalingMode(m_hdrParams->ScalingMode);
    renderHalSurface->ChromaSiting                 = surf->ChromaSiting;
    renderHalSurface->SampleType                   =
                    ConvertVpSampleTypeToRenderSampleType(surf->SampleType);

    return eStatus;
}

MOS_STATUS VpRenderHdrKernel::InitRenderHalSurface(
    SurfaceType             type,
    VP_SURFACE              *surf,
    PRENDERHAL_SURFACE      renderHalSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(surf);
    VP_RENDER_CHK_NULL_RETURN(m_hdrParams);

    if (type >= SurfaceTypeHdrInputLayer0 && type <= SurfaceTypeHdrInputLayerMax)
    {
        VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurface(surf, renderHalSurface));
        return MOS_STATUS_SUCCESS;
    }
    else if (SurfaceTypeHdrTarget0 == type)
    {
        auto       outputputSurface = m_surfaceGroup->find(SurfaceTypeHdrTarget0);
        VP_SURFACE *outputSrc       = (m_surfaceGroup->end() != outputputSurface) ? outputputSurface->second : nullptr;
        if (outputSrc == nullptr)
        {
            VP_RENDER_ASSERTMESSAGE("input surface creat failed, skip process");
            return MOS_STATUS_NULL_POINTER;
        }

        VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurface(outputSrc, renderHalSurface));
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS VpRenderHdrKernel::GetScoreboardParams(PMHW_VFE_SCOREBOARD &scoreboardParams)
{
    VP_FUNC_CALL();

    MOS_ZeroMemory(&m_scoreboardParams, sizeof(MHW_VFE_SCOREBOARD));
    m_scoreboardParams.ScoreboardMask = 0;
    m_scoreboardParams.ScoreboardType = 1;
    scoreboardParams                  = &m_scoreboardParams;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdrKernel::SetSurfaceParams(KERNEL_SURFACE_STATE_PARAM &surfParam, VP_SURFACE *layer, bool is32MWColorFillKern)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(layer);

    auto &renderSurfParams = surfParam.surfaceOverwriteParams.renderSurfaceParams;
    MOS_ZeroMemory(&renderSurfParams, sizeof(renderSurfParams));

    surfParam.surfaceOverwriteParams.updatedRenderSurfaces = true;

    // Render target or private surface
    if (layer->SurfType == SURF_OUT_RENDERTARGET)
    {
        // Set flags for RT
        surfParam.isOutput                  = true;
        renderSurfParams.isOutput           = true;
        renderSurfParams.bWidthInDword_Y    = true;
        renderSurfParams.bWidthInDword_UV   = true;
        renderSurfParams.Boundary           = RENDERHAL_SS_BOUNDARY_DSTRECT;
    }
    // other surfaces
    else
    {
        surfParam.isOutput                  = false;
        renderSurfParams.isOutput           = false;
        renderSurfParams.bWidthInDword_Y    = false;
        renderSurfParams.bWidthInDword_UV   = false;
        renderSurfParams.Boundary           = RENDERHAL_SS_BOUNDARY_SRCRECT;
    }

    renderSurfParams.b32MWColorFillKern = is32MWColorFillKern;
    renderSurfParams.Type               = m_hwInterface->m_renderHal->SurfaceTypeDefault;
    renderSurfParams.bAVS               = false;

    // Set interlacing flags
    switch (layer->SampleType)
    {
    case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
    case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
        renderSurfParams.bVertStride     = true;
        renderSurfParams.bVertStrideOffs = 0;
        break;
    case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
    case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
        renderSurfParams.bVertStride     = true;
        renderSurfParams.bVertStrideOffs = 1;
        break;
    default:
        renderSurfParams.bVertStride     = false;
        renderSurfParams.bVertStrideOffs = 0;
        break;
    }

    renderSurfParams.b2PlaneNV12NeededByKernel = true;

    VP_RENDER_NORMALMESSAGE("SurfaceTYpe %d, bAVS %d, b2PlaneNV12NeededByKernel %d",
        renderSurfParams.Type,
        renderSurfParams.bAVS,
        renderSurfParams.b2PlaneNV12NeededByKernel);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdrKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();

    PRENDERHAL_INTERFACE  renderHal = nullptr;
    int32_t               iBTentry  = 0;
    bool                  bHasAutoModeLayer = false;
    uint32_t              i                 = 0;
    uint32_t              dwUpdateMask      = 0;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    renderHal = m_hwInterface->m_renderHal;
    m_surfaceBindingIndex.clear();
    m_surfaceState.clear();
    UpdatePerLayerPipelineStates(&dwUpdateMask);

    for (i = 0; i < m_hdrParams->uSourceCount && i < VPHAL_MAX_HDR_INPUT_LAYER; i++)
    {
        KERNEL_SURFACE_STATE_PARAM surfParam = {};
        auto                       inputSrc  = m_surfaceGroup->find(SurfaceType(SurfaceTypeHdrInputLayer0 + i));
        VP_SURFACE *layer = (m_surfaceGroup->end() != inputSrc) ? inputSrc->second : nullptr;
        VP_RENDER_CHK_NULL_RETURN(layer);

        // Only need to specify binding index in surface parameters.
        surfParam.surfaceOverwriteParams.updatedSurfaceParams = true;
        surfParam.surfaceOverwriteParams.bindedKernel = true;

        UpdateCurbeBindingIndex(SurfaceType(SurfaceTypeHdrInputLayer0 + i), m_hdrParams->uSourceBindingTableIndex[i]);
        iBTentry                                   = m_hdrParams->uSourceBindingTableIndex[i];

        SetSurfaceParams(surfParam, layer, false);

        surfParam.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl = m_surfMemCacheCtl.SourceSurfMemObjCtl;

        m_surfaceState.insert(std::make_pair(SurfaceType(SurfaceTypeHdrInputLayer0 + i), surfParam));

        auto OETF1DLUT = m_surfaceGroup->find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + i));
        VP_SURFACE *OETF1DLUTSrc = (m_surfaceGroup->end() != OETF1DLUT) ? OETF1DLUT->second : nullptr;

        if (OETF1DLUTSrc && (m_hdrParams->OETF1DLUTAllocated || (dwUpdateMask & (1 << i)) || (dwUpdateMask & (1 << VPHAL_MAX_HDR_INPUT_LAYER))))
        {
            InitOETF1DLUT(m_hdrParams, i, OETF1DLUTSrc);
        }

        auto        Cri3DLUT     = m_surfaceGroup->find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + i));
        VP_SURFACE *Cri3DLUTSrc = (m_surfaceGroup->end() != Cri3DLUT) ? Cri3DLUT->second : nullptr;

        if (Cri3DLUTSrc && (m_hdrParams->Cri3DLUTAllocated || (dwUpdateMask & (1 << i)) || (dwUpdateMask & (1 << VPHAL_MAX_HDR_INPUT_LAYER))) ||
            m_hdrParams->HdrMode[i] == VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE)
        {
            InitCri3DLUT(m_hdrParams, i, Cri3DLUTSrc);
        }

        KERNEL_SURFACE_STATE_PARAM surfaceResource = {};
        surfaceResource.surfaceOverwriteParams.updatedSurfaceParams                         = true;
        surfaceResource.surfaceOverwriteParams.updatedRenderSurfaces                        = true;
        surfaceResource.surfaceOverwriteParams.bufferResource                               = false;
        surfaceResource.surfaceOverwriteParams.bindedKernel                                 = true;

        surfaceResource.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl                = false;
        surfaceResource.surfaceOverwriteParams.renderSurfaceParams.Type                     = RENDERHAL_SURFACE_TYPE_G10;
        surfaceResource.surfaceOverwriteParams.renderSurfaceParams.isOutput                 = false;
        surfaceResource.surfaceOverwriteParams.renderSurfaceParams.bWidth16Align            = false;
        surfaceResource.surfaceOverwriteParams.renderSurfaceParams.Boundary                 = RENDERHAL_SS_BOUNDARY_ORIGINAL;

        if (m_hdrParams->LUTMode[i] == VPHAL_HDR_LUT_MODE_2D)
        {
            surfaceResource.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl                = m_surfMemCacheCtl.Lut2DSurfMemObjCtl;
            UpdateCurbeBindingIndex(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + i), iBTentry + VPHAL_HDR_BTINDEX_OETF1DLUT_OFFSET);
            m_surfaceState.insert(std::make_pair(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + i), surfaceResource));
        }
        else if (m_hdrParams->LUTMode[i] == VPHAL_HDR_LUT_MODE_3D)
        {
            surfaceResource.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl                = m_surfMemCacheCtl.Lut3DSurfMemObjCtl;
            UpdateCurbeBindingIndex(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + i), iBTentry + VPHAL_HDR_BTINDEX_CRI3DLUT_OFFSET);
            surfaceResource.surfaceOverwriteParams.renderSurfaceParams.bWidthInDword_Y          = false;
            surfaceResource.surfaceOverwriteParams.renderSurfaceParams.bWidthInDword_UV         = false;
            m_surfaceState.insert(std::make_pair(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + i), surfaceResource));
        }
    }

    for (i = 0; i < m_hdrParams->uTargetCount; ++i)
    {
        KERNEL_SURFACE_STATE_PARAM surfParam = {};

        surfParam.surfaceOverwriteParams.updatedSurfaceParams = true;

        // Only need to specify binding index in surface parameters.
        surfParam.surfaceOverwriteParams.bindedKernel = true;
        UpdateCurbeBindingIndex(SurfaceType(SurfaceTypeHdrTarget0 + i), m_hdrParams->uTargetBindingTableIndex[i]);

        iBTentry = m_hdrParams->uTargetBindingTableIndex[i];
        auto    outputSrc = m_surfaceGroup->find(SurfaceTypeHdrTarget0);
        VP_SURFACE *layer = (m_surfaceGroup->end() != outputSrc) ? outputSrc->second : nullptr;
        VP_RENDER_CHK_NULL_RETURN(layer);
        VP_RENDER_CHK_NULL_RETURN(layer->osSurface);

        layer->SurfType = SURF_OUT_RENDERTARGET;

        // Used for 32x32 Media walker kernel + Color fill kernel
        // Not valid for media object.
        bool is32MWColorFillKern =
            (m_hdrParams->pColorFillParams != nullptr &&
                m_hdrParams->uSourceCount == 0 &&
                m_renderHal->pHwSizes->dwSizeMediaWalkerBlock == 32);

        SetSurfaceParams(surfParam, layer, is32MWColorFillKern);

        surfParam.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl = m_surfMemCacheCtl.TargetSurfMemObjCtl;

        m_surfaceState.insert(std::make_pair(SurfaceType(SurfaceTypeHdrTarget0 + i), surfParam));

        //update render GMM resource usage type
        m_allocator->UpdateResourceUsageType(&layer->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_RENDER);
    }

    if (m_hdrParams->HdrMode[i] == VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE)
    {
        bHasAutoModeLayer = true;
    }

    auto coeff = m_surfaceGroup->find(SurfaceTypeHdrCoeff);
    VP_SURFACE *coeffSrc = (m_surfaceGroup->end() != coeff) ? coeff->second : nullptr;
    VP_RENDER_CHK_NULL_RETURN(coeffSrc);

    if (dwUpdateMask || m_hdrParams->coeffAllocated)
    {
        HdrInitCoeff(m_hdrParams, coeffSrc);
    }

    KERNEL_SURFACE_STATE_PARAM surfCoeffParam = {};

    surfCoeffParam.surfaceOverwriteParams.updatedSurfaceParams = true;
    // Only need to specify binding index in surface parameters.
    surfCoeffParam.surfaceOverwriteParams.bindedKernel = true;

    surfCoeffParam.surfaceOverwriteParams.updatedRenderSurfaces        = true;
    surfCoeffParam.surfaceOverwriteParams.renderSurfaceParams.Type     = RENDERHAL_SURFACE_TYPE_G10;
    surfCoeffParam.surfaceOverwriteParams.renderSurfaceParams.isOutput      = false;
    surfCoeffParam.surfaceOverwriteParams.renderSurfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfCoeffParam.surfaceOverwriteParams.renderSurfaceParams.bWidth16Align = false;
    surfCoeffParam.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl     = m_surfMemCacheCtl.CoeffSurfMemObjCtl;

    if (m_hdrParams->bUsingAutoModePipe && bHasAutoModeLayer)
    {
        UpdateCurbeBindingIndex(SurfaceTypeHdrAutoModeCoeff, VPHAL_HDR_BTINDEX_COEFF);
        m_surfaceState.insert(std::make_pair(SurfaceTypeHdrAutoModeCoeff, surfCoeffParam));
    }
    else
    {
        UpdateCurbeBindingIndex(SurfaceTypeHdrCoeff, VPHAL_HDR_BTINDEX_COEFF);
        m_surfaceState.insert(std::make_pair(SurfaceTypeHdrCoeff, surfCoeffParam));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdrKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus             = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            pRenderHal          = nullptr;
    PVP_SURFACE                     pSource             = nullptr;
    VPHAL_HDR_FORMAT_DESCRIPTOR  FormatDescriptor    = VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW;
    VPHAL_HDR_ROTATION           HdrRotation         = VPHAL_HDR_LAYER_ROTATION_0;
    VPHAL_HDR_TWO_LAYER_OPTION   HdrTwoLayerOp       = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND;
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

    m_hdrCurbe    = g_cInit_MEDIA_STATIC_HDR;
    HdrTwoLayerOp = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND;
    HdrRotation   = VPHAL_HDR_LAYER_ROTATION_0;
    wAlpha        = 0x0ff;
    uiSamplerStateIndex = uiSamplerStateIndex2 = 0;

    auto        target         = m_surfaceGroup->find(SurfaceTypeHdrTarget0);
    VP_SURFACE *targetSurf = (m_surfaceGroup->end() != target) ? target->second : nullptr;
    VP_RENDER_CHK_NULL_RETURN(targetSurf);
    VP_RENDER_CHK_NULL_RETURN(targetSurf->osSurface);

    for (i = 0; i < m_hdrParams->uSourceCount; i++)
    {
        if (i >= VPHAL_MAX_HDR_INPUT_LAYER)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        auto it = m_surfaceGroup->find(SurfaceType(SurfaceTypeHdrInputLayer0 + i));
        pSource = (m_surfaceGroup->end() != it) ? it->second : nullptr;
        VP_RENDER_CHK_NULL_RETURN(pSource);
        VP_RENDER_CHK_NULL_RETURN(pSource->osSurface);

        bChannelSwap  = false;
        bBypassIEF    = true;

        fShiftX = 0;
        fShiftY = 0;

        // Source rectangle is pre-rotated, destination rectangle is post-rotated.
        if (m_hdrParams->Rotation == VPHAL_ROTATION_IDENTITY    ||
            m_hdrParams->Rotation == VPHAL_ROTATION_180         ||
            m_hdrParams->Rotation == VPHAL_MIRROR_HORIZONTAL    ||
            m_hdrParams->Rotation == VPHAL_MIRROR_VERTICAL)
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

        if (fScaleX == 1.0f && fScaleY == 1.0f && m_hdrParams->ScalingMode == VPHAL_SCALING_BILINEAR)
        {
            m_hdrParams->ScalingMode = VPHAL_SCALING_NEAREST;
        }

        if (m_hdrParams->ScalingMode == VPHAL_SCALING_AVS)
        {
            uiSamplerStateIndex = VPHAL_HDR_AVS_SAMPLER_STATE_ADAPTIVE;

            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                uiSamplerStateIndex2 = VPHAL_HDR_AVS_SAMPLER_STATE_ADAPTIVE;
            }
        }
        else if (m_hdrParams->ScalingMode == VPHAL_SCALING_BILINEAR)
        {
            uiSamplerStateIndex = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;

            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                uiSamplerStateIndex2 = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;
            }

            fShiftX = VP_HW_LINEAR_SHIFT;
            fShiftY = VP_HW_LINEAR_SHIFT;
        }
        else
        {
            uiSamplerStateIndex = VPHAL_HDR_3D_SAMPLER_STATE_NEAREST;

            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                uiSamplerStateIndex2 = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;
            }

            fShiftX = VP_HW_LINEAR_SHIFT;
            fShiftY = VP_HW_LINEAR_SHIFT;
        }

        // Normalize source co-ordinates using the width and height programmed
        // in surface state. step X, Y pre-rotated
        // Source rectangle is pre-rotated, destination rectangle is post-rotated.
        if (m_hdrParams->Rotation == VPHAL_ROTATION_IDENTITY ||
            m_hdrParams->Rotation == VPHAL_ROTATION_180 ||
            m_hdrParams->Rotation == VPHAL_MIRROR_HORIZONTAL ||
            m_hdrParams->Rotation == VPHAL_MIRROR_VERTICAL)
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

        dwDestRectWidth  = targetSurf->osSurface->dwWidth;
        dwDestRectHeight = targetSurf->osSurface->dwHeight;

        switch (m_hdrParams->Rotation)
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

        fOriginX = ((float)pSource->rcSrc.left + fShiftX * fStepX) / (float)MOS_MIN(pSource->osSurface->dwWidth, (uint32_t)pSource->rcSrc.right);
        fOriginY = ((float)pSource->rcSrc.top + fShiftY * fStepY) / (float)MOS_MIN(pSource->osSurface->dwHeight, (uint32_t)pSource->rcSrc.bottom);

        fStepX /= MOS_MIN(pSource->osSurface->dwWidth, (uint32_t)pSource->rcSrc.right);
        fStepY /= MOS_MIN(pSource->osSurface->dwHeight, (uint32_t)pSource->rcSrc.bottom);

        FormatDescriptor = GetFormatDescriptor(pSource->osSurface->Format);

        if (FormatDescriptor == VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW)
        {
            VP_RENDER_VERBOSEMESSAGE("Unsupported hdr input format");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Chroma siting setting in curbe data will only take effect in 1x scaling case when 3D sampler is being used
        // For other cases, Chroma siting will be performed by AVS coefficients, and the here will be ignored by kernel
        ChromaSiting = GetHdrChromaSiting(pSource->ChromaSiting);

        if (pSource->osSurface->Format == Format_B10G10R10A2 ||
            pSource->osSurface->Format == Format_A8R8G8B8 ||
            pSource->osSurface->Format == Format_X8R8G8B8 ||
            pSource->osSurface->Format == Format_A16R16G16B16F)
        {
            bChannelSwap = true;
        }

        if (m_hdrParams->pIEFParams)
        {
            if (m_hdrParams->pIEFParams->bEnabled)
            {
                bBypassIEF = false;
            }
        }

        HdrRotation = GetHdrRotation(m_hdrParams->Rotation);

        b3dLut = (m_hdrParams->LUTMode[i] == VPHAL_HDR_LUT_MODE_3D ? true : false);

        switch (i)
        {
        case 0:
            m_hdrCurbe.DW0.HorizontalFrameOriginLayer0       = fOriginX;
            m_hdrCurbe.DW8.VerticalFrameOriginLayer0         = fOriginY;
            m_hdrCurbe.DW16.HorizontalScalingStepRatioLayer0 = fStepX;
            m_hdrCurbe.DW24.VerticalScalingStepRatioLayer0   = fStepY;
            m_hdrCurbe.DW32.LeftCoordinateRectangleLayer0    = pSource->rcDst.left;
            m_hdrCurbe.DW32.TopCoordinateRectangleLayer0     = pSource->rcDst.top;
            m_hdrCurbe.DW40.RightCoordinateRectangleLayer0   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW40.BottomCoordinateRectangleLayer0  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW48.FormatDescriptorLayer0           = FormatDescriptor;
            m_hdrCurbe.DW48.ChromaSittingLocationLayer0      = ChromaSiting;
            m_hdrCurbe.DW48.ChannelSwapEnablingFlagLayer0    = bChannelSwap;
            m_hdrCurbe.DW48.IEFBypassEnablingFlagLayer0      = bBypassIEF;
            m_hdrCurbe.DW48.RotationAngleMirrorDirectionLayer0 = HdrRotation;
            m_hdrCurbe.DW48.SamplerIndexFirstPlaneLayer0       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW48.SamplerIndexSecondThirdPlaneLayer0 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW48.CCMExtensionEnablingFlagLayer0     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW48.ToneMappingEnablingFlagLayer0      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW48.PriorCSCEnablingFlagLayer0         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW48.EOTF1DLUTEnablingFlagLayer0        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW48.CCMEnablingFlagLayer0              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW48.OETF1DLUTEnablingFlagLayer0        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW48.PostCSCEnablingFlagLayer0          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW48.Enabling3DLUTFlagLayer0            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW56.ConstantBlendingAlphaFillColorLayer0 = wAlpha;
            }
            m_hdrCurbe.DW58.TwoLayerOperationLayer0              = VPHAL_HDR_TWO_LAYER_OPTION_COMP;
            if (pSource->SurfType == SURF_IN_PRIMARY                &&
                m_hdrParams->pBlendingParams                        &&
                m_hdrParams->pBlendingParams->BlendType == BLEND_SOURCE &&
                (IS_RGB_CSPACE(pSource->ColorSpace) || IS_COLOR_SPACE_BT2020_RGB(pSource->ColorSpace)))
            {
                // For PDVD alpha blending issue:
                // If first frame is sRGB/stRGB/BT2020RGB format delivered as primary layer
                // (which means main video content is in the first layer)
                // and blend-type set as source blending,
                // do source blending w/ bg, instead of setting as composite.
                m_hdrCurbe.DW58.TwoLayerOperationLayer0 = VPHAL_HDR_TWO_LAYER_OPTION_SBLEND;
            }
            break;
        case 1:
            m_hdrCurbe.DW1.HorizontalFrameOriginLayer1       = fOriginX;
            m_hdrCurbe.DW9.VerticalFrameOriginLayer1         = fOriginY;
            m_hdrCurbe.DW17.HorizontalScalingStepRatioLayer1 = fStepX;
            m_hdrCurbe.DW25.VerticalScalingStepRatioLayer1   = fStepY;
            m_hdrCurbe.DW33.LeftCoordinateRectangleLayer1    = pSource->rcDst.left;
            m_hdrCurbe.DW33.TopCoordinateRectangleLayer1     = pSource->rcDst.top;
            m_hdrCurbe.DW41.RightCoordinateRectangleLayer1   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW41.BottomCoordinateRectangleLayer1  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW49.FormatDescriptorLayer1           = FormatDescriptor;
            m_hdrCurbe.DW49.ChromaSittingLocationLayer1      = ChromaSiting;
            m_hdrCurbe.DW49.ChannelSwapEnablingFlagLayer1    = bChannelSwap;
            m_hdrCurbe.DW49.IEFBypassEnablingFlagLayer1      = bBypassIEF;
            m_hdrCurbe.DW49.RotationAngleMirrorDirectionLayer1 = HdrRotation;
            m_hdrCurbe.DW49.SamplerIndexFirstPlaneLayer1       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW49.SamplerIndexSecondThirdPlaneLayer1 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW49.CCMExtensionEnablingFlagLayer1     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW49.ToneMappingEnablingFlagLayer1      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW49.PriorCSCEnablingFlagLayer1         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW49.EOTF1DLUTEnablingFlagLayer1        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW49.CCMEnablingFlagLayer1              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW49.OETF1DLUTEnablingFlagLayer1        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW49.PostCSCEnablingFlagLayer1          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW49.Enabling3DLUTFlagLayer1            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW56.ConstantBlendingAlphaFillColorLayer1 = wAlpha;
            }
            m_hdrCurbe.DW58.TwoLayerOperationLayer1              = HdrTwoLayerOp;
            break;
        case 2:
            m_hdrCurbe.DW2.HorizontalFrameOriginLayer2       = fOriginX;
            m_hdrCurbe.DW10.VerticalFrameOriginLayer2        = fOriginY;
            m_hdrCurbe.DW18.HorizontalScalingStepRatioLayer2 = fStepX;
            m_hdrCurbe.DW26.VerticalScalingStepRatioLayer2   = fStepY;
            m_hdrCurbe.DW34.LeftCoordinateRectangleLayer2    = pSource->rcDst.left;
            m_hdrCurbe.DW34.TopCoordinateRectangleLayer2     = pSource->rcDst.top;
            m_hdrCurbe.DW42.RightCoordinateRectangleLayer2   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW42.BottomCoordinateRectangleLayer2  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW50.FormatDescriptorLayer2           = FormatDescriptor;
            m_hdrCurbe.DW50.ChromaSittingLocationLayer2      = ChromaSiting;
            m_hdrCurbe.DW50.ChannelSwapEnablingFlagLayer2    = bChannelSwap;
            m_hdrCurbe.DW50.IEFBypassEnablingFlagLayer2      = bBypassIEF;
            m_hdrCurbe.DW50.RotationAngleMirrorDirectionLayer2 = HdrRotation;
            m_hdrCurbe.DW50.SamplerIndexFirstPlaneLayer2       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW50.SamplerIndexSecondThirdPlaneLayer2 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW50.CCMExtensionEnablingFlagLayer2     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW50.ToneMappingEnablingFlagLayer2      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW50.PriorCSCEnablingFlagLayer2         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW50.EOTF1DLUTEnablingFlagLayer2        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW50.CCMEnablingFlagLayer2              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW50.OETF1DLUTEnablingFlagLayer2        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW50.PostCSCEnablingFlagLayer2          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW50.Enabling3DLUTFlagLayer2            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW56.ConstantBlendingAlphaFillColorLayer2 = wAlpha;
            }
            m_hdrCurbe.DW58.TwoLayerOperationLayer2              = HdrTwoLayerOp;
            break;
        case 3:
            m_hdrCurbe.DW3.HorizontalFrameOriginLayer3       = fOriginX;
            m_hdrCurbe.DW11.VerticalFrameOriginLayer3        = fOriginY;
            m_hdrCurbe.DW19.HorizontalScalingStepRatioLayer3 = fStepX;
            m_hdrCurbe.DW27.VerticalScalingStepRatioLayer3   = fStepY;
            m_hdrCurbe.DW35.LeftCoordinateRectangleLayer3    = pSource->rcDst.left;
            m_hdrCurbe.DW35.TopCoordinateRectangleLayer3     = pSource->rcDst.top;
            m_hdrCurbe.DW43.RightCoordinateRectangleLayer3   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW43.BottomCoordinateRectangleLayer3  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW51.FormatDescriptorLayer3           = FormatDescriptor;
            m_hdrCurbe.DW51.ChromaSittingLocationLayer3      = ChromaSiting;
            m_hdrCurbe.DW51.ChannelSwapEnablingFlagLayer3    = bChannelSwap;
            m_hdrCurbe.DW51.IEFBypassEnablingFlagLayer3      = bBypassIEF;
            m_hdrCurbe.DW51.RotationAngleMirrorDirectionLayer3 = HdrRotation;
            m_hdrCurbe.DW51.SamplerIndexFirstPlaneLayer3       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW51.SamplerIndexSecondThirdPlaneLayer3 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW51.CCMExtensionEnablingFlagLayer3     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW51.ToneMappingEnablingFlagLayer3      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW51.PriorCSCEnablingFlagLayer3         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW51.EOTF1DLUTEnablingFlagLayer3        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW51.CCMEnablingFlagLayer3              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW51.OETF1DLUTEnablingFlagLayer3        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW51.PostCSCEnablingFlagLayer3          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW51.Enabling3DLUTFlagLayer3            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW56.ConstantBlendingAlphaFillColorLayer3 = wAlpha;
            }
            m_hdrCurbe.DW58.TwoLayerOperationLayer3              = HdrTwoLayerOp;
            break;
        case 4:
            m_hdrCurbe.DW4.HorizontalFrameOriginLayer4       = fOriginX;
            m_hdrCurbe.DW12.VerticalFrameOriginLayer4        = fOriginY;
            m_hdrCurbe.DW20.HorizontalScalingStepRatioLayer4 = fStepX;
            m_hdrCurbe.DW28.VerticalScalingStepRatioLayer4   = fStepY;
            m_hdrCurbe.DW36.LeftCoordinateRectangleLayer4    = pSource->rcDst.left;
            m_hdrCurbe.DW36.TopCoordinateRectangleLayer4     = pSource->rcDst.top;
            m_hdrCurbe.DW44.RightCoordinateRectangleLayer4   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW44.BottomCoordinateRectangleLayer4  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW52.FormatDescriptorLayer4           = FormatDescriptor;
            m_hdrCurbe.DW52.ChromaSittingLocationLayer4      = ChromaSiting;
            m_hdrCurbe.DW52.ChannelSwapEnablingFlagLayer4    = bChannelSwap;
            m_hdrCurbe.DW52.IEFBypassEnablingFlagLayer4      = bBypassIEF;
            m_hdrCurbe.DW52.RotationAngleMirrorDirectionLayer4 = HdrRotation;
            m_hdrCurbe.DW52.SamplerIndexFirstPlaneLayer4       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW52.SamplerIndexSecondThirdPlaneLayer4 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW52.CCMExtensionEnablingFlagLayer4     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW52.ToneMappingEnablingFlagLayer4      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW52.PriorCSCEnablingFlagLayer4         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW52.EOTF1DLUTEnablingFlagLayer4        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW52.CCMEnablingFlagLayer4              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW52.OETF1DLUTEnablingFlagLayer4        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW52.PostCSCEnablingFlagLayer4          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW52.Enabling3DLUTFlagLayer4            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW57.ConstantBlendingAlphaFillColorLayer4 = wAlpha;
            }
            m_hdrCurbe.DW59.TwoLayerOperationLayer4              = HdrTwoLayerOp;
            break;
        case 5:
            m_hdrCurbe.DW5.HorizontalFrameOriginLayer5       = fOriginX;
            m_hdrCurbe.DW13.VerticalFrameOriginLayer5        = fOriginY;
            m_hdrCurbe.DW21.HorizontalScalingStepRatioLayer5 = fStepX;
            m_hdrCurbe.DW29.VerticalScalingStepRatioLayer5   = fStepY;
            m_hdrCurbe.DW37.LeftCoordinateRectangleLayer5    = pSource->rcDst.left;
            m_hdrCurbe.DW37.TopCoordinateRectangleLayer5     = pSource->rcDst.top;
            m_hdrCurbe.DW45.RightCoordinateRectangleLayer5   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW45.BottomCoordinateRectangleLayer5  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW53.FormatDescriptorLayer5           = FormatDescriptor;
            m_hdrCurbe.DW53.ChromaSittingLocationLayer5      = ChromaSiting;
            m_hdrCurbe.DW53.ChannelSwapEnablingFlagLayer5    = bChannelSwap;
            m_hdrCurbe.DW53.IEFBypassEnablingFlagLayer5      = bBypassIEF;
            m_hdrCurbe.DW53.RotationAngleMirrorDirectionLayer5 = HdrRotation;
            m_hdrCurbe.DW53.SamplerIndexFirstPlaneLayer5       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW53.SamplerIndexSecondThirdPlaneLayer5 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW53.CCMExtensionEnablingFlagLayer5     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW53.ToneMappingEnablingFlagLayer5      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW53.PriorCSCEnablingFlagLayer5         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW53.EOTF1DLUTEnablingFlagLayer5        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW53.CCMEnablingFlagLayer5              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW53.OETF1DLUTEnablingFlagLayer5        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW53.PostCSCEnablingFlagLayer5          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW53.Enabling3DLUTFlagLayer5            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW57.ConstantBlendingAlphaFillColorLayer5 = wAlpha;
            }
            m_hdrCurbe.DW59.TwoLayerOperationLayer5              = HdrTwoLayerOp;
            break;
        case 6:
            m_hdrCurbe.DW6.HorizontalFrameOriginLayer6       = fOriginX;
            m_hdrCurbe.DW14.VerticalFrameOriginLayer6        = fOriginY;
            m_hdrCurbe.DW22.HorizontalScalingStepRatioLayer6 = fStepX;
            m_hdrCurbe.DW30.VerticalScalingStepRatioLayer6   = fStepY;
            m_hdrCurbe.DW38.LeftCoordinateRectangleLayer6    = pSource->rcDst.left;
            m_hdrCurbe.DW38.TopCoordinateRectangleLayer6     = pSource->rcDst.top;
            m_hdrCurbe.DW46.RightCoordinateRectangleLayer6   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW46.BottomCoordinateRectangleLayer6  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW54.FormatDescriptorLayer6           = FormatDescriptor;
            m_hdrCurbe.DW54.ChromaSittingLocationLayer6      = ChromaSiting;
            m_hdrCurbe.DW54.ChannelSwapEnablingFlagLayer6    = bChannelSwap;
            m_hdrCurbe.DW54.IEFBypassEnablingFlagLayer6      = bBypassIEF;
            m_hdrCurbe.DW54.RotationAngleMirrorDirectionLayer6 = HdrRotation;
            m_hdrCurbe.DW54.SamplerIndexFirstPlaneLayer6       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW54.SamplerIndexSecondThirdPlaneLayer6 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW54.CCMExtensionEnablingFlagLayer6     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW54.ToneMappingEnablingFlagLayer6      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW54.PriorCSCEnablingFlagLayer6         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW54.EOTF1DLUTEnablingFlagLayer6        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW54.CCMEnablingFlagLayer6              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW54.OETF1DLUTEnablingFlagLayer6        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW54.PostCSCEnablingFlagLayer6          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW54.Enabling3DLUTFlagLayer6            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW57.ConstantBlendingAlphaFillColorLayer6 = wAlpha;
            }
            m_hdrCurbe.DW59.TwoLayerOperationLayer6              = HdrTwoLayerOp;
            break;
        case 7:
            m_hdrCurbe.DW7.HorizontalFrameOriginLayer7       = fOriginX;
            m_hdrCurbe.DW15.VerticalFrameOriginLayer7        = fOriginY;
            m_hdrCurbe.DW23.HorizontalScalingStepRatioLayer7 = fStepX;
            m_hdrCurbe.DW31.VerticalScalingStepRatioLayer7   = fStepY;
            m_hdrCurbe.DW39.LeftCoordinateRectangleLayer7    = pSource->rcDst.left;
            m_hdrCurbe.DW39.TopCoordinateRectangleLayer7     = pSource->rcDst.top;
            m_hdrCurbe.DW47.RightCoordinateRectangleLayer7   = pSource->rcDst.right - 1;
            m_hdrCurbe.DW47.BottomCoordinateRectangleLayer7  = pSource->rcDst.bottom - 1;
            m_hdrCurbe.DW55.FormatDescriptorLayer7           = FormatDescriptor;
            m_hdrCurbe.DW55.ChromaSittingLocationLayer7      = ChromaSiting;
            m_hdrCurbe.DW55.ChannelSwapEnablingFlagLayer7    = bChannelSwap;
            m_hdrCurbe.DW55.IEFBypassEnablingFlagLayer7      = bBypassIEF;
            m_hdrCurbe.DW55.RotationAngleMirrorDirectionLayer7 = HdrRotation;
            m_hdrCurbe.DW55.SamplerIndexFirstPlaneLayer7       = uiSamplerStateIndex;
            if (pSource->osSurface->Format == Format_P010 ||
                pSource->osSurface->Format == Format_P016)
            {
                m_hdrCurbe.DW55.SamplerIndexSecondThirdPlaneLayer7 = uiSamplerStateIndex2;
            }
            m_hdrCurbe.DW55.CCMExtensionEnablingFlagLayer7     = m_hdrParams->StageEnableFlags[i].CCMExt1Enable ||
                                                                m_hdrParams->StageEnableFlags[i].CCMExt2Enable;
            m_hdrCurbe.DW55.ToneMappingEnablingFlagLayer7      = m_hdrParams->StageEnableFlags[i].PWLFEnable;
            m_hdrCurbe.DW55.PriorCSCEnablingFlagLayer7         = m_hdrParams->StageEnableFlags[i].PriorCSCEnable;
            m_hdrCurbe.DW55.EOTF1DLUTEnablingFlagLayer7        = m_hdrParams->StageEnableFlags[i].EOTFEnable;
            m_hdrCurbe.DW55.CCMEnablingFlagLayer7              = m_hdrParams->StageEnableFlags[i].CCMEnable;
            m_hdrCurbe.DW55.OETF1DLUTEnablingFlagLayer7        = m_hdrParams->StageEnableFlags[i].OETFEnable;
            m_hdrCurbe.DW55.PostCSCEnablingFlagLayer7          = m_hdrParams->StageEnableFlags[i].PostCSCEnable;
            m_hdrCurbe.DW55.Enabling3DLUTFlagLayer7            = b3dLut;
            if (HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CBLEND  ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND ||
                HdrTwoLayerOp == VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND)
            {
                m_hdrCurbe.DW57.ConstantBlendingAlphaFillColorLayer7 = wAlpha;
            }
            m_hdrCurbe.DW59.TwoLayerOperationLayer7              = HdrTwoLayerOp;
            break;
        default:
            VP_RENDER_VERBOSEMESSAGE("Invalid input layer number.");
            break;
        }
    }

    FormatDescriptor = GetFormatDescriptor(targetSurf->osSurface->Format);
    ChromaSiting     = GetHdrChromaSiting(targetSurf->ChromaSiting);

    if (targetSurf->osSurface->Format == Format_B10G10R10A2 ||
        targetSurf->osSurface->Format == Format_A8R8G8B8 ||
        targetSurf->osSurface->Format == Format_X8R8G8B8 ||
        targetSurf->osSurface->Format == Format_A16R16G16B16F)
    {
        bChannelSwap = true;
    }
    else
    {
        bChannelSwap = false;
    }

    m_hdrCurbe.DW62.DestinationWidth                 = targetSurf->osSurface->dwWidth;
    m_hdrCurbe.DW62.DestinationHeight      = targetSurf->osSurface->dwHeight;
    m_hdrCurbe.DW63.TotalNumberInputLayers           = m_hdrParams->uSourceCount;

    if (0 == m_hdrParams->uSourceCount)
    {
        m_hdrCurbe.DW32.LeftCoordinateRectangleLayer0    = targetSurf->osSurface->dwWidth + 16;
        m_hdrCurbe.DW32.TopCoordinateRectangleLayer0     = targetSurf->osSurface->dwHeight + 16;
        m_hdrCurbe.DW40.RightCoordinateRectangleLayer0   = targetSurf->osSurface->dwWidth + 16;
        m_hdrCurbe.DW40.BottomCoordinateRectangleLayer0  = targetSurf->osSurface->dwHeight + 16;
        m_hdrCurbe.DW58.TwoLayerOperationLayer0          = VPHAL_HDR_TWO_LAYER_OPTION_COMP;
    }

    m_hdrCurbe.DW63.FormatDescriptorDestination        = FormatDescriptor;
    m_hdrCurbe.DW63.ChromaSittingLocationDestination   = ChromaSiting;
    m_hdrCurbe.DW63.ChannelSwapEnablingFlagDestination = bChannelSwap;

    // Set Background color (use cspace of first layer)
    if (m_hdrParams->pColorFillParams)
    {
        VPHAL_COLOR_SAMPLE_8 Src, Dst;
        VPHAL_CSPACE         src_cspace, dst_cspace;

        Src.dwValue = m_hdrParams->pColorFillParams->Color;

        // get src and dst colorspaces
        src_cspace = m_hdrParams->pColorFillParams->CSpace;
        dst_cspace = targetSurf->ColorSpace;

        // Convert BG color only if not done so before. CSC is expensive!
        if (VpUtils::GetCscMatrixForRender8Bit(&Dst, &Src, src_cspace, dst_cspace))
        {
            m_hdrCurbe.DW60.FixedPointFillColorRVChannel     = Dst.R << 8;
            m_hdrCurbe.DW60.FixedPointFillColorGYChannel     = Dst.G << 8;
            m_hdrCurbe.DW61.FixedPointFillColorBUChannel     = Dst.B << 8;
            m_hdrCurbe.DW61.FixedPointFillColorAlphaChannel  = Dst.A << 8;
        }
        else
        {
            m_hdrCurbe.DW60.FixedPointFillColorRVChannel     = Src.R << 8;
            m_hdrCurbe.DW60.FixedPointFillColorGYChannel     = Src.G << 8;
            m_hdrCurbe.DW61.FixedPointFillColorBUChannel     = Src.B << 8;
            m_hdrCurbe.DW61.FixedPointFillColorAlphaChannel  = Src.A << 8;
        }
    }

    curbe = (void*)(&m_hdrCurbe);
    curbeLength = (uint32_t)sizeof(MEDIA_WALKER_HDR_STATIC_DATA);

    //DumpCurbe(&m_hdrCurbe, sizeof(MEDIA_WALKER_HDR_STATIC_DATA));

    PrintCurbeData(&m_hdrCurbe);

    return eStatus;
}

uint32_t VpRenderHdrKernel::GetInlineDataSize()
{
    VP_FUNC_CALL();

    return 0;
}

MOS_STATUS VpRenderHdrKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS& walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{
    VP_FUNC_CALL();

    RENDERHAL_KERNEL_PARAM kernelSettings;
    int32_t                iBlocksX, iBlocksY;
    int32_t                iBlockWd;  // Block width
    int32_t                iBlockHt;  // Block Height
    uint32_t               uiMediaWalkerBlockSize;
    bool                   bVerticalPattern = false;
    RECT                   AlignedRect      = {};
    uint32_t               dwSrcWidth;
    uint32_t               dwSrcHeight;

    walkerParam.iBindingTable = renderData.bindingTable;
    walkerParam.iMediaID      = renderData.mediaID;
    walkerParam.iCurbeOffset  = renderData.iCurbeOffset;
    walkerParam.iCurbeLength  = renderData.iCurbeLength;
    // iBlocksX/iBlocksY will be calculated during prepare walker parameters in RenderCmdPacket
    walkerParam.calculateBlockXYByAlignedRect = false;

    auto        it   = m_surfaceGroup->find(SurfaceTypeHdrInputLayer0);
    VP_SURFACE *surf = (m_surfaceGroup->end() != it) ? it->second : nullptr;
    if (surf == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("input surf was not found");
        return MOS_STATUS_NULL_POINTER;
    }
    VP_RENDER_CHK_NULL_RETURN(surf->osSurface);

    auto        OutputIt   = m_surfaceGroup->find(SurfaceTypeHdrTarget0);
    VP_SURFACE *outputSurf = (m_surfaceGroup->end() != OutputIt) ? OutputIt->second : nullptr;
    if (outputSurf == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("output surf was not found");
        return MOS_STATUS_NULL_POINTER;
    }

    if (0 == m_hdrParams->uTargetCount || m_hdrParams->uTargetCount > 1)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    walkerParam.alignedRect = outputSurf->rcDst;

    if (m_hdrParams->uSourceCount == 1 &&
        surf->osSurface->TileType == MOS_TILE_LINEAR &&
        (m_hdrParams->Rotation == VPHAL_ROTATION_90 ||
            m_hdrParams->Rotation == VPHAL_ROTATION_270))
    {
        walkerParam.isVerticalPattern = true;
    }

    walkerParam.bSyncFlag                       = 0;
    walkerParam.isGroupStartInvolvedInGroupSize = true;

    if (m_hdrParams->uSourceCount == 1 &&
        surf->osSurface->TileType == MOS_TILE_LINEAR &&
        (m_hdrParams->Rotation == VPHAL_ROTATION_90 || m_hdrParams->Rotation == VPHAL_ROTATION_270))
    {
        walkerParam.isVerticalPattern = true;
    }

    iBlockWd = renderData.KernelParam.block_width;
    iBlockHt = renderData.KernelParam.block_height;

    dwSrcWidth           = outputSurf->rcDst.right - outputSurf->rcDst.left;
    dwSrcHeight          = outputSurf->rcSrc.bottom - outputSurf->rcDst.top;
    iBlocksX             = (dwSrcWidth + iBlockWd - 1) / iBlockWd;
    iBlocksY             = (dwSrcHeight + iBlockHt - 1) / iBlockHt;
    walkerParam.iBlocksX = iBlocksX;
    walkerParam.iBlocksY = iBlocksY;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdrKernel::SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs)
{
    VP_FUNC_CALL();

    if (m_hdrParams == nullptr)
    {
        m_hdrParams = (RENDER_HDR_PARAMS *)MOS_AllocAndZeroMemory(sizeof(RENDER_HDR_PARAMS));
    }
    VP_RENDER_CHK_NULL_RETURN(m_hdrParams);

    PRENDER_HDR_PARAMS hdrParams = nullptr;
    if (kernelConfigs.find(m_kernelId) != kernelConfigs.end())
    {
        hdrParams = (PRENDER_HDR_PARAMS)kernelConfigs.find(m_kernelId)->second;
    }

    VP_RENDER_CHK_NULL_RETURN(hdrParams);

    MOS_SecureMemcpy(m_hdrParams, sizeof(RENDER_HDR_PARAMS), hdrParams, sizeof(RENDER_HDR_PARAMS));

    return MOS_STATUS_SUCCESS;

}

//! \brief    Get the HDR format descriptor of a format
//! \details  Get the HDR format descriptor of a format and return.
//! \param    MOS_FORMAT Format
//!           [in] MOS_FORMAT of a surface
//! \return   VPHAL_HDR_FORMAT_DESCRIPTOR
//!           HDR format descriptor
//!
VPHAL_HDR_FORMAT_DESCRIPTOR VpRenderHdrKernel::GetFormatDescriptor(
    MOS_FORMAT      Format)
{
    VP_FUNC_CALL();

    VPHAL_HDR_FORMAT_DESCRIPTOR FormatDescriptor  = VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW;

    switch (Format)
    {
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_R10G10B10A2_UNORM;
            break;

        case Format_X8R8G8B8:
        case Format_A8R8G8B8:
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
        case Format_AYUV:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_R8G8B8A8_UNORM;
            break;

        case Format_NV12:
        case Format_NV21:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_NV12;
            break;

        case Format_YUY2:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_YUY2;
            break;

        case Format_P010:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_P010;
            break;

        case Format_P016:
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_P016;
            break;

        case Format_A16R16G16B16F:
        case Format_A16B16G16R16F:
            FormatDescriptor = VPHAL_HDR_FORMAT_R16G16B16A16_FLOAT;
            break;

        default:
            VP_RENDER_ASSERTMESSAGE("Unsupported input format.");
            FormatDescriptor = VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW;
            break;
    }

    return FormatDescriptor;
}

//! \brief    Get the HDR Chroma siting
//! \details  Get the HDR Chroma siting and return.
//! \param    uint32_t ChromaSiting
//!           [in] ChromaSiting of a surface
//! \return   VPHAL_HDR_CHROMA_SITING
//!           HDR Chroma siting
//!
VPHAL_HDR_CHROMA_SITING VpRenderHdrKernel::GetHdrChromaSiting(
    uint32_t      ChromaSiting)
{
    VP_FUNC_CALL();

    VPHAL_HDR_CHROMA_SITING HdrChromaSiting = VPHAL_HDR_CHROMA_SITTING_A;

    switch (ChromaSiting)
    {
    case CHROMA_SITING_HORZ_LEFT :
        HdrChromaSiting = VPHAL_HDR_CHROMA_SITTING_A;
        break;
    default:
        HdrChromaSiting = VPHAL_HDR_CHROMA_SITTING_A;
        break;
    }

    return HdrChromaSiting;
}

//! \brief    Get the HDR rotation
//! \details  Get the HDR rotation and return.
//! \param    VPHAL_ROTATION Rotation
//!           [in] Rotation of a surface
//! \return   VPHAL_HDR_ROTATION
//!           HDR Chroma siting
//!
VPHAL_HDR_ROTATION VpRenderHdrKernel::GetHdrRotation(
    VPHAL_ROTATION      Rotation)
{
    VP_FUNC_CALL();

    VPHAL_HDR_ROTATION HdrRotation  = VPHAL_HDR_LAYER_ROTATION_0;

    switch (Rotation)
    {
    case VPHAL_ROTATION_IDENTITY :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_0;
        break;
    case VPHAL_ROTATION_90 :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_90;
        break;
    case VPHAL_ROTATION_180 :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_180;
        break;
    case VPHAL_ROTATION_270 :
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_270;
        break;
    case VPHAL_MIRROR_HORIZONTAL :
        HdrRotation = VPHAL_HDR_LAYER_MIRROR_H;
        break;
    case VPHAL_MIRROR_VERTICAL :
        HdrRotation = VPHAL_HDR_LAYER_MIRROR_V;
        break;
    case VPHAL_ROTATE_90_MIRROR_VERTICAL :
        HdrRotation = VPHAL_HDR_LAYER_ROT_90_MIR_V;
        break;
    case VPHAL_ROTATE_90_MIRROR_HORIZONTAL :
        HdrRotation = VPHAL_HDR_LAYER_ROT_90_MIR_H;
        break;
    default:
        HdrRotation = VPHAL_HDR_LAYER_ROTATION_0;
        break;
    }

    return HdrRotation;
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
MOS_STATUS VpRenderHdrKernel::SamplerAvsCalcScalingTable(
    MOS_FORMAT                      SrcFormat,
    float                           fScale,
    bool                            bVertical,
    uint32_t                        dwChromaSiting,
    bool                            bBalancedFilter,
    bool                            b8TapAdaptiveEnable,
    PMHW_AVS_PARAMS                 pAvsParams)
{
    VP_FUNC_CALL();

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

    VP_RENDER_CHK_NULL_RETURN(pAvsParams);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piYCoefsY);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piYCoefsX);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piUVCoefsY);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piUVCoefsX);

    if (bBalancedFilter)
    {
        YCoefTableSize      = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        UVCoefTableSize     = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;
        dwHwPhrase          = NUM_HW_POLYPHASE_TABLES;
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
            VP_RENDER_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                piYCoefsParam,
                Plane,
                bBalancedFilter));
            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                VP_RENDER_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                    piUVCoefsParam,
                    MHW_U_PLANE,
                    bBalancedFilter));
            }
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScale = MOS_MIN(1.0F, fScale);

            VP_RENDER_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                piYCoefsParam,
                fScale,
                Plane,
                SrcFormat,
                fHPStrength,
                true,
                dwHwPhrase,
                0));

            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                if (!bBalancedFilter)
                {
                    VP_RENDER_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                        piUVCoefsParam,
                        fScale,
                        MHW_U_PLANE,
                        SrcFormat,
                        fHPStrength,
                        true,
                        dwHwPhrase,
                        0));
                }
                else
                {
                    // If Chroma Siting info is present
                    if (dwChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_TOP : MHW_CHROMA_SITING_HORZ_LEFT))
                    {
                        // No Chroma Siting
                        VP_RENDER_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
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

                        VP_RENDER_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUVOffset(
                            piUVCoefsParam,
                            3.0F,
                            fScale,
                            iUvPhaseOffset));
                    }
                }
            }
        }
    }

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
MOS_STATUS VpRenderHdrKernel::SetSamplerAvsTableParam(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting)
{
    VP_FUNC_CALL();

    MOS_STATUS                   eStatus                    = MOS_STATUS_SUCCESS;
    bool                         bBalancedFilter            = false;
    PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam   = nullptr;

    VP_RENDER_CHK_NULL_RETURN(pRenderHal);
    VP_RENDER_CHK_NULL_RETURN(pSamplerStateParams);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piYCoefsY);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piYCoefsX);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piUVCoefsY);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams->piUVCoefsX);

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
        return MOS_STATUS_SUCCESS;
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
    VP_RENDER_CHK_STATUS_RETURN(SamplerAvsCalcScalingTable(
        SrcFormat,
        fScaleX,
        false,
        dwChromaSiting,
        bBalancedFilter,
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
        pAvsParams));

    // Recalculate Vertical scaling table
    VP_RENDER_CHK_STATUS_RETURN(SamplerAvsCalcScalingTable(
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

    return eStatus;
}

MOS_STATUS VpRenderHdrKernel::GetSamplerIndex(
    VPHAL_SCALING_MODE scalingMode,
    uint32_t           yuvPlane,
    int32_t           &samplerIndex,
    MHW_SAMPLER_TYPE  &samplerType)
{
    const int32_t samplerindex[2][3] = {{VP_SAMPLER_INDEX_Y_NEAREST, VP_SAMPLER_INDEX_U_NEAREST, VP_SAMPLER_INDEX_V_NEAREST},
        {VP_SAMPLER_INDEX_Y_BILINEAR, VP_SAMPLER_INDEX_U_BILINEAR, VP_SAMPLER_INDEX_V_BILINEAR}};

    if (scalingMode == VPHAL_SCALING_AVS)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // if Scalingmode is BILINEAR, use the 4,5,6. if NEAREST, use 1,2,3
    samplerType  = MHW_SAMPLER_TYPE_3D;
    samplerIndex = samplerindex[scalingMode][yuvPlane];

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdrKernel::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup)
{
    MOS_STATUS                  eStatus             = MOS_STATUS_SUCCESS;
    MHW_SAMPLER_STATE_PARAM samplerStateParam       = {};
    uint32_t                    i                   = 0;
    uint32_t                       entryIndex        = 0;
    int32_t                        samplerIndex      = 0;
    MHW_SAMPLER_TYPE               samplerType       = MHW_SAMPLER_TYPE_INVALID;

    m_samplerStateGroup = &samplerStateGroup;
    m_samplerIndexes.clear();
    m_samplerIndexes.push_back(0);

    return eStatus;
}

void VpRenderHdrKernel::DumpSurfaces()
{
    VP_FUNC_CALL();

    PMOS_INTERFACE        pOsInterface = m_hwInterface->m_osInterface;

    auto inputSurface = m_surfaceGroup->find(SurfaceTypeHdrInputLayer0);
    VP_SURFACE* surf1 = (m_surfaceGroup->end() != inputSurface) ? inputSurface->second : nullptr;

    auto OETF1DLUTSurface = m_surfaceGroup->find(SurfaceTypeHdrOETF1DLUTSurface0);
    VP_SURFACE* surf2 = (m_surfaceGroup->end() != OETF1DLUTSurface) ? OETF1DLUTSurface->second : nullptr;

    auto coeffSurface = m_surfaceGroup->find(SurfaceTypeHdrCoeff);
    VP_SURFACE* surf3 = (m_surfaceGroup->end() != coeffSurface) ? coeffSurface->second : nullptr;

    auto targetSurface = m_surfaceGroup->find(SurfaceTypeHdrTarget0);
    VP_SURFACE* surf4 = (m_surfaceGroup->end() != targetSurface) ? targetSurface->second : nullptr;

    auto        RenderInputSurface = m_surfaceGroup->find(SurfaceTypeHdrInputLayer0);
    VP_SURFACE *surf5              = (m_surfaceGroup->end() != RenderInputSurface) ? RenderInputSurface->second : nullptr;

    if (surf1)
    {
        DumpSurface(surf1,VP_HDR_DUMP_INPUT_LAYER0);
    }
    if (surf5)
    {
        DumpSurface(surf5, VP_HDR_DUMP_RENDER_INPUT);
    }

    if (surf2)
    {
       DumpSurface(surf2,VP_HDR_DUMP_OETF1DLUT_SURFACE0);
    }
    if (surf3)
    {
        DumpSurface(surf3,VP_HDR_DUMP_COEFF_SURFACE);
    }
    if (surf4)
    {
       DumpSurface(surf4,VP_HDR_DUMP_TARGET_SURFACE0);
    }

    return;
}

void VpRenderHdrKernel::DumpCurbe(void *pCurbe, int32_t iSize)
{
    VP_FUNC_CALL();
    char CurbeName[MAX_PATH];

    MOS_SecureStringPrint(
        CurbeName,
        sizeof(CurbeName),
        sizeof(CurbeName),
        "c:\\\\dump\\f[%04d]hdr_Curbe.dat",
        1);
    MosUtilities::MosWriteFileFromPtr(
        (const char *)CurbeName,
        pCurbe,
        iSize);
}

void VpRenderHdrKernel::PrintCurbeData(PMEDIA_WALKER_HDR_STATIC_DATA curbeData)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (curbeData == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("The curbeData pointer is null");
        return;
    }
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW0.Value = %x", curbeData->DW0.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer0 = %x",
        curbeData->DW0.HorizontalFrameOriginLayer0);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW1.Value = %x", curbeData->DW1.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer1 = %x",
        curbeData->DW1.HorizontalFrameOriginLayer1);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW2.Value = %x", curbeData->DW2.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer2 = %x",
        curbeData->DW2.HorizontalFrameOriginLayer2);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW3.Value = %x", curbeData->DW3.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer3 = %x",
        curbeData->DW3.HorizontalFrameOriginLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW4.Value = %x", curbeData->DW4.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer4 = %x",
        curbeData->DW4.HorizontalFrameOriginLayer4);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW5.Value = %x", curbeData->DW5.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer5 = %x",
        curbeData->DW5.HorizontalFrameOriginLayer5);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW6.Value = %x", curbeData->DW6.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer6 = %x",
        curbeData->DW6.HorizontalFrameOriginLayer6);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW7.Value = %x", curbeData->DW7.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalFrameOriginLayer7 = %x",
        curbeData->DW7.HorizontalFrameOriginLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW8.Value = %x", curbeData->DW8.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer0 = %x",
        curbeData->DW8.VerticalFrameOriginLayer0);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW9.Value = %x", curbeData->DW9.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer1 = %x",
        curbeData->DW9.VerticalFrameOriginLayer1);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW10.Value = %x", curbeData->DW10.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer2 = %x",
        curbeData->DW10.VerticalFrameOriginLayer2);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW11.Value = %x", curbeData->DW11.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer3 = %x",
        curbeData->DW11.VerticalFrameOriginLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW12.Value = %x", curbeData->DW12.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer4 = %x",
        curbeData->DW12.VerticalFrameOriginLayer4);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW13.Value = %x", curbeData->DW13.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer5 = %x",
        curbeData->DW13.VerticalFrameOriginLayer5);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW14.Value = %x", curbeData->DW14.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer6 = %x",
        curbeData->DW14.VerticalFrameOriginLayer6);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW15.Value = %x", curbeData->DW15.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalFrameOriginLayer7 = %x",
        curbeData->DW15.VerticalFrameOriginLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW16.Value = %x", curbeData->DW16.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer0 = %x",
        curbeData->DW16.HorizontalScalingStepRatioLayer0);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW17.Value = %x", curbeData->DW17.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer1 = %x",
        curbeData->DW17.HorizontalScalingStepRatioLayer1);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW18.Value = %x", curbeData->DW18.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer2 = %x",
        curbeData->DW18.HorizontalScalingStepRatioLayer2);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW19.Value = %x", curbeData->DW19.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer3 = %x",
        curbeData->DW19.HorizontalScalingStepRatioLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW20.Value = %x", curbeData->DW20.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer4 = %x",
        curbeData->DW20.HorizontalScalingStepRatioLayer4);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW21.Value = %x", curbeData->DW21.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer5 = %x",
        curbeData->DW21.HorizontalScalingStepRatioLayer5);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW22.Value = %x", curbeData->DW22.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer6 = %x",
        curbeData->DW22.HorizontalScalingStepRatioLayer6);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW23.Value = %x", curbeData->DW23.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     HorizontalScalingStepRatioLayer7 = %x",
        curbeData->DW23.HorizontalScalingStepRatioLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW24.Value = %x", curbeData->DW24.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer0 = %x",
        curbeData->DW24.VerticalScalingStepRatioLayer0);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW25.Value = %x", curbeData->DW25.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer1 = %x",
        curbeData->DW25.VerticalScalingStepRatioLayer1);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW26.Value = %x", curbeData->DW26.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer2 = %x",
        curbeData->DW26.VerticalScalingStepRatioLayer2);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW27.Value = %x", curbeData->DW27.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer3 = %x",
        curbeData->DW27.VerticalScalingStepRatioLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW28.Value = %x", curbeData->DW28.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer4 = %x",
        curbeData->DW28.VerticalScalingStepRatioLayer4);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW29.Value = %x", curbeData->DW29.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer5 = %x",
        curbeData->DW29.VerticalScalingStepRatioLayer5);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW30.Value = %x", curbeData->DW30.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer6 = %x",
        curbeData->DW30.VerticalScalingStepRatioLayer6);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW31.Value = %x", curbeData->DW31.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     VerticalScalingStepRatioLayer7 = %x",
        curbeData->DW31.VerticalScalingStepRatioLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW32.Value = %d", curbeData->DW32.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer0 = 0x%d, TopCoordinateRectangleLayer0 = 0x%d",
        curbeData->DW32.LeftCoordinateRectangleLayer0,
        curbeData->DW32.TopCoordinateRectangleLayer0);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW33.Value = %d", curbeData->DW33.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer1 = 0x%d, TopCoordinateRectangleLayer2 = 0x%d",
        curbeData->DW33.LeftCoordinateRectangleLayer1,
        curbeData->DW33.TopCoordinateRectangleLayer1);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW34.Value = %d", curbeData->DW34.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer2 = 0x%d, TopCoordinateRectangleLayer2 = 0x%d",
        curbeData->DW34.LeftCoordinateRectangleLayer2,
        curbeData->DW34.TopCoordinateRectangleLayer2);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW35.Value = %d", curbeData->DW35.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer3 = 0x%d, TopCoordinateRectangleLayer3 = 0x%d",
        curbeData->DW35.LeftCoordinateRectangleLayer3,
        curbeData->DW35.TopCoordinateRectangleLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW36.Value = %d", curbeData->DW36.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer4 = 0x%d, TopCoordinateRectangleLayer4 = 0x%d",
        curbeData->DW36.LeftCoordinateRectangleLayer4,
        curbeData->DW36.TopCoordinateRectangleLayer4);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW37.Value = %d", curbeData->DW37.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer5 = 0x%d, TopCoordinateRectangleLayer5 = 0x%d",
        curbeData->DW37.LeftCoordinateRectangleLayer5,
        curbeData->DW37.TopCoordinateRectangleLayer5);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW38.Value = %d", curbeData->DW38.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer6 = 0x%d, TopCoordinateRectangleLayer6 = 0x%d",
        curbeData->DW38.LeftCoordinateRectangleLayer6,
        curbeData->DW38.TopCoordinateRectangleLayer6);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW39.Value = %d", curbeData->DW39.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     LeftCoordinateRectangleLayer7 = 0x%d, TopCoordinateRectangleLayer7 = 0x%d",
        curbeData->DW39.LeftCoordinateRectangleLayer7,
        curbeData->DW39.TopCoordinateRectangleLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW40.Value = %d", curbeData->DW40.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer0 = 0x%d, RightCoordinateRectangleLayer0 = 0x%d",
        curbeData->DW40.BottomCoordinateRectangleLayer0,
        curbeData->DW40.RightCoordinateRectangleLayer0);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW41.Value = %d", curbeData->DW41.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer1 = 0x%d, RightCoordinateRectangleLayer1 = 0x%d",
        curbeData->DW41.BottomCoordinateRectangleLayer1,
        curbeData->DW41.RightCoordinateRectangleLayer1);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW42.Value = %d", curbeData->DW42.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer2 = 0x%d, RightCoordinateRectangleLayer2 = 0x%d",
        curbeData->DW42.BottomCoordinateRectangleLayer2,
        curbeData->DW42.RightCoordinateRectangleLayer2);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW43.Value = %d", curbeData->DW43.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer3 = 0x%d, RightCoordinateRectangleLayer3 = 0x%d",
        curbeData->DW43.BottomCoordinateRectangleLayer3,
        curbeData->DW43.RightCoordinateRectangleLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW44.Value = %d", curbeData->DW44.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer4 = 0x%d, RightCoordinateRectangleLayer4 = 0x%d",
        curbeData->DW44.BottomCoordinateRectangleLayer4,
        curbeData->DW44.RightCoordinateRectangleLayer4);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW45.Value = %d", curbeData->DW45.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer5 = 0x%d, RightCoordinateRectangleLayer5 = 0x%d",
        curbeData->DW45.BottomCoordinateRectangleLayer5,
        curbeData->DW45.RightCoordinateRectangleLayer5);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW46.Value = %d", curbeData->DW46.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer6 = 0x%d, RightCoordinateRectangleLayer6 = 0x%d",
        curbeData->DW46.BottomCoordinateRectangleLayer6,
        curbeData->DW46.RightCoordinateRectangleLayer6);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW47.Value = %d", curbeData->DW47.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     BottomCoordinateRectangleLayer7 = 0x%d, RightCoordinateRectangleLayer7 = 0x%d",
        curbeData->DW47.BottomCoordinateRectangleLayer7,
        curbeData->DW47.RightCoordinateRectangleLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW48.Value = %d", curbeData->DW48.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer0 = %d, CCMExtensionEnablingFlagLayer0 = %d, ChannelSwapEnablingFlagLayer0 = %d, ChromaSittingLocationLayer0 = %d, Enabling3DLUTFlagLayer0 = %d, EOTF1DLUTEnablingFlagLayer0 = %d, FormatDescriptorLayer0 = %d, IEFBypassEnablingFlagLayer0 = %d, OETF1DLUTEnablingFlagLayer0 = %d, PostCSCEnablingFlagLayer0 = %d, PriorCSCEnablingFlagLayer0 = %d, RotationAngleMirrorDirectionLayer0 = %d, SamplerIndexFirstPlaneLayer0 = %d, SamplerIndexSecondThirdPlaneLayer0 = %d, ToneMappingEnablingFlagLayer0 = %d",
        curbeData->DW48.CCMEnablingFlagLayer0,
        curbeData->DW48.CCMExtensionEnablingFlagLayer0,
        curbeData->DW48.ChannelSwapEnablingFlagLayer0,
        curbeData->DW48.ChromaSittingLocationLayer0,
        curbeData->DW48.Enabling3DLUTFlagLayer0,
        curbeData->DW48.EOTF1DLUTEnablingFlagLayer0,
        curbeData->DW48.FormatDescriptorLayer0,
        curbeData->DW48.IEFBypassEnablingFlagLayer0,
        curbeData->DW48.OETF1DLUTEnablingFlagLayer0,
        curbeData->DW48.PostCSCEnablingFlagLayer0,
        curbeData->DW48.PriorCSCEnablingFlagLayer0,
        curbeData->DW48.RotationAngleMirrorDirectionLayer0,
        curbeData->DW48.SamplerIndexFirstPlaneLayer0,
        curbeData->DW48.SamplerIndexSecondThirdPlaneLayer0,
        curbeData->DW48.ToneMappingEnablingFlagLayer0);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW49.Value = %d", curbeData->DW49.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer1 = %d, CCMExtensionEnablingFlagLayer1 = %d, ChannelSwapEnablingFlagLayer1 = %d, ChromaSittingLocationLayer1 = %d, Enabling3DLUTFlagLayer1 = %d, EOTF1DLUTEnablingFlagLayer1 = %d, FormatDescriptorLayer1 = %d, IEFBypassEnablingFlagLayer1 = %d, OETF1DLUTEnablingFlagLayer0 = %d, PostCSCEnablingFlagLayer1 = %d, PriorCSCEnablingFlagLayer1 = %d, RotationAngleMirrorDirectionLayer1 = %d, SamplerIndexFirstPlaneLayer0 = %d, SamplerIndexSecondThirdPlaneLayer1 = %d, ToneMappingEnablingFlagLayer1 = %d",
        curbeData->DW49.CCMEnablingFlagLayer1,
        curbeData->DW49.CCMExtensionEnablingFlagLayer1,
        curbeData->DW49.ChannelSwapEnablingFlagLayer1,
        curbeData->DW49.ChromaSittingLocationLayer1,
        curbeData->DW49.Enabling3DLUTFlagLayer1,
        curbeData->DW49.EOTF1DLUTEnablingFlagLayer1,
        curbeData->DW49.FormatDescriptorLayer1,
        curbeData->DW49.IEFBypassEnablingFlagLayer1,
        curbeData->DW49.OETF1DLUTEnablingFlagLayer1,
        curbeData->DW49.PostCSCEnablingFlagLayer1,
        curbeData->DW49.PriorCSCEnablingFlagLayer1,
        curbeData->DW49.RotationAngleMirrorDirectionLayer1,
        curbeData->DW49.SamplerIndexFirstPlaneLayer1,
        curbeData->DW49.SamplerIndexSecondThirdPlaneLayer1,
        curbeData->DW49.ToneMappingEnablingFlagLayer1);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW50.Value = %d", curbeData->DW50.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer2 = %d, CCMExtensionEnablingFlagLayer2 = %d, ChannelSwapEnablingFlagLayer2 = %d, ChromaSittingLocationLayer2 = %d, Enabling3DLUTFlagLayer2 = %d, EOTF1DLUTEnablingFlagLayer2 = %d, FormatDescriptorLayer2 = %d, IEFBypassEnablingFlagLayer2 = %d, OETF1DLUTEnablingFlagLayer2 = %d, PostCSCEnablingFlagLayer2 = %d, PriorCSCEnablingFlagLayer2 = %d, RotationAngleMirrorDirectionLayer2 = %d, SamplerIndexFirstPlaneLayer2 = %d, SamplerIndexSecondThirdPlaneLayer2 = %d, ToneMappingEnablingFlagLayer2 = %d",
        curbeData->DW50.CCMEnablingFlagLayer2,
        curbeData->DW50.CCMExtensionEnablingFlagLayer2,
        curbeData->DW50.ChannelSwapEnablingFlagLayer2,
        curbeData->DW50.ChromaSittingLocationLayer2,
        curbeData->DW50.Enabling3DLUTFlagLayer2,
        curbeData->DW50.EOTF1DLUTEnablingFlagLayer2,
        curbeData->DW50.FormatDescriptorLayer2,
        curbeData->DW50.IEFBypassEnablingFlagLayer2,
        curbeData->DW50.OETF1DLUTEnablingFlagLayer2,
        curbeData->DW50.PostCSCEnablingFlagLayer2,
        curbeData->DW50.PriorCSCEnablingFlagLayer2,
        curbeData->DW50.RotationAngleMirrorDirectionLayer2,
        curbeData->DW50.SamplerIndexFirstPlaneLayer2,
        curbeData->DW50.SamplerIndexSecondThirdPlaneLayer2,
        curbeData->DW50.ToneMappingEnablingFlagLayer2);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW51.Value = %d", curbeData->DW51.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer3 = %d, CCMExtensionEnablingFlagLayer3 = %d, ChannelSwapEnablingFlagLayer3 = %d, ChromaSittingLocationLayer3 = %d, Enabling3DLUTFlagLayer3 = %d, EOTF1DLUTEnablingFlagLayer3 = %d, FormatDescriptorLayer3 = %d, IEFBypassEnablingFlagLayer3 = %d, OETF1DLUTEnablingFlagLayer3 = %d, PostCSCEnablingFlagLayer3 = %d, PriorCSCEnablingFlagLayer3 = %d, RotationAngleMirrorDirectionLayer3 = %d, SamplerIndexFirstPlaneLayer3 = %d, SamplerIndexSecondThirdPlaneLayer3 = %d, ToneMappingEnablingFlagLayer3 = %d",
        curbeData->DW51.CCMEnablingFlagLayer3,
        curbeData->DW51.CCMExtensionEnablingFlagLayer3,
        curbeData->DW51.ChannelSwapEnablingFlagLayer3,
        curbeData->DW51.ChromaSittingLocationLayer3,
        curbeData->DW51.Enabling3DLUTFlagLayer3,
        curbeData->DW51.EOTF1DLUTEnablingFlagLayer3,
        curbeData->DW51.FormatDescriptorLayer3,
        curbeData->DW51.IEFBypassEnablingFlagLayer3,
        curbeData->DW51.OETF1DLUTEnablingFlagLayer3,
        curbeData->DW51.PostCSCEnablingFlagLayer3,
        curbeData->DW51.PriorCSCEnablingFlagLayer3,
        curbeData->DW51.RotationAngleMirrorDirectionLayer3,
        curbeData->DW51.SamplerIndexFirstPlaneLayer3,
        curbeData->DW51.SamplerIndexSecondThirdPlaneLayer3,
        curbeData->DW51.ToneMappingEnablingFlagLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW52.Value = %d", curbeData->DW52.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer4 = %d, CCMExtensionEnablingFlagLayer4 = %d, ChannelSwapEnablingFlagLayer4 = %d, ChromaSittingLocationLayer4 = %d, Enabling3DLUTFlagLayer4 = %d, EOTF1DLUTEnablingFlagLayer4 = %d, FormatDescriptorLayer4 = %d, IEFBypassEnablingFlagLayer4 = %d, OETF1DLUTEnablingFlagLayer4 = %d, PostCSCEnablingFlagLayer4 = %d, PriorCSCEnablingFlagLayer4 = %d, RotationAngleMirrorDirectionLayer4 = %d, SamplerIndexFirstPlaneLayer4 = %d, SamplerIndexSecondThirdPlaneLayer4 = %d, ToneMappingEnablingFlagLayer4 = %d",
        curbeData->DW52.CCMEnablingFlagLayer4,
        curbeData->DW52.CCMExtensionEnablingFlagLayer4,
        curbeData->DW52.ChannelSwapEnablingFlagLayer4,
        curbeData->DW52.ChromaSittingLocationLayer4,
        curbeData->DW52.Enabling3DLUTFlagLayer4,
        curbeData->DW52.EOTF1DLUTEnablingFlagLayer4,
        curbeData->DW52.FormatDescriptorLayer4,
        curbeData->DW52.IEFBypassEnablingFlagLayer4,
        curbeData->DW52.OETF1DLUTEnablingFlagLayer4,
        curbeData->DW52.PostCSCEnablingFlagLayer4,
        curbeData->DW52.PriorCSCEnablingFlagLayer4,
        curbeData->DW52.RotationAngleMirrorDirectionLayer4,
        curbeData->DW52.SamplerIndexFirstPlaneLayer4,
        curbeData->DW52.SamplerIndexSecondThirdPlaneLayer4,
        curbeData->DW52.ToneMappingEnablingFlagLayer4);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW53.Value = %d", curbeData->DW53.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer5 = %d, CCMExtensionEnablingFlagLayer5 = %d, ChannelSwapEnablingFlagLayer5 = %d, ChromaSittingLocationLayer5 = %d, Enabling3DLUTFlagLayer5 = %d, EOTF1DLUTEnablingFlagLayer5 = %d, FormatDescriptorLayer5 = %d, IEFBypassEnablingFlagLayer5 = %d, OETF1DLUTEnablingFlagLayer5 = %d, PostCSCEnablingFlagLayer5 = %d, PriorCSCEnablingFlagLayer5 = %d, RotationAngleMirrorDirectionLayer5 = %d, SamplerIndexFirstPlaneLayer5 = %d, SamplerIndexSecondThirdPlaneLayer5 = %d, ToneMappingEnablingFlagLayer5 = %d",
        curbeData->DW53.CCMEnablingFlagLayer5,
        curbeData->DW53.CCMExtensionEnablingFlagLayer5,
        curbeData->DW53.ChannelSwapEnablingFlagLayer5,
        curbeData->DW53.ChromaSittingLocationLayer5,
        curbeData->DW53.Enabling3DLUTFlagLayer5,
        curbeData->DW53.EOTF1DLUTEnablingFlagLayer5,
        curbeData->DW53.FormatDescriptorLayer5,
        curbeData->DW53.IEFBypassEnablingFlagLayer5,
        curbeData->DW53.OETF1DLUTEnablingFlagLayer5,
        curbeData->DW53.PostCSCEnablingFlagLayer5,
        curbeData->DW53.PriorCSCEnablingFlagLayer5,
        curbeData->DW53.RotationAngleMirrorDirectionLayer5,
        curbeData->DW53.SamplerIndexFirstPlaneLayer5,
        curbeData->DW53.SamplerIndexSecondThirdPlaneLayer5,
        curbeData->DW53.ToneMappingEnablingFlagLayer5);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW54.Value = %d", curbeData->DW54.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer6 = %d, CCMExtensionEnablingFlagLayer6 = %d, ChannelSwapEnablingFlagLayer6 = %d, ChromaSittingLocationLayer6 = %d, Enabling3DLUTFlagLayer6 = %d, EOTF1DLUTEnablingFlagLayer6 = %d, FormatDescriptorLayer6 = %d, IEFBypassEnablingFlagLayer6 = %d, OETF1DLUTEnablingFlagLayer6 = %d, PostCSCEnablingFlagLayer6 = %d, PriorCSCEnablingFlagLayer6 = %d, RotationAngleMirrorDirectionLayer6 = %d, SamplerIndexFirstPlaneLayer6 = %d, SamplerIndexSecondThirdPlaneLayer6 = %d, ToneMappingEnablingFlagLayer6 = %d",
        curbeData->DW54.CCMEnablingFlagLayer6,
        curbeData->DW54.CCMExtensionEnablingFlagLayer6,
        curbeData->DW54.ChannelSwapEnablingFlagLayer6,
        curbeData->DW54.ChromaSittingLocationLayer6,
        curbeData->DW54.Enabling3DLUTFlagLayer6,
        curbeData->DW54.EOTF1DLUTEnablingFlagLayer6,
        curbeData->DW54.FormatDescriptorLayer6,
        curbeData->DW54.IEFBypassEnablingFlagLayer6,
        curbeData->DW54.OETF1DLUTEnablingFlagLayer6,
        curbeData->DW54.PostCSCEnablingFlagLayer6,
        curbeData->DW54.PriorCSCEnablingFlagLayer6,
        curbeData->DW54.RotationAngleMirrorDirectionLayer6,
        curbeData->DW54.SamplerIndexFirstPlaneLayer6,
        curbeData->DW54.SamplerIndexSecondThirdPlaneLayer6,
        curbeData->DW54.ToneMappingEnablingFlagLayer6);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW55.Value = %x", curbeData->DW55.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     CCMEnablingFlagLayer7 = %d, CCMExtensionEnablingFlagLayer7 = %d, ChannelSwapEnablingFlagLayer7 = %d, ChromaSittingLocationLayer7 = %d, Enabling3DLUTFlagLayer7 = %d, EOTF1DLUTEnablingFlagLayer7 = %d, FormatDescriptorLayer7 = %d, IEFBypassEnablingFlagLayer7 = %d, OETF1DLUTEnablingFlagLayer7 = %d, PostCSCEnablingFlagLayer7 = %d, PriorCSCEnablingFlagLayer7 = %d, RotationAngleMirrorDirectionLayer7 = %d, SamplerIndexFirstPlaneLayer7 = %d, SamplerIndexSecondThirdPlaneLayer7 = %d, ToneMappingEnablingFlagLayer7 = %d",
        curbeData->DW55.CCMEnablingFlagLayer7,
        curbeData->DW55.CCMExtensionEnablingFlagLayer7,
        curbeData->DW55.ChannelSwapEnablingFlagLayer7,
        curbeData->DW55.ChromaSittingLocationLayer7,
        curbeData->DW55.Enabling3DLUTFlagLayer7,
        curbeData->DW55.EOTF1DLUTEnablingFlagLayer7,
        curbeData->DW55.FormatDescriptorLayer7,
        curbeData->DW55.IEFBypassEnablingFlagLayer7,
        curbeData->DW55.OETF1DLUTEnablingFlagLayer7,
        curbeData->DW55.PostCSCEnablingFlagLayer7,
        curbeData->DW55.PriorCSCEnablingFlagLayer7,
        curbeData->DW55.RotationAngleMirrorDirectionLayer7,
        curbeData->DW55.SamplerIndexFirstPlaneLayer7,
        curbeData->DW55.SamplerIndexSecondThirdPlaneLayer7,
        curbeData->DW55.ToneMappingEnablingFlagLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW56.Value = %d", curbeData->DW56.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     ConstantBlendingAlphaFillColorLayer0 = %d, ConstantBlendingAlphaFillColorLayer1 = %d, ConstantBlendingAlphaFillColorLayer2 = %d, ConstantBlendingAlphaFillColorLayer3 = %d",
        curbeData->DW56.ConstantBlendingAlphaFillColorLayer0,
        curbeData->DW56.ConstantBlendingAlphaFillColorLayer1,
        curbeData->DW56.ConstantBlendingAlphaFillColorLayer2,
        curbeData->DW56.ConstantBlendingAlphaFillColorLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW57.Value = %d", curbeData->DW57.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     ConstantBlendingAlphaFillColorLayer4 = %d, ConstantBlendingAlphaFillColorLayer5 = %d, ConstantBlendingAlphaFillColorLayer6 = %d, ConstantBlendingAlphaFillColorLayer7 = %d",
        curbeData->DW57.ConstantBlendingAlphaFillColorLayer4,
        curbeData->DW57.ConstantBlendingAlphaFillColorLayer5,
        curbeData->DW57.ConstantBlendingAlphaFillColorLayer6,
        curbeData->DW57.ConstantBlendingAlphaFillColorLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW58.Value = %d", curbeData->DW58.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     TwoLayerOperationLayer0 = %d, TwoLayerOperationLayer1 = %d, TwoLayerOperationLayer2 = %d, TwoLayerOperationLayer3 = %d",
        curbeData->DW58.TwoLayerOperationLayer0,
        curbeData->DW58.TwoLayerOperationLayer1,
        curbeData->DW58.TwoLayerOperationLayer2,
        curbeData->DW58.TwoLayerOperationLayer3);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW59.Value = %d", curbeData->DW59.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     TwoLayerOperationLayer4 = %d, TwoLayerOperationLayer5 = %d, TwoLayerOperationLayer6 = %d, TwoLayerOperationLayer7 = %d",
        curbeData->DW59.TwoLayerOperationLayer4,
        curbeData->DW59.TwoLayerOperationLayer5,
        curbeData->DW59.TwoLayerOperationLayer6,
        curbeData->DW59.TwoLayerOperationLayer7);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW60.Value = %d", curbeData->DW60.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     FixedPointFillColorGYChannel = %d, FixedPointFillColorRVChannel = %d",
        curbeData->DW60.FixedPointFillColorGYChannel,
        curbeData->DW60.FixedPointFillColorRVChannel);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW61.Value = %x", curbeData->DW61.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     FixedPointFillColorAlphaChannel = %d, FixedPointFillColorBUChannel = %d",
        curbeData->DW61.FixedPointFillColorAlphaChannel,
        curbeData->DW61.FixedPointFillColorBUChannel);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW62.Value = %d", curbeData->DW62.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     DestinationHeight = %d, DestinationWidth = %d",
        curbeData->DW62.DestinationHeight,
        curbeData->DW62.DestinationWidth);
    VP_RENDER_VERBOSEMESSAGE("CurbeData: DW63.Value = %d", curbeData->DW63.Value);
    VP_RENDER_VERBOSEMESSAGE("CurbeData:     ChannelSwapEnablingFlagDestination = %d, ChromaSittingLocationDestination = %d, DitherRoundEnablingFlagDestinationSurface = %d, DstCSCEnablingFlagDestination = %d, FormatDescriptorDestination = %d, Reserved = %d, TotalNumberInputLayers = %d",
        curbeData->DW63.ChannelSwapEnablingFlagDestination,
        curbeData->DW63.ChromaSittingLocationDestination,
        curbeData->DW63.DitherRoundEnablingFlagDestinationSurface,
        curbeData->DW63.DstCSCEnablingFlagDestination,
        curbeData->DW63.FormatDescriptorDestination,
        curbeData->DW63.Reserved,
        curbeData->DW63.TotalNumberInputLayers);
#endif
}