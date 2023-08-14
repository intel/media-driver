/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_debug_interface.h
//! \brief    Defines the debug interface shared by vp only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#ifndef __VP_DEBUG_INTERFACE_H__
#define __VP_DEBUG_INTERFACE_H__

#include "media_debug_interface.h"
#include "vp_dumper.h"
#include "vp_common.h"
#include "vp_pipeline_common.h"
#include "vp_utils.h"

#if USE_MEDIA_DEBUG_TOOL
#define USE_VP_DEBUG_TOOL 1
#define VP_DEBUG_TOOL(expr) expr;

class VpDebugInterface : public MediaDebugInterface
{
public:
    VpDebugInterface();
    virtual ~VpDebugInterface();

    virtual MOS_STATUS Initialize(PMOS_INTERFACE pOsInterface);

    void DumpToXML(
        PVPHAL_RENDER_PARAMS            pRenderParams,
        uint32_t                        framecounter);

    void SkuWa_DumpToXML(
        MEDIA_FEATURE_TABLE             *skuTable,
        MEDIA_WA_TABLE                  *waTable);

    MOS_STATUS DumpVpSurfaceArray(
        PVPHAL_SURFACE *ppSurfaces,
        uint32_t        uiMaxSurfaces,
        uint32_t        uiNumSurfaces,
        uint32_t        uiFrameNumber,
        uint32_t        Location,
        uint32_t        uiDDI = VPHAL_SURF_DUMP_DDI_UNKNOWN);

    MOS_STATUS DumpVpSurface(
        PVPHAL_SURFACE pSurf,
        uint32_t       uiFrameNumber,
        uint32_t       uiCounter,
        uint32_t       Location,
        uint32_t       uiDDI = VPHAL_SURF_DUMP_DDI_UNKNOWN);

    MOS_STATUS DumpVpSurface(
        PVP_SURFACE pSurf,
        uint32_t    uiFrameNumber,
        uint32_t    uiCounter,
        uint32_t    Location,
        uint32_t    uiDDI = VPHAL_SURF_DUMP_DDI_UNKNOWN);

protected:
    std::string SetOutputPathKey() override;
    std::string InitDefaultOutput() override;

    VpSurfaceDumper   *m_surfaceDumper   = nullptr;
    VpParameterDumper *m_parameterDumper = nullptr;

MEDIA_CLASS_DEFINE_END(VpDebugInterface)
};

#else
#define USE_VP_DEBUG_TOOL 0
#define VP_DEBUG_TOOL(expr) ;
#endif  // USE_MEDIA_DEBUG_TOOL
#endif  /* __VP_DEBUG_INTERFACE_H__ */