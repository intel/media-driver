/*
* Copyright (c) 2021, Intel Corporation
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
#ifndef __VP_RENDER_VEBOX_UPDATE_CMD_PACKET_EXT_H__
#define __VP_RENDER_VEBOX_UPDATE_CMD_PACKET_EXT_H__

#include "vp_platform_interface.h"
#include "vp_render_kernel_obj.h"
#include "vp_render_fc_types.h"

namespace vp {

class VpRenderFcKernel : public VpRenderKernelObj
{
public:
    VpRenderFcKernel(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator);
    virtual ~VpRenderFcKernel()
    {
        MOS_Delete(m_fcParams);
    }

    virtual MOS_STATUS GetCurbeState(void*& curbe, uint32_t& curbeLength) override;

    virtual uint32_t GetInlineDataSize() override;

    virtual MOS_STATUS GetWalkerSetting(KERNEL_WALKER_PARAMS& walkerParam, KERNEL_PACKET_RENDER_DATA &renderData) override;

    virtual MOS_STATUS FreeCurbe(void*& curbe) override
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS GetKernelEntry(Kdll_CacheEntry &entry) override;
    // Should be called after VpRenderCmdPacket::SetupSurfaceState().
    virtual MOS_STATUS UpdateCompParams() override;

    virtual bool IsKernelCached() override
    {
        return true;
    }

    virtual Kdll_CacheEntry *GetCachedEntryForKernelLoad() override
    {
        return m_kernelEntry;
    }

    virtual MOS_STATUS SetCacheCntl(PVP_RENDER_CACHE_CNTL surfMemCacheCtl) override;

    virtual MOS_STATUS InitRenderHalSurface(
        SurfaceType             type,
        VP_SURFACE              *surf,
        PRENDERHAL_SURFACE      renderHalSurface) override;

    virtual void OcaDumpKernelInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext) override;

protected:
    virtual MOS_STATUS SetupSurfaceState() override;
    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs) override;
    virtual MOS_STATUS SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup) override;
    MOS_STATUS GetSamplerIndex(VPHAL_SCALING_MODE scalingMode, uint32_t yuvPlane, int32_t &samplerIndex, MHW_SAMPLER_TYPE &samplerType);
    MOS_STATUS SetSurfaceParams(KERNEL_SURFACE_STATE_PARAM &surfParam, VP_FC_LAYER &layer, bool is32MWColorFillKern);

    MOS_STATUS PrintSearchFilter(Kdll_FilterEntry *filter, int32_t filterSize);
    MOS_STATUS InitRenderHalSurface(VP_FC_LAYER *src, PRENDERHAL_SURFACE renderHalSurface);
    MOS_STATUS BuildFilter(
        VP_COMPOSITE_PARAMS             *compParams,
        PKdll_FilterEntry               pFilter,
        int32_t*                        piFilterSize);
    MOS_STATUS InitFcCurbeData();
    MOS_STATUS InitFcDpBasedCurbeData();
    VPHAL_CHROMA_SUBSAMPLING GetChromaSitting(VP_SURFACE &surf);
    MOS_STATUS InitCscInCurbeData();
    MOS_STATUS InitCscInDpCurbeData();
    MOS_STATUS InitColorFillInCurbeData();
    MOS_STATUS InitOutputFormatInCurbeData();

    MOS_STATUS InitLayerInCurbeData(VP_FC_LAYER *layer);
    bool IsEufusionBypassed();
    void PrintCurbeData(VP_FC_CURBE_DATA &curbeData);

    VP_EXECUTE_CAPS     m_executeCaps       = {};
    Kdll_FilterDesc     m_searchFilter      = {};
    Kdll_SearchState    m_kernelSearch      = {};
    Kdll_State          *m_kernelDllState   = nullptr; //!< Compositing Kernel DLL/Search state

    // Procamp
    int32_t                 m_maxProcampEntries = VP_MAX_PROCAMP;
    Kdll_Procamp            m_Procamp[VP_MAX_PROCAMP] = {};

    PRENDERHAL_INTERFACE    m_renderHal = nullptr;

    VP_FC_DP_BASED_CURBE_DATA m_curbeDataDp = {};
    VP_FC_CURBE_DATA        m_curbeData = {};
    Kdll_CacheEntry         *m_kernelEntry = nullptr;

    // CSC parameters
    VPHAL_COLOR_SAMPLE_8 m_srcColor = {};
    VPHAL_COLOR_SAMPLE_8 m_dstColor = {};
    MEDIA_CSPACE        m_srcCspace  = CSpace_None;
    MEDIA_CSPACE        m_dstCspace  = CSpace_None;

    PRENDER_FC_PARAMS   m_fcParams = nullptr;

    bool                m_cscCoeffPatchModeEnabled = false;      //!< Set CSC Coeff using patch mode
    bool                m_computeWalkerEnabled = false;

    // Cache attributes
    VPHAL_COMPOSITE_CACHE_CNTL m_surfMemCacheCtl = {};

    static const int32_t s_bindingTableIndex[];
    static const int32_t s_bindingTableIndexField[];

MEDIA_CLASS_DEFINE_END(vp__VpRenderFcKernel)
};
}


#endif // __VP_RENDER_VEBOX_UPDATE_CMD_PACKET_EXT_H__