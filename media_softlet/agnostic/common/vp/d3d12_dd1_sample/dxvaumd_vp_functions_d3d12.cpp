/*===================== begin_copyright_notice ==================================

INTEL CONFIDENTIAL
Copyright 2020 - 2022
Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to the
source code ("Material") are owned by Intel Corporation or its suppliers or
licensors. Title to the Material remains with Intel Corporation or its suppliers
and licensors. The Material contains trade secrets and proprietary and confidential
information of Intel or its suppliers and licensors. The Material is protected by
worldwide copyright and trade secret laws and treaty provisions. No part of the
Material may be used, copied, reproduced, modified, published, uploaded, posted,
transmitted, distributed, or disclosed in any way without Intel's prior express
written permission.

No license under any patent, copyright, trade secret or other intellectual
property right is granted to or conferred upon you by disclosure or delivery
of the Materials, either expressly, by implication, inducement, estoppel
or otherwise. Any license under such intellectual property rights must be
express and approved by Intel in writing.

File Name: dxvaumd_vp_functions_d3d12.cpp

Abstract:
    DXVA User Mode Media Function VP Part d3d12

Environment:
    Windows RS

Notes:

======================= end_copyright_notice ==================================*/

#include "dxvaumd_vp_functions_d3d12.h"
#include "dxvacommandlist_d3d12.h"
#include "dxvacaps_d3d12.h"
#include "cp_protectedresourcesession_d3d12.h"
#include "UmdEscape.h"
#include "dxvaumd_vp_functions_common.h"
#include "ddi_escape.h"
#include "mos_command_allocator_dx12.h"
#include "mos_command_recorder_dx12.h"
#include "mos_postponed_command_packet_dx12.h"

void DdiVPFunctionsD3D12::registerCapsFunctions()
{
    DDI_VP_FUNCTION_ENTER;
    bool registerStatus = false;

    registerStatus = RegisterCapsFunctions(D3D12DDICAPS_TYPE_VIDEO_0020_PROCESS_SUPPORT, VideoProcessCapsSupport);
    registerStatus = RegisterCapsFunctions(D3D12DDICAPS_TYPE_VIDEO_0020_PROCESS_MAX_INPUT_STREAMS, VideoProcessCapsMaxInputStreams);
    registerStatus = RegisterCapsFunctions(D3D12DDICAPS_TYPE_VIDEO_0020_PROCESS_REFERENCE_INFO, VideoProcessCapsReferenceInfo);
}

VOID DdiVPFunctionsD3D12::VideoProcessFrame(
    D3D12DDI_HCOMMANDLIST                                        drvCommandList,
    D3D12DDI_HVIDEOPROCESSOR_0020                                drvVideoProcessor,
    const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputParameters,
    const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032  *inputStreamParameters,
    uint32_t                                                     numInputStreams)
{
    DDI_VP_FUNCTION_ENTER;

    VideoProcessFrameT(drvCommandList, drvVideoProcessor, outputParameters, inputStreamParameters, numInputStreams);
}

VOID DdiVPFunctionsD3D12::VideoProcessFrame(
    D3D12DDI_HCOMMANDLIST                                         drvCommandList,
    D3D12DDI_HVIDEOPROCESSOR_0020                                 drvVideoProcessor,
    const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032  *outputParameters,
    const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043   *inputStreamParameters,
    uint32_t                                                      numInputStreams)
{
    DDI_VP_FUNCTION_ENTER;

    VideoProcessFrameT(drvCommandList, drvVideoProcessor, outputParameters, inputStreamParameters, numInputStreams);
}

SIZE_T DdiVPFunctionsD3D12::CalcPrivateVideoProcessorSize(
    D3D12DDI_HDEVICE                              deviceD3D12,
    const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0032 *createVideoProcessor)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(createVideoProcessor, 0);

    return CalcPrivateVideoProcessorSize(deviceD3D12);
}

SIZE_T DdiVPFunctionsD3D12::CalcPrivateVideoProcessorSize(
    D3D12DDI_HDEVICE                              deviceD3D12,
    const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0043 *createVideoProcessor)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(createVideoProcessor, 0);

    return CalcPrivateVideoProcessorSize(deviceD3D12);
}

SIZE_T DdiVPFunctionsD3D12::CalcPrivateVideoProcessorSize(
    D3D12DDI_HDEVICE                              deviceD3D12,
    const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0072 *createVideoProcessor)
{
    HRESULT                         hr                     = S_OK;
    SIZE_T                          privateVideoProcSize   = 0;
    uint32_t                        transcryptedKernelSize = 0;
    CProtectedResourceSession12     *resourceSession       = nullptr;
    CpContext                       ctx                    = {0};

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(createVideoProcessor, 0);

    privateVideoProcSize = CalcPrivateVideoProcessorSize(deviceD3D12);

    resourceSession = CProtectedResourceSession12::GetProtectedResourceSession(createVideoProcessor->hDrvProtectedResourceSession);
    if (resourceSession)
    {
        ctx.value = resourceSession->GetCpTag();
        if (ctx.stoutMode)
        {
            // Increment required VP object size by size of transcrypted kernels
            // If kernels were not transcrypted, it will return a kernel size of 0
            transcryptedKernelSize = resourceSession->GetTranscryptedKernelSize();

            privateVideoProcSize += transcryptedKernelSize;
        }
    }
    return privateVideoProcSize;
}

HRESULT DdiVPFunctionsD3D12::CreateVideoProcessor(
    D3D12DDI_HDEVICE                              deviceD3D12,
    const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0032 *createVideoProcessor,
    D3D12DDI_HVIDEOPROCESSOR_0020                 videoProcessor)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);
    DDI_VP_FUNCTION_ENTER;

    return CreateVideoProcessorT(deviceD3D12, createVideoProcessor, videoProcessor);
}

HRESULT DdiVPFunctionsD3D12::CreateVideoProcessor(
    D3D12DDI_HDEVICE                              deviceD3D12,
    const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0043 *createVideoProcessor,
    D3D12DDI_HVIDEOPROCESSOR_0020                 videoProcessor)
{
    DDI_VP_FUNCTION_ENTER;

    return CreateVideoProcessorT(deviceD3D12, createVideoProcessor, videoProcessor);
}

HRESULT DdiVPFunctionsD3D12::CreateVideoProcessor(
    D3D12DDI_HDEVICE                              deviceD3D12,
    const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0072 *createVideoProcessor,
    D3D12DDI_HVIDEOPROCESSOR_0020                 videoProcessor)
{
    DDI_VP_FUNCTION_ENTER;

    return CreateVideoProcessorT(deviceD3D12, createVideoProcessor, videoProcessor);
}

VOID DdiVPFunctionsD3D12::DestroyVideoProcessor(
    D3D12DDI_HDEVICE              deviceD3D12,
    D3D12DDI_HVIDEOPROCESSOR_0020 videoProcessor)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    HRESULT                 hr              = E_FAIL;
    DWORD                   mediaResetCount = 0;
    PDXVA_VP_DATA           vpData          = nullptr;
    PMOS_CONTEXT            context         = GetDxvaDeviceContext(deviceD3D12);
    DXVA_FORMAT             dxvaFormat      = DXVA_FORMAT_NONE;
    DXVA_DECODE             dxvaDecode      = DXVA_DECODE_NONE;
    uint32_t                superResulationReport = 0;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(context);

    vpData = (PDXVA_VP_DATA)DRIVER_PRIVATE_HANDLE(videoProcessor);
    if (vpData)
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_TraceEventExt(EVENT_DDI_VP_DESTROY, EVENT_TYPE_INFO, &deviceD3D12, sizeof(void*), &vpData->pVpHalState, sizeof(void*));
#endif

        for (int i = 0; i < INTEL_MAX_STREAM_STATES; i++)
        {
            if (vpData->stmStatePalette[i].pEntries)
            {
                MOS_SafeFreeMemory(vpData->stmStatePalette[i].pEntries);
                MOS_SafeFreeMemory(vpData->vpeStreamState[i].pStmCpuGpuCopyParam);
                MOS_SafeFreeMemory(vpData->vpeStreamState[i].pScalingModeParam);
                vpData->stmStatePalette[i].pEntries            = nullptr;
                vpData->stmStatePalette[i].Count               = 0;
                vpData->vpeStreamState[i].pStmCpuGpuCopyParam  = nullptr;
                vpData->vpeStreamState[i].pScalingModeParam    = nullptr;
            }

            // Destroy FDFB params
            DdiVPFunctionsCommon::VpeFDFBParamsFree(&vpData->vpeStreamState[i]);
        }
        // Destroy VPHAL rendering parameters
        DdiVPFunctionsCommon::VpHalDdiReleaseRenderParamsExt(&vpData->VpHalRenderParams);
        if (vpData->pVpHalState)
        {
            // Destroy VPHAL
            MOS_Delete(vpData->pVpHalState);
            vpData->pVpHalState = nullptr;
        }

        // Free Camera Pipe Params
        MOS_SafeFreeMemory(vpData->vpeOutputState.pCamPipeParams);
        MOS_SafeFreeMemory(vpData->vpeOutputState.pRTCpuGpuCopyParam);
        MOS_SafeFreeMemory(vpData->vpeOutputState.p360StitchParams);
        vpData->vpeOutputState.pCamPipeParams      = nullptr;
        vpData->vpeOutputState.pRTCpuGpuCopyParam  = nullptr;
        vpData->vpeOutputState.p360StitchParams    = nullptr;

        DXVA_REG_StopListener(&vpData->CUIParams);
    }
#ifndef WDDM_LINUX
    DDIEscape::DDIEscapeMediaResetKmd(context, UMD_ESCAPE_DXVA_GET_MEDIA_RESET_COUNT, &mediaResetCount);
#endif

    ReportUserSetting(
        context->m_userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_COUNT,
        (uint32_t)mediaResetCount,
        MediaUserSetting::Group::Device);

    // clean SuperResolution report status
    if (vpData && vpData->CUIParams.DxvaUseStateSeparatedRegistryPaths)
    {
        ReportUserSetting(
            context->m_userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_CUI_SUPER_RESOLUTION_STATUS_REPORT,
            superResulationReport,
            MediaUserSetting::Group::Device,
            DXVA_REG_KEY_EXTERNAL_REPORT);
    }
    else
    {
        ReportUserSetting(
            context->m_userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_CUI_SUPER_RESOLUTION_STATUS_REPORT,
            superResulationReport,
            MediaUserSetting::Group::Device,
            DXVA_REG_KEY_EXTERNAL_REPORT_LEGACY);
    }

#ifndef WDDM_LINUX
    // we call an Escape here to notify the KMD that a DxVA app is getting detached
    hr = DDIEscape::DDINotifyEscapeKmd(context, FALSE, dxvaFormat, dxvaDecode);

    // if the 'D3DDDIERR_DEVICEREMOVED' code is returned by the Escape, it must be returned to the Run-Time (per WDK).
    if (hr == D3DDDIERR_DEVICEREMOVED)
    {
        return;
    }
#endif
    return;
}

HRESULT DdiVPFunctionsD3D12::VideoProcessCapsSupport(
    D3D12DDI_HDEVICE deviceD3D12,
    void             *data,
    uint32_t         dataSize)
{
    HRESULT                                            hr                     = S_OK;
    PMOS_CONTEXT                                       dxvaDeviceContext     = nullptr;
    PADAPTER_INFO                                      adapterInfo            = nullptr;
    D3D12DDI_VIDEO_PROCESS_SUPPORT_FLAGS_0022          *supportFlags          = nullptr;
    D3D12DDI_VIDEO_SCALE_SUPPORT_0032                  *scaleSupport          = nullptr;
    D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAGS_0020  *featureSupport        = nullptr;
    D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAGS_0020      *deinterlaceSupport    = nullptr;
    D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAGS_0022  *autoProcessingSupport = nullptr;
    D3D12DDI_VIDEO_PROCESS_FILTER_FLAGS_0020           *filterSupport         = nullptr;
    D3D12DDI_VIDEO_PROCESS_FILTER_RANGE_0020           *filterRangeSupport    = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(deviceD3D12.pDrvPrivate, E_INVALIDARG);

    auto processSupport = static_cast<D3D12DDI_VIDEO_PROCESS_SUPPORT_DATA_0032 *>(data);
    DDI_VP_CHK_NULL_RETURN_VALUE(processSupport, E_INVALIDARG);
    if (dataSize != sizeof(D3D12DDI_VIDEO_PROCESS_SUPPORT_DATA_0032))
    {
        DDI_VP_ASSERTMESSAGE("Invalid Parameters.");
        return E_INVALIDARG;
    }

    dxvaDeviceContext = GetDxvaDeviceContext(deviceD3D12);
    DDI_VP_CHK_NULL_RETURN_VALUE(dxvaDeviceContext, E_INVALIDARG);

    adapterInfo = DXVAUMD_ADAPTER_INFO(dxvaDeviceContext);
    DDI_VP_CHK_NULL_RETURN_VALUE(adapterInfo, E_INVALIDARG);

    // TODO: VPP caps implementation
    // Need to return the correct caps info depending on the input parameters

    supportFlags  = &processSupport->SupportFlags;
    *supportFlags = D3D12DDI_VIDEO_PROCESS_SUPPORT_FLAG_0022_SUPPORTED;

    scaleSupport                             = &processSupport->ScaleSupport;
    scaleSupport->OutputSizeRange.MaxWidth   = DXVAD3D12_1_MAX_OUTPUT_WIDTH;
    scaleSupport->OutputSizeRange.MaxHeight  = DXVAD3D12_1_MAX_OUTPUT_HEIGHT;
    scaleSupport->OutputSizeRange.MinWidth   = DXVAD3D12_1_MIN_OUTPUT_WIDTH;
    scaleSupport->OutputSizeRange.MinHeight  = DXVAD3D12_1_MIN_OUTPUT_HEIGHT;
    scaleSupport->Flags                      = D3D12DDI_VIDEO_SCALE_SUPPORT_FLAG_0022_NONE;

    featureSupport  = &processSupport->FeatureSupport;
    *featureSupport = (D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAGS_0020)(
        D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAG_0020_ALPHA_FILL |
        D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAG_0020_LUMA_KEY |
        D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAG_0020_ROTATION |  // Rotation is a WHQL requirement
        D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAG_0020_FLIP |
        D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAG_0020_ALPHA_BLENDING);

    if (GFX_IS_SKU(adapterInfo, FtrS3D))
    {
        *featureSupport = (D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAGS_0020)(
            *featureSupport | D3D12DDI_VIDEO_PROCESS_FEATURE_SUPPORT_FLAG_0020_STEREO);
    }

    deinterlaceSupport  = &processSupport->DeinterlaceSupport;
    *deinterlaceSupport = (D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAGS_0020)(
        D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAG_0020_BOB |
        D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAG_0020_CUSTOM);

    autoProcessingSupport  = &processSupport->AutoProcessingSupport;
    *autoProcessingSupport = (D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAGS_0022)(
        D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_DENOISE |
        //D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_DERINGING            |
        D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_EDGE_ENHANCEMENT |
        D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_COLOR_CORRECTION |
        D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_FLESH_TONE_MAPPING |
        //D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_IMAGE_STABILIZATION  |
        //D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_SUPER_RESOLUTION     |
        //D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_ANAMORPHIC_SCALING   |
        D3D12DDI_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_0022_CUSTOM);

    filterSupport  = &processSupport->FilterSupport;
    *filterSupport = (D3D12DDI_VIDEO_PROCESS_FILTER_FLAGS_0020)(
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_BRIGHTNESS |
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_CONTRAST |
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_HUE |
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_SATURATION |
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_NOISE_REDUCTION |
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_EDGE_ENHANCEMENT |
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_ANAMORPHIC_SCALING |
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_STEREO_ADJUSTMENT);

    filterRangeSupport                                                            = processSupport->FilterRangeSupport;
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_BRIGHTNESS].Minimum     = (int)(PROCAMP_BRIGHTNESS_MIN / PROCAMP_BRIGHTNESS_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_BRIGHTNESS].Maximum     = (int)(PROCAMP_BRIGHTNESS_MAX / PROCAMP_BRIGHTNESS_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_BRIGHTNESS].Default     = (int)(PROCAMP_BRIGHTNESS_DEFAULT / PROCAMP_BRIGHTNESS_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_BRIGHTNESS].Multiplier  = PROCAMP_BRIGHTNESS_STEP;

    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_CONTRAST].Minimum     = (int)(PROCAMP_CONTRAST_MIN / PROCAMP_CONTRAST_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_CONTRAST].Maximum     = (int)(PROCAMP_CONTRAST_MAX / PROCAMP_CONTRAST_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_CONTRAST].Default     = (int)(PROCAMP_CONTRAST_DEFAULT / PROCAMP_CONTRAST_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_CONTRAST].Multiplier  = PROCAMP_CONTRAST_STEP;

    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_HUE].Minimum     = (int)(PROCAMP_HUE_MIN / PROCAMP_HUE_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_HUE].Maximum     = (int)(PROCAMP_HUE_MAX / PROCAMP_HUE_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_HUE].Default     = (int)(PROCAMP_HUE_DEFAULT / PROCAMP_HUE_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_HUE].Multiplier  = PROCAMP_HUE_STEP;

    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_SATURATION].Minimum     = (int)(PROCAMP_SATURATION_MIN / PROCAMP_SATURATION_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_SATURATION].Maximum     = (int)(PROCAMP_SATURATION_MAX / PROCAMP_SATURATION_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_SATURATION].Default     = (int)(PROCAMP_SATURATION_DEFAULT / PROCAMP_SATURATION_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_SATURATION].Multiplier  = PROCAMP_SATURATION_STEP;

    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_NOISE_REDUCTION].Minimum     = (int)(NOISEREDUCTION_MIN / NOISEREDUCTION_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_NOISE_REDUCTION].Maximum     = (int)(NOISEREDUCTION_MAX / NOISEREDUCTION_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_NOISE_REDUCTION].Default     = (int)(NOISEREDUCTION_DEFAULT / NOISEREDUCTION_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_NOISE_REDUCTION].Multiplier  = NOISEREDUCTION_STEP;

    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_EDGE_ENHANCEMENT].Minimum     = (int)(EDGEENHANCEMENT_MIN / EDGEENHANCEMENT_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_EDGE_ENHANCEMENT].Maximum     = (int)(EDGEENHANCEMENT_MAX / EDGEENHANCEMENT_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_EDGE_ENHANCEMENT].Default     = (int)(EDGEENHANCEMENT_DEFAULT / EDGEENHANCEMENT_STEP);
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_EDGE_ENHANCEMENT].Multiplier  = EDGEENHANCEMENT_STEP;

    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_ANAMORPHIC_SCALING].Minimum     = 0;
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_ANAMORPHIC_SCALING].Maximum     = 1;
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_ANAMORPHIC_SCALING].Default     = 0;
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_ANAMORPHIC_SCALING].Multiplier  = 1;

    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_STEREO_ADJUSTMENT].Minimum     = 0;
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_STEREO_ADJUSTMENT].Maximum     = 15;
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_STEREO_ADJUSTMENT].Default     = 0;
    filterRangeSupport[D3D12DDI_VIDEO_PROCESS_FILTER_0020_STEREO_ADJUSTMENT].Multiplier  = 1;

    return hr;
}

const uint32_t DdiVPFunctionsD3D12::m_maxinputstreams = INTEL_MAX_INPUT_STREAMS;

HRESULT DdiVPFunctionsD3D12::VideoProcessCapsMaxInputStreams(
    D3D12DDI_HDEVICE deviceD3D12,
    void             *data,
    uint32_t         dataSize)
{
    DDI_VP_FUNCTION_ENTER;

    auto maxInputStreams = static_cast<D3D12DDI_VIDEO_PROCESS_MAX_INPUT_STREAMS_DATA_0020 *>(data);
    DDI_VP_CHK_NULL_RETURN_VALUE(maxInputStreams, 0);

    maxInputStreams->MaxInputStreams = m_maxinputstreams;
    return S_OK;
}

HRESULT DdiVPFunctionsD3D12::VideoProcessCapsReferenceInfo(
    D3D12DDI_HDEVICE deviceD3D12,
    void             *data,
    uint32_t         dataSize)
{
    HRESULT              hr                = S_OK;
    PMOS_CONTEXT         dxvaDeviceContext = nullptr;
    PADAPTER_INFO        adapterInfo       = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(deviceD3D12.pDrvPrivate, E_INVALIDARG);

    auto processRefInfo = static_cast<D3D12DDI_VIDEO_PROCESS_REFERENCE_INFO_DATA_0020 *>(data);
    DDI_VP_CHK_NULL_RETURN_VALUE(processRefInfo, E_INVALIDARG);
    if (dataSize != sizeof(D3D12DDI_VIDEO_PROCESS_REFERENCE_INFO_DATA_0020))
    {
        DDI_VP_ASSERTMESSAGE("Invalid Parameters.");
        return E_INVALIDARG;
    }

    dxvaDeviceContext = GetDxvaDeviceContext(deviceD3D12);
    DDI_VP_CHK_NULL_RETURN_VALUE(dxvaDeviceContext, E_INVALIDARG);

    adapterInfo = DXVAUMD_ADAPTER_INFO(dxvaDeviceContext);
    DDI_VP_CHK_NULL_RETURN_VALUE(adapterInfo, E_INVALIDARG);

#if (_DEBUG || _RELEASE_INTERNAL)

    DDI_VP_VERBOSEMESSAGE("VideoProcessCapsReferenceInfo:: DeinterlaceMode 0x%08x, Filters 0x%08x, InputFrameRate[%d/%d], OutputFrameRate[%d/%d], FeatureSupport 0x%08x, EnableAutoProcessing %d",
        processRefInfo->DeinterlaceMode,
        processRefInfo->Filters,
        processRefInfo->InputFrameRate.Numerator,
        processRefInfo->InputFrameRate.Denominator,
        processRefInfo->OutputFrameRate.Numerator,
        processRefInfo->OutputFrameRate.Denominator,
        processRefInfo->FeatureSupport,
        processRefInfo->EnableAutoProcessing);

#endif

    processRefInfo->FutureFrames = 0;
    processRefInfo->PastFrames   = 0;

    //Disable DX12 for MPC-HC due to blankout issue
    //TO-DO: remove this after blankout issue resolved
    if ((nullptr != ::GetModuleHandle(TEXT("mpc-hc.exe"))) ||
        (nullptr != ::GetModuleHandle(TEXT("mpc-hc64.exe"))))
    {
        processRefInfo->FutureFrames = 3;
        processRefInfo->PastFrames   = 0;
    }

    if (IS_D3D12_INTERFACE_VERSION_RS4(dxvaDeviceContext->GetAdapterInfo()))
    {
        if (processRefInfo->DeinterlaceMode & D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAG_0020_CUSTOM)
        {
            // In RS4, the future/past reference issue was fixed by MSFT runtime.
            processRefInfo->PastFrames = 1;
        }
    }

    return hr;
}

void DdiVPFunctionsD3D12::CheckFormatSupport(
    D3D12DDI_HDEVICE deviceD3D12,
    DXGI_FORMAT      format,
    uint32_t         *formatCaps)
{
    PMOS_CONTEXT         context     = nullptr;
    PADAPTER_INFO        adapterInfo = nullptr;
    uint32_t             fP16Enable  = 0;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(formatCaps);

    context     = GetDxvaDeviceContext(deviceD3D12);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(context);
    adapterInfo = DXVAUMD_ADAPTER_INFO(context);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(adapterInfo);

    switch (format)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM:        // VP Input/Output requirement
        case DXGI_FORMAT_B8G8R8A8_UNORM:        // VP Input/Output requirement
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   // VP Input/Output requirement
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   // VP Input/Output requirement
        case DXGI_FORMAT_R16G16B16A16_UNORM:    // VP Input/Output requirement
        case DXGI_FORMAT_B8G8R8X8_UNORM:        // VP Input/Output requirement
        case DXGI_FORMAT_AYUV:                  // VP Input/Output requirement
        case DXGI_FORMAT_NV12:                  // VP Input/Output requirement
        case DXGI_FORMAT_YUY2:                  // VP Input/Output requirement
        case DXGI_FORMAT_420_OPAQUE:            // VP Input/Output requirement Cover the JPEG YUV formats that are not included in the DXGI list.
        case DXGI_FORMAT_R10G10B10A2_UNORM:     // VP Input/Output requirement
        case DXGI_FORMAT_P016:                  // VP Input/Output requirement
        case DXGI_FORMAT_P010:
        {
            *formatCaps |= (D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT | D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT);
            break;
        }
        case DXGI_FORMAT_AI44:                  // VP Input requirement
        case DXGI_FORMAT_IA44:                  // VP Input requirement
        case DXGI_FORMAT_P8:                    // VP Input requirement
        case DXGI_FORMAT_A8P8:                  // VP Input requirement
        if (!GFX_IS_SKU(adapterInfo, FtrPaletteRemove))
        {
            *formatCaps |= (D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT);
        }
        break;
        case DXGI_FORMAT_R16_TYPELESS:          // VP Camera Pipe Input requirement
        case DXGI_FORMAT_R8_TYPELESS:           // VP Camera Pipe Input requirement
        {
            *formatCaps |= (D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT);
            break;
        }
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_Y210:
        {
            // Starting from Gen11, VP support Y210/410 input/output
            if (GFX_IS_SKU(adapterInfo, FtrVp10BitSupport))
            {
                *formatCaps |= (D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT | D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT);
            }
            break;
        }
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y216:
        {
            // Starting from Gen12, VP support Y216/416 input/output
            if (GFX_IS_SKU(adapterInfo, FtrVp16BitSupport))
            {
                *formatCaps |= (D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT | D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT);
            }
            break;
        }
        case DXGI_FORMAT_R16G16B16A16_FLOAT:    // FP16 input/output
        {
            ReadUserSetting(
                context->m_userSettingPtr,
                fP16Enable,
                __DXVA_UF_VALUE_POSTPROC_FP16_ENABLE,
                MediaUserSetting::Group::Device);

            if (fP16Enable)
            {
                *formatCaps |= (D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT | D3D12DDI_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT);
            }
            break;
        }
    }
    return;
}

SIZE_T DdiVPFunctionsD3D12::CalcPrivateVideoProcessorSize(
    D3D12DDI_HDEVICE deviceD3D12)
{
    PMOS_CONTEXT dxvaDeviceContext = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(deviceD3D12.pDrvPrivate, 0);

    dxvaDeviceContext = GetDxvaDeviceContext(deviceD3D12);
    DDI_VP_CHK_NULL_RETURN_VALUE(dxvaDeviceContext, 0);

    return sizeof(DXVA_VP_DATA);
}

template <class T>
HRESULT DdiVPFunctionsD3D12::CreateVideoProcessorT(
    D3D12DDI_HDEVICE              deviceD3D12,
    const T                       *createVideoProcessor,
    D3D12DDI_HVIDEOPROCESSOR_0020 videoProcessor)
{
    HRESULT              hr                    = S_OK;
    PDXVA_VP_DATA        vpData                = nullptr;
    VpBase               *vpHal                = nullptr;
    VpSettingsExt        vpHalSettings         = {};
    DXVA_FORMAT          dxvaFormat            = DXVA_FORMAT_NONE;
    DXVA_DECODE          dxvaDecode            = DXVA_DECODE_NONE;
    PADAPTER_INFO        adapterInfo           = nullptr;
    DWORD                laceInUseReportedData = 0;
    PMOS_CONTEXT         context               = GetDxvaDeviceContext(deviceD3D12);
    MOS_STATUS           status                = MOS_STATUS_UNKNOWN;
    DWORD                mediaResetCount       = 0;
    int32_t              memninjaCounter       = 0;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(createVideoProcessor);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(context);

    vpData = (PDXVA_VP_DATA)DRIVER_PRIVATE_HANDLE(videoProcessor);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosUtilities::m_mosMemAllocCounter &&
        MosUtilities::m_mosMemAllocCounterGfx &&
        MosUtilities::m_mosMemAllocFakeCounter)
    {
        memninjaCounter = *MosUtilities::m_mosMemAllocCounter + *MosUtilities::m_mosMemAllocCounterGfx - *MosUtilities::m_mosMemAllocFakeCounter;
        ReportUserSettingForDebug(
            context->m_userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_MEMORY_NINJA_BEGIN_COUNTER,
            memninjaCounter,
            MediaUserSetting::Group::Device);
        DDI_VP_NORMALMESSAGE("Create videoprocessor begin.");
    }
#endif

    // initialize it
    MOS_ZeroMemory(vpData, sizeof(DXVA_VP_DATA));

    // Initialize VPP registry values
    DdiVPFunctionsCommon::InitializeVPConfigValues(context->m_userSettingPtr, &vpData->ConfigValues, VP_PATH_POST_PROCESSING, VP_COMPONENT_DXVA_DX12);

    vpData->dwLACEInUseReported                   = DXVA_REGISTRY_NOT_REPORTED;

    // init VPE data, set VPE mode, version to VPE_MODE_NONE, VERSION_UNKNOWN
    // This is to enforce App to specify version explicitly
    vpData->vpeOutputState.uVersion = VPE_VERSION_UNKNOWN;
    vpData->vpeOutputState.uMode    = VPE_MODE_NONE;

    adapterInfo = DXVAUMD_ADAPTER_INFO(context);
    DDI_VP_CHK_NULL_RETURN_VALUE(adapterInfo, E_FAIL);

    //-------------------------------------------------------------------------
    // VPHAL allocation
    //-------------------------------------------------------------------------
    vpHal = VpBase::VphalStateFactory(nullptr, context, &status);

    if (vpHal && MOS_FAILED(status))
    {
        DDI_VP_ASSERTMESSAGE("Failed to create VPHAL States.");
        // Resource allocation failed - Destroy VPHAL
        MOS_Delete(vpHal);
        vpHal = nullptr;
    }
    DDI_VP_CHK_NULL_RETURN_VALUE(vpHal, E_OUTOFMEMORY);

    DdiVPFunctionsCommon::PrepareVpHalSetting(context->m_userSettingPtr, &vpHalSettings);

    // Allocate resources (state heaps, resources, KDLL)
    if (MOS_FAILED(vpHal->Allocate(&vpHalSettings)))
    {
        DDI_VP_ASSERTMESSAGE("Failed to create VPHAL States.");

        // Resource allocation failed - Destroy VPHAL
        MOS_Delete(vpHal);
        vpHal = nullptr;
        MT_ERR2(MT_VP_BLT, MT_VP_HAL_PTR, 0, MT_ERROR_CODE, E_OUTOFMEMORY);
        return E_OUTOFMEMORY;
    }

    DdiVPFunctionsCommon::GetChromaticityValues(context, &vpData->ChromaticityTbl);

    vpHal->m_gpuAppTaskEvent = nullptr;
    vpData->pVpHalState      = vpHal;

    //-------------------------------------------------------------------------
    // VPHAL initialization
    //-------------------------------------------------------------------------
    if (InitDdiVphal(vpData, createVideoProcessor, adapterInfo) == E_INVALIDARG)
    {
        MOS_Delete(vpHal);
        vpHal = nullptr;
        return E_INVALIDARG;
    }

    // TODO: handle output format
    // This info can be obtained from GMM

    // Set output color space
    vpData->bltStateOutputColorSpace   = ConvertColorSpace(createVideoProcessor->OutputStream.ColorSpace);
    vpData->bltStateOutputChromaSiting = ColorSpace2ToChromaSiting(createVideoProcessor->OutputStream.ColorSpace);
    vpData->bltStateOutputGammaValue   = ColorSpace2ToGammaValue(createVideoProcessor->OutputStream.ColorSpace, &(vpData->stmStateOutputHDRState));

    // Set alpha fill mode
    vpData->bltStateAlphaFillData.Mode         = createVideoProcessor->OutputStream.AlphaFillMode;
    vpData->bltStateAlphaFillData.StreamNumber = createVideoProcessor->OutputStream.AlphaFillModeSourceStreamIndex;

    // Set background color
    SetBackGroundColor(&(vpData->bltStateBackgroundColor), &(createVideoProcessor->OutputStream));

    // Set output frame rate
    vpData->outputFrameRate.Numerator   = createVideoProcessor->OutputStream.FrameRate.Numerator;
    vpData->outputFrameRate.Denominator = createVideoProcessor->OutputStream.FrameRate.Denominator;

    // Set stereo enabling
    if (SetStereoEnabling(createVideoProcessor->OutputStream.EnableStereo, vpData, adapterInfo) == E_INVALIDARG)
    {
        MOS_Delete(vpHal);
        vpHal = nullptr;
        return E_INVALIDARG;
    }

    InitVpdataFromRegistryKey(context->m_userSettingPtr, vpData, &mediaResetCount);

#ifndef WDDM_LINUX
    // we call an Escape here to notify the KMD that a DxVA app is getting attached
    hr = DDIEscape::DDINotifyEscapeKmd(context, TRUE, dxvaFormat, dxvaDecode);
    DDIEscape::DDIEscapeMediaResetKmd(context, UMD_ESCAPE_DXVA_GET_MEDIA_RESET_COUNT, &mediaResetCount);
#endif
    // if the 'D3DDDIERR_DEVICEREMOVED' code is returned by the Escape, it must be returned to the Run-Time (per WDK).
    if (hr == D3DDDIERR_DEVICEREMOVED)
    {
        MT_ERR2(MT_ERR_HR_CHECK, MT_VP_HAL_PTR, (int64_t)vpHal, MT_ERROR_CODE, hr);
        return hr;
    }

    // CUI will use state seperation path from  dGPU instead of the previous path HKLM\SOFTWARE\Intel\Display\igfxcui\MediaKeys
    // e.g. the path HKLM\SYSTEM\ControlSet001\Control\Class\{4d36e968-e325-11ce-bfc1-08002be10318}\0000\Display\igfxcui\MediaKeys
    {
        // ADL still uses the previous path HKLM\SOFTWARE\Intel\Display\igfxcui\MediaKeys
        if (GFX_IS_SKU(adapterInfo, FtrStateSeparation))
        {
            vpData->CUIParams.DxvaUseStateSeparatedRegistryPaths       = 1;
            vpData->CUIParams.DxvaFeatureRegistryKeyPathInfo           = *context->GetDeviceUfPathInfo();
        }
        else
        {
            vpData->CUIParams.DxvaUseStateSeparatedRegistryPaths = 0;
        }
    }

    // Initialize CUI
    DXVA_REG_CreateListener(&vpData->CUIParams, &vpData->CUIRegistryValues);

#if (_DEBUG || _RELEASE_INTERNAL)
    {
        if (MosUtilities::m_mosMemAllocCounter &&
            MosUtilities::m_mosMemAllocCounterGfx &&
            MosUtilities::m_mosMemAllocFakeCounter)
        {
            memninjaCounter = *MosUtilities::m_mosMemAllocCounter + *MosUtilities::m_mosMemAllocCounterGfx - *MosUtilities::m_mosMemAllocFakeCounter;
            ReportUserSettingForDebug(
                context->m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_MEMORY_NINJA_END_COUNTER,
                memninjaCounter,
                MediaUserSetting::Group::Device);
            DDI_VP_NORMALMESSAGE("Create videoprocessor End.");
        }
    }

    {
        DXVA_TRACE_EVENTDATA_DEVICE eventData = {};

        DXVA_TRACE_EVENTDATA_DEVICE_INIT(eventData, (ULONG_PTR)DRIVER_PRIVATE_HANDLE(deviceD3D12), (ULONG_PTR)vpHal, 0, 0, 0, (context ? context->m_platform.eProductFamily : 0));
        MOS_TraceEventExt(EVENT_DDI_VP_CREATE, EVENT_TYPE_INFO, &eventData, sizeof(eventData), nullptr, 0);
    }

    {
        ReadUserSettingForDebug(
            context->m_userSettingPtr,
            vpData->ConfigValues.dwForceColorFill,
            __MEDIA_USER_FEATURE_DBG_FORCE_COLOR_FILL,
            MediaUserSetting::Group::Device);
    }
#endif

    return hr;
}

BOOL DdiVPFunctionsD3D12::CheckMemoryBlockNotZero(char *addr, uint32_t count)
{
    DDI_VP_FUNCTION_ENTER;

    uint32_t target = 0;

    for (uint32_t i = 0; i < count; ++i)
    {
        target |= (uint32_t)(*(addr + i));
    }

    return target ? 1 : 0;
}

D3D11_1DDI_STREAM_STATE_COLOR_SPACE DdiVPFunctionsD3D12::ConvertColorSpace(
    DXGI_COLOR_SPACE_TYPE colorSpace)
{
    D3D11_1DDI_STREAM_STATE_COLOR_SPACE colorSpaceRet = {};
    DDI_VP_FUNCTION_ENTER;

    colorSpaceRet.Usage                 = 0;               // 0: Playback; 1: VP
    colorSpaceRet.RGB_Range             = 0;               // RGB - 0: Full Range; 1: 16-235
    colorSpaceRet.YCbCr_Matrix          = 0;               // 0: BT.601; 1: BT.709
    colorSpaceRet.YCbCr_xvYCC           = 0;               // 0: Conventional YCbCr; 1: Extend YCbCr
    colorSpaceRet.Nominal_Range         = 0;               // Luma - 0: Studio for Luma
    colorSpaceRet.Reserved              = 0;
    colorSpaceRet.bBT2020               = FALSE;
    colorSpaceRet.bBT2020_FullRange     = FALSE;

    switch (colorSpace)
    {
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601:
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601:
            colorSpaceRet.Nominal_Range = D3D11_1DDI_VIDEO_PROCESSOR_NOMINAL_RANGE_0_255;
            colorSpaceRet.YCbCr_Matrix  = 0;
            break;
        case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
        case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:
            colorSpaceRet.RGB_Range    = 0;
            colorSpaceRet.YCbCr_Matrix = 1;
            break;
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709:
            colorSpaceRet.Nominal_Range = D3D11_1DDI_VIDEO_PROCESSOR_NOMINAL_RANGE_0_255;
            colorSpaceRet.YCbCr_Matrix  = 1;
            break;
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601:
            colorSpaceRet.Nominal_Range = D3D11_1DDI_VIDEO_PROCESSOR_NOMINAL_RANGE_16_235;
            colorSpaceRet.YCbCr_Matrix  = 0;
            break;
        case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709:
            colorSpaceRet.RGB_Range    = 1;
            colorSpaceRet.YCbCr_Matrix = 1;
            break;
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709:
            colorSpaceRet.Nominal_Range = D3D11_1DDI_VIDEO_PROCESSOR_NOMINAL_RANGE_16_235;
            colorSpaceRet.YCbCr_Matrix  = 1;
            break;
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020:
        case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
            colorSpaceRet.bBT2020_FullRange = TRUE;
            colorSpaceRet.bBT2020           = TRUE;
            break;
        case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020:
        case DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020:
            colorSpaceRet.bBT2020_FullRange = FALSE;
            colorSpaceRet.bBT2020           = TRUE;
            break;
        default:
            break;
    }

    return colorSpaceRet;
}

uint32_t DdiVPFunctionsD3D12::ColorSpace2ToChromaSiting(
    DXGI_COLOR_SPACE_TYPE colorSpace)
{
    uint32_t chromaSiting = CHROMA_SITING_NONE;
    DDI_VP_FUNCTION_ENTER;

    switch (colorSpace)
    {
        //For RGB color space, chroma sitting is meaningless.
        case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
        case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:
        case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709:
        case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020:
        case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
        case DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020:
            break;
            // None means siting at center-center
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601:
            chromaSiting = CHROMA_SITING_HORZ_CENTER | CHROMA_SITING_VERT_CENTER;
            break;
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601:
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020:
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020:
            chromaSiting = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER;
            break;
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020:
            chromaSiting = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
            break;
        default:
            break;
    }

    return chromaSiting;
}

VPHAL_GAMMA_VALUE DdiVPFunctionsD3D12::ColorSpace2ToGammaValue(
    DXGI_COLOR_SPACE_TYPE   colorSpace,
    VPE_HDR_STATE           *hdrState)
{
    VPHAL_GAMMA_VALUE gammaValue = GAMMA_2P2;     // Commonly used for sRGB and BT.709 according to <<Graphics - Video DDI Changes>> on Win10 released by MS
    DDI_VP_FUNCTION_ENTER;

    switch (colorSpace)
    {
        case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:
            gammaValue = GAMMA_1P0;
            break;
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601:
        case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
        case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709:
        case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020:
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601:
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020:
        case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020:
            gammaValue = GAMMA_2P2;
            if (hdrState)
            {
                hdrState->HDRMetaData.EOTF = VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR;
            }
            break;
        // Currently for G2084 new color space emun, driver only does BT2020 YUV->RGB CSC
        // So put them here in case we need to use them in the future.
        case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020:
        case DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020:
        case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020:
            if (hdrState)
            {
                hdrState->HDRMetaData.EOTF = VPHAL_HDR_EOTF_SMPTE_ST2084;
            }
            break;
        default:
            break;
    }

    return gammaValue;
}

bool DdiVPFunctionsD3D12::CheckRGBFormat(DXGI_COLOR_SPACE_TYPE colorSpace)
{
    DDI_VP_FUNCTION_ENTER;

    static const std::vector<DXGI_COLOR_SPACE_TYPE> colorSpaceArray =
        {
            DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,
            DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709,
            DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709,
            DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020,
            DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020,
            DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020,
            DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020};
    auto iter = std::find(colorSpaceArray.begin(), colorSpaceArray.end(), colorSpace);
    if (iter != colorSpaceArray.end())
    {
        return true;
    }

    return false;
}

template <class T>
HRESULT DdiVPFunctionsD3D12::InitDdiVphal(
    PDXVA_VP_DATA vpData,
    const T       *createVideoProcessor,
    PADAPTER_INFO adapterInfo)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(createVideoProcessor);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(adapterInfo);

    for (uint32_t iStream = 0; iStream < createVideoProcessor->NumInputStreams; iStream++)
    {
        // TODO: handle input format
        // This info can be obtained from GMM

        // Set input color space
        vpData->stmStateInputColorSpace[iStream]   = ConvertColorSpace(createVideoProcessor->pInputStreams[iStream].ColorSpace);
        vpData->stmStateInputChromaSiting[iStream] = ColorSpace2ToChromaSiting(createVideoProcessor->pInputStreams[iStream].ColorSpace);
        vpData->stmStateInputGammaValue[iStream]   = ColorSpace2ToGammaValue(createVideoProcessor->pInputStreams[iStream].ColorSpace, &(vpData->stmStateInputHDRState[iStream]));

        // Set aspect ratio
        if (CheckMemoryBlockNotZero((char *)&createVideoProcessor->pInputStreams[iStream].SourceAspectRatio, sizeof(DXGI_RATIONAL)) ||
            CheckMemoryBlockNotZero((char *)&createVideoProcessor->pInputStreams[iStream].DestinationAspectRatio, sizeof(DXGI_RATIONAL)))
        {
            vpData->stmStateAspectRatio[iStream].Enable                 = 1;
            vpData->stmStateAspectRatio[iStream].SourceAspectRatio      = createVideoProcessor->pInputStreams[iStream].SourceAspectRatio;
            vpData->stmStateAspectRatio[iStream].DestinationAspectRatio = createVideoProcessor->pInputStreams[iStream].DestinationAspectRatio;
        }
        else
        {
            vpData->stmStateAspectRatio[iStream].Enable = 0;
        }

        // Set frame rate
        vpData->stmStateOutputRate[iStream].CustomRate = createVideoProcessor->pInputStreams[iStream].FrameRate;

        // TODO: handle source & destination size range

        // Set orientation
        vpData->stmStateReserved[iStream].EnableOrientation = createVideoProcessor->pInputStreams[iStream].EnableOrientation;

        // Set filter flags
        DDI_VP_CHK_HR_RETURN(SetFilterFlags(vpData, createVideoProcessor->pInputStreams[iStream].FilterFlags, adapterInfo, iStream));

        // Set stereo format
        if (createVideoProcessor->pInputStreams[iStream].StereoFormat != D3D12DDI_VIDEO_FRAME_STEREO_FORMAT_0020_MONO)
        {
            if (GFX_IS_SKU(adapterInfo, FtrS3D))
            {
                vpData->stmStateStereoFormat[iStream].Enable = 1;
                vpData->stmStateStereoFormat[iStream].Format = (D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT)createVideoProcessor->pInputStreams[iStream].StereoFormat;
            }
            else
            {
                DDI_VP_ASSERTMESSAGE("S3D is not supported in this platform.");
                return E_INVALIDARG;
            }
        }
        else
        {
            vpData->stmStateStereoFormat[iStream].Enable = 0;
        }

        // Set field type
        SetFieldType(vpData, createVideoProcessor->pInputStreams, iStream);

        // Set deinterlace mode
        vpData->stmStateReserved[iStream].DeinterlaceMode = createVideoProcessor->pInputStreams[iStream].DeinterlaceMode;

        // Set alpha blending
        vpData->stmStateAlpha[iStream].Enable = createVideoProcessor->pInputStreams[iStream].EnableAlphaBlending;

        // Set luma key
        vpData->stmStateLumaKey[iStream].Enable = createVideoProcessor->pInputStreams[iStream].LumaKey.Enable;
        vpData->stmStateLumaKey[iStream].Lower  = createVideoProcessor->pInputStreams[iStream].LumaKey.Lower;
        vpData->stmStateLumaKey[iStream].Upper  = createVideoProcessor->pInputStreams[iStream].LumaKey.Upper;

        // TODO: handle past frames number
        // This info can be obtained from input stream argument

        // TODO: handle future frames number
        // This info can be obtained from input stream argument

        // Set auto processing mode
        vpData->stmStateAutoProcessingMode[iStream].Enable = createVideoProcessor->pInputStreams[iStream].EnableAutoProcessing;

        // Set input stream PastFrame/Future Number
        vpData->uiInputStreamNumPastFrames[iStream]   = createVideoProcessor->pInputStreams->NumPastFrames;
        vpData->uiInputStreamNumFutureFrames[iStream] = createVideoProcessor->pInputStreams->NumFutureFrames;
    }
    return S_OK;
}

void DdiVPFunctionsD3D12::SetFieldType(
    PDXVA_VP_DATA                                         vpData,
    const D3D12DDI_VIDEO_PROCESSOR_INPUT_STREAM_DESC_0032 *inputStreams,
    uint32_t                                              stream)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);

    vpData->stmStateFrameFormat[stream] = inputStreams[stream].FieldType;
    return;
}

template <class T>
void DdiVPFunctionsD3D12::SetFieldType(
    PDXVA_VP_DATA vpData,
    const T       *inputStreams,
    uint32_t      stream)
{
    DDI_VP_FUNCTION_ENTER;
    return;
}

void DdiVPFunctionsD3D12::SetFieldType(
    PDXVA_VP_DATA                                               vpData,
    const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 *inputStreamParameters,
    uint32_t                                                    istream)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);

    vpData->stmStateFrameFormat[istream] = inputStreamParameters[istream].FieldType;
    return;
}

HRESULT DdiVPFunctionsD3D12::SetFilterFlags(
    PDXVA_VP_DATA                            vpData,
    D3D12DDI_VIDEO_PROCESS_FILTER_FLAGS_0020 filterFlags,
    PADAPTER_INFO                            adapterInfo,
    uint32_t                                 stream)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(adapterInfo);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);

    if (!filterFlags)
    {
        return S_OK;
    }

    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_BRIGHTNESS)
    {
        vpData->stmStateFilterBrightness[stream].Enable = 1;
    }
    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_CONTRAST)
    {
        vpData->stmStateFilterContrast[stream].Enable = 1;
    }
    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_HUE)
    {
        vpData->stmStateFilterHue[stream].Enable = 1;
    }
    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_SATURATION)
    {
        vpData->stmStateFilterSaturation[stream].Enable = 1;
    }
    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_NOISE_REDUCTION)
    {
        vpData->stmStateFilterNoiseReduction[stream].Enable = 1;
    }
    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_EDGE_ENHANCEMENT)
    {
        vpData->stmStateFilterEdgeEnhancement[stream].Enable = 1;
    }
    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_ANAMORPHIC_SCALING)
    {
        vpData->stmStateFilterAnamorphicScaling[stream].Enable = 1;
    }
    if (filterFlags & D3D12DDI_VIDEO_PROCESS_FILTER_FLAG_0020_STEREO_ADJUSTMENT)
    {
        if (GFX_IS_SKU(adapterInfo, FtrS3D))
        {
            vpData->stmStateFilterStereoAdjustment[stream].Enable = 1;
        }
        else
        {
            DDI_VP_ASSERTMESSAGE("S3D is not supported in this platform.");
            return E_INVALIDARG;
        }
    }
    return S_OK;
}

void DdiVPFunctionsD3D12::SetBackGroundColor(
    INTEL_BLT_STATE_BACKGROUND_COLOR_DATA                *bltStateBackgroundColor,
    const D3D12DDI_VIDEO_PROCESS_OUTPUT_STREAM_DESC_0032 *outputStream)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(bltStateBackgroundColor);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(outputStream);

    if (CheckRGBFormat(outputStream->ColorSpace))
    {
        bltStateBackgroundColor->BackgroundColor.RGBA.R = outputStream->BackgroundColor[0];
        bltStateBackgroundColor->BackgroundColor.RGBA.G = outputStream->BackgroundColor[1];
        bltStateBackgroundColor->BackgroundColor.RGBA.B = outputStream->BackgroundColor[2];
        bltStateBackgroundColor->BackgroundColor.RGBA.A = outputStream->BackgroundColor[3];
        bltStateBackgroundColor->YCbCr                  = 0;
    }
    else
    {
        bltStateBackgroundColor->BackgroundColor.YCbCr.Y  = outputStream->BackgroundColor[0];
        bltStateBackgroundColor->BackgroundColor.YCbCr.Cb = outputStream->BackgroundColor[1];
        bltStateBackgroundColor->BackgroundColor.YCbCr.Cr = outputStream->BackgroundColor[2];
        bltStateBackgroundColor->BackgroundColor.YCbCr.A  = outputStream->BackgroundColor[3];
        bltStateBackgroundColor->YCbCr                    = 1;
    }
    return;
}

HRESULT DdiVPFunctionsD3D12::SetStereoEnabling(
    int            enableStereo,
    PDXVA_VP_DATA  vpData,
    PADAPTER_INFO  adapterInfo)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(adapterInfo);

    if (enableStereo)
    {
        if (GFX_IS_SKU(adapterInfo, FtrS3D))
        {
            vpData->bltStereoMode = enableStereo;
        }
        else
        {
            DDI_VP_ASSERTMESSAGE("S3D is not supported in this platform.");
            return E_INVALIDARG;
        }
    }
    else
    {
        vpData->bltStereoMode = 0;
    }
    return S_OK;
}

void DdiVPFunctionsD3D12::InitVpdataFromRegistryKey(
    MediaUserSettingSharedPtr userSettingPtr,
    PDXVA_VP_DATA             vpData,
    PDWORD                    mediaResetCount)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(mediaResetCount);

    ReadUserSetting(
        userSettingPtr,
        vpData->dwForceLaceToEanble,
        __DXVA_UF_VALUE_FORCE_LACE_TO_ENABLE,
        MediaUserSetting::Group::Device,
        vpData->dwForceLaceToEanble,
        true,
        MEDIA_USER_SETTING_INTERNAL);

    ReadUserSetting(
        userSettingPtr,
        *mediaResetCount,
        __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_COUNT,
        MediaUserSetting::Group::Device,
        (uint32_t)*mediaResetCount,
        true,
        MEDIA_USER_SETTING_INTERNAL);

    return;
}

template <class T>
void DdiVPFunctionsD3D12::VideoProcessFrameT(
    D3D12DDI_HCOMMANDLIST                                        drvCommandList,
    D3D12DDI_HVIDEOPROCESSOR_0020                                drvVideoProcessor,
    const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputParameters,
    const T                                                      *inputStreamParameters,
    uint32_t                                                     numInputStreams)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    HRESULT                     hr                        = E_INVALIDARG;
    PDXVA_VP_DATA               vpData                    = nullptr;
    PMOS_INTERFACE              osInterface               = nullptr;
    PMOS_CONTEXT                dxvaDevice                = nullptr;
    PADAPTER_INFO               adapterInfo               = nullptr;
    DXVA12_COMMAND_LIST         *commandList              = nullptr;
    DXVAUMD_RESOURCE            resPredicationDdiResource = nullptr;
    DXVAUMD_RESOURCE            resSetMarkerDdiResource   = nullptr;
    MOS_RESOURCE                resPredication            = {};
    MOS_RESOURCE                resSetMarker              = {};
    uint32_t                    hwVPPBypassed             = 0;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(drvVideoProcessor.pDrvPrivate);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(drvCommandList.pDrvPrivate);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(outputParameters);

    vpData = (PDXVA_VP_DATA)DRIVER_PRIVATE_HANDLE(drvVideoProcessor);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData->pVpHalState);

    osInterface = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(osInterface);

    dxvaDevice = osInterface->pOsContext;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(dxvaDevice);

    adapterInfo = DXVAUMD_ADAPTER_INFO(dxvaDevice);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(adapterInfo);

    commandList = DXVAUMD_OBJ::GetDxvaObj<DXVA12_COMMAND_LIST, D3D12DDI_HCOMMANDLIST>(drvCommandList);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(commandList);

    DDI_VP_CHK_HR_NO_STATUS_RETURN(SetupCmdListAndPool(dxvaDevice, inputStreamParameters, commandList, osInterface));

#if (_DEBUG || _RELEASE_INTERNAL)
    {  // VPBLT Entry
        DWORD eProductFamily = adapterInfo->GfxPlatform.eProductFamily;
        MOS_TraceEventExt(EVENT_DDI_VP_BLT, EVENT_TYPE_START, &vpData->pVpHalState, sizeof(void *), &eProductFamily, sizeof(eProductFamily));

        DXVAUMD_RESOURCE  outResource    = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(outputParameters->OutputStream[0].hDrvTexture2D);
        GMM_RESOURCE_INFO *outGmmResInfo = outResource ? outResource->ResourceInfo.m_pGmmResourceInfo : nullptr;
        if (inputStreamParameters)
        {
            DXVAUMD_RESOURCE  inputResource  = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputStreamParameters->InputStream[0].hDrvInputTexture);
            GMM_RESOURCE_INFO *srcGmmResInfo = inputResource ? inputResource->ResourceInfo.m_pGmmResourceInfo : nullptr;
           if (srcGmmResInfo && outGmmResInfo)
            {
                DXVA_TRACEDATA_VPBLT eventDataVPBlt = {0};
                DXVA_TRACEDATA_VPBLT_INIT(eventDataVPBlt, (ULONG_PTR)DRIVER_PRIVATE_HANDLE(drvVideoProcessor),
                    (ULONG_PTR)vpData->pVpHalState, 1, numInputStreams, outResource->ResourceInfo.m_AllocationHandle, 0, 
                    inputResource->ResourceInfo.m_AllocationHandle, 0, (uint32_t)srcGmmResInfo->GetBaseWidth(), (uint32_t)srcGmmResInfo->GetBaseHeight(),
                    (uint32_t)MosInterface::OsFmtToMosFmt(srcGmmResInfo->GetD3d9Format()),
                    (uint32_t)outGmmResInfo->GetBaseWidth(), (uint32_t)outGmmResInfo->GetBaseHeight(),
                    (uint32_t)MosInterface::OsFmtToMosFmt(outGmmResInfo->GetD3d9Format()),
                    (uint32_t)srcGmmResInfo->GetSetCpSurfTag(0, 0),
                    (uint32_t)outGmmResInfo->GetSetCpSurfTag(0, 0));
                MOS_TraceEventExt(EVENT_DDI_VP_BLT, EVENT_TYPE_INFO, &eventDataVPBlt, sizeof(eventDataVPBlt), nullptr, 0);
            }
        }
    }
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    if (vpData->ConfigValues.dwForceColorFill)
    {
        numInputStreams = 0;
        MT_LOG1(MT_VP_BLT_FORCE_COLORFILL, MT_CRITICAL, MT_VP_HAL_PTR, (int64_t)vpData->pVpHalState);
    }

    ReadUserSettingForDebug(
        dxvaDevice->m_userSettingPtr,
        hwVPPBypassed,
        __MEDIA_USER_FEATURE_BYPASS_HW_VPP,
        MediaUserSetting::Group::Device);
    if (hwVPPBypassed > 0)
    {
        DDI_VP_ASSERTMESSAGE("HW VPP Bypassed");
        MT_LOG1(MT_VP_BLT_BYPSSED, MT_CRITICAL, MT_VP_HAL_PTR, (int64_t)vpData->pVpHalState);
        return;
    }
#endif

    hr = InitializeDxvaResourceInfo(drvCommandList, vpData, outputParameters, inputStreamParameters, numInputStreams);
    if (hr)
    {
        DDI_VP_ASSERTMESSAGE("Dxva resource initialization failed.");

#if (_DEBUG || _RELEASE_INTERNAL)
        {  //VPBLT exit
            MOS_TraceEventExt(EVENT_DDI_VP_BLT, EVENT_TYPE_END, &vpData->pVpHalState, sizeof(void *), nullptr, 0);
        }
#endif
        return;
    }

    SetVpDataStreamStateInfos(vpData, inputStreamParameters, numInputStreams);

    // Set output target rectangle
    if (CheckMemoryBlockNotZero((char *)&outputParameters->TargetRectangle, sizeof(D3D12DDI_RECT)))
    {
        vpData->bltStateTargetRect.Enable     = 1;
        vpData->bltStateTargetRect.TargetRect = outputParameters->TargetRectangle;
    }
    else
    {
        vpData->bltStateTargetRect.Enable = 0;
    }

    //Prediction and SetMarker
    if (SetPredictionAndMarker(vpData, commandList, dxvaDevice, resPredication, resSetMarker, osInterface) == E_FAIL)
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        {  //VPBLT exit
            MOS_TraceEventExt(EVENT_DDI_VP_BLT, EVENT_TYPE_END, &vpData->pVpHalState, sizeof(void *), nullptr, 0);
        }
#endif
        return;
    }

    hr = DdiSetVpHalRenderingParams(drvCommandList, vpData, outputParameters, inputStreamParameters, numInputStreams);
    if (hr)
    {
        DDI_VP_ASSERTMESSAGE("set VP render param failed.");
        MT_ERR2(MT_VP_BLT, MT_VP_HAL_PTR, (int64_t)vpData->pVpHalState, MT_ERROR_CODE, hr);

#if (_DEBUG || _RELEASE_INTERNAL)
        {  //VPBLT exit
            MOS_TraceEventExt(EVENT_DDI_VP_BLT, EVENT_TYPE_END, &vpData->pVpHalState, sizeof(void *), nullptr, 0);
        }
#endif
        return;
    }

    MosUtilities::MosLockMutex(dxvaDevice->m_pGpuContextSetCriticalSection);

    hr = MosUtilities::MosStatusToOsResult(vpData->pVpHalState->Render(&vpData->VpHalRenderParams));
    MosUtilities::MosUnlockMutex(dxvaDevice->m_pGpuContextSetCriticalSection);
    if (hr)
    {
        DDI_VP_ASSERTMESSAGE("VP render failed.");
    }
    else
    {
        DdiVPFunctionsCommon::VpHalDdiReportFeatureMode(vpData->pVpHalState, &vpData->ConfigValues, vpData->dwLACEInUse);

        // Report the features to registry
        DdiVPFunctionsCommon::ReportVPFeatureMode(dxvaDevice, &(vpData->ConfigValues), vpData->dwLACEInUse, vpData->dwLACEInUseReported);
#if (_DEBUG || _RELEASE_INTERNAL)
        {
            auto cmdPacket = MOS_New(PostponedCommandPacket, DumpPacket);
            DDI_VP_CHK_NULL_NO_STATUS_RETURN(cmdPacket);

            cmdPacket->AddSurfaceToDump(osInterface->resourceDumpAttriArray);
            osInterface->osStreamState->currentCmdList->EnqueueOneCommandPacket(cmdPacket);
        }
#endif
    }
    if (!osInterface->apoMosEnabled)
    {
        DestroyDxvaResourceInfo(drvCommandList, vpData, outputParameters, inputStreamParameters, numInputStreams);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    { //VPBLT exit
        MOS_TraceEventExt(EVENT_DDI_VP_BLT, EVENT_TYPE_END, &vpData->pVpHalState, sizeof(void*), nullptr, 0);
    }
#endif

    return;
}

template <class T>
void DdiVPFunctionsD3D12::SetVpDataStreamStateInfos(
    PDXVA_VP_DATA vpData,
    const T       *inputStreamParameters,
    uint32_t      numInputStreams)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);

    for (uint32_t iStream = 0; iStream < numInputStreams; iStream++)
    {
        DDI_VP_CHK_NULL_NO_STATUS_RETURN(inputStreamParameters);

        // Set transformation
        if (CheckMemoryBlockNotZero((char*)&inputStreamParameters[iStream].Transform.SourceRectangle, sizeof(D3D12DDI_RECT)))
        {
            vpData->stmStateSourceRect[iStream].Enable          = 1;
            vpData->stmStateSourceRect[iStream].SourceRect      = inputStreamParameters[iStream].Transform.SourceRectangle;
        }
        else
        {
            vpData->stmStateSourceRect[iStream].Enable         = 0;
        }

        if (CheckMemoryBlockNotZero((char*)&inputStreamParameters[iStream].Transform.DestinationRectangle, sizeof(D3D12DDI_RECT)))
        {
            vpData->stmStateDestinationRect[iStream].Enable      = 1;
            vpData->stmStateDestinationRect[iStream].DestRect    = inputStreamParameters[iStream].Transform.DestinationRectangle;
        }
        else
        {
            vpData->stmStateDestinationRect[iStream].Enable     = 0;
        }

        // Seems the runtime did not pass the correct argument "EnableOrientation" upon the creation of VP device
        // Comemnt the following check out temporarily
        //if (vpData->stmStateReserved[iStream].EnableOrientation)
        {
            if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_FLIP_HORIZONTAL)
            {
                vpData->stmStateMirror[iStream].Enable           = 1;
                vpData->stmStateMirror[iStream].FlipHorizontal   = 1;
            }
            else if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_CLOCKWISE_90)
            {
                vpData->stmStateRotation[iStream].Enable    = 1;
                vpData->stmStateRotation[iStream].Rotation  = D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_90;
            }
            else if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_CLOCKWISE_90_FLIP_HORIZONTAL)
            {
                vpData->stmStateRotation[iStream].Enable         = 1;
                vpData->stmStateRotation[iStream].Rotation       = D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_90;
                vpData->stmStateMirror[iStream].Enable           = 1;
                vpData->stmStateMirror[iStream].FlipHorizontal   = 1;
            }
            else if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_CLOCKWISE_180)
            {
                vpData->stmStateRotation[iStream].Enable      = 1;
                vpData->stmStateRotation[iStream].Rotation    = D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_180;
            }
            else if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_FLIP_VERTICAL)
            {
                vpData->stmStateMirror[iStream].Enable         = 1;
                vpData->stmStateMirror[iStream].FlipVertical   = 1;
            }
            else if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_CLOCKWISE_180_FLIP_HORIZONTAL)
            {
                vpData->stmStateRotation[iStream].Enable        = 1;
                vpData->stmStateRotation[iStream].Rotation      = D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_180;
                vpData->stmStateMirror[iStream].Enable          = 1;
                vpData->stmStateMirror[iStream].FlipHorizontal  = 1;
            }
            else if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_CLOCKWISE_270)
            {
                vpData->stmStateRotation[iStream].Enable    = 1;
                vpData->stmStateRotation[iStream].Rotation  = D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_270;
            }
            else if (inputStreamParameters[iStream].Transform.Orientation == D3D12DDI_VIDEO_PROCESS_ORIENTATION_0020_CLOCKWISE_270_FLIP_HORIZONTAL)
            {
                vpData->stmStateRotation[iStream].Enable        = 1;
                vpData->stmStateRotation[iStream].Rotation      = D3D11_1DDI_VIDEO_PROCESSOR_ROTATION_270;
                vpData->stmStateMirror[iStream].Enable          = 1;
                vpData->stmStateMirror[iStream].FlipHorizontal  = 1;
            }
            else
            {
                vpData->stmStateRotation[iStream].Enable   = 0;
                vpData->stmStateMirror[iStream].Enable     = 0;
            }
        }

        // TODO VPP: need to do setting corresponding to the flags from input
        vpData->stmStateOutputRate[iStream].RepeatFrame = inputStreamParameters->Flags & D3D12DDI_VIDEO_PROCESS_INPUT_STREAM_FLAG_0020_FRAME_REPEAT;

        // Set filter flags
        if (vpData->stmStateFilterBrightness[iStream].Enable)
        {
            vpData->stmStateFilterBrightness[iStream].Level = inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_BRIGHTNESS];
            DDI_VP_NORMALMESSAGE("stream[%d] brightness enable %d, Level %d.", iStream, vpData->stmStateFilterBrightness[iStream].Enable, vpData->stmStateFilterBrightness[iStream].Level);
        }
        if (vpData->stmStateFilterContrast[iStream].Enable)
        {
            vpData->stmStateFilterContrast[iStream].Level = inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_CONTRAST];
            DDI_VP_NORMALMESSAGE("stream[%d] contrast enable %d, Level %d.", iStream, vpData->stmStateFilterContrast[iStream].Enable, vpData->stmStateFilterContrast[iStream].Level);
        }
        if (vpData->stmStateFilterHue[iStream].Enable)
        {
            vpData->stmStateFilterHue[iStream].Level = inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_HUE];
        }
        if (vpData->stmStateFilterSaturation[iStream].Enable)
        {
            vpData->stmStateFilterSaturation[iStream].Level = inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_SATURATION];
        }
        if (vpData->stmStateFilterNoiseReduction[iStream].Enable)
        {
            vpData->stmStateFilterNoiseReduction[iStream].Level = inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_NOISE_REDUCTION];
        }
        if (vpData->stmStateFilterEdgeEnhancement[iStream].Enable)
        {
            vpData->stmStateFilterEdgeEnhancement[iStream].Level = inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_EDGE_ENHANCEMENT];
        }
        if (vpData->stmStateFilterAnamorphicScaling[iStream].Enable)
        {
            vpData->stmStateFilterAnamorphicScaling[iStream].Level = inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_ANAMORPHIC_SCALING];
        }
        if (vpData->stmStateFilterStereoAdjustment[iStream].Enable)
        {
            vpData->stmStateFilterStereoAdjustment[iStream].Level =inputStreamParameters[iStream].FilterLevels[D3D12DDI_VIDEO_PROCESS_FILTER_0020_STEREO_ADJUSTMENT];
        }

        // Set alpha blending
        if (vpData->stmStateAlpha[iStream].Enable)
        {
            if (inputStreamParameters[iStream].AlphaBlending.Enable)
            {
                vpData->stmStateAlpha[iStream].Alpha = inputStreamParameters[iStream].AlphaBlending.Alpha;
            }
            else
            {
                vpData->stmStateAlpha[iStream].Alpha = 1.0;
                DDI_VP_NORMALMESSAGE(
                    "stream[%d] vpData->stmStateAlpha[iStream].Enable == true, " \
                    "but.inputStreamParameters[iStream].AlphaBlending.Enable == false, " \
                    "inputStreamParameters[iStream].AlphaBlending.Alpha %f ",
                    iStream,
                    inputStreamParameters[iStream].AlphaBlending.Alpha);
            }
        }
        // Set Field Type
        SetFieldType(vpData, inputStreamParameters, iStream);
    }
    return;
}

HRESULT DdiVPFunctionsD3D12::SetPredictionAndMarker(
    PDXVA_VP_DATA           vpData,
    DXVA12_COMMAND_LIST     *commandList,
    PMOS_CONTEXT            dxvaDevice,
    MOS_RESOURCE            &resPredication,
    MOS_RESOURCE            &resSetMarker,
    PMOS_INTERFACE          osInterface)
{
    HRESULT          hr                        = S_OK;
    DXVAUMD_RESOURCE resPredicationDdiResource = nullptr;
    DXVAUMD_RESOURCE resSetMarkerDdiResource   = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(commandList);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);

    MOS_ZeroMemory(&resPredication, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resSetMarker, sizeof(MOS_RESOURCE));

    // Predication
    vpData->VpHalRenderParams.PredicationParams.predicationEnabled = commandList->IsPredicationEnabled() ? true : false;
    if (vpData->VpHalRenderParams.PredicationParams.predicationEnabled)
    {
        resPredicationDdiResource = commandList->GetPredicationResource();
        vpData->VpHalRenderParams.PredicationParams.predicationResOffset   = commandList->GetPredicationResOffset();
        vpData->VpHalRenderParams.PredicationParams.ptempPredicationBuffer = (PMOS_RESOURCE)commandList->GetTempPredicationBuffer();

        if (commandList->GetPredicationOp() == D3D12DDI_PREDICATION_OP_NOT_EQUAL_ZERO)
        {
            vpData->VpHalRenderParams.PredicationParams.predicationNotEqualZero = true;
        }
        else
        {
            vpData->VpHalRenderParams.PredicationParams.predicationNotEqualZero = false;
        }

        if (Mos_Specific_SetOsResourceFromDdi(resPredicationDdiResource, 0, 0, &resPredication) != MOS_STATUS_SUCCESS)
        {
            DDI_VP_ASSERTMESSAGE("set mos resource failed.");
            MT_ERR1(MT_VP_BLT, MT_VP_HAL_PTR, (int64_t)vpData->pVpHalState);
            return E_FAIL;
        }

        vpData->VpHalRenderParams.PredicationParams.pPredicationResource = &resPredication;
    }

    // SetMarker
    vpData->VpHalRenderParams.SetMarkerParams.setMarkerEnabled = commandList->m_commandList ? commandList->m_commandList->IsSetMarkerEnabled() : 0;

    if (vpData->VpHalRenderParams.SetMarkerParams.setMarkerEnabled)
    {
        auto res                = commandList->m_commandList->GetMarkerResource();
        resSetMarkerDdiResource = res ? res->pD3dResource : nullptr;

        DXGK_HISTORY_BUFFER_HEADER *markerHeader                 = commandList->m_commandList->GetMarkerHeader();
        vpData->VpHalRenderParams.SetMarkerParams.setMarkerNumTs = markerHeader ? markerHeader->NumTimestamps : 0;

        PMOS_RESOURCE mosRes = osInterface->pfnGetMarkerResource(osInterface);
        if (mosRes)
        {
            resSetMarker = *mosRes;
        }

        vpData->VpHalRenderParams.SetMarkerParams.pSetMarkerResource = &resSetMarker;
    }

    return hr;
}

template <class T>
HRESULT DdiVPFunctionsD3D12::DdiSetVpHalRenderingParams(
    D3D12DDI_HCOMMANDLIST                                           drvCommandList,
    PDXVA_VP_DATA                                                   vpData,
    const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032    *outputStreamParameters,
    const T                                                         *inputStreamParameters,
    uint32_t                                                        streamCount)
{
    HRESULT                             hr                    = S_OK;
    DXVA_DEVICE_CONTEXT                 *dxvaDevice           = nullptr;
    VPHAL_RENDER_PARAMS_EXT             *renderParams         = &vpData->VpHalRenderParams;
    VPHAL_SURFACE_EXT                   *surface              = nullptr;
    VPHAL_SURFACE_EXT                   *srcSurface           = nullptr;
    int                                 priVideoIndex         = -1;
    D3DDDIFORMAT                        format                = D3DDDIFMT_UNKNOWN;
    REFERENCE_TIME                      rtTarget              = 0;
    VPHAL_DI_PARAMS                     diParamsPrimary       = {};
    VPHAL_DI_PARAMS                     *diParams             = nullptr;
    DWORD                               invalidCaps           = 0;
    COMPOSITING_SAMPLE_D3D11_1          auxSample             = {0};
    bool                                allRgbInput           = true;
    DXVA_DEVICE_CONTEXT                 *context              = nullptr;
    PADAPTER_INFO                       adapterInfo           = nullptr;
    PMOS_INTERFACE                      osInterface           = nullptr;
    PVPHAL_RENDER_EXTENSION_PARAMS      renderExtensionParams = nullptr;
    DXVAUMD_RESOURCE                    resource              = nullptr;
    GMM_RESOURCE_INFO                   *gmmResourceInfo      = nullptr;
    DWORD                               yuvRange              = 0;
    DWORD                               yuvRangeRegValue      = 0;
    DWORD                               yuvRangeForTarget     = 0;
    DWORD                               index                 = 0;
    DWORD                               mipSlice              = 0;
    bool                                separateRightFrame    = false;
    DWORD                               contentWidth          = 0;
    DWORD                               contentHeight         = 0;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(outputStreamParameters);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);

    osInterface    = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);

    auto skuTable = vpData->pVpHalState->GetSkuTable();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(skuTable);

    dxvaDevice     = osInterface->pOsContext;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);

    if (renderParams->pExtensionData == nullptr)
    {
        renderParams->pExtensionData = (void *)MOS_AllocAndZeroMemory(sizeof(VPHAL_RENDER_EXTENSION_PARAMS));
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams->pExtensionData);
    }

    renderExtensionParams = (PVPHAL_RENDER_EXTENSION_PARAMS)renderParams->pExtensionData;

    // Get CUI registry values
    if (DXVA_REG_CallbackHandler(&vpData->CUIParams, &vpData->CUIRegistryValues) == FALSE)
    {
        DDI_VP_NORMALMESSAGE("Unable to handle CUI Callback.");
    }

    renderParams->Component        = (MOS_COMPONENT)COMPONENT_DXVA_D3D11_1;
    renderParams->bReportStatus    = false;

    // TODO: check if we really need this field
    renderParams->StatusFeedBackID = 0;

    // Check if Pre-Processing is enabled
    if (vpData->vpeOutputState.uMode == VPE_MODE_PREPROC)
    {
        renderParams->Component = COMPONENT_VPreP;
    }
    else if (vpData->vpeOutputState.uMode == VPE_MODE_FDFB)
    {
        renderParams->Component = (MOS_COMPONENT)COMPONENT_FDFB;
    }

    // Check if BLT status report is enabled
    if (vpData->vpeOutputState.bCollectBltStatus)
    {
        renderParams->bReportStatus = true;
    }

    // Check if Capture pipe is enabled
    if (vpData->vpeOutputState.pCamPipeParams && vpData->vpeOutputState.pCamPipeParams->Active.bActive)
    {
        if (streamCount > 1)
        {
            DDI_VP_ASSERTMESSAGE("Capture Pipe does not support more than 1 layer.");
            return E_INVALIDARG;
        }

        renderParams->Component = (MOS_COMPONENT)COMPONENT_CapPipe;

        // Disable all cui features in camera pipe mode, keep vp feature modes unchanged
        uint32_t dwPostProcScalingModeExternal      = vpData->CUIRegistryValues.dwPostProcScalingModeExternal;
        uint32_t dwPostProcDeinterlaceModeExternal  = vpData->CUIRegistryValues.dwPostProcDeinterlaceModeExternal;
        uint32_t dwPreProcScalingModeExternal       = vpData->CUIRegistryValues.dwPreProcScalingModeExternal;
        uint32_t dwPreProcDeinterlaceModeExternal   = vpData->CUIRegistryValues.dwPreProcDeinterlaceModeExternal;
        uint32_t dwPostProcScalingModeInternal      = vpData->CUIRegistryValues.dwPostProcScalingModeInternal;
        uint32_t dwPostProcDeinterlaceModeInternal  = vpData->CUIRegistryValues.dwPostProcDeinterlaceModeInternal;
        uint32_t dwPreProcScalingModeInternal       = vpData->CUIRegistryValues.dwPreProcScalingModeInternal;
        uint32_t dwPreProcDeinterlaceModeInternal   = vpData->CUIRegistryValues.dwPreProcDeinterlaceModeInternal;
        MOS_ZeroMemory(&vpData->CUIRegistryValues, sizeof(vpData->CUIRegistryValues));
        vpData->CUIRegistryValues.dwPostProcScalingModeExternal     = dwPostProcScalingModeExternal;
        vpData->CUIRegistryValues.dwPostProcDeinterlaceModeExternal = dwPostProcDeinterlaceModeExternal;
        vpData->CUIRegistryValues.dwPreProcScalingModeExternal      = dwPreProcScalingModeExternal;
        vpData->CUIRegistryValues.dwPreProcDeinterlaceModeExternal  = dwPreProcDeinterlaceModeExternal;
        vpData->CUIRegistryValues.dwPostProcScalingModeInternal     = dwPostProcScalingModeInternal;
        vpData->CUIRegistryValues.dwPostProcDeinterlaceModeInternal = dwPostProcDeinterlaceModeInternal;
        vpData->CUIRegistryValues.dwPreProcScalingModeInternal      = dwPreProcScalingModeInternal;
        vpData->CUIRegistryValues.dwPreProcDeinterlaceModeInternal  = dwPreProcDeinterlaceModeInternal;
    }

    renderParams->pConstriction        = nullptr;
    renderParams->bTurboMode           = false;
    renderParams->bStereoMode          = vpData->bltStereoMode ? true : false;

    // Read from registry
    yuvRangeRegValue = vpData->CUIRegistryValues.dwInputYUVRange;

    // Check if Screen Capture Defense is enabled
    if (IsValidViewsForSCD(dxvaDevice, vpData, outputStreamParameters, streamCount, inputStreamParameters))
    {
        // SCD Enabled so skip VP processing
        return S_OK;
    }

    //-------------------------
    // Setup surface HAL parameters for render
    //-------------------------
    resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(outputStreamParameters->OutputStream[0].hDrvTexture2D);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(resource);
    index = outputStreamParameters->OutputStream[0].Subresource;
    mipSlice = 0;

    gmmResourceInfo = (resource) ? ((DXVAUMD_RESOURCE)(resource))->ResourceInfo.m_pGmmResourceInfo : nullptr;

    // Allocate RT description struct
    if (renderParams->pTarget[0] == nullptr)
    {
        renderParams->pTarget[0] = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE_EXT));
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams->pTarget[0]);
        renderParams->uDstCount = 1;    // single render target
    }

    if (vpData->bltStateTargetRect.Enable)
    {
        renderParams->pTarget[0]->rcSrc = vpData->bltStateTargetRect.TargetRect;
    }
    else
    {
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(gmmResourceInfo);
        renderParams->pTarget[0]->rcSrc.left   = renderParams->pTarget[0]->rcSrc.top = 0;
        renderParams->pTarget[0]->rcSrc.right  = (uint32_t)(gmmResourceInfo->GetBaseWidth());
        renderParams->pTarget[0]->rcSrc.bottom = gmmResourceInfo->GetBaseHeight();
    }
    renderParams->pTarget[0]->rcDst = renderParams->pTarget[0]->rcSrc;

    if (vpData->vpeOutputState.YuvRangeParamRT.bFullRangeEnabled)
    {
        yuvRangeForTarget = YUV_RANGE_0_255;
    }
    else
    {
#if defined(D3D_UMD_INTERFACE_VERSION) && (D3D_UMD_INTERFACE_VERSION > D3D_UMD_INTERFACE_VERSION_WIN8)
        yuvRangeForTarget = vpData->bltStateOutputColorSpace.Nominal_Range;
#else
        yuvRangeForTarget = YUV_RANGE_16_235;
#endif
    }

    DdiVPFunctionsCommon::SetTargetSurfaceForRender(
        (VPHAL_SURFACE_EXT*)renderParams->pTarget[0],
        vpData->bltStateOutputColorSpace,
        vpData->bltStateOutputChromaSiting,
        vpData->bltStateOutputGammaValue,
        &vpData->stmStateOutputHDRState,
        vpData->vpeOutputState.p360StitchParams,
        resource,
        index,
        mipSlice,
        yuvRangeForTarget);

    //---------------------
    // Set Background Color
    //---------------------
    DDI_VP_CHK_HR_RETURN(SetBackGroundColorParams(vpData, renderParams));

    //--------------------------
    // Set Demo Mode Parameters
    //--------------------------
    DdiVPFunctionsCommon::SetDemoModeParams(dxvaDevice->m_userSettingPtr, renderParams, vpData->CUIRegistryValues, osInterface);

    // Set DI Params to be set for pri video
    MOS_ZeroMemory(&diParamsPrimary, sizeof(VPHAL_DI_PARAMS));
    DdiVPFunctionsCommon::SetDiParamsPrimay(diParamsPrimary, renderParams->Component, vpData->ConfigValues.dwCreatedDeinterlaceMode);

    for (uint32_t iStream = 0; iStream < streamCount; iStream++)
    {
//      if (pStreams[iStream].Enable)
        {
            resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputStreamParameters[iStream].InputStream[0].hDrvInputTexture);
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(resource);
            gmmResourceInfo = (resource) ? ((DXVAUMD_RESOURCE)(resource))->ResourceInfo.m_pGmmResourceInfo : nullptr;
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(gmmResourceInfo);
            format = gmmResourceInfo->GetD3d9Format();

            if (DdiVPFunctionsCommon::IsNotPalettizedFourCC(format))
            {
                allRgbInput = false;
                break;
            }
        }
    }

    // Init Src count to 0
    renderParams->uSrcCount = 0;

    for (uint32_t iStream = 0; iStream < streamCount; iStream++)
    {
//      if (pStreams[iStream].Enable)
        {
            yuvRange = yuvRangeRegValue;
            invalidCaps = 0;
            resource     = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputStreamParameters[iStream].InputStream[0].hDrvInputTexture);
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(resource);
            index = inputStreamParameters[iStream].InputStream[0].Subresource;
            gmmResourceInfo = (resource) ? ((DXVAUMD_RESOURCE)(resource))->ResourceInfo.m_pGmmResourceInfo : nullptr;

#if defined(D3D_UMD_INTERFACE_VERSION) && (D3D_UMD_INTERFACE_VERSION > D3D_UMD_INTERFACE_VERSION_WIN8)
            yuvRange = vpData->CUIRegistryValues.InputYUVRangeApplyAlways ? vpData->CUIRegistryValues.dwInputYUVRange : vpData->stmStateInputColorSpace[iStream].Nominal_Range;
#endif

            // TODO: Check if there exist any fourcc passed from user app
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(gmmResourceInfo);
            format = gmmResourceInfo->GetD3d9Format();

            if (renderParams->pSrc[renderParams->uSrcCount] == nullptr)
            {
                renderParams->pSrc[renderParams->uSrcCount] = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE_EXT));
                DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams->pSrc[renderParams->uSrcCount]);
            }

            surface = (VPHAL_SURFACE_EXT*)renderParams->pSrc[renderParams->uSrcCount];

            // set rotation
            // set mirror
            SetRotationAndMirror(vpData->stmStateRotation[iStream], vpData->stmStateMirror[iStream], surface->Rotation);

            if (DdiVPFunctionsCommon::IsNotPalettizedFourCC(format))
            {
                if ((priVideoIndex < 0) && (format != FOURCC_AYUV))
                {
                    priVideoIndex = renderParams->uSrcCount;
                    surface->SurfType = SURF_IN_PRIMARY;
                    diParams = &diParamsPrimary;

                    // Set Interlaced Scaling Mode and Field Weaving for Pre-Processing
                    if (renderParams->Component == COMPONENT_VPreP)
                    {
                        // Disable DI when output is Interlaced or Field Weaving is enabled
                        if (vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_FIELD_WEAVE ||
                            vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_ISCALE)
                        {
                            diParams = nullptr;
                        }
                        surface->bInterlacedScaling = vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_ISCALE ? true : false;
                        surface->bFieldWeaving = vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_FIELD_WEAVE ? true : false;
                        surface->bQueryVariance = vpData->vpeStreamState[iStream].VarianceParam.Type != VPREP_VARIANCE_TYPE_NONE ? true : false;

                        if (vpData->vpeStreamState[iStream].YuvRangeParamStm.bFullRangeEnabled)
                        {
                            yuvRange = YUV_RANGE_0_255;
                        }
                    }
                }
                else if ((priVideoIndex < 0) && (format == FOURCC_AYUV) && (renderParams->Component == COMPONENT_CapPipe))
                {
                    surface->SurfType = SURF_IN_PRIMARY;
                }
                else if ((priVideoIndex < 0) && (format == FOURCC_AYUV) && (streamCount == 1))
                {
                    surface->SurfType = SURF_IN_PRIMARY;
                    diParams          = &diParamsPrimary;
                }
                else
                {
                    surface->SurfType = SURF_IN_SUBSTREAM;
                }
            }
            else if (DdiVPFunctionsCommon::IsPalettizedFormat(format))  // substreams
            {
                // Block lumakey for palettized format input
                invalidCaps |= COMPOSITING_FLAG_LUMA;

                surface->SurfType = SURF_IN_SUBSTREAM;
            }
            else if (DdiVPFunctionsCommon::IsBayerFormat(format))
            {
                if (renderParams->Component != COMPONENT_CapPipe)
                {
                    DDI_VP_ASSERTMESSAGE("Component is not COMPONENT_CapPipe.");
                }
                surface->SurfType = SURF_IN_PRIMARY;
            }
            else // RGB
            {
                // Constant alpha is supported but not DI or luma keying
                invalidCaps |= COMPOSITING_FLAG_FIELD | COMPOSITING_FLAG_LUMA;
                context = osInterface->pOsContext;
                adapterInfo = DXVAUMD_ADAPTER_INFO(context);

                if ((allRgbInput == true) && (priVideoIndex < 0))
                {
                    priVideoIndex = renderParams->uSrcCount;
                    surface->SurfType = SURF_IN_PRIMARY;
                    diParams = &diParamsPrimary;
                    // Set Interlaced Scaling Mode and Field Weaving for Pre-Processing
                    if (renderParams->Component == COMPONENT_VPreP)
                    {
                        // Disable DI when output is Interlaced or Field Weaving is enabled
                        if (vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_FIELD_WEAVE ||
                            vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_ISCALE)
                        {
                            diParams = nullptr;
                        }
                        surface->bInterlacedScaling = vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_ISCALE ? true : false;
                        surface->bFieldWeaving = vpData->vpeStreamState[iStream].InterlaceParam.Mode == VPE_VPREP_INTERLACE_MODE_FIELD_WEAVE ? true : false;
                        surface->bQueryVariance = vpData->vpeStreamState[iStream].VarianceParam.Type != VPREP_VARIANCE_TYPE_NONE ? true : false;
                    }
                }
                else if (iStream == 0 && format == D3DDDIFMT_X8R8G8B8)
                {
                    surface->SurfType = SURF_IN_BACKGROUND;
                }
                else // None of the layers are assigned as Gfx layer
                {
                    surface->SurfType = SURF_IN_SUBSTREAM;
                }
            }

            // Generate a unique identifier for the given frame.
            auxSample.Start = resource->ResourceInfo.m_AllocationHandle;
            // Set End Timestamp to an arbitrary number, (timestamp = start * 2).
            // Not used anywhere except to calculate rtTarget
            auxSample.End = auxSample.Start << 1;
            rtTarget = 0;

            //-----------------------------------------
            // Set deinterlacing flags
            //-----------------------------------------
            Setdeinterlacingflags(vpData->stmStateFrameFormat[iStream], auxSample, surface, rtTarget, inputStreamParameters[iStream].RateInfo.OutputIndex);

            if (surface->SurfType == SURF_IN_PRIMARY)
            {
                GetFrameRateandDeinterlaceMode(vpData, renderParams, iStream);
            }

            // bSeparateRightFrame frame is true if stereo format is separate
            separateRightFrame = false;

            // Set stereo 3d parameters. Ensure surface has been defined
            // completely before calling b/c SetStereoParams() may create
            // a duplicate copy of surface and add it to the source array.
            DDI_VP_CHK_HR_RETURN(SetStereoParams(vpData, renderParams, inputStreamParameters, surface, iStream));

            srcSurface = surface;

            // Do once if not stereo or stereo but not format separate.
            // Do twice for stereo separate left & right. 
            do
            {
                surface = (VPHAL_SURFACE_EXT*)renderParams->pSrc[renderParams->uSrcCount];

                if (separateRightFrame)
                {
                    resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputStreamParameters[iStream].InputStream[1].hDrvInputTexture);
                    DDI_VP_CHECK_NULL_WITH_HR_RETURN(resource);

                    index = inputStreamParameters[iStream].InputStream[1].Subresource;
                    gmmResourceInfo = (resource) ? ((DXVAUMD_RESOURCE)(resource))->ResourceInfo.m_pGmmResourceInfo : nullptr;
                }

                DDI_VP_CHECK_NULL_WITH_HR_RETURN(gmmResourceInfo);
                contentWidth = (uint32_t)(gmmResourceInfo->GetBaseWidth());
                contentHeight = gmmResourceInfo->GetBaseHeight();

                if (surface->SurfType == SURF_IN_PRIMARY)
                {
                    DdiVPFunctionsCommon::SetVPFeatureMode(
                        dxvaDevice,
                        skuTable,
                        &vpData->ConfigValues,
                        contentWidth,
                        contentHeight,
                        &vpData->CUIRegistryValues);

                    // Set DI Params to be set for pri video
                    DdiVPFunctionsCommon::UpdateDiParamsPrimayForPrimarySurface(
                        diParamsPrimary,
                        renderParams->Component,
                        vpData->ConfigValues.dwCreatedDeinterlaceMode,
                        vpData->CUIRegistryValues.bEnableFMD,
                        vpData->stmStateAutoProcessingMode[iStream].Enable);

                    // Enable Turbo mode if pri video is HD size
                    if (GRAPHICS_IS_SKU(skuTable, FtrMediaTurboMode) && MEDIA_IS_HDCONTENT(surface->dwWidth, surface->dwHeight))
                    {
                        renderParams->bTurboMode = true;
                    }

                    renderParams->uiCurrentInputFrameOrField = inputStreamParameters[iStream].RateInfo.InputFrameOrField;

                    if (GRAPHICS_IS_SKU(skuTable, FtrVpDisableFor4K) && (surface->dwHeight >= VPHAL_RNDR_4K_HEIGHT))
                    {
                        renderParams->bDisableVpFor4K = true;
                    }
                    else
                    {
                        renderParams->bDisableVpFor4K = false;
                    }

                    if (!vpData->ConfigValues.dwUseVeboxFor8K && GRAPHICS_IS_SKU(skuTable, FtrVpDisableFor8K) &&
                        ((surface->dwWidth >= VPHAL_RNDR_8K_WIDTH || surface->dwHeight >= VPHAL_RNDR_8K_HEIGHT) ||
                         (renderParams->pTarget[0]->dwWidth >= VPHAL_RNDR_8K_WIDTH || renderParams->pTarget[0]->dwHeight >= VPHAL_RNDR_8K_HEIGHT)))
                    {
                        renderParams->bDisableVeboxFor8K = true;
                    }
                    else
                    {
                        renderParams->bDisableVeboxFor8K = false;
                    }

                    if (vpData->dwForceLaceToEanble)
                    {
                        renderParams->bForceLaceToEnable = true;
                    }
                    else
                    {
                        renderParams->bForceLaceToEnable = false;
                    }

                    // Setup Procamp parameters - applied only to pri video
                    DDI_VP_CHK_HR_RETURN(SetVpHalProcampParams(vpData, renderParams, surface, iStream));

                    // Setup NLAS parameters - applied only to pri video
                    DDI_VP_CHK_HR_RETURN(SetVpHalNLASParams(vpData, renderParams, surface, iStream));

                    // Set Denoise - applied only to pri video
                    DDI_VP_CHK_HR_RETURN(SetVpHalDenoiseParams(vpData, renderParams, surface, iStream, (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(outputStreamParameters->OutputStream[0].hDrvTexture2D)));

                    // Setup IS parameters
                    DDI_VP_CHK_HR_RETURN(SetVpHalISParams(vpData, renderParams, iStream));

                    // Setup IEF parameters
                    DDI_VP_CHK_HR_RETURN(SetVpHalIEFParams(vpData, renderParams, surface, iStream));

                    // Setup IECP params from CUI
                    DDI_VP_CHK_HR_RETURN(SetVpHalColorPipeParams(dxvaDevice, vpData, renderParams, surface, iStream));

                    // Setup GC params from CUI
                    DDI_VP_CHK_HR_RETURN(SetVpHalGamutCompressionParams(vpData, renderParams, surface, iStream));

                    // Setup FRC params from CUI
                    DDI_VP_CHK_HR_RETURN(SetVpHalFrcParams(vpData, renderParams, inputStreamParameters, iStream));

                    // Setup Capture Pipe params
                    DDI_VP_CHK_HR_RETURN(SetVpHalCapPipeParams(vpData, renderParams, surface));

                    // Setup FDFB params
                    DDI_VP_CHK_HR_RETURN(SetVpHalFDFBParams(vpData, renderParams, surface, iStream, contentWidth, contentHeight));

                    // Setup Super Resolution params
                    DDI_VP_CHK_HR_RETURN(SetVpHalSuperResolutionParams(vpData, renderParams, surface));

                    SetVpeScalingtMode(dxvaDevice, vpData, renderParams, surface, iStream);
                }
                else
                {
                    surface->ScalingMode = VPHAL_SCALING_BILINEAR;
                    surface->ScalingPreference = VPHAL_SCALING_PREFER_SFC;
                }

                // Pre-Processing: AVS, IEF and Field Weaving cannot be enabled with interlaced scaling
                if (renderParams->Component == COMPONENT_VPreP)
                {
                    if ((surface->SurfType == SURF_IN_PRIMARY) && (surface->bInterlacedScaling || surface->bFieldWeaving))
                    {
                        // Switch from AVS to Bilinear Scaling
                        if (IS_PL3_FORMAT(surface->Format))
                        {
                            surface->ScalingMode = VPHAL_SCALING_BILINEAR;
                        }

                        // Disable IEF
                        if (surface->pIEFParams)
                        {
                            MOS_SafeFreeMemory(surface->pIEFParams->pExtParam);
                            MOS_FreeMemory(surface->pIEFParams);
                            surface->pIEFParams = nullptr;
                        }
                    }
                }

                // Set SrcRect
                if (vpData->stmStateSourceRect[iStream].Enable)
                {
                    auxSample.SrcRect = vpData->stmStateSourceRect[iStream].SourceRect;
                }
                else
                {
                    auxSample.SrcRect.left = auxSample.SrcRect.top = 0;
                    auxSample.SrcRect.right = contentWidth;
                    auxSample.SrcRect.bottom = contentHeight;
                }

                // Set DstRect
                if (vpData->stmStateDestinationRect[iStream].Enable)
                {
                    auxSample.DstRect = vpData->stmStateDestinationRect[iStream].DestRect;
                }
                else
                {
                    auxSample.DstRect = renderParams->pTarget[0]->rcDst;
                }

                // Set Palette Params
                MOS_SecureMemcpy(&auxSample.Palette,
                    vpData->stmStatePalette[iStream].Count * sizeof(DXVADDI_AYUVSAMPLE8),
                    vpData->stmStatePalette[iStream].pEntries,
                    vpData->stmStatePalette[iStream].Count * sizeof(DXVADDI_AYUVSAMPLE8));

                // Set Luma Key Parameters
                if (vpData->stmStateLumaKey[iStream].Enable && (invalidCaps & COMPOSITING_FLAG_LUMA) == 0)
                {
                    auxSample.dwFlags |= COMPOSITING_SRC_FLAG_LUMA;
                    auxSample.uLumaHigh = (uint32_t)(255 * vpData->stmStateLumaKey[iStream].Upper);
                    auxSample.uLumaLow  = (uint32_t)(255 * vpData->stmStateLumaKey[iStream].Lower);
                }
                else
                {
                    auxSample.dwFlags &= ~COMPOSITING_SRC_FLAG_LUMA;
                    auxSample.uLumaHigh = auxSample.uLumaLow = 0;
                }

                // Flag vpData->stmStateAlpha[iStream].Enable is used for 
                // D3D11 enabling/disabling both constant alpha blending and source alpha blending
                if (vpData->stmStateAlpha[iStream].Enable)
                {
                    auxSample.ConstantAlpha = vpData->stmStateAlpha[iStream].Alpha;
                }
                else
                {
                    auxSample.ConstantAlpha = 1.0f;
                    // Disable source blend as alpha is not enabled
                    // D3D11 disable src alpha blending because of WIDI issue 5673720
                    // DXVAHD enable src alpha blending because of PDVD issue 5641185 and Toshiba player issue 5642046
                    invalidCaps |= COMPOSITING_FLAG_SRC_BLEND;
                }

                auxSample.ColorSpace = vpData->stmStateInputColorSpace[iStream];
                auxSample.ChromaSiting = vpData->stmStateInputChromaSiting[iStream];
                auxSample.GammaValue = vpData->stmStateInputGammaValue[iStream];

                hr = DdiVPFunctionsCommon::SetSurfaceForRender(
                    surface,
                    &auxSample,
                    rtTarget,
                    resource,
                    index,
                    0,
                    0,
                    diParams,
                    invalidCaps,
                    yuvRange,
                    &vpData->stmStateInputHDRState[iStream]);

                // Reference frame irrelevant to CapPipe and FDFB
                if (surface->SurfType == SURF_IN_PRIMARY && renderParams->Component != COMPONENT_CapPipe && renderParams->Component != COMPONENT_FDFB) 
                {
                    // If right frame is provided separately set references for right frame also
                    DDI_VP_CHK_HR_RETURN(SetVpHalBackwardRefs(
                        vpData,
                        inputStreamParameters,
                        separateRightFrame,
                        surface,
                        iStream,
                        rtTarget,
                        diParams,
                        invalidCaps,
                        yuvRange,
                        &auxSample));

                    DDI_VP_CHK_HR_RETURN(SetVpHalForwardRefs(
                        vpData,
                        inputStreamParameters,
                        separateRightFrame,
                        surface,
                        iStream,
                        rtTarget,
                        diParams,
                        invalidCaps,
                        yuvRange,
                        &auxSample));

                    // Error checking for Field Weaving
                    if ((renderParams->Component == COMPONENT_VPreP) && (surface->bFieldWeaving) && (surface->pBwdRef == nullptr))
                    {
                        DDI_VP_ASSERTMESSAGE("Reference frame not present in field weaving.");
                        return E_FAIL;
                    }
                } // if primary

                // Increment Surface Count
                renderParams->uSrcCount++;
                separateRightFrame = separateRightFrame ? false : (STREAM_STEREO_SEPARATE(srcSurface) ? true : false);

            } while (separateRightFrame); // For left and Right
        } // if stream is enabled
    } // For every stream

    //-----------------------------------------------------
    // Setup Alpha parameters (from App) 
    //-----------------------------------------------------
    DDI_VP_CHK_HR_RETURN(SetAlphaParams(renderParams->pCompAlpha, vpData->bltStateAlphaFillData, priVideoIndex));

    // Add the render param for calculating alpha value(Port from Android/Linux)
    renderParams->bCalculatingAlpha = true;

    // Read ext settings from reg key
    renderExtensionParams->dwMediaFeatures     = vpData->CUIRegistryValues.dwMediaFeatures;
    renderExtensionParams->sfcPipeControl      = (SFC_PIPE_CONTROL)vpData->CUIRegistryValues.wDisableSFC;
    renderExtensionParams->bypassComposition   = (uint8_t)vpData->CUIRegistryValues.wBypassComposition;

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetAlphaParams(
    PVPHAL_ALPHA_PARAMS             &compAlpha,
    INTEL_BLT_STATE_ALPHA_FILL_DATA stateAlphaFillData,
    int                             videoIndex)
{
    DDI_VP_FUNCTION_ENTER;

    if (compAlpha == nullptr)
    {
        compAlpha = (PVPHAL_ALPHA_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_ALPHA_PARAMS));
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(compAlpha);
    }

    switch (stateAlphaFillData.Mode)
    {
        case D3D12DDI_VIDEO_PROCESS_ALPHA_FILL_MODE_0020_BACKGROUND:
            compAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_BACKGROUND;
            break;
        case D3D12DDI_VIDEO_PROCESS_ALPHA_FILL_MODE_0020_SOURCE_STREAM:
            if (videoIndex == stateAlphaFillData.StreamNumber)
            {
                compAlpha->fAlpha    = 0;
                compAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM;
            }
            else
            {
                // Not supported
                DDI_VP_ASSERTMESSAGE("D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE_SOURCE_STREAM for substreams not supported.");
                compAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_OPAQUE;
            }
            break;
        case D3D12DDI_VIDEO_PROCESS_ALPHA_FILL_MODE_0020_DESTINATION:
            DDI_VP_ASSERTMESSAGE("D3D11_1DDI_VIDEO_PROCESSOR_ALPHA_FILL_MODE_DESTINATION not supported.");
            compAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_OPAQUE;
            break;
        case D3D12DDI_VIDEO_PROCESS_ALPHA_FILL_MODE_0020_OPAQUE:
        default:
            compAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_OPAQUE;
            break;
    }

    return S_OK;
}

HRESULT DdiVPFunctionsD3D12::SetBackGroundColorParams(
    PDXVA_VP_DATA           vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams)
{
    DWORD bgColor = 0;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);

    if (renderParams->pColorFillParams == nullptr)
    {
        renderParams->pColorFillParams = (PVPHAL_COLORFILL_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORFILL_PARAMS));
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams->pColorFillParams);
    }

    // In Dxva 11.1 Bg. color primaries are dependent on render target primaries
    if (vpData->bltStateBackgroundColor.YCbCr)
    {
        if (vpData->bltStateOutputColorSpace.Nominal_Range == D3D11_1DDI_VIDEO_PROCESSOR_NOMINAL_RANGE_0_255)
        {
            // Convert from YUV[0,1] to YUV[0,255]
            bgColor                                = DdiVPFunctionsCommon::YUV_D3DCOLOR_FULL(&(vpData->bltStateBackgroundColor.BackgroundColor.YCbCr));
            renderParams->pColorFillParams->CSpace = vpData->bltStateOutputColorSpace.YCbCr_Matrix ? CSpace_BT709_FullRange : CSpace_BT601_FullRange;
        }
        else
        {
            // Convert from YUV[0,1] to Y[16,235]UV[16,240]
            bgColor                                = DdiVPFunctionsCommon::YUV_D3DCOLOR(&(vpData->bltStateBackgroundColor.BackgroundColor.YCbCr));
            renderParams->pColorFillParams->CSpace = vpData->bltStateOutputColorSpace.YCbCr_Matrix ? CSpace_BT709 : CSpace_BT601;
        }
        renderParams->pColorFillParams->bYCbCr = true;
    }
    else
    {
        if (vpData->bltStateOutputColorSpace.RGB_Range == 0)
        {
            // Convert from RGB[0,1] to RGB[0,255]
            bgColor                                = DdiVPFunctionsCommon::RGB_D3DCOLOR_FULL(&(vpData->bltStateBackgroundColor.BackgroundColor.RGBA));
            renderParams->pColorFillParams->CSpace = CSpace_sRGB;
        }
        else
        {
            // Convert from RGB[0,1] to RGB[16,255]
            bgColor                                = DdiVPFunctionsCommon::RGB_D3DCOLOR(&(vpData->bltStateBackgroundColor.BackgroundColor.RGBA));
            renderParams->pColorFillParams->CSpace = CSpace_stRGB;
        }
        renderParams->pColorFillParams->bYCbCr = false;
    }

    renderParams->pColorFillParams->Color = bgColor;

    //DEBUG ENHANCEMENT - force pure color fill
    if (vpData->ConfigValues.dwForceColorFill)
    {
        //if the dwForceColorFill value is TRUE, default color fill flag is 0xffff0000(red color with alpha = 0xff)
        //else dwForceColorFill contains the color info to be filled, use the dwForceColorFill as color value
        bgColor                                = (vpData->ConfigValues.dwForceColorFill == 1) ? 0xffff0000 : vpData->ConfigValues.dwForceColorFill;
        renderParams->pColorFillParams->Color  = bgColor;
        renderParams->pColorFillParams->CSpace = CSpace_sRGB;
        renderParams->pColorFillParams->bYCbCr = false;

        DDI_VP_NORMALMESSAGE("Forced Color Fill: yCbCr %d, CSpace %d, bgColor %x, onePixelBiasinSFC %d",
            renderParams->pColorFillParams->bYCbCr,
            renderParams->pColorFillParams->CSpace,
            renderParams->pColorFillParams->Color,
            renderParams->pColorFillParams->bOnePixelBiasinSFC);
    }

    return S_OK;
}

void DdiVPFunctionsD3D12::SetRotationAndMirror(
    INTEL_STREAM_STATE_ROTATION_DATA stmStateRotation,
    INTEL_STREAM_STATE_MIRROR_DATA   stmStateMirror,
    VPHAL_ROTATION                   &rotation)
{
    DDI_VP_FUNCTION_ENTER;

    if (stmStateRotation.Enable)
    {
        rotation = (VPHAL_ROTATION)stmStateRotation.Rotation;
    }
    else
    {
        rotation = VPHAL_ROTATION_IDENTITY;  // reset
    }

    if (!stmStateMirror.Enable)
    {
        return;
    }

    switch (rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            if (stmStateMirror.FlipHorizontal && stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_ROTATION_180;
            }
            else if (stmStateMirror.FlipHorizontal)
            {
                rotation = VPHAL_MIRROR_HORIZONTAL;
            }
            else if (stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_MIRROR_VERTICAL;
            }
            break;
        case VPHAL_ROTATION_90:
            if (stmStateMirror.FlipHorizontal && stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_ROTATION_270;
            }
            else if (stmStateMirror.FlipHorizontal)
            {
                rotation = VPHAL_ROTATE_90_MIRROR_HORIZONTAL;
            }
            else if (stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_ROTATE_90_MIRROR_VERTICAL;
            }
            break;
        case VPHAL_ROTATION_180:
            if (stmStateMirror.FlipHorizontal && stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_ROTATION_IDENTITY;
            }
            else if (stmStateMirror.FlipHorizontal)
            {
                rotation = VPHAL_MIRROR_VERTICAL;
            }
            else if (stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_MIRROR_HORIZONTAL;
            }
            break;
        case VPHAL_ROTATION_270:
            if (stmStateMirror.FlipHorizontal && stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_ROTATION_90;
            }
            else if (stmStateMirror.FlipHorizontal)
            {
                rotation = VPHAL_ROTATE_90_MIRROR_VERTICAL;
            }
            else if (stmStateMirror.FlipVertical)
            {
                rotation = VPHAL_ROTATE_90_MIRROR_HORIZONTAL;
            }
            break;
        default:
            break;
    }

    return;
}

void DdiVPFunctionsD3D12::Setdeinterlacingflags(
    INTEL_VIDEO_FRAME_FORMAT   stmStateFrameFormat,
    COMPOSITING_SAMPLE_D3D11_1 &auxSample,
    VPHAL_SURFACE_EXT          *surface,
    REFERENCE_TIME             &rtTarget,
    uint32_t                   outputIndex)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(surface);

    switch (stmStateFrameFormat)
    {
        case D3D12DDI_VIDEO_FIELD_TYPE_0020_NONE:
            auxSample.SampleFormat.SampleFormat = DXVA_SampleProgressiveFrame;

            // Disable Interlaced Scaling and Field Weaving for Progressive Input
            surface->bInterlacedScaling = 0;
            surface->bFieldWeaving      = 0;
            break;

        case D3D12DDI_VIDEO_FIELD_TYPE_0020_INTERLACED_TOP_FIELD_FIRST:
            if (surface->bFieldWeaving)
            {
                // sample is single field when field weaving enabled
                auxSample.SampleFormat.SampleFormat = DXVA_SampleFieldSingleEven;
            }
            else
            {
                auxSample.SampleFormat.SampleFormat = DXVA_SampleFieldInterleavedEvenFirst;
            }

            if (outputIndex & 1)
            {
                rtTarget = (auxSample.Start + auxSample.End) / 2;
            }
            else
            {
                rtTarget = auxSample.Start;
            }
            break;

        case D3D12DDI_VIDEO_FIELD_TYPE_0020_INTERLACED_BOTTOM_FIELD_FIRST:
            if (surface->bFieldWeaving)
            {
                // sample is single field when field weaving enabled
                auxSample.SampleFormat.SampleFormat = DXVA_SampleFieldSingleOdd;
            }
            else
            {
                auxSample.SampleFormat.SampleFormat = DXVA_SampleFieldInterleavedOddFirst;
            }

            if (outputIndex & 1)
            {
                rtTarget = (auxSample.Start + auxSample.End) / 2;
            }
            else
            {
                rtTarget = auxSample.Start;
            }
            break;

        default:
            auxSample.SampleFormat.SampleFormat = DXVA_SampleProgressiveFrame;
            break;
    }
    return;
}

template <class T>
HRESULT DdiVPFunctionsD3D12::SetStereoParams(
    DXVA_VP_DATA            *vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    const T                 *inputStreamParameters,
    VPHAL_SURFACE_EXT       *surface,
    uint32_t                stream)
{
    DDI_VP_FUNCTION_ENTER;

    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);

    PMOS_INTERFACE osInterface = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);

    DXVA_DEVICE_CONTEXT *dxvaDevice = osInterface->pOsContext;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);

    surface->Stereo.enable            = vpData->stmStateStereoFormat[stream].Enable ? true : false;
    surface->Stereo.leftViewFrame0    = vpData->stmStateStereoFormat[stream].LeftViewFrame0 ? true : false;
    surface->Stereo.baseViewFrame0    = vpData->stmStateStereoFormat[stream].BaseViewFrame0 ? true : false;
    surface->Stereo.format            = DdiVPFunctionsCommon::GetVpHalStereoFormat(vpData->stmStateStereoFormat[stream].Format);
    surface->Stereo.isSeparateRight   = false;

    if (STREAM_STEREO_SEPARATE(surface))
    {
        PVPHAL_SURFACE_EXT surfaceRight = nullptr;

        // uSrcCount will be increased at the end when all parameters are set.
        // Therefore we need to add surface at uSrcCount + 1
        if (renderParams->pSrc[renderParams->uSrcCount + 1] == nullptr)
        {
            renderParams->pSrc[renderParams->uSrcCount + 1] = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE_EXT));
        }

        surfaceRight = (VPHAL_SURFACE_EXT *)renderParams->pSrc[renderParams->uSrcCount + 1];
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(surfaceRight);

        DXVAUMD_RESOURCE resourceRight = nullptr;
        DWORD            indexRight    = 0;

        resourceRight = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputStreamParameters[stream].InputStream[1].hDrvInputTexture);
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(resourceRight);

        indexRight = inputStreamParameters[stream].InputStream[1].Subresource;

        // Copy fields
        surfaceRight->Rotation                  = surface->Rotation;
        surfaceRight->SurfType                  = surface->SurfType;
        surfaceRight->bInterlacedScaling        = surface->bInterlacedScaling;
        surfaceRight->bFieldWeaving             = surface->bFieldWeaving;
        surfaceRight->bQueryVariance            = surface->bQueryVariance;
        surfaceRight->Stereo.enable             = vpData->stmStateStereoFormat[stream].Enable ? true : false;
        surfaceRight->Stereo.leftViewFrame0     = vpData->stmStateStereoFormat[stream].LeftViewFrame0 ? true : false;
        surfaceRight->Stereo.baseViewFrame0     = vpData->stmStateStereoFormat[stream].BaseViewFrame0 ? true : false;
        surfaceRight->Stereo.format             = DdiVPFunctionsCommon::GetVpHalStereoFormat(vpData->stmStateStereoFormat[stream].Format);

        // Make 'right' specific changes
        MOS_ZeroMemory(&surfaceRight->OsResource, sizeof(MOS_RESOURCE));
        Mos_Specific_SetOsResourceFromDdi(
            resourceRight,
            indexRight,
            0,
            &surfaceRight->OsResource);

        surfaceRight->Stereo.isSeparateRight = true;
    }

    return S_OK;
}

void DdiVPFunctionsD3D12::GetFrameRateandDeinterlaceMode(
    DXVA_VP_DATA            *vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    uint32_t                stream)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(renderParams);

    // adopt the frame rate of primary video
    vpData->InputFrameRate.Numerator   = vpData->stmStateOutputRate[stream].CustomRate.Numerator;
    vpData->InputFrameRate.Denominator = vpData->stmStateOutputRate[stream].CustomRate.Denominator;

    // adopt the deinterlace mode of primary video
    if (vpData->stmStateReserved[stream].DeinterlaceMode == D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAG_0020_NONE)
    {
        vpData->ConfigValues.dwCreatedDeinterlaceMode = VPDDI_PROGRESSIVE;
    }
    else if (vpData->stmStateReserved[stream].DeinterlaceMode == D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAG_0020_BOB)
    {
        vpData->ConfigValues.dwCreatedDeinterlaceMode = VPDDI_BOB;
    }
    else if (vpData->stmStateReserved[stream].DeinterlaceMode == D3D12DDI_VIDEO_PROCESS_DEINTERLACE_FLAG_0020_CUSTOM)
    {
        vpData->ConfigValues.dwCreatedDeinterlaceMode = VPDDI_ADI;
    }

    // Get Input and Output Frame Rate
    renderParams->InputFrameRate.numerator   = vpData->InputFrameRate.Numerator;
    renderParams->InputFrameRate.denominator = vpData->InputFrameRate.Denominator;

    renderParams->OutputFrameRate.numerator   = vpData->outputFrameRate.Numerator;
    renderParams->OutputFrameRate.denominator = vpData->outputFrameRate.Denominator;

    return;
}

template <class T>
bool DdiVPFunctionsD3D12::IsValidViewsForSCD(
    DXVA_DEVICE_CONTEXT                                          *dxvaDevice,
    DXVA_VP_DATA                                                 *vpData,
    const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputStreamParameters,
    uint32_t                                                     streamCount,
    const T                                                      *inputStreamParameters)
{
    bool                                 deviceProtectionEnabled    = false;
    bool                                 outputProtected            = false;
    bool                                 inputProtected             = false;
    bool                                 scdEnabled                 = false;
    DXVAUMD_RESOURCE                     resource                   = nullptr;
    PDXVAUMD_SRESOURCEINFO               sResourceInfo              = nullptr;
    D3DDDI_RESOURCEFLAGS                 resourceFlags              = { 0 };

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(dxvaDevice, scdEnabled);
    DDI_VP_CHK_NULL_RETURN_VALUE(vpData, scdEnabled);
    DDI_VP_CHK_NULL_RETURN_VALUE(outputStreamParameters, scdEnabled);

    // Set Output View
    resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(outputStreamParameters->OutputStream[0].hDrvTexture2D);
    DDI_VP_CHK_NULL_RETURN_VALUE(resource, scdEnabled);

    // Obtain OutputView Resource Info
    sResourceInfo = &resource->ResourceInfo;

    DDI_VP_CHK_NULL_RETURN_VALUE(sResourceInfo->m_pGmmResourceInfo, false);

    // Check if input/output views are protected
    deviceProtectionEnabled = dxvaDevice->IsResourceProtectionEnabled();

    // D3D11_RESOURCE_MISC_RESTRICTED_CONTENT protection flag cached in D3DDDI_RESOURCEFLAGS in Gmm.
    resourceFlags = sResourceInfo->m_pGmmResourceInfo->GetD3d9Flags();

    outputProtected = resourceFlags.RestrictedContent;
    inputProtected  = IsVpInputViewsProtected(vpData, streamCount, inputStreamParameters);

    // For SCD (Screen Capture Defense). If device protection enabled and any one of the input streams is protected,
    // return S_OK without doing anything if the output is un-protected.
    if (deviceProtectionEnabled && !outputProtected && inputProtected)
    {
        dxvaDevice->PavpProtectionTriggered(VPBLT_PROTECTION_TRIGGERED);
        scdEnabled = true;
    }

    return scdEnabled;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalProcampParams(
    PDXVA_VP_DATA           vpData,
    PVPHAL_RENDER_PARAMS    renderParams,
    PVPHAL_SURFACE          surface,
    uint32_t                stream)
{
    HRESULT hr                 = S_OK;
    bool    cuiProcampEnabled  = false;
    bool    ddiProcampEnabled  = false;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    // CUI is not enabled for Vprep
    cuiProcampEnabled = (vpData->CUIRegistryValues.bProcAmpApplyAlways) &&
        (renderParams->Component != COMPONENT_VPreP) &&
        (vpData->stmStateAutoProcessingMode[stream].Enable);

    ddiProcampEnabled = vpData->stmStateFilterBrightness[stream].Enable ||
        vpData->stmStateFilterContrast[stream].Enable ||
        vpData->stmStateFilterHue[stream].Enable ||
        vpData->stmStateFilterSaturation[stream].Enable;

    if (cuiProcampEnabled || ddiProcampEnabled)
    {
        if (surface->pProcampParams == nullptr)
        {
            surface->pProcampParams = (PVPHAL_PROCAMP_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_PROCAMP_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pProcampParams);
        }

        surface->pProcampParams->bEnabled = true;

        if (cuiProcampEnabled)
        {
            surface->pProcampParams->fBrightness   = vpData->CUIRegistryValues.fProcAmpBrightness;
            surface->pProcampParams->fContrast     = vpData->CUIRegistryValues.fProcAmpContrast;
            surface->pProcampParams->fHue          = vpData->CUIRegistryValues.fProcAmpHue;
            surface->pProcampParams->fSaturation   = vpData->CUIRegistryValues.fProcAmpSaturation;
        }
        else
        {
            //out of bounds conditions are checked in VPHAL layer
            surface->pProcampParams->fBrightness   = vpData->stmStateFilterBrightness[stream].Enable ?
                PROCAMP_BRIGHTNESS_STEP * vpData->stmStateFilterBrightness[stream].Level :
                PROCAMP_BRIGHTNESS_DEFAULT;

            surface->pProcampParams->fContrast     = vpData->stmStateFilterContrast[stream].Enable ?
                PROCAMP_CONTRAST_STEP * vpData->stmStateFilterContrast[stream].Level :
                PROCAMP_CONTRAST_DEFAULT;

            surface->pProcampParams->fHue          = vpData->stmStateFilterHue[stream].Enable ?
                PROCAMP_HUE_STEP * vpData->stmStateFilterHue[stream].Level :
                PROCAMP_HUE_DEFAULT;

            surface->pProcampParams->fSaturation   = vpData->stmStateFilterSaturation[stream].Enable ?
                PROCAMP_SATURATION_STEP * vpData->stmStateFilterSaturation[stream].Level :
                PROCAMP_SATURATION_DEFAULT;
        }

        // If the input procamp value is default, turn off procamp.
        if (!DdiVPFunctionsCommon::VpHalDdiProcAmpValuesNotDefault(*(surface->pProcampParams)))
        {
            MOS_SafeFreeMemory(surface->pProcampParams);
            surface->pProcampParams = nullptr;
        }
    }
    else
    {
        MOS_SafeFreeMemory(surface->pProcampParams);
        surface->pProcampParams = nullptr;
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalNLASParams(
    PDXVA_VP_DATA            vpData,
    VPHAL_RENDER_PARAMS_EXT  *renderParams,
    VPHAL_SURFACE_EXT        *surface,
    uint32_t                 stream)
{
    HRESULT hr             = S_OK;
    bool    cuiNLASEnabled = false;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    auto skuTable = vpData->pVpHalState->GetSkuTable();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(skuTable);

    // CUI is not enabled for Vprep
    cuiNLASEnabled = (vpData->CUIRegistryValues.bEnableNLAS) &&
        (renderParams->Component != COMPONENT_VPreP) &&
        (vpData->stmStateAutoProcessingMode[stream].Enable);

    if (cuiNLASEnabled &&
        (renderParams->bDisableVpFor4K == false && renderParams->bDisableVeboxFor8K == false) &&
        surface->Rotation == VPHAL_ROTATION_IDENTITY &&      // Enable NLAS only when there is no rotation
        GRAPHICS_IS_SKU(skuTable, FtrNLAScaling))            // Disable NLAS by setting FtrNLAScaling to false in SkuTable.
    {
        if (surface->pNLASParams == nullptr)
        {
            surface->pNLASParams = (PVPHAL_NLAS_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_NLAS_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pNLASParams);
        }

        surface->pNLASParams->fHLinearRegion   = vpData->CUIRegistryValues.fNLASHLinearRegion;
        surface->pNLASParams->fNonLinearCrop   = vpData->CUIRegistryValues.fNLASNonLinearCrop;
        surface->pNLASParams->fVerticalCrop    = vpData->CUIRegistryValues.fNLASVerticalCrop;
    }
    else if (surface->pNLASParams)
    {
        MOS_SafeFreeMemory(surface->pNLASParams);
        surface->pNLASParams = nullptr;
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalDenoiseParams(
    PDXVA_VP_DATA           vpData,
    PVPHAL_RENDER_PARAMS    renderParams,
    PVPHAL_SURFACE          surface,
    uint32_t                stream,
    DXVAUMD_RESOURCE        resource)
{
    HRESULT hr           = S_OK;
    bool    dnEnabledCUI = false;
    bool    dnEnabledAPI = false;
    bool    iefNeedGNE   = false;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    if (renderParams->Component == COMPONENT_VPreP)
    {
        // VPreP denoise ignore CUI setting
        if (vpData->stmStateFilterNoiseReduction[stream].Enable)
        {
            if (surface->pDenoiseParams == nullptr)
            {
                surface->pDenoiseParams = (PVPHAL_DENOISE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_DENOISE_PARAMS));
                DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pDenoiseParams);
            }

            surface->pDenoiseParams->bEnableLuma   = true;
            surface->pDenoiseParams->bEnableChroma = false;
            surface->pDenoiseParams->NoiseLevel    = NOISELEVEL_DEFAULT;

            if (vpData->stmStateFilterNoiseReduction[stream].Level > NOISEREDUCTION_MIN)
            {
                surface->pDenoiseParams->bAutoDetect = false;
                if (vpData->stmStateFilterNoiseReduction[stream].Level > NOISEREDUCTION_MAX)
                {
                    surface->pDenoiseParams->fDenoiseFactor = (float)NOISEREDUCTION_MAX;
                }
                else
                {
                    surface->pDenoiseParams->fDenoiseFactor = (float)vpData->stmStateFilterNoiseReduction[stream].Level;
                }
            }
            else
            {
                surface->pDenoiseParams->bAutoDetect = true;
            }
        }
        else
        {
            MOS_SafeFreeMemory(surface->pDenoiseParams);
            surface->pDenoiseParams = nullptr;
        }
    }
    else // VPostP
    {
        // Check if DN is enabled through CUI
        dnEnabledCUI = (vpData->CUIRegistryValues.bDenoiseEnabledAlways) &&
                       (vpData->stmStateAutoProcessingMode[stream].Enable) &&
                       ((vpData->CUIRegistryValues.bDenoiseAutoDetect) ||
                        (vpData->CUIRegistryValues.fDenoiseFactor > 0.0f));

        dnEnabledAPI = (vpData->stmStateFilterNoiseReduction[stream].Enable &&
                        vpData->stmStateFilterNoiseReduction[stream].Level > NOISEREDUCTION_MIN);

        iefNeedGNE = (vpData->stmStateAutoProcessingMode[stream].Enable &&
                     (vpData->CUIRegistryValues.bUiAutoModeEnable ||
                      vpData->CUIRegistryValues.bSharpnessOptimalEnabledAlways) &&
                      vpData->CUIRegistryValues.bIEFAutoMode);

        if (dnEnabledAPI || dnEnabledCUI || iefNeedGNE)
        {
            if (surface->pDenoiseParams == nullptr)
            {
                surface->pDenoiseParams = (PVPHAL_DENOISE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_DENOISE_PARAMS));
                DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pDenoiseParams);
            }

            surface->pDenoiseParams->bEnableLuma   = true;
            surface->pDenoiseParams->bEnableChroma = true;

            // Set Denoise using CUI params
            if (dnEnabledCUI)
            {
                surface->pDenoiseParams->bEnableChroma     = vpData->CUIRegistryValues.bDenoiseEnableChroma ? true : false;
                surface->pDenoiseParams->bAutoDetect       = vpData->CUIRegistryValues.bDenoiseAutoDetect ? true : false;
                surface->pDenoiseParams->fDenoiseFactor    = vpData->CUIRegistryValues.fDenoiseFactor;
            }
            // Set Denoise using App params
            else if (dnEnabledAPI)
            {
                surface->pDenoiseParams->bAutoDetect       = true;
                surface->pDenoiseParams->fDenoiseFactor    = (float)vpData->stmStateFilterNoiseReduction[stream].Level;
            }
            // Just enable for IEF GNE auto mode
            else
            {
                surface->pDenoiseParams->bAutoDetect       = false;
                surface->pDenoiseParams->fDenoiseFactor    = 0.0f;
            }

            surface->pDenoiseParams->NoiseLevel = NOISELEVEL_DEFAULT;
        }
        else
        {
            MOS_SafeFreeMemory(surface->pDenoiseParams);
            surface->pDenoiseParams = nullptr;
        }
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalISParams(
    PDXVA_VP_DATA           vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    uint32_t                stream)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);

    auto skuTable = vpData->pVpHalState->GetSkuTable();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(skuTable);
    //---------------------------------
    // Setup Image Stabilization Params
    //---------------------------------
    // In VpreP, App sets IS Mode
    if (renderParams->Component == COMPONENT_VPreP)
    {
        // Upscale Mode is never used
        if (GRAPHICS_IS_SKU(skuTable, FtrIStabFilter)&&
           (renderParams->bDisableVpFor4K == false && renderParams->bDisableVeboxFor8K == false) &&
           (vpData->vpeStreamState[stream].ISParam.Mode == VPE_VPREP_ISTAB_MODE_CROPUPSCALE || vpData->vpeStreamState[stream].ISParam.Mode == VPE_VPREP_ISTAB_MODE_CROP))
        {
            if (renderParams->pIStabParams == nullptr)
            {
                renderParams->pIStabParams = (PVPHAL_ISTAB_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_ISTAB_PARAMS));
                DDI_VP_CHK_NULL_RETURN_VALUE(renderParams->pIStabParams, E_OUTOFMEMORY);

                renderParams->pIStabParams->bAllocated = true;
            }

            if (vpData->vpeStreamState[stream].ISParam.Mode == VPE_VPREP_ISTAB_MODE_CROPUPSCALE)
            {
                renderParams->pIStabParams->IStabMode = ISTAB_MODE_CROP_FULL_ZOOM;
            }
            else
            {
                renderParams->pIStabParams->IStabMode = ISTAB_MODE_CROP;
            }
            renderParams->pIStabParams->fISCropPercentage  = 0.09f;  // should be changed to accept app passed values.
            renderParams->pIStabParams->bEnableIStabLBD    = 1;
            renderParams->pIStabParams->bEnableIStabSCD    = 1;
            renderParams->pIStabParams->bEnableIStabSD     = 1;
        }
        else
        {
            MOS_SafeFreeMemory(renderParams->pIStabParams);
            renderParams->pIStabParams = nullptr;
        }
    }
    // In VpostP, IS Mode is read from CUI, IS supports only progressive inputs
    else if (GRAPHICS_IS_SKU(skuTable, FtrIStabFilter) &&
        vpData->CUIRegistryValues.bEnableIS &&
        (renderParams->bDisableVpFor4K == false && renderParams->bDisableVeboxFor8K == false) &&
        vpData->CUIRegistryValues.wIStabMode > ISTAB_MODE_NONE &&
        vpData->CUIRegistryValues.wIStabMode <= ISTAB_MODE_CROP_FULL_ZOOM &&
        vpData->stmStateFrameFormat[stream] == D3D12DDI_VIDEO_FIELD_TYPE_0020_NONE)
    {
        if (renderParams->pIStabParams == nullptr)
        {
            renderParams->pIStabParams = (PVPHAL_ISTAB_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_ISTAB_PARAMS));
            DDI_VP_CHK_NULL_RETURN_VALUE(renderParams->pIStabParams, E_OUTOFMEMORY);

            renderParams->pIStabParams->bAllocated = true;
        }

        renderParams->pIStabParams->IStabMode = (VPHAL_ISTAB_MODE)vpData->CUIRegistryValues.wIStabMode;
        // Fast mode vs Quality mode is Gen 8+ feature
        // Set default FQ mode to quality mode for now, we can set the default 
        // based on HW platforms once performance information is available.
        if (vpData->CUIRegistryValues.wIStabFQMode == ISTAB_FQ_MODE_FAST)
        {
            renderParams->pIStabParams->IStabFQMode = ISTAB_FQ_MODE_FAST;
        }
        else
        {
            renderParams->pIStabParams->IStabFQMode = ISTAB_FQ_MODE_QUALITY;
        }
        renderParams->pIStabParams->fISCropPercentage  = (float)vpData->CUIRegistryValues.wISCrop / 100;
        renderParams->pIStabParams->ProcessAllOnFirst  =  vpData->CUIRegistryValues.bISProcessAllOnFirst ? true : false;
        renderParams->pIStabParams->bEnableIStabLBD    = true;
        renderParams->pIStabParams->bEnableIStabSCD    = true;
        renderParams->pIStabParams->bEnableIStabSD     = true;
        renderParams->pIStabParams->bEnableIStabSTD    = true;
        renderParams->pIStabParams->bEnableIStabHME    = true;
        renderParams->pIStabParams->bEnableIStabFBD    = true;
        renderParams->pIStabParams->bEnableIStabDSW    = false;   // disable debug surface
        renderParams->pIStabParams->bEnableIStabQM     = (renderParams->pIStabParams->IStabFQMode == ISTAB_FQ_MODE_QUALITY) ? true : false;
    }
    else
    {
        MOS_SafeFreeMemory(renderParams->pIStabParams);
        renderParams->pIStabParams = nullptr;
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalIEFParams(
    PDXVA_VP_DATA           vpData,
    PVPHAL_RENDER_PARAMS    renderParams,
    PVPHAL_SURFACE          surface,
    uint32_t                stream)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    if (renderParams->Component == COMPONENT_VPreP)
    {
        // VPreP IEF ignore CUI setting
        if (vpData->stmStateFilterEdgeEnhancement[stream].Enable &&
            vpData->stmStateFilterEdgeEnhancement[stream].Level > EDGEENHANCEMENT_MIN)
        {
            if (surface->pIEFParams == nullptr)
            {
                PVPHAL_IEF_EXT_PARAMS iefParamsExt = nullptr;

                surface->pIEFParams = (PVPHAL_IEF_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_IEF_PARAMS));
                DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pIEFParams);

                iefParamsExt        = (PVPHAL_IEF_EXT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_IEF_EXT_PARAMS));
                DDI_VP_CHECK_NULL_WITH_HR_RETURN(iefParamsExt);

                surface->pIEFParams->pExtParam = iefParamsExt;
            }

            surface->pIEFParams->bEnabled = true;
            // Init default value because VPrep flow don't have default value.
            DdiVPFunctionsCommon::VpHalDdiInitIEFParamsExt(surface->pIEFParams);

            if (vpData->stmStateFilterEdgeEnhancement[stream].Level > EDGEENHANCEMENT_MAX)
            {
                surface->pIEFParams->fIEFFactor = (float)EDGEENHANCEMENT_MAX;
            }
            else
            {
                surface->pIEFParams->fIEFFactor = (float)vpData->stmStateFilterEdgeEnhancement[stream].Level;
            }
        }
        else
        {
            if (surface->pIEFParams)
            {
                MOS_SafeFreeMemory(surface->pIEFParams->pExtParam);
                MOS_FreeMemory(surface->pIEFParams);
                surface->pIEFParams = nullptr;
            }
        }
    }
    else // VPostP
    {
        // If Detail is enabled from App or CUI and primary video format is not RGB
        if (((vpData->stmStateFilterEdgeEnhancement[stream].Enable                        &&
              vpData->stmStateFilterEdgeEnhancement[stream].Level > EDGEENHANCEMENT_MIN)  ||
            (vpData->CUIRegistryValues.bDetailEnabledAlways                                &&
             vpData->stmStateAutoProcessingMode[stream].Enable                            &&
             vpData->CUIRegistryValues.fDetailFactor > 0.0f))                              &&
             (!(IS_RGB_FORMAT(surface->Format))))
        {
            if (surface->pIEFParams == nullptr)
            {
                PVPHAL_IEF_EXT_PARAMS iefParamsExt = nullptr;

                surface->pIEFParams = (PVPHAL_IEF_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_IEF_PARAMS));
                DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pIEFParams);

                iefParamsExt = (PVPHAL_IEF_EXT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_IEF_EXT_PARAMS));
                DDI_VP_CHECK_NULL_WITH_HR_RETURN(iefParamsExt);

                surface->pIEFParams->pExtParam = iefParamsExt;
            }

            surface->pIEFParams->bEnabled = true;

            // Init default value. Though CUI have default value, but API flow don't have default value.
            DdiVPFunctionsCommon::VpHalDdiInitIEFParamsExt(surface->pIEFParams);

            // Set IEF using CUI params
            if (vpData->CUIRegistryValues.bDetailEnabledAlways)
            {
                DdiVPFunctionsCommon::SetIEFParamsFromCUI(&vpData->CUIRegistryValues, surface->pIEFParams);
            }
            // Set IEF using App params
            else
            {
                surface->pIEFParams->fIEFFactor = (float)vpData->stmStateFilterEdgeEnhancement[stream].Level;
            }
        }
        else
        {
            if (surface->pIEFParams)
            {
                MOS_SafeFreeMemory(surface->pIEFParams->pExtParam);
                MOS_FreeMemory(surface->pIEFParams);
                surface->pIEFParams = nullptr;
            }
        }
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalColorPipeParams(
    DXVA_DEVICE_CONTEXT     *context,
    PDXVA_VP_DATA           vpData,
    PVPHAL_RENDER_PARAMS    renderParams,
    PVPHAL_SURFACE_EXT      surface,
    uint32_t                stream)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(context);

    auto skuTable = vpData->pVpHalState->GetSkuTable();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(skuTable);

    // CUI is Disabled for Vprep
    if (GRAPHICS_IS_SKU(skuTable, FtrCPFilter) &&
        (vpData->CUIRegistryValues.bEnableSTE ||
        vpData->CUIRegistryValues.bEnableACE ||
        vpData->CUIRegistryValues.bEnableTCC) &&
        vpData->stmStateAutoProcessingMode[stream].Enable &&
        (renderParams->Component != COMPONENT_VPreP))
    {
        if (surface->pColorPipeParams == nullptr)
        {
            surface->pColorPipeParams = (PVPHAL_COLORPIPE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pColorPipeParams);
        }
        if (surface->pColorPipeExtParams == nullptr)
        {
            surface->pColorPipeExtParams = (PVPHAL_COLORPIPE_EXT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_EXT_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pColorPipeExtParams);
        }

        DdiVPFunctionsCommon::SetIECPParamsFromCUI(context->m_userSettingPtr, vpData->pVpHalState, &vpData->CUIRegistryValues, surface->pColorPipeParams, surface->pColorPipeExtParams);
    }
    else
    {
        if (surface->pColorPipeParams)
        {
            MOS_SafeFreeMemory(surface->pColorPipeParams);
            surface->pColorPipeParams = nullptr;
        }
        if (surface->pColorPipeExtParams)
        {
            MOS_SafeFreeMemory(surface->pColorPipeExtParams);
            surface->pColorPipeExtParams = nullptr;
        }
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalGamutCompressionParams(
    DXVA_VP_DATA            *vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    VPHAL_SURFACE_EXT       *surface,
    uint32_t                stream)
{
    HRESULT      hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    auto skuTable = vpData->pVpHalState->GetSkuTable();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(skuTable);

    if (GRAPHICS_IS_SKU(skuTable, FtrGamutCompressionMedia) &&
        (vpData->CUIRegistryValues.wGCompMode != GAMUT_MODE_NONE) &&  // GC is enabled
        vpData->stmStateAutoProcessingMode[stream].Enable &&
        (renderParams->Component != COMPONENT_VPreP))
    {
        if (surface->pGamutParams == nullptr)
        {
            surface->pGamutParams = (PVPHAL_GAMUT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_GAMUT_PARAMS));
            DDI_VP_CHK_NULL_RETURN_VALUE(surface->pGamutParams, E_OUTOFMEMORY);
        }

        surface->pGamutParams->GCompMode = (VPHAL_GAMUT_MODE)vpData->CUIRegistryValues.wGCompMode;

        if (vpData->ChromaticityTbl.dwIgnoreChromaticity)
        {
            surface->pGamutParams->displayRGBW_x[0] = 0.576f;
            surface->pGamutParams->displayRGBW_x[1] = 0.331f;
            surface->pGamutParams->displayRGBW_x[2] = 0.143f;
            surface->pGamutParams->displayRGBW_x[3] = 0.315f;
            surface->pGamutParams->displayRGBW_y[0] = 0.343f;
            surface->pGamutParams->displayRGBW_y[1] = 0.555f;
            surface->pGamutParams->displayRGBW_y[2] = 0.104f;
            surface->pGamutParams->displayRGBW_y[3] = 0.329f;
        }
        else
        {
            surface->pGamutParams->displayRGBW_x[0] = vpData->ChromaticityTbl.Rx;
            surface->pGamutParams->displayRGBW_x[1] = vpData->ChromaticityTbl.Gx;
            surface->pGamutParams->displayRGBW_x[2] = vpData->ChromaticityTbl.Bx;
            surface->pGamutParams->displayRGBW_x[3] = vpData->ChromaticityTbl.Wx;
            surface->pGamutParams->displayRGBW_y[0] = vpData->ChromaticityTbl.Ry;
            surface->pGamutParams->displayRGBW_y[1] = vpData->ChromaticityTbl.Gy;
            surface->pGamutParams->displayRGBW_y[2] = vpData->ChromaticityTbl.By;
            surface->pGamutParams->displayRGBW_y[3] = vpData->ChromaticityTbl.Wy;
        }
    }
    else
    {
        if (surface->pGamutParams)
        {
            MOS_SafeFreeMemory(surface->pGamutParams);
            surface->pGamutParams = nullptr;
        }
    }

    return hr;
}

template <class T>
HRESULT DdiVPFunctionsD3D12::SetVpHalFrcParams(
    DXVA_VP_DATA            *vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    const T                 *streams,
    uint32_t                stream)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);

    auto skuTable = vpData->pVpHalState->GetSkuTable();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(skuTable);

    // Setup FRC parameters
    if ((GRAPHICS_IS_SKU(skuTable, FtrFrameRateConversion24p60p) ||
         GRAPHICS_IS_SKU(skuTable, FtrFrameRateConversion30p60p)) &&
        (renderParams->bDisableVpFor4K == false && renderParams->bDisableVeboxFor8K == false) &&
        vpData->stmStateOutputRate[stream].OutputRate == D3D11_1DDI_VIDEO_PROCESSOR_OUTPUT_RATE_CUSTOM)
    {
        DDI_VP_CHK_HR_RETURN(MosUtilities::MosStatusToOsResult(DdiVPFunctionsCommon::VpHalDdiSetFrcParams(
            renderParams,
            vpData->stmStateOutputRate[stream].RepeatFrame ? true : false,
            vpData->stmStateOutputRate[stream].CustomRate.Numerator,
            vpData->stmStateOutputRate[stream].CustomRate.Denominator,
            streams[stream].RateInfo.OutputIndex,
            streams[stream].RateInfo.InputFrameOrField)));
    }
    else
    {
        if (renderParams->pFrcParams)
        {
            MOS_SafeFreeMemory(renderParams->pFrcParams);
            renderParams->pFrcParams = nullptr;
        }
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalCapPipeParams(
    DXVA_VP_DATA            *vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    VPHAL_SURFACE_EXT       *surface)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    if (!(vpData->vpeOutputState.pCamPipeParams &&
        vpData->vpeOutputState.pCamPipeParams->Active.bActive &&
        (renderParams->Component == COMPONENT_CapPipe)))
    {
        MOS_SafeFreeMemory(surface->pCapPipeParams);
        surface->pCapPipeParams = nullptr;
        return hr;
    }

    if (surface->SampleType != SAMPLE_PROGRESSIVE)
    {
        DDI_VP_ASSERTMESSAGE("Capture Pipe does not support DI.");
        MOS_SafeFreeMemory(surface->pCapPipeParams);
        surface->pCapPipeParams = nullptr;
        return E_INVALIDARG;
    }

    if (surface->pCapPipeParams == nullptr)
    {
        surface->pCapPipeParams = (PVPHAL_CAPPIPE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_CAPPIPE_PARAMS));
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pCapPipeParams);
    }

    // Hot Pixel
    if (vpData->vpeOutputState.pCamPipeParams->HotPixel.bActive)
    {
        surface->pCapPipeParams->HotPixelParams.bActive = true;
        surface->pCapPipeParams->HotPixelParams.PixelThreshold =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->HotPixel.PixelThresholdDifference, HOTPIXEL_THRESHOLD_MAX);
        surface->pCapPipeParams->HotPixelParams.PixelCount =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->HotPixel.PixelCountThreshold, HOTPIXEL_COUNT_MAX);
    }
    else
    {
        surface->pCapPipeParams->HotPixelParams.bActive = false;
    }

    // Vignette Correction
    if (vpData->vpeOutputState.pCamPipeParams->Vignette.bActive)
    {
        surface->pCapPipeParams->VignetteParams.bActive        = vpData->vpeOutputState.pCamPipeParams->Vignette.bActive ? true : false;
        surface->pCapPipeParams->VignetteParams.Width          = vpData->vpeOutputState.pCamPipeParams->Vignette.Width;
        surface->pCapPipeParams->VignetteParams.Height         = vpData->vpeOutputState.pCamPipeParams->Vignette.Height;
        surface->pCapPipeParams->VignetteParams.Stride         = vpData->vpeOutputState.pCamPipeParams->Vignette.Stride;
        surface->pCapPipeParams->VignetteParams.pCorrectionMap = (uint8_t*)vpData->vpeOutputState.pCamPipeParams->Vignette.pCorrectionMap;
    }
    else
    {
        surface->pCapPipeParams->VignetteParams.bActive = false;
    }

    // Black Level Correction
    if (vpData->vpeOutputState.pCamPipeParams->BlackLevel.bActive)
    {
        surface->pCapPipeParams->BlackLevelParams.bActive =
            vpData->vpeOutputState.pCamPipeParams->BlackLevel.bActive ? true : false;
        surface->pCapPipeParams->BlackLevelParams.R =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->BlackLevel.R, BLC_MAX);
        surface->pCapPipeParams->BlackLevelParams.G0 =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->BlackLevel.G0, BLC_MAX);
        surface->pCapPipeParams->BlackLevelParams.B =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->BlackLevel.B, BLC_MAX);
        surface->pCapPipeParams->BlackLevelParams.G1 =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->BlackLevel.G1, BLC_MAX);
    }
    else
    {
        surface->pCapPipeParams->BlackLevelParams.bActive = false;
    }

    // White Balance
    if (vpData->vpeOutputState.pCamPipeParams->WhiteBalance.bActive)
    {
        surface->pCapPipeParams->WhiteBalanceParams.bActive = true;
        surface->pCapPipeParams->WhiteBalanceParams.RedCorrection =
            MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->WhiteBalance.RedCorrection, WHITE_BALANCE_MIN, WHITE_BALANCE_MAX);
        surface->pCapPipeParams->WhiteBalanceParams.GreenBottomCorrection =
            MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->WhiteBalance.GreenBottomCorrection, WHITE_BALANCE_MIN, WHITE_BALANCE_MAX);
        surface->pCapPipeParams->WhiteBalanceParams.BlueCorrection =
            MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->WhiteBalance.BlueCorrection, WHITE_BALANCE_MIN, WHITE_BALANCE_MAX);
        surface->pCapPipeParams->WhiteBalanceParams.GreenTopCorrection =
            MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->WhiteBalance.GreenTopCorrection, WHITE_BALANCE_MIN, WHITE_BALANCE_MAX);

        switch (vpData->vpeOutputState.pCamPipeParams->WhiteBalance.Mode)
        {
            case WHITE_BALANCE_MODE_MANUAL:
                surface->pCapPipeParams->WhiteBalanceParams.Mode = VPHAL_WB_MANUAL;
                break;

            case WHITE_BALANCE_MODE_AUTO_IMAGE:
                // Not supported. Fall back to manual rather than fail WB setup
            default:
                surface->pCapPipeParams->WhiteBalanceParams.Mode = VPHAL_WB_MANUAL;
                break;
        }
    }
    else
    {
        surface->pCapPipeParams->WhiteBalanceParams.bActive = false;
    }

    // Color Correction
    if (vpData->vpeOutputState.pCamPipeParams->ColorCorrection.bActive)
    {
        surface->pCapPipeParams->ColorCorrectionParams.bActive = true;

        for (uint32_t i = 0; i < 3; i++)
        {
            for (uint32_t j = 0; j < 3; j++)
            {
                surface->pCapPipeParams->ColorCorrectionParams.CCM[i][j] =
                    MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->ColorCorrection.CCM[i][j], COLOR_CORR_MIN, COLOR_CORR_MAX);
            }
        }
    }
    else
    {
        surface->pCapPipeParams->ColorCorrectionParams.bActive = false;

    }

    // Forward Gamma Correction
    if (vpData->vpeOutputState.pCamPipeParams->ForwardGamma.bActive)
    {
        surface->pCapPipeParams->FwdGammaParams.bActive = true;
        // Just copy the pointer here, need to set the value by different platform in vphal
        surface->pCapPipeParams->FwdGammaParams.pAppSegment = (PVPHAL_FORWARD_GAMMA_SEG)vpData->vpeOutputState.pCamPipeParams->ForwardGamma.pFGCSegment;
    }
    else
    {
        surface->pCapPipeParams->FwdGammaParams.bActive = false;
    }

    // App specified Front End CSC
    if (vpData->vpeOutputState.pCamPipeParams->RgbToYuvCSC.bActive)
    {
        surface->pCapPipeParams->FECSCParams.bActive = true;

        for (uint32_t i = 0; i < 3; i++)
        {
            surface->pCapPipeParams->FECSCParams.PreOffset[i] =
                MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->RgbToYuvCSC.PreOffset[i], FECSC_PRE_OFFSET_MIN, FECSC_PRE_OFFSET_MAX);
        }

        for (uint32_t i = 0; i < 3; i++)
        {
            for (uint32_t j = 0; j < 3; j++)
            {
                surface->pCapPipeParams->FECSCParams.Matrix[i][j] =
                    MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->RgbToYuvCSC.Matrix[i][j], FECSC_MATRIX_MIN, FECSC_MATRIX_MAX);
            }
        }

        for (uint32_t i = 0; i < 3; i++)
        {
            surface->pCapPipeParams->FECSCParams.PostOffset[i] =
                MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->RgbToYuvCSC.PostOffset[i], FECSC_POST_OFFSET_MIN, FECSC_POST_OFFSET_MAX);
        }
    }
    else
    {
        surface->pCapPipeParams->FECSCParams.bActive = false;
    }

    // App specified Back End CSC
    if (vpData->vpeOutputState.pCamPipeParams->YuvToRgbCSC.bActive)
    {
        surface->pCapPipeParams->BECSCParams.bActive = true;

        for (uint32_t i = 0; i < 3; i++)
        {
            surface->pCapPipeParams->BECSCParams.PreOffset[i] =
                MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->YuvToRgbCSC.PreOffset[i], BECSC_PRE_OFFSET_MIN, BECSC_PRE_OFFSET_MAX);
        }

        for (uint32_t i = 0; i < 3; i++)
        {
            for (uint32_t j = 0; j < 3; j++)
            {
                surface->pCapPipeParams->BECSCParams.Matrix[i][j] =
                    MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->YuvToRgbCSC.Matrix[i][j], BECSC_MATRIX_MIN, BECSC_MATRIX_MAX);
            }
        }

        for (uint32_t i = 0; i < 3; i++)
        {
            surface->pCapPipeParams->BECSCParams.PostOffset[i] =
                MOS_CLAMP_MIN_MAX(vpData->vpeOutputState.pCamPipeParams->YuvToRgbCSC.PostOffset[i], BECSC_POST_OFFSET_MIN, BECSC_POST_OFFSET_MAX);
        }
    }
    else
    {
        surface->pCapPipeParams->BECSCParams.bActive = false;
    }

    // Lens Geometry & Chromatic Aberration
    if (vpData->vpeOutputState.pCamPipeParams->LensCorrection.bActive)
    {
        surface->pCapPipeParams->LensCorrectionParams.bActive = true;

        MOS_SecureMemcpy(
            surface->pCapPipeParams->LensCorrectionParams.a,
            sizeof(float)* 3, // 1st channel, 3 coeff per chaneel
            vpData->vpeOutputState.pCamPipeParams->LensCorrection.a,
            sizeof(float)* 3);

        MOS_SecureMemcpy(
            surface->pCapPipeParams->LensCorrectionParams.b,
            sizeof(float)* 3, // 2nd channel, 3 coeff per chaneel
            vpData->vpeOutputState.pCamPipeParams->LensCorrection.b,
            sizeof(float)* 3);

        MOS_SecureMemcpy(
            surface->pCapPipeParams->LensCorrectionParams.c,
            sizeof(float)* 3, // 3rd channel, 3 coeff per chaneel
            vpData->vpeOutputState.pCamPipeParams->LensCorrection.c,
            sizeof(float)* 3);

        MOS_SecureMemcpy(
            surface->pCapPipeParams->LensCorrectionParams.d,
            sizeof(float)* 3, // 4th channel, 3 coeff per chaneel
            vpData->vpeOutputState.pCamPipeParams->LensCorrection.d,
            sizeof(float)* 3);
    }

    // ICC Color Conversion (3D LUT)
    if (vpData->vpeOutputState.pCamPipeParams->LUT.bActive)
    {
        surface->pCapPipeParams->ICCColorConversionParams.bActive = true;
        if (vpData->vpeOutputState.pCamPipeParams->LUT.LUTSize == LUT17_SEG)
        {
            surface->pCapPipeParams->ICCColorConversionParams.LUTSize      = LUT17_SEG;
            surface->pCapPipeParams->ICCColorConversionParams.LUTLength    = LUT17_SEG * LUT17_SEG * LUT17_MUL * sizeof(LUT_ENTRY);
            surface->pCapPipeParams->ICCColorConversionParams.pLUT         = (uint8_t*)vpData->vpeOutputState.pCamPipeParams->LUT.p17;
        }
        else if (vpData->vpeOutputState.pCamPipeParams->LUT.LUTSize == LUT33_SEG)
        {
            surface->pCapPipeParams->ICCColorConversionParams.LUTSize      = LUT33_SEG;
            surface->pCapPipeParams->ICCColorConversionParams.LUTLength    = LUT33_SEG * LUT33_SEG * LUT33_MUL * sizeof(LUT_ENTRY);
            surface->pCapPipeParams->ICCColorConversionParams.pLUT         = (uint8_t*)vpData->vpeOutputState.pCamPipeParams->LUT.p33;
        }
        else if (vpData->vpeOutputState.pCamPipeParams->LUT.LUTSize == LUT65_SEG)
        {
            surface->pCapPipeParams->ICCColorConversionParams.LUTSize      = LUT65_SEG;
            surface->pCapPipeParams->ICCColorConversionParams.LUTLength    = LUT65_SEG * LUT65_SEG * LUT65_MUL * sizeof(LUT_ENTRY);
            surface->pCapPipeParams->ICCColorConversionParams.pLUT         = (uint8_t*)vpData->vpeOutputState.pCamPipeParams->LUT.p65;
        }
        else
        {
            DDI_VP_ASSERTMESSAGE("The LUT size is invalid");
        }
    }

    // Debayer Params MSB/ LSB Aligned Bayer Input
    surface->pCapPipeParams->DebayerParams.BayerInput          = vpData->vpeOutputState.pCamPipeParams->DeBayer.BayerInput;
    surface->pCapPipeParams->DebayerParams.LSBBayerBitDepth    = vpData->vpeOutputState.pCamPipeParams->DeBayer.LSBBayerBitDepth;

    // IECP feature - Skin Tone Enchancement
    if (vpData->vpeOutputState.pCamPipeParams->SkinToneEnhance.bActive)
    {
        if (!surface->pColorPipeParams)
        {
            surface->pColorPipeParams = (PVPHAL_COLORPIPE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pColorPipeParams);
        }
        if (!surface->pColorPipeExtParams)
        {
            surface->pColorPipeExtParams = (PVPHAL_COLORPIPE_EXT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_EXT_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pColorPipeExtParams);
        }

        surface->pColorPipeParams->bEnableSTE = true;
        surface->pColorPipeParams->SteParams.dwSTEFactor = vpData->vpeOutputState.pCamPipeParams->SkinToneEnhance.STEFactor;
    }

    // IECP feature - Gamut Compression
    if (vpData->vpeOutputState.pCamPipeParams->GamutCompress.bActive)
    {
        if (!surface->pGamutParams)
        {
            surface->pGamutParams = (PVPHAL_GAMUT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_GAMUT_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pGamutParams);
        }

        surface->pGamutParams->GCompMode = 
            (vpData->vpeOutputState.pCamPipeParams->GamutCompress.Mode == GAMUT_COMPRESS_BASIC) ? GAMUT_MODE_BASIC : GAMUT_MODE_ADVANCED;
    }

    // IECP feature - Total Color Control
    if (vpData->vpeOutputState.pCamPipeParams->TotalColorControl.bActive)
    {
        if (!surface->pColorPipeParams)
        {
            surface->pColorPipeParams = (PVPHAL_COLORPIPE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pColorPipeParams);
        }
        if (!surface->pColorPipeExtParams)
        {
            surface->pColorPipeExtParams = (PVPHAL_COLORPIPE_EXT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_EXT_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pColorPipeExtParams);
        }

        surface->pColorPipeParams->bEnableTCC = true;
        surface->pColorPipeParams->TccParams.Red =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->TotalColorControl.Red, CP_TCC_MAX);
        surface->pColorPipeParams->TccParams.Green =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->TotalColorControl.Green, CP_TCC_MAX);
        surface->pColorPipeParams->TccParams.Blue =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->TotalColorControl.Blue, CP_TCC_MAX);
        surface->pColorPipeParams->TccParams.Cyan =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->TotalColorControl.Cyan, CP_TCC_MAX);
        surface->pColorPipeParams->TccParams.Magenta =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->TotalColorControl.Magenta, CP_TCC_MAX);
        surface->pColorPipeParams->TccParams.Yellow =
            MOS_MIN(vpData->vpeOutputState.pCamPipeParams->TotalColorControl.Yellow, CP_TCC_MAX);
    }

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalFDFBParams(
    DXVA_VP_DATA            *vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    VPHAL_SURFACE_EXT       *surface,
    uint32_t                stream,
    DWORD                   contentWidth,
    DWORD                   contentHeight)
{
    HRESULT                 hr              = S_OK;
    PVPE_FDFB_STREAM_STATE  fdfbStreamState = nullptr;
    PVPHAL_FDFB_PARAMS      fdfbParams      = nullptr;
    uint32_t                fbMinFaceSize   = 0;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(renderParams);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    // Check whether FDFB can be done
    if (!(renderParams->Component == COMPONENT_FDFB))
    {
        // Free FDFB VpHal params
        MOS_SafeFreeMemory(surface->pFDFBParams);
        surface->pFDFBParams = nullptr;

        // Free FDFB DDI params
        DdiVPFunctionsCommon::VpeFDFBParamsFree(&vpData->vpeStreamState[stream]);
        return hr;
    }

    if (surface->SampleType != SAMPLE_PROGRESSIVE || contentWidth > FDFB_MAX_IMAGE_WIDTH || contentHeight > FDFB_MAX_IMAGE_HEIGHT)
    {
        DDI_VP_ASSERTMESSAGE("FDFB does not support combination with other features.");
        MT_ERR3(MT_VP_BLT_FDFBPARAM, MT_VP_BLT_SAMPLE_TYPE, surface->SampleType, MT_SURF_WIDTH, contentWidth, MT_SURF_HEIGHT, contentHeight);
        // Free FDFB VpHal params
        MOS_SafeFreeMemory(surface->pFDFBParams);
        surface->pFDFBParams = nullptr;

        // Free FDFB DDI params
        DdiVPFunctionsCommon::VpeFDFBParamsFree(&vpData->vpeStreamState[stream]);
        return E_INVALIDARG;
    }

    // Default FDFB values will be used if the FDFB stream state is not set
    if (vpData->vpeStreamState[stream].pFDFBStreamState == nullptr)
    {
        vpData->vpeStreamState[stream].pFDFBStreamState = (PVPE_FDFB_STREAM_STATE)MOS_AllocAndZeroMemory(sizeof(VPE_FDFB_STREAM_STATE));
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->vpeStreamState[stream].pFDFBStreamState);

        DdiVPFunctionsCommon::VpeFDFBParamsInit(vpData->vpeStreamState[stream].pFDFBStreamState);
    }

    if (surface->pFDFBParams == nullptr)
    {
        surface->pFDFBParams = (PVPHAL_FDFB_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_FDFB_PARAMS));
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pFDFBParams);
    }

    fdfbStreamState    = vpData->vpeStreamState[stream].pFDFBStreamState;
    fdfbParams         = surface->pFDFBParams;

    // TODO: Check whether the FDFB params are valid
    fdfbParams->FDPMode = (VPHAL_FDP_MODE_ENUM)fdfbStreamState->FDPMode;

    // Set pFDFBStreamState->uiInitMinFaceSize value
    if (fdfbStreamState->bMinFaceSizeSetByUser)
    {
        // User-defined
    }
    else if (fdfbStreamState->uiInitMinFaceSize != FD_DEFAULT_FACE_SIZE &&
        (contentWidth == fdfbParams->uiHistoryWidth && contentHeight == fdfbParams->uiHistoryHeight))
    {
        // Initialized
        // NOTE: We assume the value need to be re-initalized after the resoultion change
    }
    else
    {
        // Assign the default value to uiInitMinFaceSize according to the source resolution.
        if (contentWidth  > FDFB_480P_WIDTH && contentHeight > FDFB_480P_HEIGHT)
        {
            fdfbStreamState->uiInitMinFaceSize = FDFB_720P_DEFAULT_MIN_FACE_SIZE;
        }
        else
        {
            fdfbStreamState->uiInitMinFaceSize = FDFB_480P_DEFAULT_MIN_FACE_SIZE;
        }

        fdfbParams->uiHistoryWidth     = contentWidth;
        fdfbParams->uiHistoryHeight    = contentHeight;
    }

    // To ensure the uiInitMinFaceSize value greater than or eqaul to which FB can support
    if (fdfbParams->FDFBMode == VPHAL_FDFB_MODE_FDFB)
    {
        fbMinFaceSize = (MOS_MIN(contentWidth, contentHeight)) / VPE_FDFB_FB_MIN_FACE_SIZE_RATIO;

        // Ensure uiFbMinFaceSize >= FD_MIN_FACE_SIZE (32)
        fbMinFaceSize = MOS_MAX(fbMinFaceSize, FD_MIN_FACE_SIZE);
        if (fdfbStreamState->uiInitMinFaceSize < fbMinFaceSize)
        {
            DDI_VP_ASSERTMESSAGE("The minimum face windows size for FD is smaller than that for FB can support!\n");
            fdfbStreamState->uiInitMinFaceSize = fbMinFaceSize;
        }
    }

    fdfbParams->uiInitMinFaceSize = MOS_CLAMP_MIN_MAX(fdfbStreamState->uiInitMinFaceSize, FD_MIN_FACE_SIZE, FD_MAX_FACE_SIZE);
    fdfbParams->uiInitStep = fdfbStreamState->uiInitStep;

    if (fdfbStreamState->uiInitMultiview == VPE_FACE_VIEWS_HALF_OMNI_MVIEW)
    {
        fdfbParams->InitMultiview = VPHAL_FACE_VIEWS_HALF_OMNI_MVIEW;
    }
    else if (fdfbStreamState->uiInitMultiview == VPE_FACE_VIEWS_FRONTAL)
    {
        fdfbParams->InitMultiview = VPHAL_FACE_VIEWS_FRONTAL;
    }
    else
    {
        DDI_VP_ASSERTMESSAGE("FDFB does not support the view.");
        fdfbParams->InitMultiview = VPHAL_FACE_VIEWS_FRONTAL;
    }

    // Face Selection Criteria
    switch (fdfbStreamState->uiInitFaceSelectCriteria)
    {
        case VPE_FD_FACE_SORT_BY_MAX_SIZE:
            fdfbParams->uiInitFaceSelectCriteria = VPHAL_FD_FACE_SORT_BY_MAX_SIZE;
            break;
        case VPE_FD_FACE_SORT_BY_CENTER:
            fdfbParams->uiInitFaceSelectCriteria = VPHAL_FD_FACE_SORT_BY_CENTER;
            break;
        default:
            DDI_VP_ASSERTMESSAGE("FDFB does not support this selection criteria, so fall back to VPHAL_FD_FACE_SORT_BY_CENTER.");
            fdfbParams->uiInitFaceSelectCriteria = VPHAL_FD_FACE_SORT_BY_CENTER;
            break;
    }

    // FDFB Mode
    switch (fdfbStreamState->FDFBMode)
    {
        case VPE_FDFB_MODE_FDFB:
            fdfbParams->FDFBMode = VPHAL_FDFB_MODE_FDFB;
            break;
        case VPE_FDFB_MODE_FD_ONLY:
            fdfbParams->FDFBMode = VPHAL_FDFB_MODE_FD_ONLY;
            break;
        case VPE_FDFB_MODE_FB_ONLY:
            fdfbParams->FDFBMode = VPHAL_FDFB_MODE_FB_ONLY;
            break;
        case VPE_FDFB_MODE_FDFLD:
            fdfbParams->FDFBMode = VPHAL_FDFB_MODE_FDFLD;
            break;
        case VPE_FDFB_MODE_FB_ONLY_WITH_PROVIDED_LM:
            fdfbParams->FDFBMode = VPHAL_FDFB_MODE_FB_ONLY_WITH_PROVIDED_LM;
            break;
        case VPE_FDFB_MODE_FDFB_EXT:
        default:
            DDI_VP_ASSERTMESSAGE("FDFB does not support the mode, so fall back to VPE_FDFB_MODE_FDFB .");
            fdfbParams->FDFBMode = VPHAL_FDFB_MODE_FDFB;
            break;
    }

    if (fdfbStreamState->FDFBMode == VPE_FDFB_MODE_FD_ONLY)
    {
        // Beta-2 kernel support the FDP max face count setting
        fdfbParams->uiFaceCount = MOS_MIN(fdfbStreamState->uiInitFDPMaxFaceCount, FD_MAX_FACE_COUNT);
    }
    else
    {
        fdfbParams->uiFaceCount = MOS_MIN(fdfbStreamState->uiFBPMaxFaceCount, FB_MAX_FACE_COUNT);
    }

    fdfbParams->uiRedLipStrength = fdfbStreamState->FBPFeatureActive.bFB_RED_LIP_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiRedLipStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiBigEyeStrength = fdfbStreamState->FBPFeatureActive.bFB_BIG_EYE_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiBigEyeStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiSlimFaceStrength = fdfbStreamState->FBPFeatureActive.bFB_SLIM_FACE_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiSlimFaceStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiHappyFaceStrength = fdfbStreamState->FBPFeatureActive.bFB_HAPPY_FACE_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiHappyFaceStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiCuteNoseStrength = fdfbStreamState->FBPFeatureActive.bFB_CUTE_NOSE_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiCuteNoseStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiSkinBrighteningStrength = fdfbStreamState->FBPFeatureActive.bFB_SKIN_BRIGHTENING_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiSkinBrighteningStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiSkinWhiteningStrength = fdfbStreamState->FBPFeatureActive.bFB_SKIN_WHITENING_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiSkinWhiteningStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiSkinFoundationStrength = fdfbStreamState->FBPFeatureActive.bFB_SKIN_FOUNDATION_Active ?
        MOS_MIN(fdfbStreamState->uiSkinFoundationStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiSkinFoundationColor = fdfbStreamState->uiSkinFoundationColor & FB_FOUNDATION_COLOR_MASK;
    fdfbParams->uiSkinSmoothingStrength = fdfbStreamState->FBPFeatureActive.bFB_SKIN_SMOOTHING_Active ?
        MOS_MIN(fdfbStreamState->uiSkinSmoothingStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiCircleRemovalStrength = fdfbStreamState->FBPFeatureActive.bFB_CIRCLES_REMOVAL_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiCircleRemovalStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiEyeBagsRemovalStrength = fdfbStreamState->FBPFeatureActive.bFB_EYE_BAGS_REMOVAL_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiEyeBagsRemovalStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiEyeWrinklesRemovalStrength = fdfbStreamState->FBPFeatureActive.bFB_EYE_WRINKLES_REMOVAL_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiEyeWrinklesRemovalStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiBlushStrength = fdfbStreamState->FBPFeatureActive.bFB_BLUSH_STRENGTH_Active ?
        MOS_MIN(fdfbStreamState->uiBlushStrength, FB_STRENGTH_MAX) : 0;
    fdfbParams->uiSkinEnhancementStrength = MOS_MIN(fdfbStreamState->uiSkinEnhancementStrength, FB_STRENGTH_MAX);

    if (fdfbStreamState->FDFBMode == VPE_FDFB_MODE_FB_ONLY)
    {
        if (fdfbStreamState->uiFaceCount > fdfbStreamState->uiFBPMaxFaceCount)
        {
            DDI_VP_ASSERTMESSAGE("Invalid input params: VPE_FDP_FACE_COUNT should be smaller than or equal to VPE_FBP_MAX_FACE_COUNT in FB only mode.");
        }

        fdfbParams->uiFaceCount    = MOS_MIN(fdfbStreamState->uiFaceCount, FB_MAX_FACE_COUNT);
        fdfbParams->pFaceList      = (PVPHAL_FACE_RECT)fdfbStreamState->pFaceList;
    }
    else if (fdfbStreamState->FDFBMode == VPE_FDFB_MODE_FB_ONLY_WITH_PROVIDED_LM)
    {
        if (fdfbStreamState->uiFaceCount > fdfbStreamState->uiFBPMaxFaceCount)
        {
            DDI_VP_ASSERTMESSAGE("Invalid input params: VPE_FDP_FACE_COUNT should be smaller than or equal to VPE_FBP_MAX_FACE_COUNT in FB only mode.");
        }

        fdfbParams->uiFaceCount = MOS_MIN(fdfbStreamState->uiFaceCount, FB_MAX_FACE_COUNT);
        fdfbParams->pLMList     = (PVPHAL_FACE_LANDMARK)fdfbStreamState->pLMList;
        fdfbParams->pFaceWeightList = fdfbStreamState->pFaceWeightList;
    }

    MT_LOG3(MT_VP_BLT_FDFBPARAM, MT_NORMAL, MT_VP_BLT_FDFBPARAM_MODE, fdfbStreamState->FDFBMode, MT_VP_BLT_FDFBPARAM_FACECOUNT, fdfbStreamState->uiFaceCount,
        MT_VP_BLT_FDFBPARAM_FBMAXFACECOUNT, fdfbStreamState->uiFBPMaxFaceCount);

    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetVpHalSuperResolutionParams(
    DXVA_VP_DATA            *vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    VPHAL_SURFACE_EXT       *surface)
{
    HRESULT hr                        = E_FAIL;
    bool    cuiSuperResolutionEnabled = false;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(vpData, E_INVALIDARG);
    DDI_VP_CHK_NULL_RETURN_VALUE(vpData->pVpHalState, E_INVALIDARG);
    DDI_VP_CHK_NULL_RETURN_VALUE(renderParams, E_INVALIDARG);
    DDI_VP_CHK_NULL_RETURN_VALUE(surface, E_INVALIDARG);

    auto skuTable = vpData->pVpHalState->GetSkuTable();
    DDI_VP_CHK_NULL_RETURN_VALUE(skuTable, E_INVALIDARG);

    // Initialize SuperResolution Report according to CUI current status
    vpData->ConfigValues.bAdvancedScalingInUseReported = vpData->CUIRegistryValues.bSuperResolutionEnabled;

    // CUI is not enabled for Vprep
    cuiSuperResolutionEnabled = (vpData->CUIRegistryValues.bEnableSuperResolution) && (renderParams->Component != COMPONENT_VPreP);

    if ((cuiSuperResolutionEnabled ||
        (vpData->vpeOutputState.pSuperResolutionParams &&
        vpData->vpeOutputState.pSuperResolutionParams->bEnable)) &&
        GRAPHICS_IS_SKU(skuTable, FtrSuperResolution))  // Disable Super Resolution by setting FtrSuperResolution to false in SkuTable.
    {
        if (surface->pSuperResolutionParams == nullptr)
        {
            surface->pSuperResolutionParams = (PVPHAL_SUPER_RESOLUTION_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_SUPER_RESOLUTION_PARAMS));
            DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface->pSuperResolutionParams);
        }

        surface->pSuperResolutionParams->bEnableSuperResolution = true;

        if (cuiSuperResolutionEnabled)
        {
            MOS_SecureMemcpy(
                &surface->pSuperResolutionParams->superResolutionMode,
                sizeof(SUPER_RESOLUTION_MODE),
                &vpData->CUIRegistryValues.dwSuperResolutionMode,
                sizeof(DWORD));

            surface->pSuperResolutionParams->superResolutionMaxInWidth  = vpData->CUIRegistryValues.dwSuperResolutionMaxInWidth;
            surface->pSuperResolutionParams->superResolutionMaxInHeight = vpData->CUIRegistryValues.dwSuperResolutionMaxInHeight;
        }
        else
        {
            MOS_SecureMemcpy(
                &surface->pSuperResolutionParams->superResolutionMode,
                sizeof(SUPER_RESOLUTION_MODE),
                &vpData->vpeOutputState.pSuperResolutionParams->SRMode,
                sizeof(VPE_SUPER_RESOLUTION_MODE));
            switch (vpData->vpeOutputState.pSuperResolutionParams->SRModelVer)
            {
            case VPE_SR_MODEL_DLSR10:
                surface->pSuperResolutionParams->superResolutionModelVer = SR_MODEL_DLSR10;
                break;
            case VPE_SR_MODEL_DLSR20:
                surface->pSuperResolutionParams->superResolutionModelVer = SR_MODEL_DLSR20;
                break;
            case VPE_SR_MODEL_DEFAULT:
            default:
                surface->pSuperResolutionParams->superResolutionModelVer = SR_MODEL_DEFAULT;
                break;
            }
        }
    }
    else if (surface->pSuperResolutionParams)
    {
        MOS_SafeFreeMemory(surface->pSuperResolutionParams);
        surface->pSuperResolutionParams = nullptr;
    }

    hr = S_OK;
    return hr;
}

void DdiVPFunctionsD3D12::SetVpeScalingtMode(
    DXVA_DEVICE_CONTEXT     *dxvaDevice,
    PDXVA_VP_DATA           vpData,
    VPHAL_RENDER_PARAMS_EXT *renderParams,
    VPHAL_SURFACE_EXT       *surface,
    uint32_t                stream)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(dxvaDevice);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(renderParams);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(surface);

    if ((renderParams->Component == COMPONENT_VPreP) && (vpData->vpeStreamState[stream].pScalingModeParam != nullptr))
    {
        surface->ScalingMode = VPHAL_SCALING_AVS;

        if (vpData->vpeStreamState[stream].pScalingModeParam->FastMode == VPE_SCALING_MODE_ADV)
        {
            surface->ScalingPreference = VPHAL_SCALING_PREFER_SFC_FOR_VEBOX;
        }
        else
        {
            // default
            surface->ScalingPreference = VPHAL_SCALING_PREFER_SFC;
        }

        // if debug reg set bi-linear scaling, change to bi-linear
        if (vpData->ConfigValues.dwCreatedScalingMode == VPDDI_SCALING)
        {
            surface->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
    }
    else
    {
        if (dxvaDevice->m_bIsVpCampipeUsed == TRUE)
        {
            // Canon campipe app WA, if campipe usage is detected, prefer render scaling if VEBOX not required
            surface->ScalingPreference = VPHAL_SCALING_PREFER_SFC_FOR_VEBOX;
        }
        else
        {
            surface->ScalingPreference = VPHAL_SCALING_PREFER_SFC;
        }

        if (vpData->ConfigValues.dwCreatedScalingMode == VPDDI_ADVANCEDSCALING)
        {
            surface->ScalingMode = VPHAL_SCALING_AVS;
        }
        else
        {
            surface->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
    }

    return;
}

bool DdiVPFunctionsD3D12::IsInputViewProtected(
    DXVA_VP_DATA            *vpData,
    D3D12DDI_HRESOURCE      inputTexture)
{
    D3DDDI_RESOURCEFLAGS                resourceFlags = { 0 };
    DXVAUMD_RESOURCE                    resource      = nullptr;
    PDXVAUMD_SRESOURCEINFO              sresourceInfo = nullptr;

    DDI_VP_CHK_NULL_RETURN_VALUE(vpData, false);
    DDI_VP_CHK_NULL_RETURN_VALUE(vpData->pVpHalState, false);

    PMOS_INTERFACE osInterface = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHK_NULL_RETURN_VALUE(osInterface, false);

    DXVA_DEVICE_CONTEXT *dxvaDevice = osInterface->pOsContext;
    DDI_VP_CHK_NULL_RETURN_VALUE(dxvaDevice, false);

    resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputTexture);
    DDI_VP_CHK_NULL_RETURN_VALUE(resource, false);

    sresourceInfo = &resource->ResourceInfo;
    DDI_VP_CHK_NULL_RETURN_VALUE(sresourceInfo, false);
    DDI_VP_CHK_NULL_RETURN_VALUE(sresourceInfo->m_pGmmResourceInfo, false);

    // D3D11_RESOURCE_MISC_RESTRICTED_CONTENT protection flag cached in D3DDDI_RESOURCEFLAGS in Gmm.
    resourceFlags = sresourceInfo->m_pGmmResourceInfo->GetD3d9Flags();

    return resourceFlags.RestrictedContent;
}

template <class T>
bool DdiVPFunctionsD3D12::IsVpInputViewsProtected(
    DXVA_VP_DATA *vpData,
    uint32_t     streamCount,
    const T      *streams)
{
    bool    isProtected = false;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_RETURN_VALUE(streams, false);

    for (uint32_t iStream = 0; iStream < streamCount; iStream++)
    {
//      if (pStreams[iStream].Enable)
        {
            isProtected |= IsInputViewProtected(vpData, streams[iStream].InputStream[0].hDrvInputTexture);

            for (uint32_t i = 0; i < MOS_MIN(streams[iStream].InputStream[0].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]); i++)
            {
                isProtected |= IsInputViewProtected(vpData, streams[iStream].InputStream[0].ReferenceInfo.hDrvPastFrames[i]);
            }

            for (uint32_t i = 0; i < MOS_MIN(streams[iStream].InputStream[0].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]); i++)
            {
                isProtected |= IsInputViewProtected(vpData, streams[iStream].InputStream[0].ReferenceInfo.hDrvFutureFrames[i]);
            }

            if (isProtected)
            {
                break;
            }

            // Handle Stereo3D case where left & right channels are described by 
            // separate input views
            if (D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_SEPARATE == vpData->stmStateStereoFormat[iStream].Format)
            {
                isProtected |= IsInputViewProtected(vpData, streams[iStream].InputStream[1].hDrvInputTexture);

                for (uint32_t i = 0; i < MOS_MIN(streams[iStream].InputStream[1].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]); i++)
                {
                    isProtected |= IsInputViewProtected(vpData, streams[iStream].InputStream[1].ReferenceInfo.hDrvPastFrames[i]);
                }

                for (uint32_t i = 0; i < MOS_MIN(streams[iStream].InputStream[1].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]); i++)
                {
                    isProtected |= IsInputViewProtected(vpData, streams[iStream].InputStream[1].ReferenceInfo.hDrvFutureFrames[i]);
                }

                if (isProtected)
                {
                    break;
                }
            }
        } // if (pStreams[iStream].Enable)
    }
    return isProtected;
}

template <class T>
HRESULT DdiVPFunctionsD3D12::SetVpHalBackwardRefs(
    DXVA_VP_DATA               *vpData,
    const T                    *streams,
    bool                       rightFrame,
    PVPHAL_SURFACE_EXT         surface,
    uint32_t                   iStream,
    REFERENCE_TIME             rtTarget,
    VPHAL_DI_PARAMS            *diParams,
    DWORD                      invalidCaps,
    DWORD                      yuvRange,
    COMPOSITING_SAMPLE_D3D11_1 *sample)
{
    HRESULT                     hr            = S_OK;
    PVPHAL_SURFACE_EXT          tempSurface   = nullptr;
    DXVAUMD_RESOURCE            resource      = nullptr;
    uint32_t                    numPastFrames = 0;
    std::function<void(void)>   destory       = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(surface);

    destory = [&]() -> void {
        if (hr != S_OK)
        {
            MOS_FreeMemAndSetNull(tempSurface->pDenoiseParams);
            MOS_FreeMemAndSetNull(tempSurface->pColorPipeParams);
            MOS_FreeMemAndSetNull(tempSurface->pColorPipeExtParams);
            MOS_FreeMemAndSetNull(tempSurface->pGamutParams);
            MOS_FreeMemAndSetNull(tempSurface->pDrDbParams);
        }
    };

    PMOS_INTERFACE osInterface = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);
    DXVA_DEVICE_CONTEXT *dxvaDevice = osInterface->pOsContext;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);

    tempSurface = surface;

    if (rightFrame)
    {
        numPastFrames = MOS_MIN(streams[iStream].InputStream[1].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]);
    }
    else
    {
        numPastFrames = MOS_MIN(streams[iStream].InputStream[0].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]);
    }
    surface->uBwdRefCount = numPastFrames;

    // If app does not pass any Bwd Ref frames, ensure it is set to NULL
    if ((surface->uBwdRefCount == 0) && surface->pBwdRef)
    {
        DdiVPFunctionsCommon::VpHalDdiDeleteSurfaceExt(surface->pBwdRef);
        surface->pBwdRef = nullptr;
    }

    for (uint32_t i = 0; i < numPastFrames; i++)
    {
        if (tempSurface->pBwdRef == nullptr)
        {
            tempSurface->pBwdRef = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE_EXT));
            if (!tempSurface->pBwdRef)
            {
                destory();
                return hr;
            }
        }

        tempSurface = (VPHAL_SURFACE_EXT*)tempSurface->pBwdRef;

        DDI_VP_CHK_HR_RETURN(DdiVPFunctionsCommon::CopyCommonCurSurfParamsToRef(surface, tempSurface));

        // pSurf points to backward ref which may point to another backward reference (hence count is decreased)
        tempSurface->uBwdRefCount      = numPastFrames - (i + 1);
        tempSurface->SurfType          = SURF_IN_REFERENCE;
        tempSurface->ScalingMode       = VPHAL_SCALING_BILINEAR;
        tempSurface->ScalingPreference = VPHAL_SCALING_PREFER_SFC;

        if (rightFrame)
        {
            resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(streams[iStream].InputStream[1].ReferenceInfo.hDrvPastFrames[i]);
        }
        else
        {
            resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(streams[iStream].InputStream[0].ReferenceInfo.hDrvPastFrames[i]);
        }
        if (!resource)
        {
            destory();
            return hr;
        }

        if (!resource->ResourceInfo.m_pGmmResourceInfo)
        {
            DdiVPFunctionsCommon::VpHalDdiDeleteSurfaceExt(surface->pBwdRef);
            surface->pBwdRef      = nullptr;
            surface->uBwdRefCount = 0;
            break;
        }

        sample->ColorSpace     = vpData->stmStateInputColorSpace[iStream]; // Get CSpace from Primary
        sample->Start          = resource->ResourceInfo.m_AllocationHandle;
        sample->End            = sample->Start;
        sample->ChromaSiting   = vpData->stmStateInputChromaSiting[iStream];
        sample->GammaValue     = vpData->stmStateInputGammaValue[iStream];

        // Set HAL layer parameters for ref layer, validates format and size
        // TODO: need to pass in the fourcc
        hr = DdiVPFunctionsCommon::SetSurfaceForRender(
            tempSurface,
            sample,
            rtTarget,
            resource,
            0,
            0,
            0,
            diParams,
            invalidCaps,
            yuvRange,
            nullptr);
    }

    destory();
    return hr;
}

template <class T>
HRESULT DdiVPFunctionsD3D12::SetVpHalForwardRefs(
    DXVA_VP_DATA               *vpData,
    const T                    *streams,
    BOOL                       rightFrame,
    PVPHAL_SURFACE_EXT         surface,
    uint32_t                   iStream,
    REFERENCE_TIME             rtTarget,
    VPHAL_DI_PARAMS            *diParams,
    DWORD                      invalidCaps,
    DWORD                      yuvRange,
    COMPOSITING_SAMPLE_D3D11_1 *sample)
{
    HRESULT                     hr              = S_OK;
    PVPHAL_SURFACE_EXT          tempSurface     = nullptr;
    DXVAUMD_RESOURCE            resource        = nullptr;
    uint32_t                    numFutureFrames = 0;
    std::function<void(void)>   destory         = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);

    destory = [&]() -> void {
        if (hr != S_OK)
        {
            MOS_FreeMemAndSetNull(tempSurface->pDenoiseParams);
            MOS_FreeMemAndSetNull(tempSurface->pColorPipeParams);
            MOS_FreeMemAndSetNull(tempSurface->pColorPipeExtParams);
            MOS_FreeMemAndSetNull(tempSurface->pGamutParams);
            MOS_FreeMemAndSetNull(tempSurface->pDrDbParams);
        }
    };

    PMOS_INTERFACE osInterface = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);

    DXVA_DEVICE_CONTEXT *dxvaDevice = osInterface->pOsContext;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);

    tempSurface = surface;

    if (rightFrame)
    {
        numFutureFrames = MOS_MIN(streams[iStream].InputStream[1].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]);
    }
    else
    {
        numFutureFrames = MOS_MIN(streams[iStream].InputStream[0].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]);
    }
    surface->uFwdRefCount = numFutureFrames;

    // If app does not pass any Fwd Ref frames, ensure it is set to NULL
    if ((surface->uFwdRefCount == 0) && surface->pFwdRef)
    {
        DdiVPFunctionsCommon::VpHalDdiDeleteSurfaceExt(surface->pFwdRef);
        surface->pFwdRef = nullptr;
    }

    for (uint32_t i = 0; i < numFutureFrames; i++)
    {
        if (tempSurface->pFwdRef == nullptr)
        {
            tempSurface->pFwdRef = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE_EXT));
            if (!tempSurface->pFwdRef)
            {
                destory();
                return hr;
            }
        }

        tempSurface = (VPHAL_SURFACE_EXT*)tempSurface->pFwdRef;
        DDI_VP_CHK_HR_RETURN(DdiVPFunctionsCommon::CopyCommonCurSurfParamsToRef(surface, tempSurface));

        // pSurf points to backward ref which may point to another backward reference (hence count is decreased)
        tempSurface->uFwdRefCount      = numFutureFrames - (i + 1);
        tempSurface->SurfType          = SURF_IN_REFERENCE;
        tempSurface->ScalingMode       = VPHAL_SCALING_BILINEAR;
        tempSurface->ScalingPreference = VPHAL_SCALING_PREFER_SFC;

        if (rightFrame)
        {
            resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(streams[iStream].InputStream[1].ReferenceInfo.hDrvFutureFrames[i]);
        }
        else
        {
            resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(streams[iStream].InputStream[0].ReferenceInfo.hDrvFutureFrames[i]);
        }
        if (!resource)
        {
            destory();
            return hr;
        }

        if (!resource->ResourceInfo.m_pGmmResourceInfo)
        {
            DdiVPFunctionsCommon::VpHalDdiDeleteSurfaceExt(surface->pFwdRef);
            surface->pFwdRef      = nullptr;
            surface->uFwdRefCount = 0;
            break;
        }

        sample->ColorSpace     = vpData->stmStateInputColorSpace[iStream]; // Get CSpace from Primary
        sample->Start          = resource->ResourceInfo.m_AllocationHandle;
        sample->End            = sample->Start;
        sample->ChromaSiting   = vpData->stmStateInputChromaSiting[iStream];
        sample->GammaValue     = vpData->stmStateInputGammaValue[iStream];

        // Set HAL layer parameters for ref layer, validates format and size
        // TODO: need to pass in the fourcc
        DDI_VP_CHK_HR_RETURN(DdiVPFunctionsCommon::SetSurfaceForRender(
            tempSurface,
            sample,
            rtTarget,
            resource,
            0,
            0,
            0,
            diParams,
            invalidCaps,
            yuvRange,
            nullptr));
    }

    destory();
    return hr;
}

template <class T>
HRESULT DdiVPFunctionsD3D12::InitializeDxvaResourceInfo(
    D3D12DDI_HCOMMANDLIST                                        drvCommandList,
    DXVA_VP_DATA                                                 *vpData,
    const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputStreamParameters,
    const T                                                      *inputStreamParameters,
    uint32_t                                                     streamCount)
{
    HRESULT                hr             = S_OK;
    PMOS_INTERFACE         osInterface    = nullptr;
    DXVA_DEVICE_CONTEXT    *dxvaDevice    = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(vpData->pVpHalState);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(outputStreamParameters);

    osInterface = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);

    dxvaDevice = osInterface->pOsContext;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);

    // init DxvaResourceInfo of output resource
    DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, outputStreamParameters->OutputStream[0].hDrvTexture2D));
    // Handle Stereo3D case where left & right channels are described by separate output views
    if (vpData->bltStereoMode)
    {
        DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, outputStreamParameters->OutputStream[1].hDrvTexture2D));
    }

    // init DxvaResourceInfo of input resource
    for (uint32_t iStream = 0; iStream < streamCount; iStream++)
    {
        // if (pStreams[iStream].Enable)
        DDI_VP_CHECK_NULL_WITH_HR_RETURN(inputStreamParameters);
        {
            DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[0].hDrvInputTexture));

            for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[0].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]); i++)
            {
                DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[0].ReferenceInfo.hDrvPastFrames[i]));
            }

            for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[0].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]); i++)
            {
                DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[0].ReferenceInfo.hDrvFutureFrames[i]));
            }

            // Handle Stereo3D case where left & right channels are described by separate input views
            if (D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_SEPARATE == vpData->stmStateStereoFormat[iStream].Format)
            {
                DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[1].hDrvInputTexture));

                for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[1].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]); i++)
                {
                    DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[1].ReferenceInfo.hDrvPastFrames[i]));
                }

                for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[1].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]); i++)
                {
                    DDI_VP_CHK_HR_RETURN(InitDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[1].ReferenceInfo.hDrvFutureFrames[i]));
                }
            }
        } // if (pStreams[iStream].Enable)
    }

    return S_OK;
}

template <class T>
void DdiVPFunctionsD3D12::DestroyDxvaResourceInfo(
    D3D12DDI_HCOMMANDLIST                                        drvCommandList,
    DXVA_VP_DATA                                                 *vpData,
    const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputStreamParameters,
    const T                                                      *inputStreamParameters,
    uint32_t                                                     streamCount)
{
    PMOS_INTERFACE      osInterface = nullptr;
    DXVA_DEVICE_CONTEXT *dxvaDevice = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(vpData->pVpHalState);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(outputStreamParameters);

    osInterface = vpData->pVpHalState->GetOsInterface();
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(osInterface);

    dxvaDevice = osInterface->pOsContext;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(dxvaDevice);

    // Destroy DxvaResourceInfo of output resource
    DestroyDXVAResourceInfo(dxvaDevice, outputStreamParameters->OutputStream[0].hDrvTexture2D);
    // Handle Stereo3D case where left & right channels are described by separate output views
    if (vpData->bltStereoMode)
    {
        DestroyDXVAResourceInfo(dxvaDevice, outputStreamParameters->OutputStream[1].hDrvTexture2D);
    }

    // Destroy DxvaResourceInfo of input resource
    for (uint32_t iStream = 0; iStream < streamCount; iStream++)
    {
        {
            DestroyDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[0].hDrvInputTexture);

            for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[0].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]); i++)
            {
                DestroyDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[0].ReferenceInfo.hDrvPastFrames[i]);
            }

            for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[0].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]); i++)
            {
                DestroyDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[0].ReferenceInfo.hDrvFutureFrames[i]);
            }

            // Handle Stereo3D case where left & right channels are described by
            // separate input views
            if (D3D11_1DDI_VIDEO_PROCESSOR_STEREO_FORMAT_SEPARATE == vpData->stmStateStereoFormat[iStream].Format)
            {
                DestroyDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[1].hDrvInputTexture);

                for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[1].ReferenceInfo.NumPastFrames, vpData->uiInputStreamNumPastFrames[iStream]); i++)
                {
                    DestroyDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[1].ReferenceInfo.hDrvPastFrames[i]);
                }

                for (uint32_t i = 0; i < MOS_MIN(inputStreamParameters[iStream].InputStream[1].ReferenceInfo.NumFutureFrames, vpData->uiInputStreamNumFutureFrames[iStream]); i++)
                {
                    DestroyDXVAResourceInfo(dxvaDevice, inputStreamParameters[iStream].InputStream[1].ReferenceInfo.hDrvFutureFrames[i]);
                }
            }
        }
    }
    return;
}

HRESULT DdiVPFunctionsD3D12::InitDXVAResourceInfo(
    DXVA_DEVICE_CONTEXT *dxvaDevice,
    D3D12DDI_HRESOURCE  inputTexture)
{
    HRESULT                hr            = S_OK;
    DXVAUMD_RESOURCE       resource      = nullptr;
    PDXVAUMD_SRESOURCEINFO sResourceInfo = nullptr;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);

    resource = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputTexture);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(resource);

    sResourceInfo = &resource->ResourceInfo;
    if (sResourceInfo->m_pDxvaResourceInfo == nullptr && sResourceInfo->m_pGmmResourceInfo)
    {
        // allocate dxva resource info memory and initialize
        // 3D will call Media to destroy dxva resourc info before the resource is destroyed.
        dxvaDevice->InitializeDxvaResourceInfo(sResourceInfo);

        // return FAIL if initialization fails
        DDI_VP_CHK_NULL_RETURN_VALUE(sResourceInfo->m_pDxvaResourceInfo, E_OUTOFMEMORY);
    }

    return hr;
}

void DdiVPFunctionsD3D12::DestroyDXVAResourceInfo(
    DXVA_DEVICE_CONTEXT *dxvaDevice,
    D3D12DDI_HRESOURCE  inputTexture)
{
    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(dxvaDevice);

    DXVAUMD_RESOURCE resource      = (DXVAUMD_RESOURCE)dxvaDevice->GetResourceInfo(inputTexture);
    DDI_VP_CHK_NULL_NO_STATUS_RETURN(resource);

    PDXVAUMD_SRESOURCEINFO sResourceInfo = &resource->ResourceInfo;
    if (sResourceInfo->m_pDxvaResourceInfo)
    {
        // Deallocate dxva resource info memory
        dxvaDevice->DestroyDxvaResourceInfo(sResourceInfo);
    }
    return;
}

HRESULT DdiVPFunctionsD3D12::SetupCmdListAndPool(
    PMOS_CONTEXT                                                 dxvaDevice,
    const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032  *inputStreamParameters,
    DXVA12_COMMAND_LIST                                          *commandList,
    PMOS_INTERFACE                                               osInterface)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(dxvaDevice);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(commandList);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(commandList->GetCommandAllocator());
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);

    DDI_VP_CHECK_NULL_WITH_HR_RETURN(commandList->GetCommandAllocator()->m_commandAllocator);

    MosInterface::SetupCurrentCmdListAndPool(
        osInterface->osStreamState,
        commandList->m_commandList,
        commandList->GetCommandAllocator()->m_commandAllocator->GetCmdBufMgr());
    return hr;
}

HRESULT DdiVPFunctionsD3D12::SetupCmdListAndPool(
    PMOS_CONTEXT                                                dxvaDevice,
    const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 *inputStreamParameters,
    DXVA12_COMMAND_LIST                                         *commandList,
    PMOS_INTERFACE                                              osInterface)
{
    HRESULT hr = S_OK;

    DDI_VP_FUNCTION_ENTER;
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(commandList);
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(commandList->GetCommandRecorder());
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(osInterface);

    DDI_VP_CHECK_NULL_WITH_HR_RETURN(commandList->GetCommandRecorder()->m_commandRecorder);

    auto cmdPool = commandList->GetCommandRecorder()->m_commandRecorder->GetCommandPool();
    DDI_VP_CHECK_NULL_WITH_HR_RETURN(cmdPool)

    MosInterface::SetupCurrentCmdListAndPool(
        osInterface->osStreamState,
        commandList->m_commandList,
        cmdPool->GetCmdBufMgr());
    return hr;
}
