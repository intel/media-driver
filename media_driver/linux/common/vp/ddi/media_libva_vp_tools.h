/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     media_libva_vp_tools.h
//! \brief    Header for Video Acceleration (LibVA) VP tool functions for VPG drivers
//!

#ifndef _MEDIA_LIBVA_VP_TOOLS_H_
#define _MEDIA_LIBVA_VP_TOOLS_H_

#include "media_libva_common.h"
#include <va/va_vpp.h>

#define LIBVA_VP_CONFIG_NOT_REPORTED  0xffffffff

//!
//! \brief   allocate and initialize DDI Dump parameters
//! \param   [in] pVpCtx
//!          vp context
//! \return  VAStatus
//!          return VA_STATUS_SUCCESS if DDI Params is initialized
//!
VAStatus    VpInitDumpConfig(PDDI_VP_CONTEXT pVpCtx);

//!
//! \brief   destory DDI Dump parameters
//! \param   [in] pVpCtx
//!          vp context
//!
void        VpDestoryDumpConfig(PDDI_VP_CONTEXT pVpCtx);

//!
//! \brief   dump procpipeline parameters
//! \param   [in] pVaDrvCtx
//!          driver context
//! \param   [in] pVpCtx
//!          vp context
//! \return  VAStatus
//!          return VA_STATUS_SUCCESS, if params is dumped to file.
//!
VAStatus    VpDumpProcPipelineParams(VADriverContextP pVaDrvCtx, PDDI_VP_CONTEXT pVpCtx);

//!
//! \brief   dump feature mode parameters for Android 
//! \param   [in] pVpCtx
//!          vp context
//! \return  VAStatus
//!          return VA_STATUS_SUCCESS if params is dumped to file.
//!
VAStatus    VpReportFeatureMode(PDDI_VP_CONTEXT pVpCtx);

//!
//! \brief   initialize configuration values for Android 
//! \param   [in] pConfigValues
//!          vp config values
//!
void VpConfigValuesInit(
    PVP_CONFIG           pConfigValues);

//!
//! \brief   dump config values for Android 
//! \param   [in] pConfig
//!          vp config values
//!
void VpFeatureReport(
    PVP_CONFIG         pConfig);

#endif //_MEDIA_LIBVA_VP_TOOLS_H_

