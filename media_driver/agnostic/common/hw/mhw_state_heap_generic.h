/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file     mhw_state_heap_generic.h
//! \brief    MHW interface templates for state heap
//! \details  Impelements shared HW command construction functions across all platforms as templates
//!

#ifndef __MHW_STATE_HEAP_GENERIC_H__
#define __MHW_STATE_HEAP_GENERIC_H__

#include "mhw_state_heap.h"
#include "mhw_mi.h"

template <class TCmds>
class MHW_STATE_HEAP_INTERFACE_GENERIC : public XMHW_STATE_HEAP_INTERFACE
{

public:
    MHW_STATE_HEAP_INTERFACE_GENERIC(
        PMOS_INTERFACE      pInputOsInterface,
        int8_t              bDynamicMode) : XMHW_STATE_HEAP_INTERFACE(pInputOsInterface, bDynamicMode) {}

    virtual ~MHW_STATE_HEAP_INTERFACE_GENERIC() { MHW_FUNCTION_ENTER; }

    MOS_STATUS SetBindingTable(PMHW_KERNEL_STATE  pKernelState)
    {
        MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(pKernelState);
        MHW_MI_CHK_NULL(m_pOsInterface);

        uint8_t             *pIndirectState = nullptr;
        uint32_t            uiIndirectStateSize = 0, uiIndirectStateOffset = 0;
        MHW_MI_CHK_STATUS(m_pOsInterface->pfnGetIndirectStatePointer(m_pOsInterface, &pIndirectState));
        MHW_MI_CHK_STATUS(m_pOsInterface->pfnGetIndirectState(m_pOsInterface, &uiIndirectStateOffset, &uiIndirectStateSize));

        if ((pKernelState->dwSshOffset + pKernelState->dwSshSize) > uiIndirectStateSize)
        {
            MHW_ASSERTMESSAGE("SSH not large enough to hold data for this kernel.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        uint32_t ui32BindingTableSize = pKernelState->dwSshSize;
        uint8_t *pBindingTablePtr     = (uint8_t*)(pIndirectState + pKernelState->dwSshOffset);
        if (pBindingTablePtr != nullptr)
        {
            MOS_ZeroMemory(pBindingTablePtr, ui32BindingTableSize);
        }

        typename            TCmds::BINDING_TABLE_STATE_CMD Cmd;
        for (uint32_t i = 0; i < (uint32_t)pKernelState->KernelParams.iBTCount; i++)
        {
            MHW_MI_CHK_NULL(pBindingTablePtr);

            Cmd.DW0.SurfaceStatePointer =
                ((pKernelState->dwSshOffset + pKernelState->dwBindingTableSize) +
                (i * m_dwMaxSurfaceStateSize)) >>
                m_mhwBindingTableSurfaceShift;
            MHW_MI_CHK_STATUS(MOS_SecureMemcpy(pBindingTablePtr, ui32BindingTableSize, &Cmd, Cmd.byteSize));
            pBindingTablePtr += Cmd.byteSize;
            ui32BindingTableSize -= Cmd.byteSize;
        }

        return eStatus;
    }

    MOS_STATUS SetBindingTableEntry(PMHW_BINDING_TABLE_PARAMS pParams)
    {
        MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;
        MHW_MI_CHK_NULL(pParams);

        uint8_t*     pBindingTablePtr = pParams->pBindingTableEntry;
        MHW_MI_CHK_NULL(pBindingTablePtr);

        //Init Cmds
        typename     TCmds::BINDING_TABLE_STATE_CMD Cmd;
        Cmd.DW0.SurfaceStatePointer = pParams->dwSurfaceStateOffset >> m_mhwBindingTableSurfaceShift;

        //Copy to binding table Entry
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(pBindingTablePtr, Cmd.byteSize, &Cmd, Cmd.byteSize));

        // Move to next BT entry
        pParams->pBindingTableEntry += Cmd.byteSize;

        return eStatus;
    }

    MOS_STATUS SendBindingTableEntry(PMHW_BINDING_TABLE_SEND_PARAMS   pParams)
    {
        MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

        if (pParams == nullptr ||
            pParams->pBindingTableSource == nullptr ||
            pParams->pBindingTableTarget == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        typename TCmds::BINDING_TABLE_STATE_CMD *pBtSrc =
            (typename TCmds::BINDING_TABLE_STATE_CMD *)pParams->pBindingTableSource ;
        MHW_MI_CHK_NULL(pBtSrc);

        typename TCmds::BINDING_TABLE_STATE_CMD *pBtDst =
            (typename TCmds::BINDING_TABLE_STATE_CMD *)pParams->pBindingTableTarget;
        MHW_MI_CHK_NULL(pBtDst);

        uint32_t CmdByteSize = TCmds::BINDING_TABLE_STATE_CMD::byteSize;

        // Setup and increment BT pointers
        pParams->pBindingTableSource += CmdByteSize;
        pParams->pBindingTableTarget += CmdByteSize;

        if (pBtSrc->DW0.SurfaceStatePointer != 0)
        {
            // Set binding table entry in indirect state
            *pBtDst = *pBtSrc;

            // Return surface state parameters associated with BT entry
            pParams->iSurfaceStateOffset = pBtSrc->DW0.Value;
            pParams->iSurfaceState       = \
                (pParams->iSurfaceStateOffset - pParams->iSurfaceStateBase)>> m_mhwBindingTableSurfaceShift;
        }
        else
        {
            *pBtDst = typename TCmds::BINDING_TABLE_STATE_CMD();
            pParams->iSurfaceState       = -1;
        }

        return eStatus;
    }

    MOS_STATUS SetInterfaceDescriptor(
        uint32_t                            dwNumIdsToSet,
        PMHW_INTERFACE_DESCRIPTOR_PARAMS    pParams)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        return eStatus;
    }
};

#endif
