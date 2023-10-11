/*
* Copyright (c) 2017-2022, Intel Corporation
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
//! \file     mos_solo_generic.h
//! \brief    Include Mediasolo toolset functions used for ddi and hal
//! \details  Include Mediasolo toolset functions used for ddi and hal
//!

#ifndef __MOS_SOLO_GENERIC_H__
#define __MOS_SOLO_GENERIC_H__

#include "mos_defs.h"

#if MOS_MEDIASOLO_SUPPORTED

#include "mos_os_solo.h"
#include "mos_os_solo_next_specific.h"

#include "mos_os_solo_next.h"

#else

#define Mos_Solo_DecodeMapGpuNodeToGpuContex(a, b, c, d)

#define Mos_Solo_IsEnabled(a)     false
#define Mos_Solo_IsInUse(a)      false
#define Mos_Solo_Extension(a)    false
#define Mos_Solo_CreateExtension(a, b, c)         MOS_STATUS_SUCCESS
#define Mos_Solo_ReplaceSkuWaTable(a, b, c, d)
#define Mos_Solo_ForceDumps(a, b)                 MOS_STATUS_SUCCESS
#define Mos_Solo_PreProcessDecode(a, b)           MOS_STATUS_SUCCESS
#define Mos_Solo_PostProcessDecode(a, b)          MOS_STATUS_SUCCESS
#define Mos_Solo_PreProcessEncode(a, b, c)        MOS_STATUS_SUCCESS
#define Mos_Solo_PostProcessEncode(a, b, c)       MOS_STATUS_SUCCESS
#define Mos_Solo_CheckNodeLimitation(a, b)
#define Mos_Solo_DisableAubcaptureOptimizations(a, b)  MOS_STATUS_SUCCESS

#define Mos_Solo_CalTileNum(a)
#define Mos_Solo_DdiInitializeSkuWaTable(a, b, c, d)

#define Mos_Solo_DdiInitializeDeviceId(a, b, c, d, e, f, g, h, i,l)  MOS_STATUS_SUCCESS
#define Mos_Solo_OverrideBufferSize(a, b)
#define Mos_Solo_SetOsResource(a, b)
#define Mos_Solo_SetGpuAppTaskEvent(a, b)  MOS_STATUS_SUCCESS
#define Mos_Solo_SetReadyToExecute(a, b)

#define Mos_Solo_SetPlatform(a, b)
#define Mos_Solo_SetSkuwaTable(a, b, c)
#define Mos_Solo_SetSkuwaGtInfo(a, b, c, d, e, f)

#endif // MOS_MEDIASOLO_SUPPORTED
#endif

