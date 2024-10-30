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
#ifndef __VP_RENDER_VEBOX_HDR_3DLUT_OCL_KERNEL_H__
#define __VP_RENDER_VEBOX_HDR_3DLUT_OCL_KERNEL_H__

#include "vp_render_cmd_packet.h"
#include "vp_render_vebox_hdr_3dlut_kernel.h"
#define VP_HDR_KERNEL_NAME_OCL_3DLUT "fillLutTable_3dlut"
namespace vp
{
//!
//! \brief    Tone Mapping Source Type, Please don't change the Enmu Value.
//! \details  These Enmu Values are passed to media kernel for further processing.
//!

class VpRenderHdr3DLutOclKernel : public VpRenderKernelObj
{
public:
    VpRenderHdr3DLutOclKernel(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator);
    virtual ~VpRenderHdr3DLutOclKernel();

    virtual MOS_STATUS Init(VpRenderKernel& kernel);
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
    virtual MOS_STATUS CpPrepareResources() override;
    virtual MOS_STATUS SetupStatelessBuffer() override;
    virtual MOS_STATUS SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1 = false);
    virtual MOS_STATUS SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext);
    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs) override;
    virtual void       DumpSurfaces();

    //kernel Arguments
    KERNEL_ARGS          m_kernelArgs  = {};
    KERNEL_WALKER_PARAMS m_walkerParam = {};
    uint8_t              m_inlineData[32] = {};

    float           m_ccmMatrix[VP_CCM_MATRIX_SIZE] = {0.0};
    uint32_t        m_maxDisplayLum         = 1000;         //!< Maximum Display Luminance
    uint32_t        m_maxContentLevelLum    = 4000;         //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE  m_hdrMode               = VPHAL_HDR_MODE_NONE;
    uint32_t        m_hdrLutSize            = LUT65_SEG_SIZE;
    void*           m_curbe                 = nullptr;
    uint32_t        m_curbeSize              = 0;
    KERNEL_BTIS                  m_kernelBtis                    = {};
    KRN_EXECUTE_ENV              m_kernelEnv                     = {};
    KERNEL_ARG_INDEX_SURFACE_MAP m_argIndexSurfMap  = {};

    MEDIA_CLASS_DEFINE_END(vp__VpRenderHdr3DLutOclKernel)
};

}  // namespace vp

#endif //__RENDER_VEBOX_HDR_3DLUT_OCL_KERNEL__