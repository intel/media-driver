/*
* Copyright (c) 2014-2022, Intel Corporation
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
//! \file      mhw_state_heap_legacy.c 
//! \brief         This modules implements HW interface layer to be used on all platforms on     all operating systems/DDIs, across MHW components. 
//!
#include "mhw_state_heap_legacy.h"
#include "media_interfaces_mhw.h"
#include "mhw_cp_interface.h"

//!
//! \brief    Initializes the state heap interface
//! \details  Internal MHW function to initialize all function pointers and some parameters
//! \param    PMHW_STATE_HEAP_INTERFACE* ppStateHeapInterface
//!           [in/out] Poitner to state heap interface pointer to be allocated
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_InitInterface_Legacy(
    PMHW_STATE_HEAP_INTERFACE   *ppCommonStateHeapInterface,
    PMOS_INTERFACE              pOsInterface,
    uint8_t                     bDynamicMode)
{
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface = nullptr;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MhwInterfaces::CreateParams params;
    MhwInterfaces               *mhwInterfaces = nullptr;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(ppCommonStateHeapInterface);
    MHW_CHK_NULL(pOsInterface);

    pCommonStateHeapInterface =
        (PMHW_STATE_HEAP_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MHW_STATE_HEAP_INTERFACE));
    MHW_CHK_NULL(pCommonStateHeapInterface);
    MHW_CHK_STATUS(Mhw_StateHeapInterface_AssignInterfaces(pCommonStateHeapInterface));

    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_stateHeap = true;
    params.m_heapMode = bDynamicMode;
    mhwInterfaces = MhwInterfaces::CreateFactory(params, pOsInterface);
    if (mhwInterfaces)
    {
        MHW_CHK_NULL(mhwInterfaces->m_stateHeapInterface);
        pCommonStateHeapInterface->pStateHeapInterface = mhwInterfaces->m_stateHeapInterface;
        // MhwInterfaces always create CP and MI interfaces, so we have to delete those we don't need.
        MOS_Delete(mhwInterfaces->m_miInterface);
        pOsInterface->pfnDeleteMhwCpInterface(mhwInterfaces->m_cpInterface);
        mhwInterfaces->m_cpInterface = NULL;
        MOS_Delete(mhwInterfaces);
        mhwInterfaces = nullptr;
    }
    else
    {
        MHW_ASSERTMESSAGE("Allocate MhwInterfaces failed");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    *ppCommonStateHeapInterface = pCommonStateHeapInterface;

finish:

    if (eStatus != MOS_STATUS_SUCCESS && pCommonStateHeapInterface)
    {
        pCommonStateHeapInterface->pfnDestroy(pCommonStateHeapInterface);
        *ppCommonStateHeapInterface = nullptr;
        if (mhwInterfaces)
        {
            MOS_Delete(mhwInterfaces->m_miInterface);
            pOsInterface->pfnDeleteMhwCpInterface(mhwInterfaces->m_cpInterface);
            mhwInterfaces->m_cpInterface = NULL;
            MOS_Delete(mhwInterfaces);
        }
    }

    return eStatus;
}
