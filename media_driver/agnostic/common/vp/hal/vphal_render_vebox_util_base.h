/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file 
//! \brief 
//!
//!
//! \file     vphal_render_vebox_util_base.h
//! \brief    Common utility interface and structure used in Vebox
//! \details  Common utility interface and structure used in Vebox which are platform independent
//!
#ifndef __VPHAL_RENDER_VEBOX_UTIL_BASE_H__
#define __VPHAL_RENDER_VEBOX_UTIL_BASE_H__

#include "mos_os.h"
#include "renderhal_legacy.h"
#include "mhw_vebox.h"
#include "vphal_render_vebox_base.h"

MHW_CSPACE VPHal_VpHalCspace2MhwCspace(
    VPHAL_CSPACE cspace);

MOS_STATUS VpHal_InitVeboxSurfaceParams(
    PVPHAL_SURFACE                 pVpHalVeboxSurface,
    PMHW_VEBOX_SURFACE_PARAMS      pMhwVeboxSurface);

MOS_STATUS VpHal_InitVeboxSurfaceStateCmdParams(
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS      pMhwVeboxSurfaceStateCmdParams);

#endif // __VPHAL_RENDER_VEBOX_UTIL_BASE_H__
