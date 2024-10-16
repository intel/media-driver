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
#ifndef __VP_RENDER_CMD_PACKET_H__
#define __VP_RENDER_CMD_PACKET_H__
#include "media_render_cmd_packet.h"
#include "vp_allocator.h"
#include "vp_cmd_packet.h"
#include "vp_kernelset.h"
#include "vp_render_common.h"
#include "vp_render_kernel_obj.h"
#include "mhw_mi_itf.h"

namespace vp
{

class VpRenderCmdPacket : virtual public RenderCmdPacket, virtual public VpCmdPacket
{
public:
    VpRenderCmdPacket(MediaTask* task, PVP_MHWINTERFACE hwInterface, PVpAllocator& allocator, VPMediaMemComp* mmc, VpKernelSet* kernelSet);

    virtual ~VpRenderCmdPacket();

    MOS_STATUS Prepare() override;

    MOS_STATUS Init() override;

    MOS_STATUS Destroy() override
    {
        return RenderCmdPacket::Destroy();
    }

    MOS_STATUS SetEuThreadSchedulingMode(uint32_t mode);

    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

    virtual MOS_STATUS SubmitWithMultiKernel(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket);

    MOS_STATUS PacketInit(
        VP_SURFACE* inputSurface,
        VP_SURFACE* outputSurface,
        VP_SURFACE* previousSurface,
        VP_SURFACE_SETTING& surfSetting,
        VP_EXECUTE_CAPS packetCaps) override;

    virtual MOS_STATUS SetFcParams(PRENDER_FC_PARAMS params);

    virtual MOS_STATUS SetHdr3DLutParams(PRENDER_HDR_3DLUT_CAL_PARAMS params);

    virtual MOS_STATUS SetDnHVSParams(PRENDER_DN_HVS_CAL_PARAMS params);

    virtual MOS_STATUS SetOclFcParams(PRENDER_OCL_FC_PARAMS params);

    virtual MOS_STATUS DumpOutput() override;

    void PrintWalkerParas(MHW_WALKER_PARAMS &WalkerParams);
    void PrintWalkerParas(MHW_GPGPU_WALKER_PARAMS &WalkerParams);

    virtual MOS_STATUS SetHdrParams(PRENDER_HDR_PARAMS params);
    virtual bool IsRenderUncompressedWriteNeeded(PVP_SURFACE VpSurface);

    void RenderMMCLimitationCheck(VP_SURFACE *vpSurface, RENDERHAL_SURFACE_NEXT &renderHalSurface, SurfaceType type);

protected:

    virtual MOS_STATUS LoadKernel() override;

    MOS_STATUS KernelStateSetup();

    virtual MOS_STATUS SetupSamplerStates();

    virtual MOS_STATUS SetupSurfaceState();

    virtual MOS_STATUS SetupCurbeState();

    virtual VP_SURFACE* GetSurface(SurfaceType type);

    virtual MOS_STATUS SetupWalkerParams();

    virtual MOS_STATUS SetupMediaWalker() override;

    virtual void UpdateKernelConfigParam(RENDERHAL_KERNEL_PARAM &kernelParam) override;

    virtual void OcaDumpDbgInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext) override;

    virtual MOS_STATUS SetMediaFrameTracking(RENDERHAL_GENERIC_PROLOG_PARAMS &genericPrologParams) override;

    MOS_STATUS SendMediaStates(PRENDERHAL_INTERFACE pRenderHal, PMOS_COMMAND_BUFFER pCmdBuffer);

    MOS_STATUS InitRenderHalSurface(
        VP_SURFACE         &surface,
        RENDERHAL_SURFACE  &renderSurface);

    MOS_STATUS InitStateHeapSurface(
        SurfaceType        type,
        RENDERHAL_SURFACE& renderSurface);

    // comments here: Hight overwite params if needed
    MOS_STATUS UpdateRenderSurface(RENDERHAL_SURFACE_NEXT &renderSurface, KERNEL_SURFACE_STATE_PARAM& kernelParams);

   MOS_STATUS SamplerAvsCalcScalingTable(
        MHW_AVS_PARAMS& avsParameters,
        MOS_FORMAT      SrcFormat,
        bool            bVertical,
        float           fLumaScale,
        float           fChromaScale,
        uint32_t        dwChromaSiting,
        bool            b8TapAdaptiveEnable);

    MOS_STATUS SetNearestModeTable(
        int32_t   * iCoefs,
        uint32_t dwPlane,
        bool     bBalancedFilter);

     MOS_STATUS CalcPolyphaseTablesUV(
        int32_t * piCoefs,
        float fLanczosT,
        float fInverseScaleFactor);

    MOS_STATUS CalcPolyphaseTablesY(
        int32_t     *iCoefs,
        float      fScaleFactor,
        uint32_t   dwPlane,
        MOS_FORMAT srcFmt,
        float      fHPStrength,
        bool       bUse8x8Filter,
        uint32_t   dwHwPhase);

    MOS_STATUS CalcPolyphaseTablesUVOffset(
        int32_t* piCoefs,
        float   fLanczosT,
        float   fInverseScaleFactor,
        int32_t iUvPhaseOffset);

    virtual MOS_STATUS InitFcMemCacheControlForTarget(PVP_RENDER_CACHE_CNTL settings);
    virtual MOS_STATUS InitFcMemCacheControl(PVP_RENDER_CACHE_CNTL settings);
    MOS_STATUS InitSurfMemCacheControl(VP_EXECUTE_CAPS packetCaps);

    MHW_SETPAR_DECL_HDR(PIPE_CONTROL);

protected:

    KERNEL_OBJECTS                     m_kernelObjs;
    // Only for MULTI_KERNELS_WITH_ONE_MEDIA_STATE case.
    KERNEL_RENDER_DATA                 m_kernelRenderData;

    KERNEL_CONFIGS                     m_kernelConfigs; // Kernel parameters for legacy kernels.

    bool                               m_firstFrame = true;
    VpKernelSet                       *m_kernelSet = nullptr;
    VpRenderKernelObj                 *m_kernel    = nullptr; // processing kernel pointer

    KERNEL_PARAMS_LIST                 m_renderKernelParams;
    KERNEL_SAMPLER_STATE_GROUP         m_kernelSamplerStateGroup;

    KERNEL_SUBMISSION_MODE             m_submissionMode   = SINGLE_KERNEL_ONLY;
    uint32_t                           m_slmSize          = 0;
    uint32_t                           m_totalCurbeSize   = 0;
    uint32_t                           m_totoalInlineSize = 0;

    VP_SURFACE                        *m_currentSurface  = nullptr;              //!< Current frame
    PVP_RENDER_CACHE_CNTL              m_surfMemCacheCtl = nullptr;              //!< Surface memory cache control
    vp::VpUserFeatureControl          *m_vpUserFeatureControl     = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpRenderCmdPacket)
};
}

#endif
