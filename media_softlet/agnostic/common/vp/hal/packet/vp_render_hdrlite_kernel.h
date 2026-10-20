#ifndef __VP_RENDER_HDRLITE_KERNEL_H__
#define __VP_RENDER_HDRLITE_KERNEL_H__

#include "vp_platform_interface.h"
#include "vp_render_kernel_obj.h"

namespace vp
{

class VpRenderHDRLITEKernel : public VpRenderKernelObj
{
public:
    VpRenderHDRLITEKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator);
    virtual ~VpRenderHDRLITEKernel();

    virtual MOS_STATUS Init(VpRenderKernel &kernel);
    virtual MOS_STATUS GetCurbeState(void *&curbe, uint32_t &curbeLength);
    virtual uint32_t   GetInlineDataSize() override
    {
        return 0;
    }
    virtual MOS_STATUS SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP &samplerStateGroup);
    virtual MOS_STATUS GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData);

    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs) override;
    virtual MOS_STATUS SetPerfTag() override;

    MOS_STATUS FreeCurbe(void *&curbe)
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    virtual MOS_STATUS SetupSurfaceState() override;
    virtual MOS_STATUS SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1 = false);
    virtual MOS_STATUS SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext);
    virtual MOS_STATUS GetKernelSurfaceParam(SURFACE_PARAMS &surfParam, KERNEL_SURFACE_STATE_PARAM &kernelSurfaceParam);

    PRENDERHAL_INTERFACE m_renderHal      = nullptr;

    //kernel Arguments
    KERNEL_INDEX_ARG_MAP         m_kernelArgs          = {};
    KERNEL_BTIS                  m_kernelBtis          = {};
    KRN_EXECUTE_ENV              m_kernelEnv           = {};
    KRN_PER_THREAD_ARG_INFO      m_kernelPerThreadArgInfo = {};
    KERNEL_WALKER_PARAMS         m_walkerParam         = {};
    void                        *m_curbe               = nullptr;
    uint32_t                     m_kernelIndex         = 0;
    int32_t                      m_linearSamplerIndex  = 1;
    int32_t                      m_nearestSamplerIndex = 0;
    std ::vector<uint8_t>        m_inlineData          = {};

MEDIA_CLASS_DEFINE_END(vp__VpRenderHDRLITEKernel)
};
}  // namespace vp
#endif  //__VP_RENDER_HDRLITE_KERNEL_H__
