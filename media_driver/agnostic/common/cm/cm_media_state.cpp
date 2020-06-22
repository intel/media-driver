/*
* Copyright (c) 2018, Intel Corporation
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
//! \file      cm_ssh.cpp 
//! \brief     Contains Class CmSSH  definitions 
//!

#include "cm_media_state.h"
#include "cm_kernel_ex.h"
#include <string>
#include <iostream>
#include <sstream>

using namespace CMRT_UMD;
using namespace std;

CmMediaState::CmMediaState(CM_HAL_STATE *cmhal):
    m_cmhal(cmhal),
    m_heapMgr(nullptr),
    m_curbeOffsetInternal(0),
    m_mediaIDOffsetInternal(0),
    m_samplerHeapOffsetInternal(0),
    m_scratchSpaceOffsetExternal(0),
    m_totalCurbeSize(0),
    m_totalMediaIDSize(0),
    m_totalSamplerHeapSize(0),
    m_totalScratchSpaceSize(0),
    m_mediaIDSize(0),
    m_scratchSizePerThread(0),
    m_state(_Empty)
{
}

CmMediaState::~CmMediaState()
{
    if (m_state == _Allocated)
    {
        Submit();
    }
}

MOS_STATUS CmMediaState::Initialize(HeapManager *heapMgr)
{
    if (heapMgr == nullptr || m_cmhal == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    m_heapMgr = heapMgr;

    m_mediaIDSize = m_cmhal->renderHal->pHwSizes->dwSizeInterfaceDescriptor;

    MOS_ZeroMemory(m_curbeOffsets, sizeof(m_curbeOffsets));
    MOS_ZeroMemory(m_samplerOffsets, sizeof(m_samplerOffsets));
    MOS_ZeroMemory(m_next3dSamplerOffsets, sizeof(m_next3dSamplerOffsets));
    MOS_ZeroMemory(m_nextAvsSamplerOffsets, sizeof(m_nextAvsSamplerOffsets));
    MOS_ZeroMemory(m_nextIndStateOffsets, sizeof(m_nextIndStateOffsets));
    MOS_ZeroMemory(m_samplerCount, sizeof(m_samplerCount));
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmMediaState::Submit()
{
    std::vector<MemoryBlock> blocks;
    blocks.push_back(m_memoryBlock);
    CM_CHK_MOSSTATUS_RETURN(m_heapMgr->SubmitBlocks(blocks));
    m_state = _Submitted;

    return MOS_STATUS_SUCCESS;
}

#if defined(ANDROID) || defined(LINUX)
#define PLATFORM_DIR_SEPERATOR   "/"
#else
#define PLATFORM_DIR_SEPERATOR   "\\"
#endif

void CmMediaState::Dump()
{
#if MDF_CURBE_DATA_DUMP
    if (m_cmhal->dumpCurbeData)
    {
        char curbeFileNamePrefix[MAX_PATH];
        char idFileNamePrefix[MAX_PATH];
        static int fileCount = 0;
        stringstream curbeFilename;
        curbeFilename << "HALCM_Curbe_Data_Dumps" << PLATFORM_DIR_SEPERATOR << "curbe_" << fileCount << ".fast.log";
        stringstream idFilename;
        idFilename << "HALCM_Curbe_Data_Dumps" << PLATFORM_DIR_SEPERATOR << "id_" << fileCount << ".fast.log";

        ++fileCount;

        GetLogFileLocation(curbeFilename.str().c_str(), curbeFileNamePrefix);
        GetLogFileLocation(idFilename.str().c_str(), idFileNamePrefix);

        m_memoryBlock.Dump(curbeFileNamePrefix, m_curbeOffsetInternal, m_totalCurbeSize);
        m_memoryBlock.Dump(idFilename.str(), m_mediaIDOffsetInternal, m_totalMediaIDSize);
    }
#endif
}

MOS_STATUS CmMediaState::Allocate(CmKernelEx **kernels, int count, uint32_t trackerIndex, uint32_t trackerID)
{
    // calculate the curbe size
    m_curbeOffsetInternal = 0;
    uint32_t offset = 0;
    uint32_t totalCurbeSize = 0;
    for (int i = 0; i < count; i++)
    {
        CmKernelEx *kernel = kernels[i];
        uint32_t curbeSize = kernel->GetCurbeSize();
        m_curbeOffsets[i] = totalCurbeSize;
        totalCurbeSize += curbeSize;
    }
    m_totalCurbeSize = totalCurbeSize;

    // calculate the sampler
    m_samplerHeapOffsetInternal = MOS_ALIGN_CEIL(m_totalCurbeSize, MHW_SAMPLER_STATE_ALIGN);
    uint32_t totalHeapSize = 0;
    uint32_t maxSpillSize = 0;
    for (int i = 0; i < count; i++)
    {
        CmKernelEx *kernel = kernels[i];
        uint32_t heapSize = UpdateHeapSizeAndOffsets(kernel, i);
        m_samplerOffsets[i] = totalHeapSize;
        totalHeapSize += heapSize;

        // get the spill size
        maxSpillSize = MOS_MAX(maxSpillSize, kernel->GetSpillMemUsed());
    }
    m_totalSamplerHeapSize = totalHeapSize;
    
    // calculate the media id
    m_mediaIDOffsetInternal = m_samplerHeapOffsetInternal + m_totalSamplerHeapSize;
    m_totalMediaIDSize = count * m_mediaIDSize;

    // caculate the scratch space
    uint32_t tempScratchOffset = m_mediaIDOffsetInternal + m_totalMediaIDSize;
    if (maxSpillSize > 0 && (!m_cmhal->cmHalInterface->IsSeparateScratch()))
    {
        uint32_t perThreadScratchSpace = 1024;
        for (perThreadScratchSpace; perThreadScratchSpace < maxSpillSize; perThreadScratchSpace <<= 1);

        // get max thread number
        MEDIA_SYSTEM_INFO *gtSystemInfo = m_cmhal->osInterface->pfnGetGtSystemInfo(m_cmhal->osInterface);
        uint32_t numHWThreadsPerEU = gtSystemInfo->ThreadCount / gtSystemInfo->EUCount;
        uint32_t maxHWThreads = gtSystemInfo->MaxEuPerSubSlice * numHWThreadsPerEU * gtSystemInfo->MaxSubSlicesSupported;
        // add additional 1k, because the offset of scratch space needs to be 1k aligned
        m_totalScratchSpaceSize = maxHWThreads * perThreadScratchSpace + MHW_SCRATCH_SPACE_ALIGN;
        m_scratchSizePerThread = perThreadScratchSpace;
        // change the extend step in gdsh
        uint32_t currentExtendSize = m_heapMgr->GetExtendSize();
        if (currentExtendSize < m_totalScratchSpaceSize)
        {
            // update extend size for scratch space
            m_heapMgr->SetExtendHeapSize(m_totalScratchSpaceSize);
        }
    }

    // allocate the memory block
    uint32_t totalSize = tempScratchOffset + m_totalScratchSpaceSize;
    PrepareMemoryBlock(totalSize, trackerIndex, trackerID);

    // adjust the offset of scratch space to be 1k aligned
    if (maxSpillSize > 0 && (!m_cmhal->cmHalInterface->IsSeparateScratch()))
    {
        uint32_t scratchOffsetExternal = m_memoryBlock.GetOffset() + tempScratchOffset;
        m_scratchSpaceOffsetExternal = MOS_ALIGN_CEIL(scratchOffsetExternal, MHW_SCRATCH_SPACE_ALIGN);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmMediaState::PrepareMemoryBlock(uint32_t size, uint32_t trackerIndex, uint32_t trackerID)
{
    uint32_t   spaceNeeded = 0;
    std::vector<MemoryBlock> blocks;
    std::vector<uint32_t> blockSizes;
    MemoryBlockManager::AcquireParams acquireParams = 
        MemoryBlockManager::AcquireParams(trackerID, blockSizes);
    acquireParams.m_trackerIndex = trackerIndex;
    if (blockSizes.empty())
    {
        blockSizes.emplace_back(size);
    }
    else
    {
        blockSizes[0] = size;
    }

    m_heapMgr->AcquireSpace(acquireParams, blocks, spaceNeeded);

    if (blocks.empty())
    {
        MHW_RENDERHAL_ASSERTMESSAGE("No blocks were acquired");
        return MOS_STATUS_UNKNOWN;
    }
    if (!(blocks[0].IsValid()))
    {
        MHW_RENDERHAL_ASSERTMESSAGE("No blocks were acquired");
        return MOS_STATUS_UNKNOWN;
    }

    m_memoryBlock = blocks[0];

    // zero memory block
    m_memoryBlock.AddData(nullptr, 0, 0, true);

    m_state = _Allocated;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmMediaState::LoadCurbe(CmKernelEx *kernel, int index)
{
    return LoadCurbe(kernel->GetCurbe(), kernel->GetCurbeSize(), index);
}

MOS_STATUS CmMediaState::LoadCurbe(uint8_t *curbe, uint32_t size, int index)
{
    if (m_state != _Allocated)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Media State not allocated yet");
        return MOS_STATUS_UNKNOWN;
    }

    m_memoryBlock.AddData(curbe, m_curbeOffsetInternal+m_curbeOffsets[index], size);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmMediaState::LoadMediaID(CmKernelEx *kernel, int index, uint32_t btOffset, CmThreadGroupSpace *threadGroupSpace)
{
    if (m_state != _Allocated)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Media State not allocated yet");
        return MOS_STATUS_UNKNOWN;
    }
    MHW_ID_ENTRY_PARAMS params;
    uint32_t mediaStateOffset = m_memoryBlock.GetOffset();

    // Get states, params
    params.dwMediaIdOffset = mediaStateOffset + m_mediaIDOffsetInternal;
    params.iMediaId = index;
    params.dwKernelOffset = kernel->GetOffsetInIsh();
    params.dwSamplerOffset = GetSamplerHeapOffset(index);
    params.dwSamplerCount = MOS_MIN(4, (GetSamplerCount(index) + 3 ) / 4);
    params.dwBindingTableOffset = btOffset;
    params.iCurbeOffset = m_curbeOffsets[index];
    params.iCurbeLength = kernel->GetCurbeSizePerThread();
    if (threadGroupSpace == nullptr)
    {
        params.bBarrierEnable = false;
        params.bGlobalBarrierEnable = false;    //It's only applied for BDW+
        params.dwNumberofThreadsInGPGPUGroup = 1;
        params.dwSharedLocalMemorySize = 0;
        params.iCrsThdConDataRdLn = 0;
    }
    else
    {
        uint32_t threadW = 0;
        uint32_t threadH = 0;
        uint32_t threadD = 0;
        uint32_t groupW = 0;
        uint32_t groupH = 0;
        uint32_t groupD = 0;
        threadGroupSpace->GetThreadGroupSpaceSize(threadW,
                                                  threadH,
                                                  threadD,
                                                  groupW,
                                                  groupH,
                                                  groupD);
        params.bBarrierEnable = (kernel->GetBarrierMode() != CM_NO_BARRIER);
        params.bGlobalBarrierEnable = (kernel->GetBarrierMode() == CM_GLOBAL_BARRIER);
        params.dwNumberofThreadsInGPGPUGroup = threadW * threadH * threadD;
        params.dwSharedLocalMemorySize =
            m_cmhal->renderHal->pfnEncodeSLMSize(m_cmhal->renderHal, kernel->GetSLMSize());;
        params.iCrsThdConDataRdLn = kernel->GetCurbeSizeCrossThread();
    }

    params.memoryBlock = &m_memoryBlock;

    CM_CHK_MOSSTATUS_RETURN(m_cmhal->renderHal->pMhwStateHeap->AddInterfaceDescriptorData(&params));

    return MOS_STATUS_SUCCESS;
}

uint32_t CmMediaState::UpdateHeapSizeAndOffsets(CmKernelEx *kernel, uint32_t kernelIdx)
{
    uint32_t count3D; // not include the reserved
    uint32_t countAVS; // not include the reserved
    uint32_t reservedCount3D = 0;
    kernel->GetSamplerCount(&count3D, &countAVS);
    std::map<int, void *>reservedSamplers = kernel->GetReservedSamplerBteIndex();

    m_samplerCount[kernelIdx] = count3D + countAVS + reservedSamplers.size();

    if (m_samplerCount[kernelIdx] ==0)
    {
        // no sampler in the kernel
        return 0;
    }

    // simplified the sampler allocations
    // reserved samplers
    // avs samplers
    // 3d samplers
    // 3d indirect states

    // get the area of reserved samplers
    uint32_t reservedEnd = 0;
    uint32_t heapSize = 0;
    for (auto it = reservedSamplers.begin(); it != reservedSamplers.end(); it ++)
    {
        int bteIndex = it->first;
        MHW_SAMPLER_STATE_PARAM *param = (MHW_SAMPLER_STATE_PARAM *)it->second;
        uint32_t elementSize;
        if (param->SamplerType == MHW_SAMPLER_TYPE_3D)
        {
            reservedCount3D ++;
            elementSize = m_3dSamplerElementSize;
        }
        else
        {
            elementSize = m_avsSamplerElementSize;
        }
        uint32_t end = (bteIndex + 1) * elementSize;
        reservedEnd = MOS_MAX(reservedEnd, end);
    }
    heapSize = MOS_ALIGN_CEIL(reservedEnd, MHW_SAMPLER_STATE_ALIGN);
    m_nextAvsSamplerOffsets[kernelIdx] = heapSize;
    heapSize += countAVS*m_avsSamplerElementSize;
    m_next3dSamplerOffsets[kernelIdx] = heapSize;
    heapSize += count3D*m_3dSamplerElementSize;
    heapSize = MOS_ALIGN_CEIL(heapSize, 1 << MHW_SAMPLER_INDIRECT_SHIFT);
    m_nextIndStateOffsets[kernelIdx] = heapSize;
    heapSize += (reservedCount3D + count3D) * m_cmhal->renderHal->pMhwStateHeap->m_HwSizes.dwSizeSamplerIndirectState;
    heapSize = MOS_ALIGN_CEIL(heapSize, MHW_SAMPLER_STATE_ALIGN);

    return heapSize;
}

int CmMediaState::AddSampler(void *samplerParam, int index, int bteIndex)
{
    uint32_t offset;
    MHW_SAMPLER_STATE_PARAM *param = (MHW_SAMPLER_STATE_PARAM *)samplerParam;
    uint32_t elementSize = (param->SamplerType == MHW_SAMPLER_TYPE_3D)?m_3dSamplerElementSize:m_avsSamplerElementSize;
    if (bteIndex == -1)
    {
        uint32_t *nextOffset = (param->SamplerType == MHW_SAMPLER_TYPE_3D)?m_next3dSamplerOffsets:m_nextAvsSamplerOffsets;
        offset = nextOffset[index];
        nextOffset[index] += elementSize;
    }
    else
    {
        offset = bteIndex * elementSize;
    }

    if (param->SamplerType == MHW_SAMPLER_TYPE_3D)
    {
        param->Unorm.IndirectStateOffset = m_samplerHeapOffsetInternal + m_samplerOffsets[index] + m_nextIndStateOffsets[index];
        m_nextIndStateOffsets[index] += m_cmhal->renderHal->pMhwStateHeap->m_HwSizes.dwSizeSamplerIndirectState;
    }

    uint32_t heapOffset = m_samplerHeapOffsetInternal + m_samplerOffsets[index] + offset;
    MOS_STATUS ret = m_cmhal->renderHal->pMhwStateHeap->AddSamplerStateData(heapOffset, &m_memoryBlock, param);
    if (ret != MOS_STATUS_SUCCESS)
    {
        return -1;
    }
    return offset/elementSize;
}
