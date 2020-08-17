/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file      mhw_state_heap_g12.h
//! \brief         This modules implements HW interface layer to be used on all platforms on
//!            all operating systems/DDIs, across MHW components.
//!

#ifndef __MHW_STATE_HEAP_G12_H__
#define __MHW_STATE_HEAP_G12_H__

#include "mhw_state_heap_generic.h"
#include "mhw_state_heap_hwcmd_g11_X.h"  // Still use Gen11 state heap cmd before fixing Gen12 state heap
#include "mhw_state_heap_hwcmd_g12_X.h"

struct MHW_STATE_HEAP_INTERFACE_G12_X : public MHW_STATE_HEAP_INTERFACE_GENERIC<mhw_state_heap_g11_X>
{

private:
    uint16_t                m_wSizeOfInterfaceDescriptor;
    uint32_t                m_dwSizeSurfaceState;
    uint32_t                m_dwSizeSurfaceStateAdv;

public:
    MHW_STATE_HEAP_INTERFACE_G12_X(PMOS_INTERFACE pInputOSInterface, int8_t bDynamicMode);

    ~MHW_STATE_HEAP_INTERFACE_G12_X();

    MOS_STATUS SetInterfaceDescriptorEntry(
        PMHW_ID_ENTRY_PARAMS      pParams);

    MOS_STATUS AddInterfaceDescriptorData(
        PMHW_ID_ENTRY_PARAMS      pParams);

    MOS_STATUS SetSurfaceStateEntry(
        PMHW_SURFACE_STATE_PARAMS   pParams);

    MOS_STATUS SetSurfaceState(
        PMHW_KERNEL_STATE           pKernelState,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        uint32_t                    dwNumSurfaceStatesToSet,
        PMHW_RCS_SURFACE_PARAMS     pParams);

    MOS_STATUS SetSamplerState(
        void                        *pSampler,
        PMHW_SAMPLER_STATE_PARAM    pParam);

    MOS_STATUS AddSamplerStateData(
        uint32_t                    samplerOffset,
        MemoryBlock                 *memoryBlock,
        PMHW_SAMPLER_STATE_PARAM    pParam);

    MOS_STATUS InitSamplerStates(
        void                        *pSamplerStates,
        int32_t                     iSamplers);

    MOS_STATUS LoadSamplerAvsTable(
        void                         *pTable,
        PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam);

    MOS_STATUS InitHwSizes();

};
#endif
