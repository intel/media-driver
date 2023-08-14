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
//! \file     vp_debug_interface.cpp
//! \brief    Defines the debug interface shared by vp only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!

#include "vp_debug_interface.h"
#if USE_VP_DEBUG_TOOL
#include "vp_debug_config_manager.h"

VpDebugInterface::VpDebugInterface()
{
    memset(m_fileName, 0, sizeof(m_fileName));
    memset(m_path, 0, sizeof(m_path));
}

VpDebugInterface::~VpDebugInterface()
{
    // Destroy surface dumper
    VP_SURF_DUMP_DESTORY(m_surfaceDumper);
    // Destroy vphal parameter dump
    VP_PARAMETERS_DUMPPER_DESTORY(m_parameterDumper);

    if (nullptr != m_configMgr)
    {
        MOS_Delete(m_configMgr);
    }
}

MOS_STATUS VpDebugInterface::Initialize(PMOS_INTERFACE pOsInterface)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VP_DEBUG_FUNCTION_ENTER;

    VP_DEBUG_CHK_NULL_RETURN(pOsInterface);
    m_osInterface = pOsInterface;

    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    //dump loctaion is vpdump
    MediaDebugInterface::SetOutputFilePath();

    m_configMgr = MOS_New(VpDebugConfigMgr, this, m_outputFilePath);
    VP_DEBUG_CHK_NULL_RETURN(m_configMgr);
    m_configMgr->ParseConfig(m_osInterface->pOsContext);

    MediaDebugInterface::InitDumpLocation();

    // Initialize Surface Dumper
    VP_SURF_DUMP_CREATE(m_surfaceDumper);
    VP_DEBUG_CHK_NULL_RETURN(m_surfaceDumper);
    // Initialize Parameter Dumper
    VP_PARAMETERS_DUMPPER_CREATE(m_parameterDumper);
    VP_DEBUG_CHK_NULL_RETURN(m_parameterDumper);

    return eStatus;
}

void VpDebugInterface::DumpToXML(PVPHAL_RENDER_PARAMS pRenderParams, uint32_t framecounter)
{
    VP_FUNC_CALL();

    if (m_surfaceDumper && m_parameterDumper)
    {
        m_parameterDumper->DumpToXML(
            framecounter,
            m_surfaceDumper->m_dumpSpec.pcOutputPath,
            pRenderParams);
    }
}

void VpDebugInterface::SkuWa_DumpToXML(MEDIA_FEATURE_TABLE *skuTable, MEDIA_WA_TABLE *waTable)
{
    VP_FUNC_CALL();

    if (m_parameterDumper)
    {
        m_parameterDumper->SkuWa_DumpToXML(
            skuTable,
            waTable);
    }
}

MOS_STATUS VpDebugInterface::DumpVpSurface(
    PVPHAL_SURFACE pSurf,
    uint32_t       uiFrameNumber,
    uint32_t       uiCounter,
    uint32_t       Location,
    uint32_t       uiDDI)
{
    VP_FUNC_CALL();

    VP_DEBUG_CHK_NULL_RETURN(m_surfaceDumper)
    return m_surfaceDumper->DumpSurface(
        pSurf,
        uiFrameNumber,
        uiCounter,
        Location,
        uiDDI);
}

MOS_STATUS VpDebugInterface::DumpVpSurface(
    PVP_SURFACE    pSurf,
    uint32_t       uiFrameNumber,
    uint32_t       uiCounter,
    uint32_t       Location,
    uint32_t       uiDDI)
{
    VP_FUNC_CALL();

    VP_DEBUG_CHK_NULL_RETURN(m_surfaceDumper)
    return m_surfaceDumper->DumpSurface(
        pSurf,
        uiFrameNumber,
        uiCounter,
        Location,
        uiDDI);
}

MOS_STATUS VpDebugInterface::DumpVpSurfaceArray(
    PVPHAL_SURFACE                  *ppSurfaces,
    uint32_t                        uiMaxSurfaces,
    uint32_t                        uiNumSurfaces,
    uint32_t                        uiFrameNumber,
    uint32_t                        Location,
    uint32_t                        uiDDI)
{
    VP_FUNC_CALL();

    VP_DEBUG_CHK_NULL_RETURN(m_surfaceDumper)
    return m_surfaceDumper->DumpSurfaceArray(
        ppSurfaces,
        uiMaxSurfaces,
        uiNumSurfaces,
        uiFrameNumber,
        Location,
        uiDDI);
}

std::string VpDebugInterface::SetOutputPathKey()
{
    VP_FUNC_CALL();

    return __VPHAL_DBG_SURF_DUMP_OUTFILE_KEY_NAME;
}

std::string VpDebugInterface::InitDefaultOutput()
{
    VP_FUNC_CALL();

    m_outputFilePath.append(MEDIA_DEBUG_VPHAL_DUMP_OUTPUT_FOLDER);
    return SetOutputPathKey();
}

#endif  // USE_VP_DEBUG_INTERFACE

