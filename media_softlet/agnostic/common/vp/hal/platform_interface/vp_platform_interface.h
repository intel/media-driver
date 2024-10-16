/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_platform_interface.h
//! \brief    platform specific vp interfaces.
//!
#ifndef __VP_PLATFORM_INTERFACE_H__
#define __VP_PLATFORM_INTERFACE_H__

#include "hal_kerneldll_next.h"
#include "vp_feature_manager.h"
#include "vp_render_common.h"
#include "vp_kernel_config.h"
#include "media_copy.h"
#include "vp_frametracker.h"

namespace vp
{
class VPFeatureManager;
class SfcRenderBase;
class VpKernelSet;
typedef void (*DelayLoadedFunc)(vp::VpPlatformInterface &vpPlatformInterface);

//! Struct VP_FEATURE_SUPPORT_BITS
//! \brief define VP support feature bits
//!
typedef struct _VP_FEATURE_SUPPORT_BITS
{
    union
    {
        struct
        {
#ifdef _MEDIA_RESERVED
#include "vp_feature_support_bits_ext.h"
#else
            uint64_t vpFeatureSupportBitsReserved : 1;
#endif
        };
        uint64_t value = 0;
    };
} VP_FEATURE_SUPPORT_BITS;

C_ASSERT(sizeof(_VP_FEATURE_SUPPORT_BITS) == sizeof(uint64_t));

struct VP_KERNEL_BINARY_ENTRY
{
    const uint32_t        *kernelBin    = nullptr;
    uint32_t              kernelBinSize = 0;
    std::string           postfix       = "";
    DelayLoadedKernelType kernelType    = KernelNone;
    uint32_t              payloadOffset = 0;
};

struct VP_KERNEL_BINARY
{
    const uint32_t        *kernelBin           = nullptr;
    uint32_t              kernelBinSize        = 0;
    const uint32_t        *fcPatchKernelBin    = nullptr;
    uint32_t              fcPatchKernelBinSize = 0;
};

class VpRenderKernel
{
public:
    VpRenderKernel() {};
    virtual ~VpRenderKernel() {};

    MOS_STATUS InitVPKernel(
        const Kdll_RuleEntry* kernelRules,
        const uint32_t*       kernelBin,
        uint32_t              kernelSize,
        const uint32_t*       patchKernelBin,
        uint32_t              patchKernelSize,
        void(*ModifyFunctionPointers)(PKdll_State));

    MOS_STATUS Destroy();

    Kdll_State* GetKdllState()
    {
        return m_kernelDllState;
    }

    MOS_STATUS SetKernelName(std::string kernelname);

    std::string& GetKernelName()
    {
        return m_kernelName;
    }

    uint32_t& GetKernelSize()
    {
        return m_kernelBinSize;
    }

    uint32_t& GetKernelBinOffset()
    {
        return m_kernelBinOffset;
    }

    const void* GetKernelBinPointer()
    {
        return m_kernelBin;
    }

    MOS_STATUS SetKernelBinOffset(uint32_t offset);

    MOS_STATUS SetKernelBinSize(uint32_t size);

    MOS_STATUS SetKernelBinPointer(void* pBin);

    MOS_STATUS AddKernelArg(KRN_ARG& arg);

    KERNEL_ARGS GetKernelArgs()
    {
        return m_kernelArgs;
    }


    //for OCL use only
    uint32_t &GetCurbeSize()
    {
        return m_curbeSize;
    }

    MOS_STATUS SetKernelExeEnv(KRN_EXECUTE_ENV &exeEnv);

    MOS_STATUS SetKernelCurbeSize(uint32_t size);

    MOS_STATUS AddKernelBti(KRN_BTI &bti);

    KERNEL_BTIS GetKernelBtis()
    {
        return m_kernelBtis;
    }

    KRN_EXECUTE_ENV &GetKernelExeEnv()
    {
        return m_kernelExeEnv;
    }

protected:
    // Compositing Kernel DLL/Search state
    const Kdll_RuleEntry        *m_kernelDllRules = nullptr;
    Kdll_State                  *m_kernelDllState = nullptr;
    // Compositing Kernel buffer and size
    const void                  *m_kernelBin = nullptr;
    uint32_t                    m_kernelBinSize = 0;
    // CM Kernel Execution Code Offset
    uint32_t                    m_kernelBinOffset = 0;
    // CM Kernel Arguments or OCL Kernel Arguments
    KERNEL_ARGS                 m_kernelArgs;
    std::string                 m_kernelName = {};
    // CM Compositing Kernel patch file buffer and size
    const void                  *m_fcPatchBin = nullptr;
    uint32_t                    m_fcPatchBinSize = 0;

    //for OCL use only
    uint32_t m_curbeSize = 0;
    KERNEL_BTIS     m_kernelBtis;
    KRN_EXECUTE_ENV m_kernelExeEnv = {};

public:
    const static std::string          s_kernelNameNonAdvKernels;

MEDIA_CLASS_DEFINE_END(vp__VpRenderKernel)
};

using KERNEL_POOL = std::map<std::string, VpRenderKernel>;

class VpPlatformInterface
{
public:

    VpPlatformInterface(PMOS_INTERFACE pOsInterface, bool clearViewMode = false);

    virtual ~VpPlatformInterface();

    virtual MOS_STATUS InitVpCmKernels(
        const uint32_t*       cisaCode,
        uint32_t              cisaCodeSize,
        std::string           postfix = "",
        uint32_t              payloadOffset = CM_PAYLOAD_OFFSET);

    virtual void InitVpNativeAdvKernels(
        std::string kernelName,
        VP_KERNEL_BINARY_ENTRY kernelBinaryEntry);

    virtual MOS_STATUS InitVpHwCaps(VP_HW_CAPS &vpHwCaps)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(InitVpVeboxSfcHwCaps(vpHwCaps.m_veboxHwEntry, Format_Count, vpHwCaps.m_sfcHwEntry, Format_Count));
        VP_PUBLIC_CHK_STATUS_RETURN(InitVpRenderHwCaps());
        VP_PUBLIC_CHK_STATUS_RETURN(InitPolicyRules(vpHwCaps.m_rules));
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS InitVPFCKernels(
        const Kdll_RuleEntry* kernelRules,
        const uint32_t* kernelBin,
        uint32_t              kernelSize,
        const uint32_t* patchKernelBin,
        uint32_t              patchKernelSize,
        void (*ModifyFunctionPointers)(PKdll_State) = nullptr);

    virtual MOS_STATUS InitPolicyRules(VP_POLICY_RULES &rules);
    virtual MOS_STATUS InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount, VP_SFC_ENTRY_REC *sfcHwEntry, uint32_t sfcEntryCount)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
    virtual MOS_STATUS        InitVpRenderHwCaps();
    virtual VPFeatureManager *CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
    {
        return nullptr;
    }
    virtual VpCmdPacket *CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
    {
        return nullptr;
    }
    virtual VpCmdPacket *CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel)
    {
        return nullptr;
    }

    virtual MediaCopyBaseState* CreateMediaCopy()
    {
        return nullptr;
    }

    virtual MOS_STATUS CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    KERNEL_POOL& GetKernelPool()
    {
        return m_kernelPool;
    }

    PMOS_INTERFACE &GetOsInterface()
    {
        return m_pOsInterface;
    }

    virtual MOS_STATUS VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_TYPE QueryType,
        uint32_t* pQuery)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t VeboxQueryStaticSurfaceSize() = 0;

    virtual bool IsPlatformCompressionEnabled()
    {
        return !m_vpMmcDisabled;
    }

    virtual bool IsGpuContextCreatedInPipelineInit()
    {
        return true;
    }

    virtual MOS_STATUS GetInputFrameWidthHeightAlignUnit(
        PVP_MHWINTERFACE          pvpMhwInterface,
        uint32_t                 &widthAlignUnit,
        uint32_t                 &heightAlignUnit,
        bool                      bVdbox,
        CODECHAL_STANDARD         codecStandard,
        CodecDecodeJpegChromaType jpegChromaType);

    virtual bool IsVeboxScalabilityWith4KNotSupported(
        VP_MHWINTERFACE           vpMhwInterface);

    virtual MOS_STATUS GetVeboxHeapInfo(
        PVP_MHWINTERFACE          pvpMhwInterface,
        const MHW_VEBOX_HEAP    **ppVeboxHeap);

    inline void SetMhwSfcItf(std::shared_ptr<mhw::sfc::Itf> sfcItf)
    {
        m_sfcItf = sfcItf;
    }

    inline void SetMhwVeboxItf(std::shared_ptr<mhw::vebox::Itf> veboxItf)
    {
        m_veboxItf = veboxItf;
    }

    inline void SetMhwRenderItf(std::shared_ptr<mhw::render::Itf> renderItf)
    {
        m_renderItf = renderItf;
    }

    inline void SetMhwMiItf(std::shared_ptr<mhw::mi::Itf> miItf)
    {
        m_miItf = miItf;
    }

    inline std::shared_ptr<mhw::sfc::Itf> GetMhwSfcItf()
    {
        return m_sfcItf;
    }

    inline std::shared_ptr<mhw::vebox::Itf> GetMhwVeboxItf()
    {
        return m_veboxItf;
    }

    inline std::shared_ptr<mhw::render::Itf> GetMhwRenderItf()
    {
        return m_renderItf;
    }

    inline std::shared_ptr<mhw::mi::Itf> GetMhwMiItf()
    {
        return m_miItf;
    }

    virtual VpKernelConfig* GetKernelConfig()
    {
        return m_vpKernelConfig;
    }

    virtual MOS_STATUS SetKernelConfig(VpKernelConfig* vpKernelConfig)
    {
        m_vpKernelConfig = vpKernelConfig;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetKernelParam(VpKernelID kernlId, RENDERHAL_KERNEL_PARAM &param);

    void SetVpFCKernelBinary(
                const uint32_t   *kernelBin,
                uint32_t         kernelBinSize,
                const uint32_t   *fcPatchKernelBin,
                uint32_t         fcPatchKernelBinSize);

    virtual void AddVpIsaKernelEntryToList(
        const uint32_t       *kernelBin,
        uint32_t              kernelBinSize,
        std::string           postfix         = "",
        DelayLoadedKernelType delayKernelType = KernelNone,
        uint32_t              payloadOffset   = CM_PAYLOAD_OFFSET);

    virtual void AddVpNativeAdvKernelEntryToList(
                const uint32_t *kernelBin,
                uint32_t        kernelBinSize,
                std::string     kernelName);

    virtual void InitVpDelayedNativeAdvKernel(
        const uint32_t *kernelBin,
        uint32_t        kernelBinSize,
        std::string     kernelName);

    //for OCL kernel use only
    virtual void InitVpDelayedNativeAdvKernel(
        const uint32_t  *kernelBin,
        uint32_t         kernelBinSize,
        KRN_ARG         *kernelArgs,
        uint32_t         kernelArgSize,
        uint32_t         kernelCurbeSize,
        KRN_EXECUTE_ENV& kernelExeEnv,
        KRN_BTI         *kernelBtis,
        uint32_t         kernelBtiSize,
        std::string      kernelName);

    virtual void AddNativeAdvKernelToDelayedList(
        DelayLoadedKernelType kernelType,
        DelayLoadedFunc       func);

    //only for get kernel binary in legacy path not being used in APO path.
    virtual MOS_STATUS GetKernelBinary(const void *&kernelBin, uint32_t &kernelSize, const void *&patchKernelBin, uint32_t &patchKernelSize);

    virtual MOS_STATUS InitializeDelayedKernels(DelayLoadedKernelType type);

    virtual MOS_STATUS ConfigVirtualEngine() = 0;

    virtual MOS_STATUS ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface) = 0;

    virtual bool IsEufusionBypassWaEnabled()
    {
        return false;
    }

    virtual bool IsAdvanceNativeKernelSupported()
    {
        return true;
    }

    virtual bool IsRenderMMCLimitationCheckNeeded()
    {
        return false;
    }

    virtual bool IsDecompForInterlacedSurfWaEnabled()
    {
        return true;
    }

    virtual bool IsLegacyEuCountInUse()
    {
        return false;
    }

    bool IsRenderDisabled()
    {
        return m_isRenderDisabled;
    }

    void DisableRender();

    virtual int GetModelConfig(int eu, int width, int height, double fps)
    {
        return 0;
    };

    virtual MOS_STATUS InitFrameTracker()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual bool SupportOclFC()
    {
        return false;
    }

    virtual MOS_STATUS InitVpFeatureSupportBits()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual VP_FEATURE_SUPPORT_BITS& GetVpFeatureSupportBits()
    {
        return m_vpFeatureSupportBits;
    }

protected:
    PMOS_INTERFACE m_pOsInterface = nullptr;
    VP_KERNEL_BINARY m_vpKernelBinary = {};                 //!< vp kernels
    VpKernelConfig  *m_vpKernelConfig = nullptr;
    KERNEL_POOL    m_kernelPool;
    void (*m_modifyKdllFunctionPointers)(PKdll_State) = nullptr;
    bool m_sfc2PassScalingEnabled = false;
    bool m_sfc2PassScalingPerfMode = false;
    bool m_vpMmcDisabled = false;

    MediaUserSettingSharedPtr m_userSettingPtr  = nullptr;  //!< usersettingInstance
    std::shared_ptr<mhw::vebox::Itf>        m_veboxItf  = nullptr;
    std::shared_ptr<mhw::sfc::Itf>          m_sfcItf    = nullptr;
    std::shared_ptr<mhw::render::Itf>       m_renderItf = nullptr;
    std::shared_ptr<mhw::mi::Itf>           m_miItf     = nullptr;

    std::vector<VP_KERNEL_BINARY_ENTRY>    m_vpIsaKernelBinaryList;
    std::vector<VP_KERNEL_BINARY_ENTRY>    m_vpDelayLoadedBinaryList;
    std::map<DelayLoadedKernelType, bool>  m_vpDelayLoadedFeatureSet;
    std::map<std::string, VP_KERNEL_BINARY_ENTRY> m_vpNativeAdvKernelBinaryList;
    std::map<DelayLoadedKernelType, DelayLoadedFunc> m_vpDelayLoadedNativeFunctionSet;

    bool m_isRenderDisabled = false;
    VpFrameTracker *m_frameTracker     = nullptr;
    VP_FEATURE_SUPPORT_BITS m_vpFeatureSupportBits = {};
    MEDIA_CLASS_DEFINE_END(vp__VpPlatformInterface)
};

}
#endif // !__VP_PLATFORM_INTERFACE_H__
