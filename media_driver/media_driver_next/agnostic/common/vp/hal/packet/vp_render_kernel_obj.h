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
#ifndef __VP_RENDER_KERNEL_OBJ_H__
#define __VP_RENDER_KERNEL_OBJ_H__

#include "vp_pipeline_common.h"
#include "sw_filter.h"
#include "media_render_cmd_packet.h"
#include "vp_platform_interface.h"
#include <vector>
#include <map>

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

enum KernelId
{
    Kernel_Invalidate = 0,
    Kernel_FastComposition,
    Kernel_Max
};

typedef struct _KERNEL_SURFACE_STATE_PARAM
{
    struct {
        bool                           updatedSurfaceParams; // true if using included surface params
        MOS_FORMAT                     format;               // MOS_FORMAT for processing surfaces
        uint32_t                       width;
        uint32_t                       height;
        uint32_t                       pitch;
        uint32_t                       surface_offset;     // Offset to the origin of the surface, in bytes.
        MOS_TILE_TYPE                  tileType;
        bool                           bufferResource;
        bool                           bindedKernel;
        uint32_t                       bindIndex;
        bool                           updatedRenderSurfaces;
        RENDERHAL_SURFACE_STATE_PARAMS renderSurfaceParams;  // default can be skip. for future usages, if surface configed by kernel, use it directlly
    } surfaceOverwriteParams;
    bool       renderTarget;        // true for render target
    uint32_t   reserved[2]; // for future usage
} KERNEL_SURFACE_STATE_PARAM;

using KERNEL_ARGS = std::vector<KRN_ARG>;
using KERNEL_CONFIGS = std::map<KernelIndex, void*>;
using KERNEL_SAMPLER_STATE_GROUP = std::map<SamplerIndex, MHW_SAMPLER_STATE_PARAM>;
using KERNEL_SAMPLER_STATES = std::vector<MHW_SAMPLER_STATE_PARAM>;
using KERNEL_SAMPLER_INDEX = std::vector<SamplerIndex>;
using KERNEL_SURFACE_CONFIG = std::map<SurfaceIndex, KERNEL_SURFACE_STATE_PARAM>;
using KERNEL_SURFACE_BINDING_INDEX = std::map<SurfaceIndex, uint32_t>;

typedef struct _KERNEL_PARAMS
{
    KernelIndex          kernelId;
    KERNEL_ARGS          kernelArgs;
    KERNEL_THREAD_SPACE  kernelThreadSpace;
    KERNEL_SAMPLER_INDEX kernelSamplerIndex;
    bool                 syncFlag;
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
    VpRenderKernelObj(PVP_MHWINTERFACE hwInterface);
    VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, uint32_t kernelID, uint32_t kernelIndex);
    virtual ~VpRenderKernelObj();

    // Kernel Specific, which will inplenment be each kernel
    // GetCurbeState should be called after UpdateCurbeBindingIndex for all processed surfaces being called
    virtual MOS_STATUS Init(VpRenderKernel& kernel);

    virtual MOS_STATUS GetCurbeState(void*& curbe, uint32_t& curbeLength);

    virtual MOS_STATUS GetInlineState(void** inlineData, uint32_t& inlineLength);

    virtual uint32_t GetKernelIndex();

    virtual KERNEL_WALKER_PARAMS& GetWalkerSetting();

    virtual MOS_STATUS SetKernelConfigs(
        KERNEL_CONFIGS& kernelConfigs,
        KERNEL_PARAMS& kernelParams,
        VP_SURFACE_GROUP& surfaces,
        KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup);

    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs, uint32_t kernelExecuteID);

    virtual KERNEL_SAMPLER_STATES& GetSamplerStates();

    // Kernel Common configs
    virtual MOS_STATUS GetKernelSettings(RENDERHAL_KERNEL_PARAM &settsings, uint32_t executeKernelID)
    {
        MOS_ZeroMemory(&settsings, sizeof(RENDERHAL_KERNEL_PARAM));

        if (m_hwInterface && m_hwInterface->m_vpPlatformInterface)
        {
            // adding when insert new kernels
            if (executeKernelID >= VeboxSecureBlockCopy && executeKernelID < VeboxKernelMax)
            {
                settsings = m_hwInterface->m_vpPlatformInterface->GetVeboxKernelSettings(executeKernelID - VeboxSecureBlockCopy);
            }
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            return MOS_STATUS_INVALID_HANDLE;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetKernelID(uint32_t kid);

    MOS_STATUS SetKernelIndex(uint32_t kid);

    virtual uint32_t GetKernelID();

    void* GetKernelBinary()
    {
        return m_kernelBinary;
    }

    uint32_t GetKernelSize()
    {
        return m_kernelSize;
    }

    KERNEL_SURFACE_CONFIG& GetKernelSurfaceConfig()
    {
        return m_surfaceState;
    }

    std::string& GetKernelName()
    {
        return m_kernelName;
    }

    MOS_STATUS UpdateCurbeBindingIndex(SurfaceIndex surface, uint32_t index)
    {
        // Surface Type is sepsrated during one submission
        m_surfaceBindingIndex.insert(std::make_pair(surface, index));

        return MOS_STATUS_SUCCESS;
    }

    uint32_t GetSurfaceBindingIndex(SurfaceIndex surface)
    {
        auto it = m_surfaceBindingIndex.find(surface);

        if (it != m_surfaceBindingIndex.end())
        {
            return it->second;
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("No surface index created for current surface");
            return 0;
        }
    }

    MOS_STATUS InitKernel(void* binary, uint32_t size)
    {
        VP_RENDER_CHK_NULL_RETURN(binary);
        m_kernelBinary = binary;
        m_kernelSize = size;
        return MOS_STATUS_SUCCESS;
    }

protected:

    virtual MOS_STATUS SetWalkerSetting(KERNEL_THREAD_SPACE& threadSpace, bool bSyncFlag);

    virtual MOS_STATUS SetKernelArgs(KERNEL_ARGS& kernelArgs);

    virtual MOS_STATUS SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup, KERNEL_SAMPLER_INDEX &kernelSamplerIndex);

    virtual MOS_STATUS SetupSurfaceState();

    MOS_STATUS SetProcessSurfaceGroup(VP_SURFACE_GROUP& surfaces)
    {
        m_surfaceGroup = &surfaces;
        VP_RENDER_CHK_STATUS_RETURN(SetupSurfaceState());
        return MOS_STATUS_SUCCESS;
    }

protected:

    VP_SURFACE_GROUP                                        *m_surfaceGroup = nullptr;   // input surface process surface groups
    PVP_MHWINTERFACE                                        m_hwInterface = nullptr;
    KERNEL_SURFACE_CONFIG                                   m_surfaceState;             // surfaces processed pool where the surface state will generated here, if KERNEL_SURFACE_STATE_PARAM 
    KERNEL_SURFACE_BINDING_INDEX                            m_surfaceBindingIndex;      // store the binding index for processed surface

    // kernel attribute 
    std::string                                             m_kernelName;
    void *                                                  m_kernelBinary = nullptr;
    uint32_t                                                m_kernelSize = 0;
    uint32_t                                                m_kernelID = 0;
    uint32_t                                                m_kernelIndex = 0;

    //kernel Arguments
    KERNEL_ARGS                                             m_kernelArgs;
    KERNEL_SAMPLER_STATES                                   m_samplerStates;
    KERNEL_WALKER_PARAMS                                    m_walkerParam;

    static MEDIA_OBJECT_KA2_INLINE_DATA                     g_cInit_VP_MEDIA_OBJECT_KA2_INLINE_DATA;

};
}
#endif // __VP_RENDER_KERNEL_OBJ_H__
