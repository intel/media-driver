/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_command_buffer.cpp
//! \brief     Contains Class CmCommandBuffer  definitions
//!

#include "cm_command_buffer.h"
#include "cm_ish.h"
#include "cm_ssh.h"
#include "cm_media_state.h"
#include "cm_thread_space_rt.h"
#include "cm_mem.h"
#include "cm_kernel_ex.h"
#include "cm_group_space.h"

#include "mhw_render_g12_X.h"
#include "mhw_render_g11_X.h"
#include "mhw_mi_g12_X.h"
#include "mhw_state_heap_hwcmd_g9_X.h" 

#include "mos_solo_generic.h"
#include "mhw_mmio_g9.h"

#include "cm_hal_g12.h"

CmCommandBuffer::CmCommandBuffer(CM_HAL_STATE *cmhal):
    m_cmhal(cmhal),
    m_osInterface(nullptr),
    m_miInterface(nullptr),
    m_hwRender(nullptr),
    m_ssh(nullptr),
    m_origRemain(0)
{
    MOS_ZeroMemory(&m_cmdBuf, sizeof(m_cmdBuf));
    MOS_ZeroMemory(m_masks, sizeof(m_masks));
}

CmCommandBuffer::~CmCommandBuffer()
{
    if (m_ssh)
    {
        MOS_Delete(m_ssh);
    }
}

MOS_STATUS CmCommandBuffer::Initialize()
{
    if (m_cmhal == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    m_osInterface = m_cmhal->osInterface;
    m_miInterface = m_cmhal->renderHal->pMhwMiInterface;
    m_hwRender = m_cmhal->renderHal->pMhwRenderInterface;
    if (m_osInterface == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    CM_CHK_MOSSTATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_cmdBuf, 0));

    m_cmdBuf.Attributes.bIsMdfLoad = true;
    m_origRemain = m_cmdBuf.iRemaining;

    return MOS_STATUS_SUCCESS;
}

CmSSH* CmCommandBuffer::GetSSH()
{
    if (m_ssh != nullptr)
    {
        return m_ssh;
    }
    m_ssh = MOS_New(CmSSH, m_cmhal, &m_cmdBuf);
    return m_ssh;
}

MOS_STATUS CmCommandBuffer::AddFlushCacheAndSyncTask(bool isRead,
                                                     bool rtCache,
                                                     MOS_RESOURCE *syncBuffer)
{
    MHW_PIPE_CONTROL_PARAMS pipeCtlParams;
    MOS_ZeroMemory(&pipeCtlParams, sizeof(pipeCtlParams));
    pipeCtlParams.presDest = syncBuffer;
    pipeCtlParams.bFlushRenderTargetCache = rtCache;
    pipeCtlParams.dwFlushMode = isRead ? MHW_FLUSH_READ_CACHE : MHW_FLUSH_WRITE_CACHE;
    pipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
    return m_miInterface->AddPipeControl(&m_cmdBuf, nullptr, &pipeCtlParams);
}

MOS_STATUS CmCommandBuffer::AddReadTimeStamp(MOS_RESOURCE *resource, uint32_t offset, bool isRead)
{
    MHW_PIPE_CONTROL_PARAMS pipeCtlParams;
    MOS_ZeroMemory(&pipeCtlParams, sizeof(pipeCtlParams));
    pipeCtlParams.bFlushRenderTargetCache = true;
    pipeCtlParams.presDest          = resource;
    pipeCtlParams.dwResourceOffset  = offset;
    pipeCtlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    pipeCtlParams.dwFlushMode       = isRead ? MHW_FLUSH_READ_CACHE : MHW_FLUSH_WRITE_CACHE;
    return m_miInterface->AddPipeControl(&m_cmdBuf, nullptr, &pipeCtlParams);
}

MOS_STATUS CmCommandBuffer::AddL3CacheConfig(L3ConfigRegisterValues *l3Values)
{
    if (m_cmhal->platform.eRenderCoreFamily <= IGFX_GEN10_CORE) //gen10-
    {
        MHW_RENDER_ENGINE_L3_CACHE_SETTINGS l3CacheSettting = {};
        if (l3Values->config_register3)
        {
            l3CacheSettting.dwCntlReg = l3Values->config_register3;
        }
        else
        {
            l3CacheSettting.dwCntlReg = 0x60000060;
        }
        CM_CHK_MOSSTATUS_RETURN(m_hwRender->EnableL3Caching(&l3CacheSettting));
        return m_hwRender->SetL3Cache(&m_cmdBuf);
    }
    else if (m_cmhal->platform.eRenderCoreFamily == IGFX_GEN11_CORE)
    {
        MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G11 l3CacheSettting = {};
        l3CacheSettting.dwTcCntlReg = l3Values->config_register1;
        l3CacheSettting.dwCntlReg = (l3Values->config_register0 == 0)?0xA0000420:l3Values->config_register0;
        CM_CHK_MOSSTATUS_RETURN(m_hwRender->EnableL3Caching(&l3CacheSettting));
        return m_hwRender->SetL3Cache(&m_cmdBuf);
    }
    else //gen12
    {
        MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12 l3CacheSettting = {};

        l3CacheSettting.dwAllocReg = (l3Values->config_register0 == 0)?
                                                m_cmhal->cmHalInterface->m_l3Plane[0].config_register0
                                               :l3Values->config_register0;
        l3CacheSettting.dwTcCntlReg = (l3Values->config_register1 == 0)?
                                                m_cmhal->cmHalInterface->m_l3Plane[0].config_register1
                                               :l3Values->config_register1;

        CM_CHK_MOSSTATUS_RETURN(m_hwRender->EnableL3Caching(&l3CacheSettting));
        return m_hwRender->SetL3Cache(&m_cmdBuf);
    }
}

MOS_STATUS CmCommandBuffer::AddPipelineSelect(bool gpgpu)
{
    return m_hwRender->AddPipelineSelectCmd(&m_cmdBuf, gpgpu);
}

MOS_STATUS CmCommandBuffer::AddStateBaseAddress(CmISH *ish, CmMediaState *mediaState)
{
    MHW_STATE_BASE_ADDR_PARAMS stateBaseAddressParams;
    MOS_ZeroMemory(&stateBaseAddressParams, sizeof(stateBaseAddressParams));

    MOS_RESOURCE *gshResource = mediaState->GetHeapResource();
    uint32_t gshSize = mediaState->GetHeapSize();
    MOS_RESOURCE *ishResource = ish->GetResource();
    uint32_t ishSize = ish->GetSize();

    stateBaseAddressParams.presGeneralState              = gshResource;
    stateBaseAddressParams.dwGeneralStateSize            = gshSize;
    stateBaseAddressParams.presDynamicState              = gshResource;
    stateBaseAddressParams.dwDynamicStateSize            = gshSize;
    stateBaseAddressParams.bDynamicStateRenderTarget     = false;
    stateBaseAddressParams.presIndirectObjectBuffer      = gshResource;
    stateBaseAddressParams.dwIndirectObjectBufferSize    = gshSize;
    stateBaseAddressParams.presInstructionBuffer         = ishResource;
    stateBaseAddressParams.dwInstructionBufferSize       = ishSize;

    uint32_t heapMocs = m_osInterface->pfnCachePolicyGetMemoryObject(MOS_CM_RESOURCE_USAGE_SurfaceState,
            m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
    stateBaseAddressParams.mocs4DynamicState = heapMocs;
    stateBaseAddressParams.mocs4GeneralState = heapMocs;
    stateBaseAddressParams.mocs4InstructionCache = heapMocs;
    stateBaseAddressParams.mocs4SurfaceState = heapMocs;
    stateBaseAddressParams.mocs4IndirectObjectBuffer = heapMocs;
    stateBaseAddressParams.mocs4StatelessDataport = heapMocs;

    return m_hwRender->AddStateBaseAddrCmd(&m_cmdBuf, &stateBaseAddressParams);
}

MOS_STATUS CmCommandBuffer::AddMediaVFE(CmMediaState *mediaState, bool fusedEuDispatch, CMRT_UMD::CmThreadSpaceRT **threadSpaces, uint32_t count)
{
    MHW_VFE_PARAMS vfeParams = {};
    MHW_VFE_PARAMS_G12 vfeParamsG12 = {};
    MHW_VFE_PARAMS *param = nullptr;
    if (m_cmhal->platform.eRenderCoreFamily <= IGFX_GEN11_CORE)
    {
        param = &vfeParams;
    }
    else
    {
        param = &vfeParamsG12;
        vfeParamsG12.bFusedEuDispatch = fusedEuDispatch;
    }
    MHW_RENDER_ENGINE_CAPS *hwCaps = m_hwRender->GetHwCaps();

    param->dwDebugCounterControl = MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING;
    param->dwNumberofURBEntries = 32;
    param->dwMaximumNumberofThreads = hwCaps->dwMaxThreads;
    param->dwCURBEAllocationSize = MOS_ROUNDUP_SHIFT(mediaState->GetCurbeSize(), 5) << 5;
    param->dwURBEntryAllocationSize = 1;
    param->dwPerThreadScratchSpace = 0;
    param->dwScratchSpaceBasePointer = mediaState->GetScratchSpaceOffset();

    uint32_t scratchSizePerThread = mediaState->GetScratchSizePerThread();
    if (scratchSizePerThread > 0)
    {
        scratchSizePerThread = scratchSizePerThread >> 9;
        int remain = scratchSizePerThread % 2;
        scratchSizePerThread = scratchSizePerThread / 2;
        int sizeParam = 0;
        while ((scratchSizePerThread / 2) && !remain)
        {
            sizeParam++;
            remain = scratchSizePerThread % 2;
            scratchSizePerThread = scratchSizePerThread / 2;
        }
        param->dwPerThreadScratchSpace = sizeParam;
    }

    if (threadSpaces != nullptr && m_cmhal->cmHalInterface->IsScoreboardParamNeeded())
    {
        bool globalSpace = (count == 0);
        uint32_t spaceCount = globalSpace ? 1 : count;

        uint8_t map[256] = {0}; // x*4+y as index, order in the global dependency vector as value (starting from 1)
        uint8_t index = 1;
        for (uint32_t i = 0; i < spaceCount; i ++)
        {
            if (threadSpaces[i] == nullptr)
            {
                continue;
            }
            CM_HAL_DEPENDENCY *dependency;
            threadSpaces[i]->GetDependency(dependency);
            for (uint32_t j = 0; j < dependency->count; j ++)
            {
                uint8_t depVec = (uint8_t)(dependency->deltaX[j]) * 16 + (uint8_t)(dependency->deltaY[j]);
                if (map[depVec] == 0)
                {
                    param->Scoreboard.ScoreboardDelta[index - 1].x = (uint8_t)(dependency->deltaX[j]);
                    param->Scoreboard.ScoreboardDelta[index - 1].y = (uint8_t)(dependency->deltaY[j]);
                    map[depVec] = index ++;
                    
                }
                m_masks[i] |= 1 << (map[depVec] - 1);
            }
        }

        if (globalSpace)
        {
            CmSafeMemSet(m_masks, m_masks[0], sizeof(m_masks));
        }

        param->Scoreboard.ScoreboardEnable = 1;
        param->Scoreboard.ScoreboardMask = (1 << (index-1)) - 1;
        param->Scoreboard.ScoreboardType = (param->Scoreboard.ScoreboardMask != 0);
    }
    else
    {
        param->Scoreboard.ScoreboardEnable = 1;
    }
  
    return m_hwRender->AddMediaVfeCmd(&m_cmdBuf, param);
    
}

MOS_STATUS CmCommandBuffer::AddCurbeLoad(CmMediaState *mediaState)
{
    MHW_CURBE_LOAD_PARAMS curbeLoadParams;
    MOS_ZeroMemory(&curbeLoadParams, sizeof(curbeLoadParams));

    uint32_t curbeSize = mediaState->GetCurbeSize();
    if (curbeSize > 0)
    {
        curbeLoadParams.pKernelState = nullptr;
        curbeLoadParams.bOldInterface = false;
        curbeLoadParams.dwCURBETotalDataLength  = curbeSize;
        curbeLoadParams.dwCURBEDataStartAddress = mediaState->GetCurbeOffset();

        return m_hwRender->AddMediaCurbeLoadCmd(&m_cmdBuf, &curbeLoadParams);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddMediaIDLoad(CmMediaState *mediaState)
{
    MHW_ID_LOAD_PARAMS idLoadParams;
    MOS_ZeroMemory(&idLoadParams, sizeof(idLoadParams));

    idLoadParams.dwInterfaceDescriptorStartOffset = mediaState->GetMediaIDOffset();
    idLoadParams.dwInterfaceDescriptorLength = mediaState->GetMediaIDSize();

    return m_hwRender->AddMediaIDLoadCmd(&m_cmdBuf, &idLoadParams);
}

MOS_STATUS CmCommandBuffer::AddSyncBetweenKernels()
{
    MHW_PIPE_CONTROL_PARAMS pipeCtlParams;
    MOS_ZeroMemory(&pipeCtlParams, sizeof(pipeCtlParams));
    pipeCtlParams.bInvalidateTextureCache = true;
    pipeCtlParams.bFlushRenderTargetCache = true;
    pipeCtlParams.dwFlushMode = MHW_FLUSH_CUSTOM;
    pipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
    return m_miInterface->AddPipeControl(&m_cmdBuf, nullptr, &pipeCtlParams);
}

MOS_STATUS CmCommandBuffer::AddMediaObjectWalker(CMRT_UMD::CmThreadSpaceRT *threadSpace, uint32_t mediaID)
{
    MHW_WALKER_PARAMS mediaWalkerParams;
    MOS_ZeroMemory(&mediaWalkerParams, sizeof(mediaWalkerParams));

    mediaWalkerParams.CmWalkerEnable = true;
    mediaWalkerParams.InterfaceDescriptorOffset = mediaID;

    mediaWalkerParams.InlineDataLength = 0;
    mediaWalkerParams.pInlineData = nullptr;

    uint32_t colorCountM1 = 0;
    CM_MW_GROUP_SELECT groupSelect = CM_MW_GROUP_NONE;

    CM_WALKING_PATTERN walkPattern = CM_WALK_DEFAULT;
    CM_DEPENDENCY_PATTERN dependencyPattern = CM_NONE_DEPENDENCY;
    uint32_t threadSpaceWidth = 1;
    uint32_t threadSpaceHeight = 1;

    if (threadSpace != nullptr)
    {
        threadSpace->GetColorCountMinusOne(colorCountM1);
        threadSpace->GetMediaWalkerGroupSelect(groupSelect);
        threadSpace->GetWalkingPattern(walkPattern);
        threadSpace->GetDependencyPatternType(dependencyPattern);
        threadSpace->GetThreadSpaceSize(threadSpaceWidth, threadSpaceHeight);
    }
    
    mediaWalkerParams.ColorCountMinusOne = colorCountM1;
    mediaWalkerParams.GroupIdLoopSelect = (uint32_t)groupSelect;
    
    uint32_t threadCount = threadSpaceWidth * threadSpaceHeight;
    switch (dependencyPattern)
    {
        case CM_NONE_DEPENDENCY:
            break;
        case CM_HORIZONTAL_WAVE:
            walkPattern = CM_WALK_HORIZONTAL;
            break;
        case CM_VERTICAL_WAVE:
            walkPattern = CM_WALK_VERTICAL;
            break;
        case CM_WAVEFRONT:
            walkPattern = CM_WALK_WAVEFRONT;
            break;
        case CM_WAVEFRONT26:
            walkPattern = CM_WALK_WAVEFRONT26;
            break;
        case CM_WAVEFRONT26X:
            if (threadSpaceWidth > 1)
            {
                walkPattern = CM_WALK_WAVEFRONT26X;
            }
            else
            {
                walkPattern = CM_WALK_DEFAULT;
            }
            break;
        case CM_WAVEFRONT26ZIG:
            if (threadSpaceWidth > 2)
            {
                walkPattern = CM_WALK_WAVEFRONT26ZIG;
            }
            else
            {
                walkPattern = CM_WALK_DEFAULT;
            }
            break;
        default:
            CM_ASSERTMESSAGE("Error: Invalid walking pattern.");
            walkPattern = CM_WALK_DEFAULT;
            break;
    }

    mediaWalkerParams.BlockResolution.x = threadSpaceWidth;
    mediaWalkerParams.BlockResolution.y = threadSpaceHeight;

    mediaWalkerParams.LocalStart.x = 0;
    mediaWalkerParams.LocalStart.y = 0;
    mediaWalkerParams.LocalEnd.x = 0;
    mediaWalkerParams.LocalEnd.y = 0;

    mediaWalkerParams.dwGlobalLoopExecCount = 1;
    mediaWalkerParams.MidLoopUnitX = 0;
    mediaWalkerParams.MidLoopUnitY = 0;
    mediaWalkerParams.MiddleLoopExtraSteps = 0;

    uint32_t adjHeight = ((threadSpaceHeight + 1) >> 1) << 1;
    uint32_t adjWidth = ((threadSpaceWidth + 1) >> 1) << 1;

    uint32_t maxThreadWidth = m_cmhal->cmHalInterface->GetMediaWalkerMaxThreadWidth();

    switch (walkPattern)
    {
        case CM_WALK_DEFAULT:
        case CM_WALK_HORIZONTAL:
            if (threadSpaceWidth == threadCount && threadSpaceHeight == 1)
            {
                mediaWalkerParams.BlockResolution.x = MOS_MIN(threadCount, maxThreadWidth);
                mediaWalkerParams.BlockResolution.y = 1 + threadCount / maxThreadWidth;
            }
            mediaWalkerParams.dwLocalLoopExecCount = mediaWalkerParams.BlockResolution.y - 1;

            mediaWalkerParams.LocalOutLoopStride.x = 0;
            mediaWalkerParams.LocalOutLoopStride.y = 1;
            mediaWalkerParams.LocalInnerLoopUnit.x = 1;
            mediaWalkerParams.LocalInnerLoopUnit.y = 0;

            mediaWalkerParams.LocalEnd.x = mediaWalkerParams.BlockResolution.x - 1;

            break;

        case CM_WALK_WAVEFRONT:
            mediaWalkerParams.dwLocalLoopExecCount = threadSpaceWidth + (threadSpaceHeight - 1) * 1 - 1;

            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
            mediaWalkerParams.LocalInnerLoopUnit.y = 1;
            break;

        case CM_WALK_WAVEFRONT26:
            mediaWalkerParams.dwLocalLoopExecCount = threadSpaceWidth + (threadSpaceHeight - 1) * 2 - 1;

            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
            mediaWalkerParams.LocalInnerLoopUnit.y = 1;
            break;

        case CM_WALK_WAVEFRONT26X:
        case CM_WALK_WAVEFRONT26XALT:
            mediaWalkerParams.dwLocalLoopExecCount = 0x7ff;
            mediaWalkerParams.dwGlobalLoopExecCount = 0;

            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
            mediaWalkerParams.LocalInnerLoopUnit.y = 2;

            mediaWalkerParams.MiddleLoopExtraSteps = 1;
            mediaWalkerParams.MidLoopUnitX = 0;
            mediaWalkerParams.MidLoopUnitY = 1;
            break;

        case CM_WALK_WAVEFRONT26ZIG:
            mediaWalkerParams.dwLocalLoopExecCount = 1;
            mediaWalkerParams.dwGlobalLoopExecCount = (adjHeight / 2 - 1) * 2 + (adjWidth / 2) - 1;

            mediaWalkerParams.LocalOutLoopStride.x = 0;
            mediaWalkerParams.LocalOutLoopStride.y = 1;
            mediaWalkerParams.LocalInnerLoopUnit.x = 1;
            mediaWalkerParams.LocalInnerLoopUnit.y = 0;

            mediaWalkerParams.BlockResolution.x = 2;
            mediaWalkerParams.BlockResolution.y = 2;

            mediaWalkerParams.LocalEnd.x = mediaWalkerParams.BlockResolution.x - 1;
            break;

        case CM_WALK_VERTICAL:
            mediaWalkerParams.dwLocalLoopExecCount = mediaWalkerParams.BlockResolution.x - 1;

            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0;
            mediaWalkerParams.LocalInnerLoopUnit.y = 1;

            mediaWalkerParams.LocalEnd.y = mediaWalkerParams.BlockResolution.y - 1;

            break;

        case CM_WALK_WAVEFRONT45D:
            mediaWalkerParams.dwLocalLoopExecCount = 0x7ff;
            mediaWalkerParams.dwGlobalLoopExecCount = 0x7ff;

            mediaWalkerParams.LocalStart.x = threadSpaceWidth;
            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
            mediaWalkerParams.LocalInnerLoopUnit.y = 1;
            break;

        case CM_WALK_WAVEFRONT45XD_2:
            mediaWalkerParams.dwLocalLoopExecCount = 0x7ff;
            mediaWalkerParams.dwGlobalLoopExecCount = 0x7ff;

            // Local
            mediaWalkerParams.LocalStart.x = threadSpaceWidth;
            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
            mediaWalkerParams.LocalInnerLoopUnit.y = 2;

            // Mid
            mediaWalkerParams.MiddleLoopExtraSteps = 1;
            mediaWalkerParams.MidLoopUnitX = 0;
            mediaWalkerParams.MidLoopUnitY = 1;

            break;

        case CM_WALK_WAVEFRONT26D:
            mediaWalkerParams.dwLocalLoopExecCount = 0x7ff;
            mediaWalkerParams.dwGlobalLoopExecCount = 0x7ff;

            mediaWalkerParams.LocalStart.x = threadSpaceWidth;
            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
            mediaWalkerParams.LocalInnerLoopUnit.y = 1;
            break;

        case CM_WALK_WAVEFRONT26XD:
            mediaWalkerParams.dwLocalLoopExecCount = 0x7ff;
            mediaWalkerParams.dwGlobalLoopExecCount = 0x7ff;

            // Local
            mediaWalkerParams.LocalStart.x = threadSpaceWidth;
            mediaWalkerParams.LocalOutLoopStride.x = 1;
            mediaWalkerParams.LocalOutLoopStride.y = 0;
            mediaWalkerParams.LocalInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
            mediaWalkerParams.LocalInnerLoopUnit.y = 2;

            // Mid
            mediaWalkerParams.MiddleLoopExtraSteps = 1;
            mediaWalkerParams.MidLoopUnitX = 0;
            mediaWalkerParams.MidLoopUnitY = 1;
            break;

        default:
            mediaWalkerParams.dwLocalLoopExecCount = MOS_MIN(threadCount, 0x3FF);

            mediaWalkerParams.LocalOutLoopStride.x = 0;
            mediaWalkerParams.LocalOutLoopStride.y = 1;
            mediaWalkerParams.LocalInnerLoopUnit.x = 1;
            mediaWalkerParams.LocalInnerLoopUnit.y = 0;
            break;
    }

    //Global loop parameters: execution count, resolution and strides
    //Since no global loop, global resolution equals block resolution.
    mediaWalkerParams.GlobalStart.x = 0;
    mediaWalkerParams.GlobalStart.y = 0;
    mediaWalkerParams.GlobalOutlerLoopStride.y = 0;

    if (walkPattern == CM_WALK_WAVEFRONT26ZIG)
    {
        mediaWalkerParams.GlobalResolution.x = threadSpaceWidth;
        mediaWalkerParams.GlobalResolution.y = threadSpaceHeight;
        mediaWalkerParams.GlobalOutlerLoopStride.x = 2;
        mediaWalkerParams.GlobalInnerLoopUnit.x = 0xFFFC;
        mediaWalkerParams.GlobalInnerLoopUnit.y = 2;
    }
    else
    {
        mediaWalkerParams.GlobalResolution.x = mediaWalkerParams.BlockResolution.x;
        mediaWalkerParams.GlobalResolution.y = mediaWalkerParams.BlockResolution.y;
        mediaWalkerParams.GlobalOutlerLoopStride.x = mediaWalkerParams.GlobalResolution.x;
        mediaWalkerParams.GlobalInnerLoopUnit.x = 0;
        mediaWalkerParams.GlobalInnerLoopUnit.y = mediaWalkerParams.GlobalResolution.y;
    }

    mediaWalkerParams.UseScoreboard = 1;
    mediaWalkerParams.ScoreboardMask = m_masks[mediaID];

    return m_hwRender->AddMediaObjectWalkerCmd(&m_cmdBuf, &mediaWalkerParams);
}

MOS_STATUS CmCommandBuffer::AddDummyVFE()
{
    // Add PipeControl to invalidate ISP and MediaState to avoid PageFault issue
    MHW_PIPE_CONTROL_PARAMS pipeControlParams;

    MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
    pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    pipeControlParams.bGenericMediaStateClear = true;
    pipeControlParams.bIndirectStatePointersDisable = true;
    pipeControlParams.bDisableCSStall = false;
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddPipeControl(&m_cmdBuf, nullptr, &pipeControlParams));

    if (MEDIA_IS_WA(m_cmhal->renderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
    {
        MHW_VFE_PARAMS vfeStateParams;

        MOS_ZeroMemory(&vfeStateParams, sizeof(vfeStateParams));
        vfeStateParams.dwNumberofURBEntries = 1;
        CM_CHK_MOSSTATUS_RETURN(m_hwRender->AddMediaVfeCmd(&m_cmdBuf, &vfeStateParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddBatchBufferEnd()
{
    return m_miInterface->AddMiBatchBufferEnd(&m_cmdBuf, nullptr);
}

MOS_STATUS CmCommandBuffer::AddMMCProlog()
{
    uint64_t auxTableBaseAddr = 0;
    
    auxTableBaseAddr = m_cmhal->osInterface->pfnGetAuxTableBaseAddr(m_cmhal->osInterface);

    if (auxTableBaseAddr)
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS lriParams;
        MOS_ZeroMemory(&lriParams, sizeof(MHW_MI_LOAD_REGISTER_IMM_PARAMS));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseLow;
        lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
        CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&m_cmdBuf, &lriParams));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseHigh;
        lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&m_cmdBuf, &lriParams));
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddProtectedProlog()
{
    return m_miInterface->AddProtectedProlog(&m_cmdBuf);
}

void CmCommandBuffer::ReturnUnusedBuffer()
{
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_cmdBuf, 0);
}

void CmCommandBuffer::ReturnWholeBuffer()
{
    int tmp = m_origRemain - m_cmdBuf.iRemaining;
    m_cmdBuf.iRemaining = m_origRemain;
    m_cmdBuf.iOffset -= tmp;
    m_cmdBuf.pCmdPtr = m_cmdBuf.pCmdBase + m_cmdBuf.iOffset/sizeof(uint32_t);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_cmdBuf, 0);
}

MOS_STATUS CmCommandBuffer::Submit()
{
    return m_osInterface->pfnSubmitCommandBuffer(m_osInterface,
        &m_cmdBuf,
        m_cmhal->nullHwRenderCm);
}

MOS_STATUS CmCommandBuffer::AddPreemptionConfig(bool isGpgpu)
{
    bool csrEnable = !m_cmhal->midThreadPreemptionDisabled;
    if (MEDIA_IS_SKU(m_cmhal->skuTable, FtrPerCtxtPreemptionGranularityControl))
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegImm;
        MOS_ZeroMemory(&loadRegImm, sizeof(MHW_MI_LOAD_REGISTER_IMM_PARAMS));

        loadRegImm.dwRegister = MHW_RENDER_ENGINE_PREEMPTION_CONTROL_OFFSET;

        // Same reg offset and value for gpgpu pipe and media pipe
        if (isGpgpu)
        {
            if (MEDIA_IS_SKU(m_cmhal->skuTable, FtrGpGpuMidThreadLevelPreempt))
            {
                if (csrEnable)
                {
                    loadRegImm.dwData = MHW_RENDER_ENGINE_MID_THREAD_PREEMPT_VALUE;
                }
                else
                {
                    loadRegImm.dwData = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
                }
            }
            else if (MEDIA_IS_SKU(m_cmhal->skuTable, FtrGpGpuThreadGroupLevelPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
            }
            else if (MEDIA_IS_SKU(m_cmhal->skuTable, FtrGpGpuMidBatchPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
            else
            {
                // if hit this branch then platform does not support any media preemption in render engine. Still program the register to avoid GPU hang
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
        }
        else
        {
            if ( MEDIA_IS_SKU(m_cmhal->skuTable, FtrMediaMidThreadLevelPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_THREAD_PREEMPT_VALUE;
            }
            else if ( MEDIA_IS_SKU(m_cmhal->skuTable, FtrMediaThreadGroupLevelPreempt) )
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
            }
            else if ( MEDIA_IS_SKU(m_cmhal->skuTable, FtrMediaMidBatchPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
            else
            {
                // if hit this branch then platform does not support any media preemption in render engine. Still program the register to avoid GPU hang
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
        }
        m_cmdBuf.Attributes.bMediaPreemptionEnabled = m_hwRender->IsPreemptionEnabled();
        CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&m_cmdBuf, &loadRegImm));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddSipState(uint32_t sipKernelOffset)
{
    if (m_cmhal->midThreadPreemptionDisabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    // Send CS_STALL pipe control
    //Insert a pipe control as synchronization
    MHW_PIPE_CONTROL_PARAMS pipeCtlParams;
    MOS_ZeroMemory(&pipeCtlParams, sizeof(MHW_PIPE_CONTROL_PARAMS));
    pipeCtlParams.presDest = &m_cmhal->renderTimeStampResource.osResource;
    pipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
    pipeCtlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    pipeCtlParams.bDisableCSStall = 0;
    pipeCtlParams.bFlushRenderTargetCache = true;
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddPipeControl(&m_cmdBuf, nullptr, &pipeCtlParams));

    MHW_SIP_STATE_PARAMS sipStateParams;
    MOS_ZeroMemory(&sipStateParams, sizeof(MHW_SIP_STATE_PARAMS));
    sipStateParams.bSipKernel = true;
    sipStateParams.dwSipBase = sipKernelOffset;

    CM_CHK_MOSSTATUS_RETURN(m_hwRender->AddSipStateCmd(&m_cmdBuf, &sipStateParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddCsrBaseAddress(MOS_RESOURCE *resource)
{
    if (m_cmhal->midThreadPreemptionDisabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    // Send csr base addr command
    CM_CHK_MOSSTATUS_RETURN(m_hwRender->AddGpgpuCsrBaseAddrCmd(&m_cmdBuf, resource));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddConditionalBatchBufferEnd(CM_HAL_CONDITIONAL_BB_END_INFO *cbbInfo)
{
    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS cbbParams;
    MOS_ZeroMemory(&cbbParams, sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

    cbbParams.presSemaphoreBuffer = &(m_cmhal->bufferTable[cbbInfo->bufferTableIndex].osResource);
    cbbParams.dwValue = cbbInfo->compareValue;
    cbbParams.bDisableCompareMask = cbbInfo->disableCompareMask;
    cbbParams.dwOffset = cbbInfo->offset;

    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(&m_cmdBuf, &cbbParams));

    return MOS_STATUS_SUCCESS;
}

template <typename T1, typename T2>
static inline T1 NonZeroMin(T1 a, T2 b)
{
    return (a==0)?b:MOS_MIN(a, b);
}

MOS_STATUS CmCommandBuffer::AddPowerOption(CM_POWER_OPTION *option)
{
    if (option == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }
    if (m_cmhal->cmHalInterface->IsOverridePowerOptionPerGpuContext())
    {
        return MOS_STATUS_SUCCESS;
    }

    MEDIA_FEATURE_TABLE *skuTable = m_cmhal->renderHal->pSkuTable;
    MEDIA_SYSTEM_INFO *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);

    // set command buffer attributes
    if (skuTable && 
        (MEDIA_IS_SKU(skuTable, FtrSSEUPowerGating)|| MEDIA_IS_SKU(skuTable, FtrSSEUPowerGatingControlByUMD)))
    {
        if ((option->nSlice || option->nSubSlice || option->nEU)
            && (gtSystemInfo->SliceCount && gtSystemInfo->SubSliceCount))
        {
            m_cmdBuf.Attributes.dwNumRequestedEUSlices = NonZeroMin(option->nSlice, gtSystemInfo->SliceCount);
            m_cmdBuf.Attributes.dwNumRequestedSubSlices = NonZeroMin(option->nSubSlice,
                                                                     (gtSystemInfo->SubSliceCount / gtSystemInfo->SliceCount));
            m_cmdBuf.Attributes.dwNumRequestedEUs = NonZeroMin(option->nEU,
                                                               (gtSystemInfo->EUCount / gtSystemInfo->SubSliceCount));
            m_cmdBuf.Attributes.bValidPowerGatingRequest = true;
            if (m_cmhal->platform.eRenderCoreFamily == IGFX_GEN12_CORE)
            {
                m_cmdBuf.Attributes.bUmdSSEUEnable = true;
            }
        }
        if (m_cmhal->requestSingleSlice)
        {
            m_cmdBuf.Attributes.dwNumRequestedEUSlices = 1;
        }

        if (GFX_IS_PRODUCT(m_cmhal->platform, IGFX_SKYLAKE) && m_osInterface->pfnSetSliceCount)
        {
            uint32_t sliceCount = m_cmdBuf.Attributes.dwNumRequestedEUSlices;
            m_osInterface->pfnSetSliceCount(m_osInterface, &sliceCount);
        }
    }

    // Add Load register command
    if(m_cmdBuf.Attributes.bUmdSSEUEnable)
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS MiLoadRegImmParams;
        MHW_RENDER_PWR_CLK_STATE_PARAMS params;

        MOS_ZeroMemory(&params, sizeof(params));
        params.PowerClkStateEn  = true;
        params.SCountEn         = true;
        params.SSCountEn        = true;
        params.SliceCount       = m_cmdBuf.Attributes.dwNumRequestedEUSlices;
        params.SubSliceCount    = m_cmdBuf.Attributes.dwNumRequestedSubSlices;
        params.EUmax            = m_cmdBuf.Attributes.dwNumRequestedEUs;
        params.EUmin            = m_cmdBuf.Attributes.dwNumRequestedEUs;

        MOS_ZeroMemory(&MiLoadRegImmParams, sizeof(MiLoadRegImmParams));
        MiLoadRegImmParams.dwRegister = MHW__PWR_CLK_STATE_REG;
        MiLoadRegImmParams.dwData = params.Data;
        CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
            &m_cmdBuf,
            &MiLoadRegImmParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddUmdProfilerStart()
{
    if (m_cmhal->perfProfiler != nullptr)
    {
        CM_CHK_MOSSTATUS_RETURN(m_cmhal->perfProfiler->AddPerfCollectStartCmd((void *)m_cmhal, m_osInterface, m_miInterface, &m_cmdBuf));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddUmdProfilerEnd()
{
    if (m_cmhal->perfProfiler != nullptr)
    {
        CM_CHK_MOSSTATUS_RETURN(m_cmhal->perfProfiler->AddPerfCollectEndCmd((void *)m_cmhal, m_osInterface, m_miInterface, &m_cmdBuf));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmCommandBuffer::AddGpgpuWalker(CMRT_UMD::CmThreadGroupSpace *threadGroupSpace,
                                                 CmKernelEx *kernel,
                                                 uint32_t mediaID)
{
    MHW_GPGPU_WALKER_PARAMS gpGpuWalkerParams;
    MOS_ZeroMemory(&gpGpuWalkerParams, sizeof(MHW_GPGPU_WALKER_PARAMS));
    gpGpuWalkerParams.InterfaceDescriptorOffset = mediaID;
    gpGpuWalkerParams.GpGpuEnable = true;

    threadGroupSpace->GetThreadGroupSpaceSize(gpGpuWalkerParams.ThreadWidth,
                                              gpGpuWalkerParams.ThreadHeight,
                                              gpGpuWalkerParams.ThreadDepth,
                                              gpGpuWalkerParams.GroupWidth,
                                              gpGpuWalkerParams.GroupHeight,
                                              gpGpuWalkerParams.GroupDepth);
    gpGpuWalkerParams.SLMSize = kernel->GetSLMSize();

    CM_CHK_MOSSTATUS_RETURN(m_hwRender->AddGpGpuWalkerStateCmd(&m_cmdBuf, &gpGpuWalkerParams));
    return MOS_STATUS_SUCCESS;
}

struct PACKET_SURFACE_STATE
{
    SURFACE_STATE_TOKEN_COMMON token;
    union
    {
        mhw_state_heap_g9_X::RENDER_SURFACE_STATE_CMD cmdSurfaceState;
        mhw_state_heap_g9_X::MEDIA_SURFACE_STATE_CMD cmdSurfaceStateAdv;
    };
};

void CmCommandBuffer::Dump()
{
#if MDF_COMMAND_BUFFER_DUMP
    if (m_cmhal->dumpCommandBuffer)
    {
        m_cmhal->pfnDumpCommadBuffer(
            m_cmhal,
            &m_cmdBuf,
            offsetof(PACKET_SURFACE_STATE, cmdSurfaceState),
            mhw_state_heap_g9_X::RENDER_SURFACE_STATE_CMD::byteSize);
    }
#endif
}
