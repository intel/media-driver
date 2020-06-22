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

#include "cm_ssh.h"
#include "renderhal_platform_interface.h"
#include "cm_kernel_ex.h"
#include "cm_scratch_space.h"
#include "mos_solo_generic.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

CmSSH::CmSSH(CM_HAL_STATE *cmhal, PMOS_COMMAND_BUFFER cmdBuf):
    m_stateBase(nullptr),
    m_stateOffset(0),
    m_length(0),
    m_btStart(0),
    m_ssStart(0),
    m_bteNum(0),
    m_maxSsNum(0),
    m_btEntrySize(0),
    m_ssCmdSize(0),
    m_cmdBuf(cmdBuf),
    m_curBTIndex(0),
    m_normalBteStart(0),
    m_curSsIndex(0),
    m_cmhal(cmhal),
    m_renderhal(nullptr),
    m_resCount(0),
    m_occupiedBteIndexes(nullptr)
{
}

MOS_STATUS CmSSH::Initialize(CmKernelEx **kernels, uint32_t count)
{
    CM_CHK_NULL_RETURN_MOSERROR(m_cmhal);

    m_renderhal = m_cmhal->renderHal;
    if (!m_renderhal)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    PMOS_INTERFACE osInterface = m_cmhal->osInterface;
    if (m_cmdBuf && osInterface)
    {
        osInterface->pfnGetIndirectState(osInterface, &m_stateOffset, &m_length);
        m_stateBase = (uint8_t *)m_cmdBuf->pCmdBase + m_stateOffset;
    }

    if (!m_stateBase)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    PRENDERHAL_STATE_HEAP_SETTINGS pSettings = &m_renderhal->StateHeapSettings;
    m_btStart = 0;

    m_btEntrySize = m_renderhal->pHwSizes->dwSizeBindingTableState;
    m_ssCmdSize = m_renderhal->pRenderHalPltInterface->GetSurfaceStateCmdSize();

    // Get the total binding table entry number for one ssh
    m_bteNum = pSettings->iBindingTables * pSettings->iSurfacesPerBT;

    CM_SURFACE_BTI_INFO surfBTIInfo;
    m_cmhal->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);
    m_normalBteStart = surfBTIInfo.normalSurfaceStart;

    m_ssStart = m_btStart + m_bteNum * m_btEntrySize;
    m_maxSsNum = pSettings->iSurfaceStates;

    uint32_t requiredSize = m_ssStart + m_maxSsNum * m_ssCmdSize;

    // set binding table and surface states to 0
    MOS_ZeroMemory(m_stateBase + m_btStart, requiredSize);

    if (requiredSize > m_length)
    {
        return MOS_STATUS_NO_SPACE;
    }

    MOS_ZeroMemory(m_btStartPerKernel, sizeof(m_btStartPerKernel));
    MOS_ZeroMemory(m_curBteIndexes, sizeof(m_curBteIndexes));
    MOS_ZeroMemory(m_surfStatesInSsh, sizeof(m_surfStatesInSsh));
    MOS_ZeroMemory(m_resourcesAdded, sizeof(m_resourcesAdded));
    m_occupiedBteIndexes = MOS_NewArray(_BteFlag, count);

    return MOS_STATUS_SUCCESS;
}

CmSSH::~CmSSH()
{
    if (m_occupiedBteIndexes)
    {
        MOS_DeleteArray(m_occupiedBteIndexes);
    }
}


int CmSSH::AssignBindingTable()
{
    int index = m_curBTIndex;
    m_curBTIndex ++;
    m_curBteIndexes[index] = m_normalBteStart;
    if (index > 0) // align the last BT size
    {
        uint32_t size = (m_btStartPerKernel[index] - m_btStartPerKernel[index -1]) * m_btEntrySize;
        uint32_t alignedSize = MOS_ALIGN_CEIL(size, m_renderhal->StateHeapSettings.iBTAlignment);
        m_btStartPerKernel[index] = m_btStartPerKernel[index -1] + alignedSize/m_btEntrySize;
    }

    return index;
}

int CmSSH::GetFreeBindingTableEntries(int surfNum, int btIndex)
{
    int index = (btIndex == -1)?(m_curBTIndex - 1):btIndex;
    if (index < 0 || index >= CM_MAX_KERNELS_PER_TASK)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Binding Table not assigned yet");
        return -1;
    }
    uint32_t bteIndex = m_curBteIndexes[index];
    for (; bteIndex < m_bteNum - m_btStartPerKernel[index]; bteIndex ++)
    {
        if (!m_occupiedBteIndexes[index].IsSet(bteIndex, surfNum))
        {
            break;
        }
    }

    m_curBteIndexes[index] = bteIndex + surfNum;
    if (m_curBteIndexes[index] > m_bteNum - m_btStartPerKernel[index] + 1)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("No available binding table entries in current binding table");
        return -1;
    }
    return bteIndex;
}

int CmSSH::GetFreeSurfStateIndex(int surfNum)
{
    if (m_curSsIndex + surfNum > m_maxSsNum)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Unable to allocate Surface State. Exceeds Maximum.");
        return -1;
    }

    int ssIdx = m_curSsIndex;
    m_curSsIndex += surfNum;
    return ssIdx;
}

int CmSSH::AddSurfaceState(CmSurfaceState *surfState, int bteIndex, int btIndex)
{
    int btIdx = (btIndex == -1)?(m_curBTIndex - 1):btIndex;

    bool bteRecorded = false;
    bool ssRecorded = false;

    CmSSH *rSsh;
    int rBtIdx;
    int rBteIdx;
    int rSsIdx;
    surfState->GetRecordedInfo(&rSsh, &rBtIdx, &rBteIdx, &rSsIdx);

    if (rSsh == this && rSsIdx >= 0)
    {
        bool valid = true;
        for (uint32_t idx = 0; idx < surfState->GetNumPlane(); idx ++)
        {
            if (m_surfStatesInSsh[rSsIdx+idx] != surfState)
            {
                valid = false;
                break;
            }
        }
        if (valid)
        {
            ssRecorded = true;
            if (rBtIdx == btIdx)
            {
                bteRecorded = true;
            }
        }
    }

    if (bteRecorded && (bteIndex == -1 || rBteIdx == bteIndex))
    {
        return rBteIdx;
    }
    // if bteRecorded by user required a different bteIndex, still add a new entry
    bteRecorded = false;

    MOS_RESOURCE *resource = surfState->GetResource();

    int bteIdx = (bteIndex == -1)?(GetFreeBindingTableEntries(surfState->GetNumBte(), btIdx)):bteIndex;
    if (btIdx == -1 || bteIdx == -1)
    {
        return -1;
    }

    int ssIdx;
    if (ssRecorded)
    {
        ssIdx = rSsIdx;
    }
    else
    {
        ssIdx = GetFreeSurfStateIndex(surfState->GetNumPlane());
    }

    m_occupiedBteIndexes[btIdx].Set(bteIdx, surfState->GetNumBte());

    if (btIdx == m_curBTIndex - 1) // current binding table, allow next bt start to change
    {
        if ((btIdx + 1) >= (CM_MAX_KERNELS_PER_TASK + 1))
        {
            MHW_RENDERHAL_ASSERTMESSAGE("Binding table index exceeds the maximun.");
            return -1;
        }
        m_btStartPerKernel[btIdx + 1] = MOS_MAX(m_btStartPerKernel[btIdx + 1], m_btStartPerKernel[btIdx] + bteIdx + surfState->GetNumBte());
        if (m_btStartPerKernel[btIdx + 1] > m_bteNum)
        {
            MHW_RENDERHAL_ASSERTMESSAGE("Binding table entry index exceeds the maximun.");
            return -1;
        }
    }
    else if ((uint32_t)btIdx > m_curBTIndex - 1)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Can't set to a binding table not assigned.");
        return -1;
    }
    else if((btIdx + 1) >= (CM_MAX_KERNELS_PER_TASK + 1) ||
            (m_btStartPerKernel[btIdx] + bteIdx + surfState->GetNumBte() > m_btStartPerKernel[btIdx + 1]))
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Binding table entry index exceeds the maximun.");
        return -1;
    }

    int stateIdx = 0;
    for (uint32_t surfIdx = 0; surfIdx < surfState->GetNumBte(); surfIdx ++)
    {
        uint8_t *pSS = surfState->GetSurfaceState(surfIdx);
        if (pSS == nullptr)
        {
            continue;
        }
        // Get the pointer to the binding table entry
        uint8_t *pBte = m_stateBase + m_btStart + m_btStartPerKernel[btIdx] * m_btEntrySize + (bteIdx+surfIdx) * m_btEntrySize;
        // Get the offset to the surface state
        uint32_t surfStateOffset = m_ssStart + (ssIdx + stateIdx) * m_ssCmdSize;

        MHW_BINDING_TABLE_PARAMS params;
        params.pBindingTableEntry   = pBte;
        params.dwSurfaceStateOffset = surfStateOffset;
        params.bSurfaceStateAvs     = surfState->isAVS(surfIdx)? true : false;
        params.iBindingTableEntry   = bteIdx + surfIdx;

        m_renderhal->pMhwStateHeap->SetBindingTableEntry(&params);

        if (!ssRecorded)
        {
            int patchDWOffset = 0;

            MOS_RESOURCE *resourcePerPlane = surfState->GetResource(surfIdx);
            if (resourcePerPlane)
            {
                if (m_cmhal->syncOnResource && (!Mos_Solo_IsInUse(m_cmhal->osInterface)))
                {
                    if (m_cmhal->osInterface->CurrentGpuContextOrdinal != MOS_GPU_CONTEXT_RENDER4)
                    {
                        m_cmhal->osInterface->pfnSyncOnResource(
                            m_cmhal->osInterface,
                            resourcePerPlane,
                            m_cmhal->osInterface->CurrentGpuContextOrdinal, //state->GpuContext,
                            true);
                    }
                }
                m_renderhal->pOsInterface->pfnRegisterResource(m_renderhal->pOsInterface, resourcePerPlane, true, true);
                m_resourcesAdded[m_resCount ++] = resourcePerPlane;
            }
            else
            {
                continue;
            }

            if (surfState->isAVS(surfIdx))
            {
                MOS_SecureMemcpy(m_stateBase + surfStateOffset, m_renderhal->pHwSizes->dwSizeSurfaceState,
                    pSS, m_renderhal->pHwSizes->dwSizeSurfaceState);
                patchDWOffset = 6;
            }
            else
            {
                MOS_SecureMemcpy(m_stateBase + surfStateOffset, m_renderhal->pHwSizes->dwSizeSurfaceState,
                    pSS, m_renderhal->pHwSizes->dwSizeSurfaceState);
                patchDWOffset = 8;
            }

            uint64_t ui64GfxAddress = 0;
            if (m_renderhal->pOsInterface->bUsesGfxAddress)
            {
                uint32_t *pAddr = (uint32_t *)(m_stateBase + surfStateOffset) + patchDWOffset;

                if ( resourcePerPlane->user_provided_va != 0 )
                {
                    ui64GfxAddress = resourcePerPlane->user_provided_va;
                }
                else
                {
                    ui64GfxAddress = m_renderhal->pOsInterface->pfnGetResourceGfxAddress( m_renderhal->pOsInterface, resourcePerPlane) 
                                        + surfState->GetSurfaceOffset(surfIdx);
                }
                *pAddr = ( uint32_t )( ui64GfxAddress & 0x00000000FFFFFFFF );
                *(pAddr + 1) = ( uint32_t )( ( ui64GfxAddress & 0x0000FFFF00000000 ) >> 32 );
            }

            MOS_PATCH_ENTRY_PARAMS PatchEntryParams;

            uint8_t *pbPtrCmdBuf = (uint8_t *)m_cmdBuf->pCmdBase;

            MOS_ZeroMemory(&PatchEntryParams, sizeof(PatchEntryParams));
            PatchEntryParams.uiAllocationIndex  = m_renderhal->pOsInterface->pfnGetResourceAllocationIndex(m_renderhal->pOsInterface, resourcePerPlane);
            PatchEntryParams.uiResourceOffset = surfState->GetSurfaceOffset(surfIdx);
            PatchEntryParams.uiPatchOffset    = m_stateOffset + surfStateOffset + patchDWOffset*4;
            PatchEntryParams.bWrite           = surfState->IsRenderTarget();
            PatchEntryParams.HwCommandType    = surfState->isAVS(surfIdx)? MOS_SURFACE_STATE_ADV : MOS_SURFACE_STATE;
            PatchEntryParams.forceDwordOffset = 0;
            PatchEntryParams.cmdBufBase       = pbPtrCmdBuf;
            PatchEntryParams.presResource     = resourcePerPlane;

            // Set patch for surface state address
            m_renderhal->pOsInterface->pfnSetPatchEntry(
                m_renderhal->pOsInterface,
                &PatchEntryParams);

            if (surfState->GetMmcState(surfIdx) == MOS_MEMCOMP_RC && (!surfState->isAVS(surfIdx))
                && (!m_cmhal->cmHalInterface->SupportCompressedOutput()))
            {
                if (m_renderhal->pOsInterface->bUsesGfxAddress)
                {
                    uint32_t *pSurfStateAddr = (uint32_t *)(m_stateBase + surfStateOffset);
                    uint64_t auxAddress = ui64GfxAddress + (uint32_t)resourcePerPlane->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS);
                    *(pSurfStateAddr + 10) = ( uint32_t )( auxAddress & 0x00000000FFFFFFFF );
                    *(pSurfStateAddr + 11) = ( uint32_t )( ( auxAddress & 0x0000FFFF00000000 ) >> 32 );
                    uint64_t clearAddress = ui64GfxAddress + (uint32_t)resourcePerPlane->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CC);
                    *(pSurfStateAddr + 12) = ( uint32_t )( clearAddress & 0x00000000FFFFFFFF );
                    *(pSurfStateAddr + 13) = ( uint32_t )( ( clearAddress & 0x0000FFFF00000000 ) >> 32 );
                }
                else
                {
                    // Set patch for AuxiliarySurfaceBaseAddress
                    MOS_ZeroMemory(&PatchEntryParams, sizeof(PatchEntryParams));
                    PatchEntryParams.uiAllocationIndex  = m_renderhal->pOsInterface->pfnGetResourceAllocationIndex(m_renderhal->pOsInterface, resourcePerPlane);
                    PatchEntryParams.uiResourceOffset = surfState->GetSurfaceOffset(surfIdx) + (uint32_t)resourcePerPlane->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS);
                    PatchEntryParams.uiPatchOffset    = m_stateOffset + surfStateOffset + 10 * sizeof(uint32_t);
                    PatchEntryParams.bWrite           = surfState->IsRenderTarget();
                    PatchEntryParams.HwCommandType    = surfState->isAVS(surfIdx)? MOS_SURFACE_STATE_ADV : MOS_SURFACE_STATE;
                    PatchEntryParams.forceDwordOffset = 0;
                    PatchEntryParams.cmdBufBase       = pbPtrCmdBuf;
                    PatchEntryParams.presResource     = resourcePerPlane;
                    m_renderhal->pOsInterface->pfnSetPatchEntry(
                        m_renderhal->pOsInterface,
                        &PatchEntryParams);

                    // Set patch for ClearAddress
                    MOS_ZeroMemory(&PatchEntryParams, sizeof(PatchEntryParams));
                    PatchEntryParams.uiAllocationIndex  = m_renderhal->pOsInterface->pfnGetResourceAllocationIndex(m_renderhal->pOsInterface, resourcePerPlane);
                    PatchEntryParams.uiResourceOffset = surfState->GetSurfaceOffset(surfIdx) + (uint32_t)resourcePerPlane->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CC);
                    PatchEntryParams.uiPatchOffset    = m_stateOffset + surfStateOffset + 12 * sizeof(uint32_t);
                    PatchEntryParams.bWrite           = surfState->IsRenderTarget();
                    PatchEntryParams.HwCommandType    = surfState->isAVS(surfIdx)? MOS_SURFACE_STATE_ADV : MOS_SURFACE_STATE;
                    PatchEntryParams.forceDwordOffset = 0;
                    PatchEntryParams.cmdBufBase       = pbPtrCmdBuf;
                    PatchEntryParams.presResource     = resourcePerPlane;
                    m_renderhal->pOsInterface->pfnSetPatchEntry(
                        m_renderhal->pOsInterface,
                        &PatchEntryParams);
                }
            }

            // Record the surface state added
            m_surfStatesInSsh[ssIdx + stateIdx] = surfState;
            stateIdx ++;
        }
    }

    surfState->Recorded(this, btIdx, bteIdx, ssIdx);

    return bteIdx;
}

int CmSSH::AddScratchSpace(CmScratchSpace *scratch)
{
    int ssIdx = GetFreeSurfStateIndex(1); // one surface state for scratch
    if (ssIdx < 0)
    {
        return ssIdx;
    }
    MOS_RESOURCE *resource =  scratch->GetResource();
    m_renderhal->pOsInterface->pfnRegisterResource(m_renderhal->pOsInterface,
                                                   resource,
                                                   true, true);
    m_resourcesAdded[m_resCount ++] = resource;

    uint32_t surfStateOffset = m_ssStart + ssIdx * m_ssCmdSize;

    // create a surface state for scratch buffer
    CmSurfaceStateBuffer surfState(m_cmhal);
    surfState.Initialize(scratch->GetResource(), scratch->GetSize());
    surfState.GenerateSurfaceState();

    uint8_t *pSS = surfState.GetSurfaceState(0);

    MOS_SecureMemcpy(m_stateBase + surfStateOffset, m_renderhal->pHwSizes->dwSizeSurfaceState,
        pSS, m_renderhal->pHwSizes->dwSizeSurfaceState);
    int patchDWOffset = 8;

    uint64_t ui64GfxAddress = 0;
    if (m_renderhal->pOsInterface->bUsesGfxAddress)
    {
        uint32_t *pAddr = (uint32_t *)(m_stateBase + surfStateOffset) + patchDWOffset;

        if ( resource->user_provided_va != 0 )
        {
            ui64GfxAddress = resource->user_provided_va;
        }
        else
        {
            ui64GfxAddress = m_renderhal->pOsInterface->pfnGetResourceGfxAddress( m_renderhal->pOsInterface, resource) 
                                + surfState.GetSurfaceOffset(0);
        }
        *pAddr = ( uint32_t )( ui64GfxAddress & 0x00000000FFFFFFFF );
        *(pAddr + 1) = ( uint32_t )( ( ui64GfxAddress & 0x0000FFFF00000000 ) >> 32 );
    }

    MOS_PATCH_ENTRY_PARAMS PatchEntryParams;
    uint8_t *pbPtrCmdBuf = (uint8_t *)m_cmdBuf->pCmdBase;

    MOS_ZeroMemory(&PatchEntryParams, sizeof(PatchEntryParams));
    PatchEntryParams.uiAllocationIndex  = m_renderhal->pOsInterface->pfnGetResourceAllocationIndex(m_renderhal->pOsInterface, resource);
    PatchEntryParams.uiResourceOffset = surfState.GetSurfaceOffset(0);
    PatchEntryParams.uiPatchOffset    = m_stateOffset + surfStateOffset + patchDWOffset*4;
    PatchEntryParams.bWrite           = surfState.IsRenderTarget();
    PatchEntryParams.HwCommandType    = surfState.isAVS(0)? MOS_SURFACE_STATE_ADV : MOS_SURFACE_STATE;
    PatchEntryParams.forceDwordOffset = 0;
    PatchEntryParams.cmdBufBase       = pbPtrCmdBuf;
    PatchEntryParams.presResource     = resource;

    // Set patch for surface state address
    m_renderhal->pOsInterface->pfnSetPatchEntry(m_renderhal->pOsInterface,
                                                &PatchEntryParams);

    return surfStateOffset;
}

int CmSSH::GetBindingTableOffset(int btIndex)
{
    if (btIndex == -1)
    {
        btIndex = m_curBTIndex - 1;
    }
    if (btIndex == -1) // not assigned a bt
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Binding Table not Allocated.");
    }
    return m_btStart + m_btStartPerKernel[btIndex]*m_btEntrySize;
}

int CmSSH::EstimateBTSize(int maxBteNum, std::map<int, CmSurfaceState *> &reservedBteIndex)
{
    int estimatedSize = maxBteNum + m_normalBteStart;
    if (reservedBteIndex.empty())
    {
        return estimatedSize;
    }

    for (auto it = reservedBteIndex.begin(); it != reservedBteIndex.end(); it++)
    {
        if (it->first < (int)m_normalBteStart)
        {
            continue;
        }
        else if (it->first > estimatedSize - 1)
        {
            estimatedSize = it->first + it->second->GetNumBte();
        }
        else
        {
            estimatedSize += it->second->GetNumBte() + 2; // reserved 2 extra entries for each inserted bte
        }
    }

    return estimatedSize;
}

MOS_STATUS CmSSH::PrepareResourcesForCp()
{
    if (m_resCount > 0 && m_renderhal->pOsInterface->osCpInterface)
    {
        return m_renderhal->pOsInterface->osCpInterface->PrepareResources((void **)m_resourcesAdded, m_resCount, nullptr, 0);
    }
    return MOS_STATUS_SUCCESS;
}

using namespace std;

#if defined(ANDROID) || defined(LINUX)
#define PLATFORM_DIR_SEPERATOR   "/"
#else
#define PLATFORM_DIR_SEPERATOR   "\\"
#endif

void CmSSH::DumpSSH()
{
#if MDF_SURFACE_STATE_DUMP
    if (m_cmhal->dumpSurfaceState)
    {
        char fileNamePrefix[MAX_PATH];
        static int fileCount = 0;
        stringstream filename;
        filename << "HALCM_Surface_State_Dumps" << PLATFORM_DIR_SEPERATOR << "ssh_" << fileCount++ << ".fast.log";
        GetLogFileLocation(filename.str().c_str(), fileNamePrefix);

        fstream sshFile;
        sshFile.open(fileNamePrefix, ios_base::out);

        sshFile << "==============================" << endl;
        sshFile << "Binding Table:" << endl;

        for (uint32_t idx = 0; idx < CM_MAX_KERNELS_PER_TASK; idx ++)
        {
            sshFile << "BT " << dec << idx << ":" << endl;
            for (int i = 0; i < (int)(m_btStartPerKernel[idx + 1] - m_btStartPerKernel[idx]); i++)
            {
                sshFile << "    " << dec << i << ": ";
                uint32_t *pData = (uint32_t *)(m_stateBase + m_btStart + (m_btStartPerKernel[idx] + i)*m_btEntrySize);
                for (uint32_t j = 0; j < m_btEntrySize/4; j ++)
                {
                    sshFile << "0x" << hex << *(pData + j) <<", ";
                }
                sshFile << endl;
            }
        }

        sshFile << endl << endl << endl;
        sshFile << "================================" << endl;
        sshFile << "Surface States:" << endl;
        for (uint32_t idx = 0; idx < m_maxSsNum; idx ++)
        {
            int offset = m_ssStart + idx * m_ssCmdSize;
            sshFile << dec << idx << ": offset 0x" << hex << offset << ":  ";
            uint32_t *pData = (uint32_t *)(m_stateBase + offset);
            for (uint32_t i = 0; i < m_ssCmdSize/4; i++)
            {
                sshFile << "0x" << hex << *(pData + i) << ", ";
            }
            sshFile << endl;
        }

        sshFile.close();
    }
#endif
}

void CmSSH::DumpSSH(CM_HAL_STATE *cmhal, PMOS_COMMAND_BUFFER cmdBuf)
{
    uint8_t *stateBase = nullptr;
    uint32_t stateOffset;
    uint32_t length;
    uint32_t btStart; // start of binding tables
    uint32_t ssStart; // start of surface states
    uint32_t maxBtNum;
    uint32_t btSize; // size of each binding table
    uint32_t maxSsNum;
    uint32_t btEntrySize; // size of each binding table entry
    uint32_t ssCmdSize; // size of each Surface state cmd
    RENDERHAL_INTERFACE *renderhal = nullptr;
    if (cmhal)
    {
        renderhal = cmhal->renderHal;
    }
    if (!renderhal)
    {
        return;
    }

    PMOS_INTERFACE osInterface = cmhal->osInterface;
    if (cmdBuf && osInterface)
    {
        osInterface->pfnGetIndirectState(osInterface, &stateOffset, &length);
        stateBase = (uint8_t *)cmdBuf->pCmdBase + stateOffset;
    }

    if (!stateBase)
    {
        return;
    }

    PRENDERHAL_STATE_HEAP_SETTINGS pSettings = &renderhal->StateHeapSettings;
    btStart = 0;
    maxBtNum = pSettings->iBindingTables;
    btSize = MOS_ALIGN_CEIL(pSettings->iSurfacesPerBT * renderhal->pHwSizes->dwSizeBindingTableState,
                              pSettings->iBTAlignment);
    ssStart = btStart + maxBtNum * btSize;
    maxSsNum = pSettings->iSurfaceStates;
    btEntrySize = renderhal->pHwSizes->dwSizeBindingTableState;
    ssCmdSize = renderhal->pRenderHalPltInterface->GetSurfaceStateCmdSize();

    // Dump
    static int fileCount = 0;
    stringstream filename;
    filename << "ssh_" << fileCount++ << ".orig.log";

    fstream sshFile;
    sshFile.open(filename.str(), ios_base::out);

    sshFile << "==============================" << endl;
    sshFile << "Binding Table:" << endl;

    for (uint32_t idx = 0; idx < maxBtNum; idx ++)
    {
        sshFile << "BT " << dec << idx << ":" << endl;
        for (uint32_t i = 0; i < btSize/btEntrySize; i++)
        {
            sshFile << "    " << dec << i << ": ";
            uint32_t *pData = (uint32_t *)(stateBase + btStart + idx*btSize + i*btEntrySize);
            for (uint32_t j = 0; j < btEntrySize/4; j ++)
            {
                sshFile << "0x" << hex << *(pData + j) <<", ";
            }
            sshFile << endl;
        }
    }

    sshFile << endl << endl << endl;
    sshFile << "================================" << endl;
    sshFile << "Surface States:" << endl;
    for (uint32_t idx = 0; idx < maxSsNum; idx ++)
    {
        int offset = ssStart + idx * ssCmdSize;
        sshFile << dec << idx << ": offset 0x" << hex << offset << ":  ";
        uint32_t *pData = (uint32_t *)(stateBase + offset);
        for (uint32_t i = 0; i < ssCmdSize/4; i++)
        {
            sshFile << "0x" << hex << *(pData + i) << ", ";
        }
        sshFile << endl;
    }
    sshFile.close();
}

