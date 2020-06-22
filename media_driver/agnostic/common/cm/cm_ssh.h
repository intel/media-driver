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
//! \file      cm_ssh.h
//! \brief     Contains Class CmSSH  definitions 
//!
#pragma once

#include "cm_surface_state.h"
#include <bitset>

class CmKernelEx;
class CmScratchSpace;
class CmSSH
{
public:
    CmSSH(CM_HAL_STATE *cmhal, PMOS_COMMAND_BUFFER cmdBuf);
    ~CmSSH();
    MOS_STATUS Initialize(CmKernelEx **kernels = nullptr, uint32_t count = 0);

    int AssignBindingTable();

    int GetBindingTableOffset(int btIndex = -1);

    int AddSurfaceState(CmSurfaceState *surfState, int bteIndex = -1, int btIndex = -1);

    int AddScratchSpace(CmScratchSpace *scratch);

    MOS_STATUS PrepareResourcesForCp();

    static void DumpSSH(CM_HAL_STATE *cmhal, PMOS_COMMAND_BUFFER cmdBuf); // to dump legacy path

    void DumpSSH();

protected:
    int GetFreeBindingTableEntries(int surfNum, int btIndex = -1);
    int GetFreeSurfStateIndex(int surfNum);
    int EstimateBTSize(int maxBteNum, std::map<int, CmSurfaceState *> &reservedBteIndex);
    
private:
    struct _BteFlag
    {
        std::bitset<256> _map;

        inline void Set(int start, int count)
        {
            uint64_t mask = 0xffffffff >> (32-count);
            std::bitset<256> temp(mask);
            temp <<= start;
            _map |= temp;
        }

        inline bool IsSet(int start, int count)
        {
            uint64_t mask = 0xffffffff >> (32-count);
            std::bitset<256> temp(mask);
            temp <<= start;
            temp &= _map;
            return temp.any();
        }

    };
    
    // SSH buffer
    uint8_t *m_stateBase;
    uint32_t m_stateOffset;
    uint32_t m_length;
    uint32_t m_btStart; // start of binding tables
    uint32_t m_ssStart; // start of surface states
    uint32_t m_bteNum;
    uint32_t m_btStartPerKernel[CM_MAX_KERNELS_PER_TASK + 1]; // start of each bt, the last one means the end of last BT
    uint32_t m_maxSsNum;
    uint32_t m_btEntrySize; // size of each binding table entry
    uint32_t m_ssCmdSize; // size of each Surface state cmd

    // current ssh is in the end of a command buffer
    PMOS_COMMAND_BUFFER m_cmdBuf;

    // Current BT
    uint32_t m_curBTIndex;
    uint32_t m_curBteIndexes[CM_MAX_KERNELS_PER_TASK];
    uint32_t m_normalBteStart;

    // Surface States added in the ssh
    CmSurfaceState *m_surfStatesInSsh[CM_MAX_SURFACE_STATES];

    // Current Surface State
    uint32_t m_curSsIndex;
    
    CM_HAL_STATE *m_cmhal;
    RENDERHAL_INTERFACE *m_renderhal;

    // All MOS_RESOURCE added in the ssh
    MOS_RESOURCE *m_resourcesAdded[CM_MAX_SURFACE_STATES];
    uint32_t m_resCount;

    // Occupied BteIndexes
    _BteFlag *m_occupiedBteIndexes; // maximun 64x4 = 256 entries for each BT
};
