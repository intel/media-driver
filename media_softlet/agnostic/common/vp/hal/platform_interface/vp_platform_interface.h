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

#include "hal_kerneldll.h"
#include "vp_feature_manager.h"
#include "vp_render_common.h"
#include "vp_kernel_config.h"

namespace vp
{
class VPFeatureManager;
class SfcRenderBase;
class VpKernelSet;

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

protected:
    // Compositing Kernel DLL/Search state
    const Kdll_RuleEntry        *m_kernelDllRules = nullptr;
    Kdll_State                  *m_kernelDllState = nullptr;
    // Compositing Kernel buffer and size
    const void                  *m_kernelBin = nullptr;
    uint32_t                    m_kernelBinSize = 0;
    // CM Kernel Execution Code Offset
    uint32_t                    m_kernelBinOffset = 0;
    // CM Kernel Arguments
    KERNEL_ARGS                 m_kernelArgs;
    std::string                 m_kernelName = {};
    // CM Compositing Kernel patch file buffer and size
    const void                  *m_fcPatchBin = nullptr;
    uint32_t                    m_fcPatchBinSize = 0;

public:
    const static std::string          s_kernelNameNonAdvKernels;

MEDIA_CLASS_DEFINE_END(VpRenderKernel)
};

using KERNEL_POOL = std::map<std::string, VpRenderKernel>;

class VpPlatformInterface
{
public:

    VpPlatformInterface(PMOS_INTERFACE pOsInterface);

    virtual ~VpPlatformInterface();

    virtual MOS_STATUS InitVpCmKernels(
        const uint32_t*       cisaCode,
        uint32_t              cisaCodeSize);

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
    virtual MOS_STATUS InitVpRenderHwCaps()
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
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

    virtual VpKernelConfig &GetKernelConfig() = 0;

    MOS_STATUS GetKernelParam(VpKernelID kernlId, RENDERHAL_KERNEL_PARAM &param);
    //only for get kernel binary in legacy path not being used in APO path.
    virtual MOS_STATUS GetKernelBinary(const void *&kernelBin, uint32_t &kernelSize, const void *&patchKernelBin, uint32_t &patchKernelSize);

    virtual MOS_STATUS ConfigVirtualEngine() = 0;

protected:
    PMOS_INTERFACE m_pOsInterface = nullptr;
    KERNEL_POOL    m_kernelPool;
    void (*m_modifyKdllFunctionPointers)(PKdll_State) = nullptr;
    bool m_sfc2PassScalingEnabled = false;
    bool m_sfc2PassScalingPerfMode = false;
    bool m_vpMmcDisabled = false;
    MediaUserSettingSharedPtr m_userSettingPtr  = nullptr;  //!< usersettingInstance

    MEDIA_CLASS_DEFINE_END(VpPlatformInterface)
};

}
#endif // !__VP_PLATFORM_INTERFACE_H__
