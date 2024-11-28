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
#ifndef __VP_RENDER_KERNEL_OBJ_H__
#define __VP_RENDER_KERNEL_OBJ_H__

#include "vp_pipeline_common.h"
#include "sw_filter.h"
#include "media_render_cmd_packet.h"
#include "vp_platform_interface.h"
#include <vector>
#include <map>
#include <set>

class RenderCmdPacket;

namespace vp {

//!
//! \brief Secure Block Copy kernel inline data size
//!
#define SECURE_BLOCK_COPY_KERNEL_INLINE_SIZE    (1 * sizeof(uint32_t))
//!
//! \brief Secure Block Copy kernel width
//!
#define SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH     64

//!
//! \brief Secure Block Copy kernel block height
//!
#define SECURE_BLOCK_COPY_KERNEL_BLOCK_HEIGHT   24

#define KERNEL_BINARY_PADDING_SIZE CM_KERNEL_BINARY_PADDING_SIZE

typedef struct _KERNEL_SURFACE_STATE_PARAM
{
    struct {
        bool                           updatedSurfaceParams; // true if update format/width/height/pitch to renderSurface.OsSurface.
        MOS_FORMAT                     format;               // MOS_FORMAT for processing surfaces
        uint32_t                       width;
        uint32_t                       height;
        uint32_t                       pitch;

        uint32_t                       surface_offset;     // Offset to the origin of the surface, in bytes.
        MOS_TILE_TYPE                  tileType;
        bool                           bufferResource;
        bool                           bindedKernel;        // true if bind index is hardcoded by bindIndex.
        bool                           updatedRenderSurfaces; // true if renderSurfaceParams be used.
        RENDERHAL_SURFACE_STATE_PARAMS renderSurfaceParams;  // default can be skip. for future usages, if surface configed by kernel, use it directlly
    } surfaceOverwriteParams;

    bool                                isOutput;        // true for render target
    PRENDERHAL_SURFACE_STATE_ENTRY      *surfaceEntries;
    uint32_t                            *sizeOfSurfaceEntries;
    uint32_t                            iCapcityOfSurfaceEntry = 0;
    bool                                isBindlessSurface = false;
} KERNEL_SURFACE_STATE_PARAM;

typedef struct _KERNEL_TUNING_PARAMS
{
    uint32_t euThreadSchedulingMode;
} KERNEL_TUNING_PARAMS, *PKERNEL_TUNING_PARAMS;

using KERNEL_CONFIGS = std::map<VpKernelID, void *>; // Only for legacy/non-cm kernels
using KERNEL_ARGS = std::vector<KRN_ARG>;
using KERNEL_SAMPLER_STATE_GROUP = std::map<SamplerIndex, MHW_SAMPLER_STATE_PARAM>;
using KERNEL_SAMPLER_STATES = std::vector<MHW_SAMPLER_STATE_PARAM>;
using KERNEL_SAMPLER_INDEX = std::vector<SamplerIndex>;
using KERNEL_SURFACE_CONFIG = std::map<SurfaceType, KERNEL_SURFACE_STATE_PARAM>;
using KERNEL_SURFACE_BINDING_INDEX = std::map<SurfaceType, std::set<uint32_t>>;
using KERNEL_STATELESS_BUFF_CONFIG = std::map<SurfaceType, uint64_t>;
using KERNEL_BINDELESS_SURFACE = std::map<SurfaceType, std::set<uint32_t>>;
using KERNEL_BINDELESS_SAMPLER = std::map<uint32_t, uint32_t>;

typedef struct _KERNEL_PARAMS
{
    VpKernelID                   kernelId;
    KERNEL_ARGS                  kernelArgs;
    KERNEL_THREAD_SPACE          kernelThreadSpace;
    bool                         syncFlag;
    bool                         flushL1;
    KERNEL_TUNING_PARAMS         kernelTuningParams;
    KERNEL_ARG_INDEX_SURFACE_MAP kernelStatefulSurfaces;
} KERNEL_PARAMS;

struct MEDIA_OBJECT_KA2_INLINE_DATA
{
    // DWORD 0 - GRF R7.0
    union
    {
        // All
        struct
        {
            uint32_t       DestinationBlockHorizontalOrigin : 16;
            uint32_t       DestinationBlockVerticalOrigin : 16;
        };

        // Secure Block Copy
        struct
        {
            uint32_t       BlockHeight : 16;
            uint32_t       BufferOffset : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartRowOffset;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1 - GRF R7.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer0 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer0 : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalRows;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2 - GRF R7.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer1 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer1 : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartColumnOffset;
        };

        uint32_t       Value;
    } DW02;

    // DWORD 3 - GRF R7.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer2 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer2 : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalColumns;
        };

        uint32_t       Value;
    } DW03;

    // DWORD 4 - GRF R7.4
    union
    {
        // Sampler Load
        struct
        {
            float       VideoXScalingStep;
        };

        uint32_t       Value;
    } DW04;

    // DWORD 5 - GRF R7.5
    union
    {
        // NLAS
        struct
        {
            float       VideoStepDelta;
        };

        uint32_t       Value;
    } DW05;

    // DWORD 6 - GRF R7.6
    union
    {
        // AVScaling
        struct
        {
            uint32_t       VerticalBlockNumber : 17;
            uint32_t       AreaOfInterest : 1;
            uint32_t : 14;
        };

        uint32_t       Value;
    } DW06;

    // DWORD 7 - GRF R7.7
    union
    {
        // AVScaling
        struct
        {
            uint32_t       GroupIDNumber;
        };

        uint32_t       Value;
    } DW07;

    // DWORD 8 - GRF R8.0
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer3 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer3 : 16;
        };

        uint32_t       Value;
    } DW08;

    // DWORD 9 - GRF R8.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer4 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer4 : 16;
        };

        uint32_t       Value;
    } DW09;

    // DWORD 10 - GRF R8.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer5 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer5 : 16;
        };

        uint32_t       Value;
    } DW10;

    // DWORD 11 - GRF R8.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer6 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer6 : 16;
        };

        uint32_t       Value;
    } DW11;

    // DWORD 12 - GRF R8.4
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer7 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer7 : 16;
        };

        uint32_t       Value;
    } DW12;

    // DWORD 13 - GRF R8.5
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW13;

    // DWORD 14 - GRF R8.6
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW14;

    // DWORD 15 - GRF R8.7
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW15;
};

class VpRenderKernelObj
{
public:
    VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator);
    VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, std::string kernelName = "", PVpAllocator allocator = nullptr);
    virtual ~VpRenderKernelObj();

    // For Adv kernel
    // Kernel Specific, which will inplenment be each kernel
    // GetCurbeState should be called after UpdateCurbeBindingIndex for all processed surfaces being called
    virtual MOS_STATUS Init(VpRenderKernel& kernel);

    MOS_STATUS GetCurbeState(void *&curbe, uint32_t &curbeLength, uint32_t &curbeLengthAligned, RENDERHAL_KERNEL_PARAM kernelParam, uint32_t dwBlockAlign)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(GetCurbeState(curbe, curbeLength));
        VP_PUBLIC_CHK_STATUS_RETURN(GetAlignedLength(curbeLength, curbeLengthAligned, kernelParam, dwBlockAlign));
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t GetInlineDataSize() = 0;

    virtual uint32_t GetKernelIndex();

    VpKernelID GetKernelId()
    {
        return m_kernelId;
    }

    DelayLoadedKernelType GetKernelType()
    {
        return m_kernelType;
    }

    virtual bool IsKernelCached()
    {
        return false;
    }

    virtual Kdll_CacheEntry *GetCachedEntryForKernelLoad()
    {
        return nullptr;
    }

    virtual MOS_STATUS GetWalkerSetting(KERNEL_WALKER_PARAMS& walkerParam, KERNEL_PACKET_RENDER_DATA &renderData);

    virtual MOS_STATUS SetKernelConfigs(
        KERNEL_PARAMS& kernelParams,
        VP_SURFACE_GROUP& surfaces,
        KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup,
        KERNEL_CONFIGS& kernelConfigs,
        VP_PACKET_SHARED_CONTEXT* sharedContext);

    virtual MOS_STATUS GetScoreboardParams(PMHW_VFE_SCOREBOARD &scoreboardParams)
    {
        scoreboardParams = nullptr;
        return MOS_STATUS_SUCCESS;
    }

    virtual void DumpSurfaces()
    {
        return;
    }

    virtual void DumpSurface(VP_SURFACE *pSurface,PCCHAR fileName);

    // Kernel Common configs
    virtual MOS_STATUS GetKernelSettings(RENDERHAL_KERNEL_PARAM &settsings)
    {
        if (IsAdvKernel())
        {
            // For adv kernel, no need for kernel param.
            return MOS_STATUS_SUCCESS;
        }
        if (m_hwInterface && m_hwInterface->m_vpPlatformInterface)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(m_hwInterface->m_vpPlatformInterface->GetKernelParam(m_kernelId, settsings));
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            return MOS_STATUS_INVALID_HANDLE;
        }
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS GetKernelEntry(Kdll_CacheEntry &entry);

    virtual MOS_STATUS FreeCurbe(void*& curbe)
    {
        VP_FUNC_CALL();

        MOS_SafeFreeMemory(curbe);
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t GetKernelBinaryID();

    void* GetKernelBinary()
    {
        return m_kernelBinary;
    }

    KERNEL_SURFACE_CONFIG& GetKernelSurfaceConfig()
    {
        return m_surfaceState;
    }

    std::string& GetKernelName()
    {
        return m_kernelName;
    }

    MOS_STATUS UpdateCurbeBindingIndex(SurfaceType surface, uint32_t index)
    {
        // Surface Type is specified during one submission
        auto it = m_surfaceBindingIndex.find(surface);
        if (it != m_surfaceBindingIndex.end())
        {
            it->second.insert(index);
        }
        else
        {
            std::set<uint32_t> bindingMap;
            bindingMap.insert(index);
            m_surfaceBindingIndex.insert(std::make_pair(surface, bindingMap));
        }

        return MOS_STATUS_SUCCESS;
    }

    std::set<uint32_t>& GetSurfaceBindingIndex(SurfaceType surface)
    {
        auto it = m_surfaceBindingIndex.find(surface);

        if (it == m_surfaceBindingIndex.end())
        {
            VP_RENDER_ASSERTMESSAGE("No surface index created for current surface");
            std::set<uint32_t> bindingMap;
            it = m_surfaceBindingIndex.insert(std::make_pair(surface, bindingMap)).first;
        }
        return it->second;
    }

    MOS_STATUS InitKernel(void* binary, uint32_t size, KERNEL_CONFIGS& kernelConfigs,
                        VP_SURFACE_GROUP& surfacesGroup, VP_RENDER_CACHE_CNTL& surfMemCacheCtl);

    bool IsAdvKernel()
    {
        return m_isAdvKernel;
    }

    bool UseIndependentSamplerGroup()
    {
        return m_useIndependentSamplerGroup;
    }

    virtual MOS_STATUS SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup);

    virtual MOS_STATUS UpdateCompParams()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetCacheCntl(PVP_RENDER_CACHE_CNTL)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetPerfTag()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS InitRenderHalSurface(
        SurfaceType             type,
        VP_SURFACE              *surf,
        PRENDERHAL_SURFACE      renderHalSurface)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual void OcaDumpKernelInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext);

    virtual uint32_t GetEuThreadSchedulingMode()
    {
        // hw default mode
        return 0;
    }

    virtual MOS_STATUS InitRenderHalSurfaceCMF(MOS_SURFACE* src, PRENDERHAL_SURFACE renderHalSurface);

    virtual MOS_STATUS SetInlineDataParameter(KRN_ARG args, RENDERHAL_INTERFACE *renderhal);

    virtual MOS_STATUS UpdateBindlessSurfaceResource(SurfaceType surf, std::set<uint32_t> surfStateOffset)
    {
        if (surf != SurfaceTypeInvalid)
        {
            m_bindlessSurfaceArray.insert(std::make_pair(surf, surfStateOffset));
        }

        return MOS_STATUS_SUCCESS;
    }

    virtual std::map<uint32_t, uint32_t>& GetBindlessSamplers()
    {
        return m_bindlessSamperArray;
    }

    virtual MOS_STATUS InitBindlessResources()
    {
        m_bindlessSurfaceArray.clear();
        m_bindlessSamperArray.clear();
        return MOS_STATUS_SUCCESS;
    }

protected:

    virtual MOS_STATUS SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1 = false);

    virtual MOS_STATUS SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext);

    virtual MOS_STATUS SetKernelStatefulSurfaces(KERNEL_ARG_INDEX_SURFACE_MAP &statefulSurfaces);

    virtual MOS_STATUS SetupSurfaceState() = 0;

    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs);

    virtual MOS_STATUS SetProcessSurfaceGroup(VP_SURFACE_GROUP &surfaces);

    virtual MOS_STATUS CpPrepareResources();

    virtual MOS_STATUS SetupStatelessBuffer();

    virtual MOS_STATUS SetupStatelessBufferResource(SurfaceType surf, bool isWrite);

    virtual MOS_STATUS GetCurbeState(void *&curbe, uint32_t &curbeLength) = 0;

    virtual MOS_STATUS GetAlignedLength(uint32_t &curbeLength, uint32_t &curbeLengthAligned, RENDERHAL_KERNEL_PARAM kernelParam, uint32_t dwBlockAlign)
    {
        curbeLengthAligned = MOS_ALIGN_CEIL(curbeLength, dwBlockAlign);
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetTuningFlag(PKERNEL_TUNING_PARAMS tuningParams);

protected:

    VP_SURFACE_GROUP                                        *m_surfaceGroup = nullptr;  // input surface process surface groups
    PVP_MHWINTERFACE                                        m_hwInterface = nullptr;
    KERNEL_SURFACE_CONFIG                                   m_surfaceState;             // surfaces processed pool where the surface state will generated here, if KERNEL_SURFACE_STATE_PARAM 
    KERNEL_SURFACE_BINDING_INDEX                            m_surfaceBindingIndex;      // store the binding index for processed surface
    PVpAllocator                                            m_allocator = nullptr;
    MediaUserSettingSharedPtr                               m_userSettingPtr = nullptr;  // usersettingInstance
    KERNEL_STATELESS_BUFF_CONFIG                            m_statelessArray;
    KERNEL_BINDELESS_SURFACE                                m_bindlessSurfaceArray;
    KERNEL_BINDELESS_SAMPLER                                m_bindlessSamperArray;
    // kernel attribute 
    std::string                                             m_kernelName = "";
    void *                                                  m_kernelBinary = nullptr;
    uint32_t                                                m_kernelBinaryID = 0;
    uint32_t                                                m_kernelSize = 0;
    uint32_t                                                m_kernelPaddingSize = 0;
    VpKernelID                                              m_kernelId = kernelCombinedFc;
    DelayLoadedKernelType                                   m_kernelType     = KernelNone;
    KernelIndex                                             m_kernelIndex = 0;          // index of current kernel in KERNEL_PARAMS_LIST

    PKERNEL_TUNING_PARAMS                                   m_kernelTuningParams = nullptr;

    bool                                                    m_isAdvKernel = false;      // true mean multi kernel can be submitted in one workload.
    bool                                                    m_useIndependentSamplerGroup = false; //true means multi kernels has their own stand alone sampler states group. only can be true when m_isAdvKernel is true.

    std::shared_ptr<mhw::vebox::Itf>                        m_veboxItf = nullptr;
    std ::vector<MHW_INLINE_DATA_PARAMS>                    m_inlineDataParams = {};

MEDIA_CLASS_DEFINE_END(vp__VpRenderKernelObj)
};
}
#endif // __VP_RENDER_KERNEL_OBJ_H__
