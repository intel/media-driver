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
//! \file      mhw_state_heap_legacy.h 
//! \brief         This modules implements HW interface layer to be used on all platforms on     all operating systems/DDIs, across MHW components. 
//!
#ifndef __MHW_STATE_HEAP_LEGACY_H__
#define __MHW_STATE_HEAP_LEGACY_H__
#include "mhw_state_heap.h"

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
    PMOS_INTERFACE               pOsInterface,
    uint8_t                      bDynamicMode);


#endif // __MHW_STATE_HEAP_LEGACY_H__