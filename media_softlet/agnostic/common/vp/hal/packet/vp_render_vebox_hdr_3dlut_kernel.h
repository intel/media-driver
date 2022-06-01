/*
* Copyright (c) 2022, Intel Corporation
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
#ifndef __VP_RENDER_VEBOX_HDR_3DLUT_H__
#define __VP_RENDER_VEBOX_HDR_3DLUT_H__

#include "vp_render_cmd_packet.h"
namespace vp
{
// Defined in "VEBOX HDR 3DLut Kernel"
struct VEBOX_HDR_3DLUT_STATIC_DATA
{
    //0 - GRF R1.0
    union
    {
        struct
        {
            uint32_t hdr3DLutSurface;  // HDR 3D Lut Surface
        };
        uint32_t Value;
    } DW00;

    //1 - GRF R1.1
    union
    {
        struct
        {
            uint32_t hdrCoefSurface;  // HDR Coef Surface
        };
        uint32_t Value;
    } DW01;

    //2 - GRF R1.2
    union
    {
        struct
        {
            uint16_t hdr3DLutSurfaceWidth   : 16;
            uint16_t hdr3DLutSurfaceHeight  : 16;
        };
        uint32_t Value;
    } DW03;
};
C_ASSERT(SIZE32(VEBOX_HDR_3DLUT_STATIC_DATA) == 3);

//!
//! \brief    Tone Mapping Source Type, Please don't change the Enmu Value.
//! \details  These Enmu Values are passed to media kernel for further processing.
//!
typedef enum _TONE_MAPPING_SOURCE_TYPE
{
    TONE_MAPPING_SOURCE_NONE            = 0,
    TONE_MAPPING_SOURCE_RGB             = 1,
    TONE_MAPPING_SOURCE_PSEUDO_Y_BT2020 = 2,
    TONE_MAPPING_SOURCE_FULL_Y_BT2020   = 3,
    TONE_MAPPING_SOURCE_FULL_Y_BT709    = 4,
    TONE_MAPPING_SOURCE_PSEUDO_Y_BT709  = 5
} TONE_MAPPING_SOURCE_TYPE;

//!
//! \brief    Tone Mapping Mode.
//! \details  These Enmu Values are passed to media kernel for further processing.
//!
typedef enum _TONE_MAPPING_MODE
{
    TONE_MAPPING_MODE_H2H = 0,
    TONE_MAPPING_MODE_H2S = 1
} TONE_MAPPING_MODE;

//!
//! \brief    OETF Type.
//! \details  These Enmu Values are passed to media kernel for further processing.
//!
typedef enum _OETF_CURVE_TYPE
{
    OETF_CURVE_SDR_709  = 0,
    OETF_CURVE_HDR_2084 = 1,
    OETF_SRGB           = 2
} OETF_CURVE_TYPE;

// VpRenderHdr3DLutKernel::InitCoefSurface need be updated if value of VP_CCM_MATRIX_SIZE
// being modified.
#define VP_CCM_MATRIX_SIZE  12

class VpRenderHdr3DLutKernel : public VpRenderKernelObj
{
public:
    VpRenderHdr3DLutKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator);
    virtual ~VpRenderHdr3DLutKernel();

    virtual MOS_STATUS Init(VpRenderKernel &kernel) override;
    virtual MOS_STATUS GetCurbeState(void *&curbe, uint32_t &curbeLength) override;

    virtual MOS_STATUS FreeCurbe(void*& curbe) override
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t   GetInlineDataSize() override
    {
        return 0;
    }

    virtual bool IsKernelCached() override
    {
        return true;
    }

    virtual MOS_STATUS GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData) override;
    virtual MOS_STATUS InitCoefSurface(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode);

protected:
    virtual MOS_STATUS SetupSurfaceState() override;
    virtual MOS_STATUS SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag) override;
    virtual MOS_STATUS SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext) override;
    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs) override;

    //kernel Arguments
    KERNEL_ARGS          m_kernelArgs  = {};
    KERNEL_WALKER_PARAMS m_walkerParam = {};

    VEBOX_HDR_3DLUT_STATIC_DATA m_curbe = {};

    float           m_ccmMatrix[VP_CCM_MATRIX_SIZE] = {0.0};
    uint32_t        m_maxDisplayLum         = 1000;         //!< Maximum Display Luminance
    uint32_t        m_maxContentLevelLum    = 4000;         //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE  m_hdrMode               = VPHAL_HDR_MODE_NONE;

    MEDIA_CLASS_DEFINE_END(vp__VpRenderHdr3DLutKernel)
};

}  // namespace vp

#endif //__VP_RENDER_VEBOX_HDR_3DLUT_H__