/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     codechal_common.cpp
//! \brief    Impelements the public interface for CodecHal.
//! \details
//!

#include "codechal_common.h"
#include "codec_hw_next.h"
#include "codechal_debug.h"
#include "mos_solo_generic.h"
#include "codechal_setting.h"

Codechal::Codechal(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface *debugInterface)
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;
    if (hwInterface)
    {
        CODECHAL_PUBLIC_CHK_NULL_NO_STATUS_RETURN(hwInterface->GetOsInterface());
        MOS_UNUSED(debugInterface);

        m_hwInterface = hwInterface;
        m_osInterface = hwInterface->GetOsInterface();

        m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
        ;
    }
#if USE_CODECHAL_DEBUG_TOOL
        CODECHAL_PUBLIC_CHK_NULL_NO_STATUS_RETURN(debugInterface);
        m_debugInterface = debugInterface;
#endif  // USE_CODECHAL_DEBUG_TOOL


}

Codechal::~Codechal()
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;

    MOS_TraceEvent(EVENT_CODECHAL_DESTROY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

#if USE_CODECHAL_DEBUG_TOOL
    if (m_debugInterface != nullptr)
    {
        MOS_Delete(m_debugInterface);
        m_debugInterface = nullptr;
    }

    if (m_statusReportDebugInterface != nullptr)
    {
        MOS_Delete(m_statusReportDebugInterface);
        m_statusReportDebugInterface = nullptr;
    }
#endif // USE_CODECHAL_DEBUG_TOOL

    // Destroy HW interface objects (GSH, SSH, etc)
    if (m_hwInterface != nullptr)
    {
        MOS_Delete(m_hwInterface);
        m_hwInterface = nullptr;
    }

    // Destroy OS interface objects (CBs, etc)
    if (m_osInterface != nullptr)
    {
        m_osInterface->pfnDestroy(m_osInterface, false);

        // Deallocate OS interface structure (except if externally provided)
        if (m_osInterface->bDeallocateOnExit)
        {
            MOS_FreeMemory(m_osInterface);
        }
    }

    MOS_TraceEvent(EVENT_CODECHAL_DESTROY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
}

MOS_STATUS Codechal::Allocate(CodechalSetting *codecHalSettings)
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;

    CODECHAL_PUBLIC_CHK_NULL_RETURN(codecHalSettings);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(m_osInterface);

    MOS_TraceEvent(EVENT_CODECHAL_CREATE,
                   EVENT_TYPE_INFO,
                   &codecHalSettings->codecFunction,
                   sizeof(uint32_t),
                   nullptr,
                   0);

    CODECHAL_PUBLIC_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecHalSettings));

    MOS_NULL_RENDERING_FLAGS nullHWAccelerationEnable;
    nullHWAccelerationEnable.Value = 0;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (!m_statusReportDebugInterface)
    {
        m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        CODECHAL_PUBLIC_CHK_NULL_RETURN(m_statusReportDebugInterface);
        CODECHAL_PUBLIC_CHK_STATUS_RETURN(
            m_statusReportDebugInterface->Initialize(m_hwInterface, codecHalSettings->codecFunction));
    }

    ReadUserSettingForDebug(
        m_userSettingPtr,
        nullHWAccelerationEnable.Value,
        __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE,
        MediaUserSetting::Group::Device);

    m_useNullHw[MOS_GPU_CONTEXT_VIDEO]         =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxVideo);
    m_useNullHw[MOS_GPU_CONTEXT_VIDEO2]        =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxVideo2);
    m_useNullHw[MOS_GPU_CONTEXT_VIDEO3]        =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxVideo3);
    m_useNullHw[MOS_GPU_CONTEXT_VDBOX2_VIDEO]  =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxVDBox2Video);
    m_useNullHw[MOS_GPU_CONTEXT_VDBOX2_VIDEO2] =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxVDBox2Video2);
    m_useNullHw[MOS_GPU_CONTEXT_VDBOX2_VIDEO3] =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxVDBox2Video3);
    m_useNullHw[MOS_GPU_CONTEXT_RENDER]        =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxRender);
    m_useNullHw[MOS_GPU_CONTEXT_RENDER2]       =
        (nullHWAccelerationEnable.CodecGlobal || nullHWAccelerationEnable.CtxRender2);
#endif // _DEBUG || _RELEASE_INTERNAL

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Codechal::BeginFrame()
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Codechal::EndFrame()
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Codechal::Reformat()
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Codechal::Execute(void *params)
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;

    CODECHAL_PUBLIC_CHK_NULL_RETURN(params);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_PUBLIC_CHK_NULL_RETURN(m_osInterface);
        CODECHAL_PUBLIC_CHK_NULL_RETURN(m_debugInterface);

        CODECHAL_PUBLIC_CHK_STATUS_RETURN(Mos_Solo_ForceDumps(
            m_debugInterface->m_bufferDumpFrameNum,
            m_osInterface));)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Codechal::GetStatusReport(
    void                *status,
    uint16_t            numStatus)
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;
    MOS_UNUSED(status);
    MOS_UNUSED(numStatus);
    CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported codec function requested.");
    return MOS_STATUS_UNKNOWN;
}

void Codechal::Destroy()
{
    CODECHAL_PUBLIC_FUNCTION_ENTER;
}

MOS_STATUS Codechal::ResolveMetaData(PMOS_RESOURCE pInput, PMOS_RESOURCE pOutput)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Codechal::ReportErrorFlag(PMOS_RESOURCE pMetadataBuffer, uint32_t size,
                                     uint32_t offset, uint32_t flag)
{
    return MOS_STATUS_SUCCESS;
}
