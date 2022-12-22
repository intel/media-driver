/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file      mhw_state_heap_xe_hpg.h
//! \brief         This modules implements HW interface layer to be used on all platforms on
//!            all operating systems/DDIs, across MHW components.
//!

#ifndef __MHW_STATE_HEAP_XE_HPG_H__
#define __MHW_STATE_HEAP_XE_HPG_H__

#include "mhw_state_heap_generic.h"
#include "mhw_state_heap_hwcmd_xe_hpg.h"

struct MHW_STATE_HEAP_INTERFACE_XE_HPG : public MHW_STATE_HEAP_INTERFACE_GENERIC<mhw_state_heap_xe_hpg>
{
private:
    uint16_t m_wSizeOfInterfaceDescriptor;
    uint32_t m_dwSizeSurfaceState;
    uint32_t m_dwSizeSurfaceStateAdv;
    PMHW_STATE_HEAP m_pLastDSHPinter = nullptr;
    PMHW_STATE_HEAP m_pLastISHPinter = nullptr;
    uint32_t        m_lastDwNumDsh = 0;
    uint32_t        m_lastDwNumIsh = 0;

public:
    MHW_STATE_HEAP_INTERFACE_XE_HPG(PMOS_INTERFACE pInputOSInterface, int8_t bDynamicMode);

    ~MHW_STATE_HEAP_INTERFACE_XE_HPG();

    MOS_STATUS SetInterfaceDescriptorEntry(
        PMHW_ID_ENTRY_PARAMS pParams);

    MOS_STATUS AddInterfaceDescriptorData(
        PMHW_ID_ENTRY_PARAMS pParams);

    MOS_STATUS SetSurfaceStateEntry(
        PMHW_SURFACE_STATE_PARAMS pParams);

    MOS_STATUS SetSurfaceState(
        PMHW_KERNEL_STATE       pKernelState,
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        uint32_t                dwNumSurfaceStatesToSet,
        PMHW_RCS_SURFACE_PARAMS pParams);

    MOS_STATUS SetSamplerState(
        void                    *pSampler,
        PMHW_SAMPLER_STATE_PARAM pParam);

    MOS_STATUS AddSamplerStateData(
        uint32_t                 samplerOffset,
        MemoryBlock             *memoryBlock,
        PMHW_SAMPLER_STATE_PARAM pParam);

    MOS_STATUS InitSamplerStates(
        void   *pSamplerStates,
        int32_t iSamplers);

    MOS_STATUS LoadSamplerAvsTable(
        void                        *pTable,
        PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam);

    MOS_STATUS InitHwSizes();

    MOS_STATUS SetInterfaceDescriptor(
        uint32_t                         dwNumIdsToSet,
        PMHW_INTERFACE_DESCRIPTOR_PARAMS pParams);

    MOS_STATUS SetMissingShaderChannels(
        mhw_state_heap_xe_hpg::RENDER_SURFACE_STATE_CMD *pSurfaceState,
        uint32_t                                         dwFormat);

    PMHW_STATE_HEAP GetDSHPointer()
    {
        PMHW_STATE_HEAP pDSHPinter = XMHW_STATE_HEAP_INTERFACE::GetDSHPointer();
        uint32_t        numDSH     = XMHW_STATE_HEAP_INTERFACE::GetNumDsh();
        if (numDSH > 1)
        {
            if (numDSH != m_lastDwNumDsh)
            {
                for (uint32_t i = 0; i < (numDSH - 1); i++)
                {
                    pDSHPinter = pDSHPinter->pNext;
                }
                m_lastDwNumDsh = numDSH;
                m_pLastDSHPinter = pDSHPinter;
            }
            else
            {
                pDSHPinter = m_pLastDSHPinter;
            }

        }
        return pDSHPinter;
    }

    PMHW_STATE_HEAP GetISHPointer()
    {
        PMHW_STATE_HEAP pISHPinter = XMHW_STATE_HEAP_INTERFACE::GetISHPointer();
        uint32_t        numISH     = XMHW_STATE_HEAP_INTERFACE::GetNumIsh();
        if (numISH > 1)
        {
            if (numISH != m_lastDwNumIsh)
            {
                for (uint32_t i = 0; i < (numISH - 1); i++)
                {
                    pISHPinter = pISHPinter->pNext;
                }
                m_lastDwNumIsh = numISH;
                m_pLastISHPinter = pISHPinter;
            }
            else
            {
                pISHPinter = m_pLastISHPinter;
            }
        }
        return pISHPinter;
    }
};
#endif
