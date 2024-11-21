/*
* Copyright (c) 2016-2018, Intel Corporation
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
//! \file     mos_os_virtualengine.h
//! \brief    Common interface and structure used in MOS OS VirtualEngine
//! \details  Common interface and structure used in MOS OS VirtualEngine, and only necessary when KMD supports virtual engine
//!
#ifndef __MOS_OS_VIRTUALENGINE_H__
#define __MOS_OS_VIRTUALENGINE_H__

#include "mos_os_virtualengine_specific.h"
#include "mos_os_virtualengine_next.h"

//!
//! \brief     initialize virtual engine interface
//! \details  initialize virtual engine interface
//! \param    [in]  PMOS_INTERFACE
//!                pointer to mos interface
//! \param    [in]  PMOS_VIRTUALENGINE_INIT_PARAMS pVEInitParms
//!                pointer to VE init parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mos_VirtualEngineInterface_Initialize(
    PMOS_INTERFACE                    pOsInterface,
    PMOS_VIRTUALENGINE_INIT_PARAMS    pVEInitParms);

#endif //__MOS_OS_VIRTUALENGINE_SCALABILITY_H__

