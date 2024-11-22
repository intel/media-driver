/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_ocl_fc_filter.cpp
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all ocl fc in driver.
//!
#include "vp_ocl_fc_filter.h"
#include "vp_render_cmd_packet.h"
#include "igvpfc_common_args.h"
#include "igvpfc_fp_args.h"
#include "igvpfc_420PL3_input_args.h"
#include "igvpfc_420PL3_output_args.h"
#include "igvpfc_444PL3_input_args.h"
#include "igvpfc_444PL3_output_args.h"
#include "igvpfc_422HV_input_args.h"
#include <vector>

namespace vp
{

#if (_DEBUG || _RELEASE_INTERNAL)
enum class OclFcDiffReportShift
{
    BilinearScaling          = 0,   // bilinear scaling shift difference
    MediaSpecificSampler     = 1,   //1 plane and 2 plane surface state read difference
    Format400PRead           = 2,   //400P has issue on legacy FC even with nearest sampler
    Rotation                 = 3,   //rotation shift place is different with legacy FC
    Procamp                  = 4,   //procamp is not enabled
    LumaKey                  = 5,   //luma key cases will have difference for float(OclFC) vs int(FC)
    ChromasittingOn422Packed = 6,   //422 packed no chromasiting on legacy FC
    FixedAlpha               = 7,   //fixed alpha not used in legacy FC
    FormatRGB565Write        = 8,   //legacy FC will drop (16 - 5/6/5) of LSB
    BT2020ColorFill          = 9,   //legacy didn't support color fill w/ BT2020 as target color space. It will use black or green as background in legacy
    ChromaSitingOnPL3        = 10,  //legacy didn't support 3 plane chromasiting CDS. So legacy FC will only do left top for PL3 output
    FastExpress              = 16,  //walked into fastexpress path
    OclFcEnabled             = 31   //actually walked into Ocl FC. Always set to 1 when Ocl FC Filter take effect. "OCL FC Enabled" may be 1 but not walked into OCL FC, cause it may fall back in wrapper class
};

union OclFcFeatureReport
{
    struct
    {
        //following bits for input layer features. Showing the count of the layers which use the feature.
        uint32_t layerCount     : 4;   //input layer count
        uint32_t procamp        : 4;
        uint32_t rotation       : 4;
        uint32_t chromaUpSample : 4;
        uint32_t lumaKey        : 4;
        uint32_t deinterlace    : 4;
        uint32_t reserved1      : 4;
        //following bits for output layer features. Whether the feature is used
        uint32_t colorFill      : 1;
        uint32_t alpha          : 1;
        uint32_t reserved2      : 2;
    };
    uint32_t value = 0;
};
C_ASSERT(sizeof(OclFcFeatureReport) == sizeof(uint32_t));
#endif

VpOclFcFilter::VpOclFcFilter(PVP_MHWINTERFACE vpMhwInterface) : VpFilter(vpMhwInterface)
{
}

MOS_STATUS VpOclFcFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_renderOclFcParams)
    {
        MOS_Delete(m_renderOclFcParams);
        m_renderOclFcParams = nullptr;
    }

    for (auto &handle : m_fcCommonKrnArgs)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }
    for (auto &handle : m_fcFastExpressKrnArgs)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }
    for (auto &handle : m_fc420PL3OutputKrnArgs)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }
    for (auto &handle : m_fc444PL3OutputKrnArgs)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }
    for (std::pair<const uint32_t, KERNEL_INDEX_ARG_MAP> &singleLayerHandle : m_fc420PL3InputMultiLayersKrnArgs)
    {
        KERNEL_INDEX_ARG_MAP &krnArgs = singleLayerHandle.second;
        for (auto &handle : krnArgs)
        {
            KRN_ARG &krnArg = handle.second;
            MOS_FreeMemAndSetNull(krnArg.pData);
        }
    }
    for (std::pair<const uint32_t, KERNEL_INDEX_ARG_MAP> &singleLayerHandle : m_fc444PL3InputMultiLayersKrnArgs)
    {
        KERNEL_INDEX_ARG_MAP &krnArgs = singleLayerHandle.second;
        for (auto &handle : krnArgs)
        {
            KRN_ARG &krnArg = handle.second;
            MOS_FreeMemAndSetNull(krnArg.pData);
        }
    }
    for (std::pair<const uint32_t, KERNEL_INDEX_ARG_MAP> &singleLayerHandle : m_fc422HVInputMultiLayersKrnArgs)
    {
        KERNEL_INDEX_ARG_MAP &krnArgs = singleLayerHandle.second;
        for (auto &handle : krnArgs)
        {
            KRN_ARG &krnArg = handle.second;
            MOS_FreeMemAndSetNull(krnArg.pData);
        }
    }
    return MOS_STATUS_SUCCESS;
}

void OCL_FC_KERNEL_PARAM::Init()
{
    kernelArgs.clear();
    kernelName.clear();
    kernelId     = kernelCombinedFc;
    threadWidth  = 0;
    threadHeight = 0;
    localWidth   = 0;
    localHeight  = 0;
    kernelConfig = {};
    kernelStatefulSurfaces.clear();
}

void _RENDER_OCL_FC_PARAMS::Init()
{
    fc_kernelParams.clear();
}

MOS_STATUS VpOclFcFilter::SetExecuteEngineCaps(
    SwFilterPipe   *executingPipe,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executingPipe = executingPipe;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::InitKrnParams(OCL_FC_KERNEL_PARAMS &krnParams, SwFilterPipe &executingPipe)
{
    VP_FUNC_CALL();

    krnParams.clear();

    OCL_FC_COMP_PARAM compParam = {};
    VP_RENDER_CHK_STATUS_RETURN(InitCompParam(executingPipe, compParam));
    bool isFastExpressSupported = FastExpressConditionMeet(compParam);
    PrintCompParam(compParam);
    ReportFeatureLog(compParam);
    ReportDiffLog(compParam, isFastExpressSupported);

    OCL_FC_KERNEL_PARAM param = {};
    // convert from PL3 input surface to intermedia surface
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        if (compParam.inputLayersParam[i].needIntermediaSurface == true)
        {
            VP_PUBLIC_CHK_NULL_RETURN(compParam.inputLayersParam[i].surf);
            VP_PUBLIC_CHK_NULL_RETURN(compParam.inputLayersParam[i].surf->osSurface);
            switch (compParam.inputLayersParam[i].surf->osSurface->Format)
            {
            case Format_I420:
            case Format_IYUV:
            case Format_YV12:
            case Format_IMC3:
                VP_RENDER_CHK_STATUS_RETURN(GenerateFc420PL3InputParam(compParam.inputLayersParam[i], i, param));
                krnParams.push_back(param);
                break;
            case Format_RGBP:
            case Format_BGRP:
            case Format_444P:
                VP_RENDER_CHK_STATUS_RETURN(GenerateFc444PL3InputParam(compParam.inputLayersParam[i], compParam.layerNumber, param, i));
                krnParams.push_back(param);
                break;
            case Format_422H:
            case Format_422V:
            case Format_411P:
                VP_RENDER_CHK_STATUS_RETURN(GenerateFc422HVInputParam(compParam.inputLayersParam[i], i, param));
                krnParams.push_back(param);
                break;
            default:
                break;
            }
        }
    }

    if (isFastExpressSupported)
    {
        VP_RENDER_CHK_STATUS_RETURN(GenerateFcFastExpressKrnParam(compParam, param));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(GenerateFcCommonKrnParam(compParam, param));
    }
    //Set Perf Tag should be called after Generate Krn Param
    VP_PUBLIC_CHK_STATUS_RETURN(SetPerfTag(compParam, isFastExpressSupported, param.kernelConfig.perfTag));
    krnParams.push_back(param);

    // convert from PL3 output surface to intermedia surface
    if (compParam.outputLayerParam.needIntermediaSurface == true)
    {
        VP_PUBLIC_CHK_NULL_RETURN(compParam.outputLayerParam.surf);
        VP_PUBLIC_CHK_NULL_RETURN(compParam.outputLayerParam.surf->osSurface);
        switch (compParam.outputLayerParam.surf->osSurface->Format)
        {
        case Format_I420:
        case Format_IMC3:
        case Format_YV12:
        case Format_IYUV:
            VP_RENDER_CHK_STATUS_RETURN(GenerateFc420PL3OutputParam(compParam.outputLayerParam, param));
            krnParams.push_back(param);
            break;
        case Format_RGBP:
        case Format_BGRP:
        case Format_444P:
            VP_RENDER_CHK_STATUS_RETURN(GenerateFc444PL3OutputParam(compParam.outputLayerParam, param));
            krnParams.push_back(param);
            break;
        default:
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateFc420PL3InputParam(OCL_FC_LAYER_PARAM &inputLayersParam, uint32_t index, OCL_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    param.Init();
    VP_SURFACE *inputSurf               = inputLayersParam.surf;
    uint32_t    chromaChannelIndices[4] = {};
    uint32_t    lumaChannelIndices      = 0;
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputOutputSingleChannelIndexToKrnParam(inputSurf->osSurface->Format, lumaChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(inputSurf->osSurface->Format, Format_Any, chromaChannelIndices));

    uint32_t                     srcSurfaceWidth     = inputSurf->osSurface->dwWidth;
    uint32_t                     srcSurfaceHeight    = inputSurf->osSurface->dwHeight;
    uint32_t                     tarSurfaceWidth     = inputSurf->osSurface->dwWidth;
    uint32_t                     tarSurfaceHeight    = inputSurf->osSurface->dwHeight;
    uint32_t                     localSize[3]        = {128, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     threadWidth         = tarSurfaceWidth / localSize[0] + (tarSurfaceWidth % localSize[0] != 0);
    uint32_t                     threadHeight        = tarSurfaceHeight / localSize[1] + (tarSurfaceHeight % localSize[1] != 0);
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};
    std::string                  krnName             = "ImageRead_fc_420PL3_input";
    auto                         handle              = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find(krnName);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis     = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs     = handle->second.GetKernelArgs();
    auto        argLayerHandle = m_fc420PL3InputMultiLayersKrnArgs.find(index);
    if (argLayerHandle == m_fc420PL3InputMultiLayersKrnArgs.end())
    {
        KERNEL_INDEX_ARG_MAP fc420PL3InputSingleLayerKrnArgs = {};
        argLayerHandle                                       = m_fc420PL3InputMultiLayersKrnArgs.insert(std::make_pair(index, fc420PL3InputSingleLayerKrnArgs)).first;
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(argLayerHandle, &m_fc420PL3InputMultiLayersKrnArgs);
    }
    KERNEL_INDEX_ARG_MAP &fc420PL3InputKrnArgs = argLayerHandle->second;

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = fc420PL3InputKrnArgs.find(uIndex);
        if (argHandle == fc420PL3InputKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = fc420PL3InputKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &fc420PL3InputKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc420PL3InputKrnArg(srcSurfaceWidth, srcSurfaceHeight, lumaChannelIndices, chromaChannelIndices, localSize, krnArg, bInit))
        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc420PL3InputBti(uIndex, index, surfaceParam, bInit));
        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }
    param.kernelArgs             = krnArgs;
    param.kernelId               = kernelOclFc420PL3Input;
    param.threadWidth            = threadWidth;
    param.threadHeight           = threadHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateFc420PL3OutputParam(OCL_FC_LAYER_PARAM &outputLayersParam, OCL_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    param.Init();
    VP_SURFACE *outputSurf              = outputLayersParam.surf;
    uint32_t    chromaChannelIndices[2] = {};
    uint32_t    lumaChannelIndices      = 0;
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputOutputSingleChannelIndexToKrnParam(outputSurf->osSurface->Format, lumaChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertOutputChannelIndicesToKrnParam(outputSurf->osSurface->Format, chromaChannelIndices));

    uint32_t                     srcSurfaceWidth     = outputSurf->osSurface->dwWidth;
    uint32_t                     srcSurfaceHeight    = outputSurf->osSurface->dwHeight;
    uint32_t                     tarSurfaceWidth     = outputSurf->osSurface->dwWidth;
    uint32_t                     tarSurfaceHeight    = outputSurf->osSurface->dwHeight;
    uint32_t                     localSize[3]        = {128, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     threadWidth         = tarSurfaceWidth / localSize[0] + (tarSurfaceWidth % localSize[0] != 0);
    uint32_t                     threadHeight        = tarSurfaceHeight / localSize[1] + (tarSurfaceHeight % localSize[1] != 0);
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};
    std::string                  krnName             = "ImageWrite_fc_420PL3_output";
    auto                         handle              = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find(krnName);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_fc420PL3OutputKrnArgs.find(uIndex);
        if (argHandle == m_fc420PL3OutputKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_fc420PL3OutputKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_fc420PL3OutputKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc420PL3OutputKrnArg(srcSurfaceWidth, srcSurfaceHeight, lumaChannelIndices, chromaChannelIndices, localSize, krnArg, bInit))
        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc420PL3OutputBti(uIndex, surfaceParam, bInit));
        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }
    param.kernelArgs             = krnArgs;
    param.kernelId               = kernelOclFc420PL3Output;
    param.threadWidth            = threadWidth;
    param.threadHeight           = threadHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateFc422HVInputParam(OCL_FC_LAYER_PARAM &inputLayersParam, uint32_t index, OCL_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    param.Init();
    VP_SURFACE *inputSurf               = inputLayersParam.surf;
    uint32_t    chromaChannelIndices[4] = {};
    uint32_t    channelIndex            = 0;
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputOutputSingleChannelIndexToKrnParam(inputSurf->osSurface->Format, channelIndex));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(inputSurf->osSurface->Format, Format_Any, chromaChannelIndices));

    uint32_t surfaceWidthPL1  = inputSurf->osSurface->dwWidth;
    uint32_t surfaceHeightPL1 = inputSurf->osSurface->dwHeight;
    switch (inputSurf->osSurface->Format)
    {
    case Format_422H:
        surfaceWidthPL1 /= 2;
        break;
    case Format_422V:
        surfaceHeightPL1 /= 2;
        break;
    case Format_411P:
        surfaceWidthPL1 /= 4;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    uint32_t                     tarSurfaceWidth     = inputSurf->osSurface->dwWidth;
    uint32_t                     tarSurfaceHeight    = inputSurf->osSurface->dwHeight;
    uint32_t                     localSize[3]        = {128, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     threadWidth         = tarSurfaceWidth / localSize[0] + (tarSurfaceWidth % localSize[0] != 0);
    uint32_t                     threadHeight        = tarSurfaceHeight / localSize[1] + (tarSurfaceHeight % localSize[1] != 0);
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};
    std::string                  krnName             = "ImageRead_fc_422HV_input";
    auto                         handle              = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find(krnName);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis     = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs     = handle->second.GetKernelArgs();
    auto        argLayerHandle = m_fc422HVInputMultiLayersKrnArgs.find(index);
    if (argLayerHandle == m_fc422HVInputMultiLayersKrnArgs.end())
    {
        KERNEL_INDEX_ARG_MAP fc422HVInputSingleLayerKrnArgs = {};
        argLayerHandle                                      = m_fc422HVInputMultiLayersKrnArgs.insert(std::make_pair(index, fc422HVInputSingleLayerKrnArgs)).first;
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(argLayerHandle, &m_fc422HVInputMultiLayersKrnArgs);
    }
    KERNEL_INDEX_ARG_MAP &fc422HVInputKrnArgs = argLayerHandle->second;

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = fc422HVInputKrnArgs.find(uIndex);
        if (argHandle == fc422HVInputKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = fc422HVInputKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &fc422HVInputKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc422HVInputKrnArg(surfaceWidthPL1, surfaceHeightPL1, channelIndex, chromaChannelIndices, localSize, krnArg, bInit));
        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc422HVInputBti(uIndex, index, surfaceParam, bInit));
        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }
    param.kernelArgs             = krnArgs;
    param.kernelId               = kernelOclFc422HVInput;
    param.threadWidth            = threadWidth;
    param.threadHeight           = threadHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc422HVInputBti(uint32_t uIndex, uint32_t layIndex, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    switch (uIndex)
    {
    case FC_422HV_INPUT_IMAGEREAD_INPUTPLANE0:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + layIndex);
        break;
    case FC_422HV_INPUT_IMAGEREAD_INPUTPLANE1:
    case FC_422HV_INPUT_IMAGEREAD_INPUTPLANE2:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    case FC_422HV_INPUT_IMAGEREAD_OUTPUTPLANE0:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcIntermediaInput + layIndex);
        surfaceParam.isOutput = true;
        break;
    case FC_422HV_INPUT_IMAGEREAD_OUTPUTPLANE1:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + layIndex);
        surfaceParam.isOutput = true;
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc422HVInputKrnArg(uint32_t srcSurfaceWidthPL1, uint32_t srcSurfaceHeightPL1, uint32_t channelIndex, uint32_t chromaChannelIndices[4], uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit)
{
    switch (krnArg.uIndex)
    {
    case FC_422HV_INPUT_IMAGEREAD_WIDTHUV:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = srcSurfaceWidthPL1;
        break;
    case FC_422HV_INPUT_IMAGEREAD_HEIGHTUV:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = srcSurfaceHeightPL1;
        break;
    case FC_422HV_INPUT_IMAGEREAD_INPUTINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = channelIndex;
        break;
    case FC_422HV_INPUT_IMAGEREAD_OUTPUTINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = chromaChannelIndices[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = chromaChannelIndices[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = chromaChannelIndices[2];
        static_cast<uint32_t *>(krnArg.pData)[3] = chromaChannelIndices[3];
        break;
    case FC_422HV_INPUT_IMAGEREAD_LOCAL_SIZE:
    case FC_422HV_INPUT_IMAGEREAD_ENQUEUED_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateFc444PL3OutputParam(OCL_FC_LAYER_PARAM &outputLayersParam, OCL_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    param.Init();
    VP_SURFACE *outputSurf              = outputLayersParam.surf;
    uint32_t    inputChannelIndices[4]  = {};
    uint32_t    outputChannelIndices[4] = {};
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(outputLayersParam.intermediaFormat, outputLayersParam.separateIntermediaSecPlaneFormat, inputChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertOutputChannelIndicesToKrnParam(outputSurf->osSurface->Format, outputChannelIndices));

    uint32_t                     srcSurfaceWidth     = outputSurf->osSurface->dwWidth;
    uint32_t                     srcSurfaceHeight    = outputSurf->osSurface->dwHeight;
    uint32_t                     tarSurfaceWidth     = outputSurf->osSurface->dwWidth;
    uint32_t                     tarSurfaceHeight    = outputSurf->osSurface->dwHeight;
    uint32_t                     localSize[3]        = {128, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     threadWidth         = tarSurfaceWidth / localSize[0] + (tarSurfaceWidth % localSize[0] != 0);
    uint32_t                     threadHeight        = tarSurfaceHeight / localSize[1] + (tarSurfaceHeight % localSize[1] != 0);
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};
    std::string                  krnName             = "ImageWrite_fc_444PL3_output";
    auto                         handle              = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find(krnName);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_fc444PL3OutputKrnArgs.find(uIndex);
        if (argHandle == m_fc444PL3OutputKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_fc444PL3OutputKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_fc444PL3OutputKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc444PL3OutputKrnArg(localSize, krnArg, bInit, inputChannelIndices, outputChannelIndices));
        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc444PL3OutputBti(uIndex, surfaceParam, bInit));
        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }
    param.kernelArgs             = krnArgs;
    param.kernelId               = kernelOclFc444PL3Output;
    param.threadWidth            = threadWidth;
    param.threadHeight           = threadHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc444PL3OutputBti(uint32_t uIndex, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    VP_FUNC_CALL();
    switch (uIndex)
    {
    case FC_444PL3_OUTPUT_IMAGEWRITE_INPUTPLANE0:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcIntermediaOutput);
        break;
    case FC_444PL3_OUTPUT_IMAGEWRITE_OUTPUTPLANE0:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcTarget0);
        surfaceParam.isOutput = true;
        break;
    case FC_444PL3_OUTPUT_IMAGEWRITE_OUTPUTPLANE1:
    case FC_444PL3_OUTPUT_IMAGEWRITE_OUTPUTPLANE2:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc444PL3OutputKrnArg(uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit, uint32_t inputChannelIndices[4], uint32_t outputChannelIndices[4])
{
    VP_FUNC_CALL();
    switch (krnArg.uIndex)
    {
    case FC_444PL3_OUTPUT_IMAGEWRITE_INPUTINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = inputChannelIndices[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = inputChannelIndices[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = inputChannelIndices[2];
        static_cast<uint32_t *>(krnArg.pData)[3] = inputChannelIndices[3];
        break;
    case FC_444PL3_OUTPUT_IMAGEWRITE_OUTPUTINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = outputChannelIndices[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = outputChannelIndices[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = outputChannelIndices[2];
        static_cast<uint32_t *>(krnArg.pData)[3] = outputChannelIndices[3];
        break;
    case FC_444PL3_OUTPUT_IMAGEWRITE_LOCAL_SIZE:
    case FC_444PL3_OUTPUT_IMAGEWRITE_ENQUEUED_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}
MOS_STATUS VpOclFcFilter::GenerateFc444PL3InputParam(OCL_FC_LAYER_PARAM &layer, uint32_t layerNumber, OCL_FC_KERNEL_PARAM &param, uint32_t layerIndex)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    param                                                = {};
    uint32_t                     localSize[3]            = {128, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     threadWidth             = layer.surf->osSurface->dwWidth / localSize[0] + (layer.surf->osSurface->dwWidth % localSize[0] != 0);
    uint32_t                     threadHeight            = layer.surf->osSurface->dwHeight / localSize[1] + (layer.surf->osSurface->dwHeight % localSize[1] != 0);
    KERNEL_ARGS                  krnArgs                 = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces     = {};
    uint32_t                     inputChannelIndices[4]  = {};
    uint32_t                     outputChannelIndices[4] = {};
    uint32_t                     planeChannelIndics      = 0;
    std::string                  krnName                 = "ImageRead_fc_444PL3_input";

    auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find(krnName);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(layer.surf->osSurface->Format, Format_Any, inputChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertOutputChannelIndicesToKrnParam(layer.intermediaFormat, outputChannelIndices));
    auto argLayerHandle = m_fc444PL3InputMultiLayersKrnArgs.find(layerIndex);
    if (argLayerHandle == m_fc444PL3InputMultiLayersKrnArgs.end())
    {
        KERNEL_INDEX_ARG_MAP fc444PL3InputSingleLayerKrnArgs = {};
        argLayerHandle                                       = m_fc444PL3InputMultiLayersKrnArgs.insert(std::make_pair(layerIndex, fc444PL3InputSingleLayerKrnArgs)).first;
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(argLayerHandle, &m_fc444PL3InputMultiLayersKrnArgs);
    }
    KERNEL_INDEX_ARG_MAP &fc444PL3InputKrnArgs = argLayerHandle->second;

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = fc444PL3InputKrnArgs.find(uIndex);
        if (argHandle == fc444PL3InputKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = fc444PL3InputKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &fc444PL3InputKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc444PL3InputKrnArg(localSize, krnArg, bInit, inputChannelIndices, outputChannelIndices, planeChannelIndics));

        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFc444PL3InputBti(uIndex, surfaceParam, layerIndex, bInit));

        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }

    param.kernelArgs             = krnArgs;
    param.kernelName             = krnName;
    param.kernelId               = kernelOclFc444PL3Input;
    param.threadWidth            = threadWidth;
    param.threadHeight           = threadHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc444PL3InputKrnArg(uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit, uint32_t inputChannelIndices[4], uint32_t outputChannelIndices[4], uint32_t planeChannelIndices)
{
    switch (krnArg.uIndex)
    {
    case FC_444PL3_INPUT_IMAGEREAD_INPUTINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = inputChannelIndices[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = inputChannelIndices[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = inputChannelIndices[2];
        static_cast<uint32_t *>(krnArg.pData)[3] = inputChannelIndices[3];
        break;
    case FC_444PL3_INPUT_IMAGEREAD_OUTPUTINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = outputChannelIndices[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = outputChannelIndices[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = outputChannelIndices[2];
        static_cast<uint32_t *>(krnArg.pData)[3] = outputChannelIndices[3];
        break;
    case FC_444PL3_INPUT_IMAGEREAD_PLANEINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = planeChannelIndices;
        break;
    case FC_444PL3_INPUT_IMAGEREAD_ENQUEUED_LOCAL_SIZE:
    case FC_444PL3_INPUT_IMAGEREAD_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc444PL3InputBti(uint32_t uIndex, SURFACE_PARAMS &surfaceParam, uint32_t layerIndex, bool &bInit)
{
    switch (uIndex)
    {
    case FC_444PL3_INPUT_IMAGEREAD_INPUTPLANE0:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + layerIndex);
        break;
    case FC_444PL3_INPUT_IMAGEREAD_INPUTPLANE1:
    case FC_444PL3_INPUT_IMAGEREAD_INPUTPLANE2:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    case FC_444PL3_INPUT_IMAGEREAD_OUTPUTPLANE:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcIntermediaInput + layerIndex);
        surfaceParam.isOutput = true;
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateFcCommonKrnParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);

    param = {};
    std::vector<OCL_FC_KRN_IMAGE_PARAM> imageParams(compParam.layerNumber);
    OCL_FC_KRN_TARGET_PARAM             targetParam = {};
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        VP_RENDER_CHK_STATUS_RETURN(GenerateInputImageParam(compParam.inputLayersParam[i], compParam.mainCSpace, imageParams.at(i)));
    }
    VP_RENDER_CHK_STATUS_RETURN(GenerateTargetParam(compParam, targetParam));
    PrintKrnParam(imageParams, targetParam);

    uint32_t                     localSize[3]        = {128, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     threadWidth         = targetParam.targetROI.right / localSize[0] + (targetParam.targetROI.right % localSize[0] != 0);
    uint32_t                     threadHeight        = targetParam.targetROI.bottom / localSize[1] + (targetParam.targetROI.bottom % localSize[1] != 0);
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};

    auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find("FastComp_fc_common");
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_fcCommonKrnArgs.find(uIndex);
        if (argHandle == m_fcCommonKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_fcCommonKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_fcCommonKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcCommonKrnArg(compParam.layerNumber, imageParams, targetParam, localSize, krnArg, bInit));

        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcCommonBti(uIndex, compParam, surfaceParam, bInit));

        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }

    param.kernelArgs             = krnArgs;
    param.kernelName             = "FastComp_fc_common";
    param.kernelId               = kernelOclFcCommon;
    param.threadWidth            = threadWidth;
    param.threadHeight           = threadHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc420PL3InputKrnArg(uint32_t srcSurfaceWidth, uint32_t srcSurfaceHeight, uint32_t lumaChannelIndices, uint32_t chromaChannelIndices[4], uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit)
{
    switch (krnArg.uIndex)
    {
    case FC_420PL3_INPUT_IMAGEREAD_WIDTH:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = srcSurfaceWidth;
        break;
    case FC_420PL3_INPUT_IMAGEREAD_HEIGHT:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = srcSurfaceHeight;
        break;
    case FC_420PL3_INPUT_IMAGEREAD_LUMAINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = lumaChannelIndices;
        break;
    case FC_420PL3_INPUT_IMAGEREAD_CHROMAINDEXS:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = chromaChannelIndices[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = chromaChannelIndices[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = chromaChannelIndices[2];
        static_cast<uint32_t *>(krnArg.pData)[3] = chromaChannelIndices[3];
        break;
    case FC_420PL3_INPUT_IMAGEREAD_LOCAL_SIZE:
    case FC_420PL3_INPUT_IMAGEREAD_ENQUEUED_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc420PL3OutputKrnArg(uint32_t srcSurfaceWidth, uint32_t srcSurfaceHeight, uint32_t lumaChannelIndices, uint32_t chromaChannelIndices[2], uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit)
{
    VP_FUNC_CALL();
    switch (krnArg.uIndex)
    {
    case FC_420PL3_INPUT_IMAGEREAD_WIDTH:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = srcSurfaceWidth;
        break;
    case FC_420PL3_INPUT_IMAGEREAD_HEIGHT:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = srcSurfaceHeight;
        break;
    case FC_420PL3_INPUT_IMAGEREAD_LUMAINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = lumaChannelIndices;
        break;
    case FC_420PL3_OUTPUT_IMAGEWRITE_UPLINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = chromaChannelIndices[0];
        break;
    case FC_420PL3_OUTPUT_IMAGEWRITE_VPLINDEX:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = chromaChannelIndices[1];
        break;
    case FC_420PL3_OUTPUT_IMAGEWRITE_LOCAL_SIZE:
    case FC_420PL3_OUTPUT_IMAGEWRITE_ENQUEUED_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFcCommonKrnArg(uint32_t layerNum, std::vector<OCL_FC_KRN_IMAGE_PARAM> &imageParams, OCL_FC_KRN_TARGET_PARAM &targetParam, uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit)
{
    switch (krnArg.uIndex)
    {
    case FC_COMMON_FASTCOMP_LAYERNUMBER:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = layerNum;
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM0:
        if (imageParams.size() > 0)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(0)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(0), sizeof(imageParams.at(0)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM1:
        if (imageParams.size() > 1)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(1)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(1), sizeof(imageParams.at(1)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM2:
        if (imageParams.size() > 2)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(2)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(2), sizeof(imageParams.at(2)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM3:
        if (imageParams.size() > 3)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(3)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(3), sizeof(imageParams.at(3)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM4:
        if (imageParams.size() > 4)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(4)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(4), sizeof(imageParams.at(4)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM5:
        if (imageParams.size() > 5)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(5)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(5), sizeof(imageParams.at(5)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM6:
        if (imageParams.size() > 6)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(6)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(6), sizeof(imageParams.at(6)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM7:
        if (imageParams.size() > 7)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(7)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(7), sizeof(imageParams.at(7)));
        }
        break;

        break;
    case FC_COMMON_FASTCOMP_OUTPUTPARAM:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(targetParam));
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &targetParam, sizeof(targetParam));
        break;
    case FC_COMMON_FASTCOMP_INLINE_SAMPLER_LINEAR_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_BILINEAR;
        break;
    case FC_COMMON_FASTCOMP_INLINE_SAMPLER_NEAREST_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_NEAREST;
        break;
    case FC_COMMON_FASTCOMP_ENQUEUED_LOCAL_SIZE:
    case FC_COMMON_FASTCOMP_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc420PL3InputBti(uint32_t uIndex, uint32_t layIndex, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    switch (uIndex)
    {
    case FC_420PL3_INPUT_IMAGEREAD_INPUT0PLY:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + layIndex);
        break;
    case FC_420PL3_INPUT_IMAGEREAD_INPUT0PL1:
    case FC_420PL3_INPUT_IMAGEREAD_INPUT0PL2:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    case FC_420PL3_INPUT_IMAGEREAD_OUTPUTPLY:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcIntermediaInput + layIndex);
        surfaceParam.isOutput = true;
        break;
    case FC_420PL3_INPUT_IMAGEREAD_OUTPUTPLUV:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFc420PL3OutputBti(uint32_t uIndex, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    VP_FUNC_CALL();
    switch (uIndex)
    {
    case FC_420PL3_OUTPUT_IMAGEWRITE_INPUTPLY:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcIntermediaOutput);
        break;
    case FC_420PL3_OUTPUT_IMAGEWRITE_INPUTPLUV:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    case FC_420PL3_OUTPUT_IMAGEWRITE_OUTPUTPLY:
        surfaceParam.surfType = SurfaceType(SurfaceTypeFcTarget0);
        surfaceParam.isOutput = true;
        break;
    case FC_420PL3_OUTPUT_IMAGEWRITE_OUTPUTPLU:
    case FC_420PL3_OUTPUT_IMAGEWRITE_OUTPUTPLV:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    default:
        bInit = false;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFcCommonBti(uint32_t uIndex, const OCL_FC_COMP_PARAM &compParam, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    switch (uIndex)
    {
    case FC_COMMON_FASTCOMP_INPUT0PL0:
        if (compParam.layerNumber > 0)
        {
            surfaceParam.surfType = compParam.inputLayersParam[0].needIntermediaSurface ? SurfaceTypeFcIntermediaInput : SurfaceTypeFcInputLayer0;
            if (compParam.inputLayersParam[0].diParams.enabled &&
                compParam.inputLayersParam[0].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT1PL0:
        if (compParam.layerNumber > 1)
        {
            surfaceParam.surfType = compParam.inputLayersParam[1].needIntermediaSurface ? SurfaceType(SurfaceTypeFcIntermediaInput + 1) : SurfaceType(SurfaceTypeFcInputLayer0 + 1);
            if (compParam.inputLayersParam[1].diParams.enabled &&
                compParam.inputLayersParam[1].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT2PL0:
        if (compParam.layerNumber > 2)
        {
            surfaceParam.surfType = compParam.inputLayersParam[2].needIntermediaSurface ? SurfaceType(SurfaceTypeFcIntermediaInput + 2) : SurfaceType(SurfaceTypeFcInputLayer0 + 2);
            if (compParam.inputLayersParam[2].diParams.enabled &&
                compParam.inputLayersParam[2].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT3PL0:
        if (compParam.layerNumber > 3)
        {
            surfaceParam.surfType = compParam.inputLayersParam[3].needIntermediaSurface ? SurfaceType(SurfaceTypeFcIntermediaInput + 3) : SurfaceType(SurfaceTypeFcInputLayer0 + 3);
            if (compParam.inputLayersParam[3].diParams.enabled &&
                compParam.inputLayersParam[3].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT4PL0:
        if (compParam.layerNumber > 4)
        {
            surfaceParam.surfType = compParam.inputLayersParam[4].needIntermediaSurface ? SurfaceType(SurfaceTypeFcIntermediaInput + 4) : SurfaceType(SurfaceTypeFcInputLayer0 + 4);
            if (compParam.inputLayersParam[4].diParams.enabled &&
                compParam.inputLayersParam[4].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT5PL0:
        if (compParam.layerNumber > 5)
        {
            surfaceParam.surfType = compParam.inputLayersParam[5].needIntermediaSurface ? SurfaceType(SurfaceTypeFcIntermediaInput + 5) : SurfaceType(SurfaceTypeFcInputLayer0 + 5);
            if (compParam.inputLayersParam[5].diParams.enabled &&
                compParam.inputLayersParam[5].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT6PL0:
        if (compParam.layerNumber > 6)
        {
            surfaceParam.surfType = compParam.inputLayersParam[6].needIntermediaSurface ? SurfaceType(SurfaceTypeFcIntermediaInput + 6) : SurfaceType(SurfaceTypeFcInputLayer0 + 6);
            if (compParam.inputLayersParam[6].diParams.enabled &&
                compParam.inputLayersParam[6].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT7PL0:
        if (compParam.layerNumber > 7)
        {
            surfaceParam.surfType = compParam.inputLayersParam[7].needIntermediaSurface ? SurfaceType(SurfaceTypeFcIntermediaInput + 7) : SurfaceType(SurfaceTypeFcInputLayer0 + 7);
            if (compParam.inputLayersParam[7].diParams.enabled &&
                compParam.inputLayersParam[7].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_OUTPUTPL0:
        surfaceParam.surfType = compParam.outputLayerParam.needIntermediaSurface ? SurfaceTypeFcIntermediaOutput : SurfaceTypeFcTarget0;
        surfaceParam.isOutput = true;
        break;
    case FC_COMMON_FASTCOMP_INPUT0PL1:
        if (compParam.layerNumber > 0 && compParam.inputLayersParam[0].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane);
            if (compParam.inputLayersParam[0].diParams.enabled &&
                compParam.inputLayersParam[0].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT1PL1:
        if (compParam.layerNumber > 1 && compParam.inputLayersParam[1].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + 1);
            if (compParam.inputLayersParam[1].diParams.enabled &&
                compParam.inputLayersParam[1].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT2PL1:
        if (compParam.layerNumber > 2 && compParam.inputLayersParam[2].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + 2);
            if (compParam.inputLayersParam[2].diParams.enabled &&
                compParam.inputLayersParam[2].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT3PL1:
        if (compParam.layerNumber > 3 && compParam.inputLayersParam[3].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + 3);
            if (compParam.inputLayersParam[3].diParams.enabled &&
                compParam.inputLayersParam[3].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT4PL1:
        if (compParam.layerNumber > 4 && compParam.inputLayersParam[4].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + 4);
            if (compParam.inputLayersParam[4].diParams.enabled &&
                compParam.inputLayersParam[4].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT5PL1:
        if (compParam.layerNumber > 5 && compParam.inputLayersParam[5].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + 5);
            if (compParam.inputLayersParam[5].diParams.enabled &&
                compParam.inputLayersParam[5].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT6PL1:
        if (compParam.layerNumber > 6 && compParam.inputLayersParam[6].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + 6);
            if (compParam.inputLayersParam[6].diParams.enabled &&
                compParam.inputLayersParam[6].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT7PL1:
        if (compParam.layerNumber > 7 && compParam.inputLayersParam[7].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcSeparateIntermediaInputSecPlane + 7);
            if (compParam.inputLayersParam[7].diParams.enabled &&
                compParam.inputLayersParam[7].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_COMMON_FASTCOMP_OUTPUTPL1:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::InitCompParam(SwFilterPipe &executingPipe, OCL_FC_COMP_PARAM &compParam)
{
    VP_FUNC_CALL();

    auto &surfGroup       = executingPipe.GetSurfacesSetting().surfGroup;
    compParam.layerNumber = executingPipe.GetSurfaceCount(true);
    if (SurfaceTypeFcInputLayer0 + compParam.layerNumber - 1 > SurfaceTypeFcInputLayerMax)
    {
        VP_RENDER_ASSERTMESSAGE("Invalid source count (%d)!", compParam.layerNumber);
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // Select default scaling mode for 3D sampler.
    VPHAL_SCALING_MODE defaultScalingMode = VPHAL_SCALING_NEAREST;
    VP_PUBLIC_CHK_STATUS_RETURN(GetDefaultScalingMode(defaultScalingMode, executingPipe));

    for (uint32_t i = 0; i < executingPipe.GetSurfaceCount(true); ++i)
    {
        VP_RENDER_CHK_STATUS_RETURN(InitLayer(executingPipe, true, i, defaultScalingMode, compParam.inputLayersParam[i]));
    }

    VP_RENDER_CHK_STATUS_RETURN(InitLayer(executingPipe, false, 0, defaultScalingMode, compParam.outputLayerParam));

    //use target Cspace as main color space instead of using RGB as main Cspace for no obvious difference
    compParam.mainCSpace = compParam.outputLayerParam.surf->ColorSpace;

    SwFilterColorFill *colorFill = dynamic_cast<SwFilterColorFill *>(executingPipe.GetSwFilter(false, 0, FeatureType::FeatureTypeColorFill));
    if (colorFill && colorFill->GetSwFilterParams().colorFillParams)
    {
        compParam.enableColorFill = true;
        compParam.colorFillParams = *colorFill->GetSwFilterParams().colorFillParams;
    }
    else
    {
        compParam.enableColorFill = false;
    }

    SwFilterAlpha *alpha = dynamic_cast<SwFilterAlpha *>(executingPipe.GetSwFilter(false, 0, FeatureType::FeatureTypeAlpha));
    if (alpha && alpha->GetSwFilterParams().compAlpha)
    {
        compParam.compAlpha             = *alpha->GetSwFilterParams().compAlpha;
        compParam.bAlphaCalculateEnable = alpha->GetSwFilterParams().calculatingAlpha;
    }
    else
    {
        compParam.bAlphaCalculateEnable = false;
        compParam.compAlpha             = {};
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::InitLayer(SwFilterPipe &executingPipe, bool isInputPipe, int index, VPHAL_SCALING_MODE defaultScalingMode, OCL_FC_LAYER_PARAM &layer)
{
    VP_FUNC_CALL();
    auto &surfGroup = executingPipe.GetSurfacesSetting().surfGroup;

    SurfaceType surfId     = isInputPipe ? (SurfaceType)(SurfaceTypeFcInputLayer0 + index) : SurfaceTypeFcTarget0;
    auto        surfHandle = surfGroup.find(surfId);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfHandle, &surfGroup);
    layer.surf = surfHandle->second;

    VP_PUBLIC_CHK_NULL_RETURN(layer.surf);
    VP_PUBLIC_CHK_NULL_RETURN(layer.surf->osSurface);
    layer.layerID       = index;
    layer.layerIDOrigin = index;
    switch (layer.surf->osSurface->Format)
    {
    case Format_RGBP:
    case Format_BGRP:
        layer.needIntermediaSurface = true;
        layer.intermediaFormat      = Format_A8R8G8B8;
        break;
    case Format_444P:
        layer.needIntermediaSurface = true;
        layer.intermediaFormat      = Format_AYUV;
        break;
    case Format_I420:
    case Format_IMC3:
    case Format_IYUV:
    case Format_YV12:
        layer.needIntermediaSurface = true;
        layer.intermediaFormat      = Format_NV12;
        break;
    case Format_422H:
    case Format_422V:
    case Format_411P:
        if (isInputPipe)
        {
            layer.needIntermediaSurface            = true;
            layer.intermediaFormat                 = Format_R8UN;
            layer.needSepareateIntermediaSecPlane  = true;
            layer.separateIntermediaSecPlaneFormat = Format_R8G8UN;
        }
        break;
    default:
        break;
    }

    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeScaling));
    layer.scalingMode        = scaling ? scaling->GetSwFilterParams().scalingMode : defaultScalingMode;

    SwFilterRotMir *rotation = dynamic_cast<SwFilterRotMir *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeRotMir));
    layer.rotation           = rotation ? rotation->GetSwFilterParams().rotation : VPHAL_ROTATION_IDENTITY;

    SwFilterDeinterlace *di = dynamic_cast<SwFilterDeinterlace *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeDi));
    if (di && di->GetSwFilterParams().diParams)
    {
        layer.diParams.enabled = true;
        layer.diParams.params  = *di->GetSwFilterParams().diParams;
    }
    else
    {
        layer.diParams.enabled = false;
    }

    SwFilterLumakey *lumakey = dynamic_cast<SwFilterLumakey *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeLumakey));
    if (lumakey && lumakey->GetSwFilterParams().lumaKeyParams)
    {
        layer.lumaKey.enabled = true;
        layer.lumaKey.params  = *lumakey->GetSwFilterParams().lumaKeyParams;
    }
    else
    {
        layer.lumaKey.enabled = false;
    }

    SwFilterBlending *blending = dynamic_cast<SwFilterBlending *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeBlending));
    if (blending && blending->GetSwFilterParams().blendingParams)
    {
        layer.blendingParams = *blending->GetSwFilterParams().blendingParams;
    }
    else
    {
        layer.blendingParams.BlendType = BLEND_NONE;
    }

    SwFilterProcamp *procamp = dynamic_cast<SwFilterProcamp *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeProcamp));
    if (procamp && procamp->GetSwFilterParams().procampParams)
    {
        layer.procampParams = *procamp->GetSwFilterParams().procampParams;
    }
    else
    {
        layer.procampParams.bEnabled = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GetDefaultScalingMode(VPHAL_SCALING_MODE &defaultScalingMode, SwFilterPipe &executedPipe)
{
    bool isInited = false;
    // Select default scaling mode for 3D sampler.
    defaultScalingMode = VPHAL_SCALING_NEAREST;

    if (!PolicyFcHandler::s_forceNearestToBilinearIfBilinearExists)
    {
        return MOS_STATUS_SUCCESS;
    }

    for (uint32_t i = 0; i < executedPipe.GetSurfaceCount(true); ++i)
    {
        SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executedPipe.GetSwFilter(true, i, FeatureType::FeatureTypeScaling));
        if (scaling &&
            (VPHAL_SCALING_NEAREST == scaling->GetSwFilterParams().scalingMode ||
                VPHAL_SCALING_BILINEAR == scaling->GetSwFilterParams().scalingMode))
        {
            if (isInited)
            {
                if (scaling->GetSwFilterParams().scalingMode != defaultScalingMode)
                {
                    VP_PUBLIC_ASSERTMESSAGE("Different 3D sampler scaling mode being selected!");
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
                }
            }
            else
            {
                defaultScalingMode = scaling->GetSwFilterParams().scalingMode;
                isInited           = true;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GetChromaSitingFactor(MOS_FORMAT format, uint8_t &hitSecPlaneFactorX, uint8_t &hitSecPlaneFactorY)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A16R16G16B16:
    case Format_R10G10B10A2:
    case Format_AYUV:
    case Format_A16R16G16B16F:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_A16B16G16R16:
    case Format_B10G10R10A2:
    case Format_A16B16G16R16F:
    case Format_Y410:
    case Format_Y416:
    case Format_400P:
    case Format_R5G6B5:
    case Format_R8G8B8:
        hitSecPlaneFactorX = 1;
        hitSecPlaneFactorY = 1;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        hitSecPlaneFactorX = 2;
        hitSecPlaneFactorY = 2;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_Y210:
    case Format_Y216:
        hitSecPlaneFactorX = 2;
        hitSecPlaneFactorY = 1;
        break;
    case Format_RGBP:
    case Format_BGRP:
    case Format_444P:
        hitSecPlaneFactorX = 1;
        hitSecPlaneFactorY = 1;
        break;
    case Format_I420:
    case Format_IMC3:
    case Format_IYUV:
    case Format_YV12:
        hitSecPlaneFactorX = 2;
        hitSecPlaneFactorY = 2;
        break;
    case Format_422H:
        hitSecPlaneFactorX = 2;
        hitSecPlaneFactorY = 1;
        break;
    case Format_422V:
        hitSecPlaneFactorX = 1;
        hitSecPlaneFactorY = 2;
        break;
    case Format_411P:
        hitSecPlaneFactorX = 4;
        hitSecPlaneFactorY = 1;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GetBitNumber(MOS_FORMAT format, uint8_t *pOriginBitNumber, uint8_t *pStoredBitNumber, uint8_t *pAlphaBitNumber)
{
    uint8_t storedBitNumber = 0;
    uint8_t originBitNumber = 0;
    uint8_t alphaBitNumber  = 0;

    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_AYUV:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
        storedBitNumber = 8;
        originBitNumber = 8;
        alphaBitNumber  = 8;
        break;
    case Format_A16R16G16B16:
    case Format_A16B16G16R16:
    case Format_Y416:
        storedBitNumber = 16;
        originBitNumber = 16;
        alphaBitNumber  = 16;
        break;
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
    case Format_Y410:
        storedBitNumber = 10;
        originBitNumber = 10;
        alphaBitNumber  = 2;
        break;
    case Format_A16R16G16B16F:
    case Format_A16B16G16R16F:
        storedBitNumber = 0;
        originBitNumber = 0;
        alphaBitNumber  = 0;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_NV12:
    case Format_400P:
    case Format_R8G8B8:
        storedBitNumber = 8;
        originBitNumber = 8;
        alphaBitNumber  = 0;
        break;
    case Format_Y210:
    case Format_P010:
    case Format_P210:
        storedBitNumber = 16;
        originBitNumber = 10;
        alphaBitNumber  = 0;
        break;
    case Format_Y216:
    case Format_P016:
    case Format_P216:
        storedBitNumber = 16;
        originBitNumber = 16;
        alphaBitNumber  = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    if (pOriginBitNumber)
    {
        *pOriginBitNumber = originBitNumber;
    }
    if (pStoredBitNumber)
    {
        *pStoredBitNumber = storedBitNumber;
    }
    if (pAlphaBitNumber)
    {
        *pAlphaBitNumber = alphaBitNumber;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateInputImageParam(OCL_FC_LAYER_PARAM &layer, VPHAL_CSPACE mainCSpace, OCL_FC_KRN_IMAGE_PARAM &imageParam)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(layer.surf);
    VP_PUBLIC_CHK_NULL_RETURN(layer.surf->osSurface);
    MOS_FORMAT surfOverwriteFormat = layer.needIntermediaSurface ? layer.intermediaFormat : layer.surf->osSurface->Format;
    uint32_t   inputWidth          = MOS_MIN(static_cast<uint32_t>(layer.surf->osSurface->dwWidth), static_cast<uint32_t>(layer.surf->rcSrc.right));
    uint32_t   inputHeight         = MOS_MIN(static_cast<uint32_t>(layer.surf->osSurface->dwHeight), static_cast<uint32_t>(layer.surf->rcSrc.bottom));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertProcampAndCscToKrnParam(layer.surf->ColorSpace, mainCSpace, imageParam.csc, layer.procampParams));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(surfOverwriteFormat, layer.separateIntermediaSecPlaneFormat, imageParam.inputChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertScalingRotToKrnParam(layer.surf->rcSrc, layer.surf->rcDst, layer.scalingMode, inputWidth, inputHeight, layer.rotation, imageParam.scale, imageParam.controlSetting.samplerType, imageParam.coordShift));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaUpsampleToKrnParam(layer.surf->osSurface->Format, layer.surf->ChromaSiting, layer.scalingMode, inputWidth, inputHeight, imageParam.coordShift.chromaShiftX, imageParam.coordShift.chromaShiftY, imageParam.controlSetting.isChromaShift));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(surfOverwriteFormat, layer.needSepareateIntermediaSecPlane, true, imageParam.inputPlaneNum));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertBlendingToKrnParam(layer.blendingParams, imageParam.controlSetting.ignoreSrcPixelAlpha, imageParam.controlSetting.ignoreDstPixelAlpha, imageParam.constAlphs));

    if (layer.lumaKey.enabled)
    {
        imageParam.lumaKey.low  = (float)layer.lumaKey.params.LumaLow / 255;
        imageParam.lumaKey.high = (float)layer.lumaKey.params.LumaHigh / 255;
    }
    else
    {
        imageParam.lumaKey.low  = -1.f;
        imageParam.lumaKey.high = -1.f;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertRotationToKrnParam(VPHAL_ROTATION rotation, float strideX, float strideY, float startLeft, float startRight, float startTop, float startBottom, OCL_FC_KRN_SCALE_PARAM &scaling)
{
    switch (rotation)
    {
    case VPHAL_ROTATION_IDENTITY:
        scaling.rotateIndices[0] = 0;
        scaling.rotateIndices[1] = 1;
        scaling.src.startX       = startLeft;
        scaling.src.startY       = startTop;
        scaling.src.strideX      = strideX;
        scaling.src.strideY      = strideY;
        break;
    case VPHAL_ROTATION_90:
        scaling.rotateIndices[0] = 1;
        scaling.rotateIndices[1] = 0;
        scaling.src.startX       = startLeft;
        scaling.src.startY       = startBottom;
        scaling.src.strideX      = strideX;
        scaling.src.strideY      = -strideY;
        break;
    case VPHAL_ROTATION_180:
        scaling.rotateIndices[0] = 0;
        scaling.rotateIndices[1] = 1;
        scaling.src.startX       = startRight;
        scaling.src.startY       = startBottom;
        scaling.src.strideX      = -strideX;
        scaling.src.strideY      = -strideY;
        break;
    case VPHAL_ROTATION_270:
        scaling.rotateIndices[0] = 1;
        scaling.rotateIndices[1] = 0;
        scaling.src.startX       = startRight;
        scaling.src.startY       = startTop;
        scaling.src.strideX      = -strideX;
        scaling.src.strideY      = strideY;
        break;
    case VPHAL_MIRROR_HORIZONTAL:
        scaling.rotateIndices[0] = 0;
        scaling.rotateIndices[1] = 1;
        scaling.src.startX       = startRight;
        scaling.src.startY       = startTop;
        scaling.src.strideX      = -strideX;
        scaling.src.strideY      = strideY;
        break;
    case VPHAL_MIRROR_VERTICAL:
        scaling.rotateIndices[0] = 0;
        scaling.rotateIndices[1] = 1;
        scaling.src.startX       = startLeft;
        scaling.src.startY       = startBottom;
        scaling.src.strideX      = strideX;
        scaling.src.strideY      = -strideY;
        break;
    case VPHAL_ROTATE_90_MIRROR_VERTICAL:
        scaling.rotateIndices[0] = 1;
        scaling.rotateIndices[1] = 0;
        scaling.src.startX       = startRight;
        scaling.src.startY       = startBottom;
        scaling.src.strideX      = -strideX;
        scaling.src.strideY      = -strideY;
        break;
    case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
        scaling.rotateIndices[0] = 1;
        scaling.rotateIndices[1] = 0;
        scaling.src.startX       = startLeft;
        scaling.src.startY       = startTop;
        scaling.src.strideX      = strideX;
        scaling.src.strideY      = strideY;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS VpOclFcFilter::GenerateProcampCscMatrix(VPHAL_CSPACE srcColorSpace, VPHAL_CSPACE dstColorSpace, float *cscMatrix, VPHAL_PROCAMP_PARAMS &procampParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_NORMALMESSAGE("Procamp enabled. srcColorSpace %d, dstColorSpace %d.", srcColorSpace, dstColorSpace);
    float backCscMatrix[12] = {};  // back  matrix (YUV->RGB)
    float preCscMatrix[12]  = {};  // pre matrix (RGB->YUV) (YUV->YUV)
    bool  bBackCscEnabled   = false;
    bool  bPreCscEnabled    = false;

    if (IS_COLOR_SPACE_RGB(dstColorSpace) && !IS_COLOR_SPACE_RGB(srcColorSpace))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(srcColorSpace, dstColorSpace, backCscMatrix));
        bBackCscEnabled = true;  // YUV -> RGB
    }
    else if (IS_COLOR_SPACE_RGB(srcColorSpace) && !IS_COLOR_SPACE_RGB(dstColorSpace))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(srcColorSpace, dstColorSpace, preCscMatrix));
        bPreCscEnabled = true;  // RGB -> YUV
    }
    else if (IS_COLOR_SPACE_BT709_RGB(srcColorSpace) && IS_COLOR_SPACE_BT709_RGB(dstColorSpace))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(srcColorSpace, CSpace_BT709, preCscMatrix));
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(CSpace_BT709, dstColorSpace, backCscMatrix));
        bPreCscEnabled = bBackCscEnabled = true;  // 8bit RGB -> RGB
    }
    else if (IS_COLOR_SPACE_BT2020_RGB(srcColorSpace) && IS_COLOR_SPACE_BT2020_RGB(dstColorSpace))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(srcColorSpace, CSpace_BT2020, preCscMatrix));
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(CSpace_BT2020, dstColorSpace, backCscMatrix));
        bPreCscEnabled = bBackCscEnabled = true;  // 10bit RGB -> RGB
    }
    else
    {
        if (srcColorSpace != dstColorSpace)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(srcColorSpace, dstColorSpace, preCscMatrix));
            bPreCscEnabled = true;  // YUV -> YUV
            VP_PUBLIC_NORMALMESSAGE("YUV to YUV colorspace. Need pre csc matrix.");
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("The same colorspace. No need pre or back csc matrix.");
        }
    }

    // Calculate procamp parameters
    float brightness, contrast, hue, saturation;
    brightness = procampParams.fBrightness;
    contrast   = procampParams.fContrast;
    hue        = procampParams.fHue * (3.1415926535897932f / 180.0f);
    saturation = procampParams.fSaturation;

    // procamp matrix
    //
    // [Y']   [ c            0          0  ] [Y]   [ 16  - 16 * c + b              ]
    // [U'] = [ 0   c*s*cos(h)  c*s*sin(h) ] [U] + [ 128 - 128*c*s*(cos(h)+sin(h)) ]
    // [V']   [ 0  -c*s*sin(h)  c*s*cos(h) ] [V]   [ 128 - 128*c*s*(cos(h)-sin(h)) ]

    float procampMatrix[12] = {};

    procampMatrix[0]  = contrast;
    procampMatrix[1]  = 0.0f;
    procampMatrix[2]  = 0.0f;
    procampMatrix[3]  = (16.0f - 16.0f * contrast + brightness) / 255.f;
    procampMatrix[4]  = 0.0f;
    procampMatrix[5]  = (float)cos(hue) * contrast * saturation;
    procampMatrix[6]  = (float)sin(hue) * contrast * saturation;
    procampMatrix[7]  = (128.0f * (1.0f - procampMatrix[5] - procampMatrix[6])) / 255.f;
    procampMatrix[8]  = 0.0f;
    procampMatrix[9]  = -procampMatrix[6];
    procampMatrix[10] = procampMatrix[5];
    procampMatrix[11] = (128.0f * (1.0f - procampMatrix[5] + procampMatrix[6])) / 255.f;

    // Calculate final CSC matrix [backcsc] * [pa] * [precsc]
    if (bPreCscEnabled)
    {  // Calculate [pa] * [precsc]
        KernelDll_MatrixProduct(procampMatrix, procampMatrix, preCscMatrix);
    }

    if (bBackCscEnabled)
    {  // Calculate [backcsc] * [pa]
        //        or [backcsc] * [pa] * [precsc]
        KernelDll_MatrixProduct(procampMatrix, backCscMatrix, procampMatrix);
    }

    // Use the output matrix copy into csc matrix to generate kernel CSC parameters
    MOS_SecureMemcpy(cscMatrix, sizeof(float) * 12, (void *)procampMatrix, sizeof(float) * 12);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertProcampAndCscToKrnParam(VPHAL_CSPACE srcColorSpace, VPHAL_CSPACE dstColorSpace, OCL_FC_KRN_CSC_MATRIX &csc, VPHAL_PROCAMP_PARAMS &procampParams)
{
    VP_FUNC_CALL();
    csc                 = {};
    float cscMatrix[12] = {};
    if (procampParams.bEnabled)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(GenerateProcampCscMatrix(srcColorSpace, dstColorSpace, cscMatrix, procampParams));
    }
    else
    {
        if (srcColorSpace == dstColorSpace)
        {
            csc.s0123[0] = csc.s4567[1] = csc.s89AB[2] = 1;
            return MOS_STATUS_SUCCESS;
        }

        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetNormalizedCSCMatrix(srcColorSpace, dstColorSpace, cscMatrix));
    }

    // Save finalMatrix into csc
    VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(csc.s0123, sizeof(csc.s0123), &cscMatrix[0], sizeof(float) * 3));
    VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(csc.s4567, sizeof(csc.s4567), &cscMatrix[4], sizeof(float) * 3));
    VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(csc.s89AB, sizeof(csc.s89AB), &cscMatrix[8], sizeof(float) * 3));
    csc.sCDEF[0] = cscMatrix[3];
    csc.sCDEF[1] = cscMatrix[7];
    csc.sCDEF[2] = cscMatrix[11];

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertScalingRotToKrnParam(
    RECT                         &rcSrc,
    RECT                         &rcDst,
    VPHAL_SCALING_MODE            scalingMode,
    uint32_t                      inputWidth,
    uint32_t                      inputHeight,
    VPHAL_ROTATION                rotation,
    OCL_FC_KRN_SCALE_PARAM       &scaling,
    uint8_t                      &samplerType,
    OCL_FC_KRN_COORD_SHIFT_PARAM &coordShift)

{
    VP_FUNC_CALL();
    if (scalingMode == VPHAL_SCALING_BILINEAR)
    {
        coordShift.commonShiftX = VP_HW_LINEAR_SHIFT / inputWidth;
        coordShift.commonShiftY = VP_HW_LINEAR_SHIFT / inputHeight;
        samplerType             = 1;
    }
    else if (scalingMode == VPHAL_SCALING_NEAREST)
    {
        coordShift.commonShiftX = VP_SAMPLER_BIAS / inputWidth;
        coordShift.commonShiftY = VP_SAMPLER_BIAS / inputHeight;
        samplerType             = 0;
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    scaling.trg.left   = rcDst.left;
    scaling.trg.right  = rcDst.right;
    scaling.trg.top    = rcDst.top;
    scaling.trg.bottom = rcDst.bottom;

    bool  isVerticalRotate = VpUtils::IsVerticalRotation(rotation);
    float rcDstWidth       = static_cast<float>(rcDst.right - rcDst.left);
    float rcDstHeight      = static_cast<float>(rcDst.bottom - rcDst.top);
    float rcSrcWidth       = static_cast<float>(rcSrc.right - rcSrc.left);
    float rcSrcHeight      = static_cast<float>(rcSrc.bottom - rcSrc.top);
    float strideX          = isVerticalRotate ? rcSrcWidth / rcDstHeight / inputWidth : rcSrcWidth / rcDstWidth / inputWidth;
    float strideY          = isVerticalRotate ? rcSrcHeight / rcDstWidth / inputHeight : rcSrcHeight / rcDstHeight / inputHeight;
    float startLeft        = (float)rcSrc.left / inputWidth;
    float startRight       = (float)(rcSrc.right - 1) / inputWidth;
    float startTop         = (float)rcSrc.top / inputHeight;
    float startBottom      = (float)(rcSrc.bottom - 1) / inputHeight;
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertRotationToKrnParam(rotation, strideX, strideY, startLeft, startRight, startTop, startBottom, scaling));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertChromaUpsampleToKrnParam(MOS_FORMAT format, uint32_t chromaSitingLoc, VPHAL_SCALING_MODE scalingMode, uint32_t inputWidth, uint32_t inputHeight, float &chromaShiftX, float &chromaShiftY, uint8_t &isChromaShift)
{
    uint8_t hitSecPlaneFactorX = 0;
    uint8_t hitSecPlaneFactorY = 0;
    isChromaShift              = 0;
    chromaShiftX               = 0.5f;
    chromaShiftY               = 0.5f;
    VP_PUBLIC_CHK_STATUS_RETURN(GetChromaSitingFactor(format, hitSecPlaneFactorX, hitSecPlaneFactorY));

    // If there is no DDI setting, we use the Horizontal Left Vertical Center as default for PL2 surface.
    if (chromaSitingLoc == CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            //Center Left
            isChromaShift = 1;
            chromaShiftY -= 0.5f;
        }
    }
    else
    {
        // PL2, 6 positions are available
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            // Horizontal Left
            if (chromaSitingLoc & CHROMA_SITING_HORZ_LEFT)
            {
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Left
                    isChromaShift = 1;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Left
                    isChromaShift = 1;
                    chromaShiftY -= 0.5f;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Left
                    isChromaShift = 1;
                    chromaShiftY -= 1.f;
                }
            }
            // Horizontal Center
            else if (chromaSitingLoc & CHROMA_SITING_HORZ_CENTER)
            {
                chromaShiftX -= 0.5f;
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Center
                    isChromaShift = 1;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Center
                    isChromaShift = 0;
                    chromaShiftY -= 0.5f;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Center
                    isChromaShift = 1;
                    chromaShiftY -= 1.f;
                }
            }
        }
        else if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 1)
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (chromaSitingLoc & (CHROMA_SITING_HORZ_CENTER))
            {
                //Top Center
                isChromaShift = 1;
                chromaShiftX -= 0.5f;
            }
        }
        else if (hitSecPlaneFactorX == 1 && hitSecPlaneFactorY == 2)
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (chromaSitingLoc & (CHROMA_SITING_VERT_CENTER))
            {
                //Top Center
                isChromaShift = 1;
                chromaShiftY -= 0.5f;
            }
        }
        else if (hitSecPlaneFactorX == 4 && hitSecPlaneFactorY == 1)
        {
            chromaShiftY += 1;
            isChromaShift = 1;
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (chromaSitingLoc & (CHROMA_SITING_VERT_CENTER))
            {
                //Top Center
                //common shift 0.5, inital chromaShiftX 0.5, (0.5 + 0.5 + 1 - 0.5) / 4 = 0.5(3d sampler shift) - 0.125(chroma siting shift, 0.25 is one step for 411, half of 0.125)
                chromaShiftX += 0.5f;
            }
            else
            {
                //Top Left
                chromaShiftX += 1;
            }
        }
    }

    if (scalingMode == VPHAL_SCALING_NEAREST)
    {
        isChromaShift = 0;
    }
    chromaShiftX /= inputWidth;
    chromaShiftY /= inputHeight;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertInputOutputSingleChannelIndexToKrnParam(MOS_FORMAT format, uint32_t &inputChannelIndex)
{
    VP_FUNC_CALL();
    switch (format)
    {
    case Format_YV12:
    case Format_I420:
    case Format_IMC3:
    case Format_IYUV:
    case Format_422H:
    case Format_422V:
    case Format_411P:
        inputChannelIndex = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertInputChannelIndicesToKrnParam(MOS_FORMAT format, MOS_FORMAT separateIntermediaSecPlaneFormat, uint32_t *inputChannelIndices)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_B10G10R10A2:
    case Format_A16R16G16B16:
    case Format_A16R16G16B16F:
    case Format_R5G6B5:
    case Format_R8G8B8:
    case Format_444P:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 1;
        inputChannelIndices[2] = 2;
        inputChannelIndices[3] = 3;
        break;
    case Format_RGBP:
        inputChannelIndices[0] = 2;
        inputChannelIndices[1] = 0;
        inputChannelIndices[2] = 1;
        inputChannelIndices[3] = 3;
        break;
    case Format_AYUV:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 2;
        inputChannelIndices[2] = 0;
        inputChannelIndices[3] = 3;
        break;
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_R10G10B10A2:
    case Format_A16B16G16R16:
    case Format_A16B16G16R16F:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 1;
        inputChannelIndices[2] = 2;
        inputChannelIndices[3] = 3;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_Y210:
    case Format_Y216:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 5;
        inputChannelIndices[2] = 7;
        inputChannelIndices[3] = 3;
        break;
    case Format_YVYU:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 7;
        inputChannelIndices[2] = 5;
        inputChannelIndices[3] = 3;
        break;
    case Format_UYVY:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 4;
        inputChannelIndices[2] = 6;
        inputChannelIndices[3] = 3;
        break;
    case Format_VYUY:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 6;
        inputChannelIndices[2] = 4;
        inputChannelIndices[3] = 3;
        break;
    case Format_Y410:
    case Format_Y416:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 0;
        inputChannelIndices[2] = 2;
        inputChannelIndices[3] = 3;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 4;
        inputChannelIndices[2] = 5;
        inputChannelIndices[3] = 3;
        break;
    case Format_400P:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 0;
        inputChannelIndices[2] = 0;
        inputChannelIndices[3] = 3;
        break;
    case Format_BGRP:
        inputChannelIndices[0] = 2;
        inputChannelIndices[1] = 1;
        inputChannelIndices[2] = 0;
        inputChannelIndices[3] = 3;
        break;
    case Format_YV12:
    case Format_I420:
    case Format_IMC3:
    case Format_IYUV:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 4;
        inputChannelIndices[2] = 5;
        inputChannelIndices[3] = 5;
        break;
    case Format_422H:
    case Format_422V:
    case Format_411P:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 2;
        inputChannelIndices[2] = 3;
        inputChannelIndices[3] = 3;
        break;
    case Format_R8UN:
        if (separateIntermediaSecPlaneFormat == Format_R8G8UN)
        {
            inputChannelIndices[0] = 0;
            inputChannelIndices[1] = 4;
            inputChannelIndices[2] = 5;
            inputChannelIndices[3] = 3;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertPlaneNumToKrnParam(MOS_FORMAT format, bool needSeparateIntermediaSecPlane, bool isInput, uint32_t &planeNum)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A16R16G16B16:
    case Format_R10G10B10A2:
    case Format_AYUV:
    case Format_A16R16G16B16F:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_A16B16G16R16:
    case Format_B10G10R10A2:
    case Format_A16B16G16R16F:
    case Format_Y410:
    case Format_Y416:
    case Format_400P:
    case Format_R5G6B5:
    case Format_R8G8B8:
        planeNum = 1;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        planeNum = 2;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_Y210:
    case Format_Y216:
        if (isInput)
        {
            planeNum = 2;
        }
        else
        {
            planeNum = 3;
        }
        break;
    default:
        if (needSeparateIntermediaSecPlane)
        {
            planeNum = 2;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertBlendingToKrnParam(VPHAL_BLENDING_PARAMS &blend, uint8_t &ignoreSrcPixelAlpha, uint8_t &ignoreDstPixelAlpha, float &constAlpha)
{
    switch (blend.BlendType)
    {
    case BLEND_NONE:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = 1;
        ignoreDstPixelAlpha = 1;
        break;
    case BLEND_SOURCE:
        ignoreSrcPixelAlpha = 0;
        constAlpha          = 1;
        ignoreDstPixelAlpha = 0;
        break;
    case BLEND_PARTIAL:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = 1;
        ignoreDstPixelAlpha = 0;
        break;
    case BLEND_CONSTANT:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = blend.fAlpha;
        ignoreDstPixelAlpha = 1;
        break;
    case BLEND_CONSTANT_SOURCE:
        ignoreSrcPixelAlpha = 0;
        constAlpha          = blend.fAlpha;
        ignoreDstPixelAlpha = 0;
        break;
    case BLEND_CONSTANT_PARTIAL:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = blend.fAlpha;
        ignoreDstPixelAlpha = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateTargetParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_KRN_TARGET_PARAM &targetParam)
{
    VP_FUNC_CALL();
    VP_SURFACE *targetSurf = compParam.outputLayerParam.surf;
    VP_PUBLIC_CHK_NULL_RETURN(targetSurf);
    VP_PUBLIC_CHK_NULL_RETURN(targetSurf->osSurface);
    MOS_FORMAT outputSurfOverwriteFormat = compParam.outputLayerParam.needIntermediaSurface ? compParam.outputLayerParam.intermediaFormat : targetSurf->osSurface->Format;
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(outputSurfOverwriteFormat, compParam.outputLayerParam.needSepareateIntermediaSecPlane, false, targetParam.planeNumber));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertOutputChannelIndicesToKrnParam(outputSurfOverwriteFormat, targetParam.dynamicChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertTargetRoiToKrnParam(targetSurf->rcDst, targetSurf->osSurface->dwWidth, targetSurf->osSurface->dwHeight, targetParam.targetROI));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaDownsampleToKrnParam(targetSurf->osSurface->Format, targetSurf->ChromaSiting, targetParam.chromaSitingFactor, targetParam.controlSetting.hitSecPlaneFactorX, targetParam.controlSetting.hitSecPlaneFactorY));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertColorFillToKrnParam(compParam.enableColorFill, compParam.colorFillParams, compParam.mainCSpace, targetParam.controlSetting.isColorFill, targetParam.background));
    //ConvertAlphaToKrnParam must be called after ConvertColorFillToKrnParam
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertAlphaToKrnParam(compParam.bAlphaCalculateEnable, compParam.compAlpha, targetParam.background[3], targetParam.controlSetting.alphaLayerIndex, targetParam.alpha));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertOutputChannelIndicesToKrnParam(MOS_FORMAT format, uint32_t *dynamicChannelIndices)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_B10G10R10A2:
    case Format_A16R16G16B16:
    case Format_A16R16G16B16F:
    case Format_R5G6B5:
    case Format_R8G8B8:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 2;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_AYUV:
        dynamicChannelIndices[0] = 2;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 1;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_R10G10B10A2:
    case Format_A16B16G16R16:
    case Format_A16B16G16R16F:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 2;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_RGBP:
    case Format_BGRP:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 2;
        dynamicChannelIndices[2] = 0;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_444P:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 2;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_Y410:
    case Format_Y416:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 2;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 2;
        dynamicChannelIndices[2] = 3;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_400P:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 0;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_Y210:
    case Format_Y216:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 0;
        dynamicChannelIndices[3] = 1;
        break;
    case Format_YVYU:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 1;
        dynamicChannelIndices[3] = 0;
        break;
    case Format_UYVY:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 0;
        dynamicChannelIndices[3] = 1;
        break;
    case Format_VYUY:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 1;
        dynamicChannelIndices[3] = 0;
        break;
    case Format_YV12:
    case Format_I420:
    case Format_IMC3:
    case Format_IYUV:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertTargetRoiToKrnParam(RECT &outputRcDst, uint32_t outputWidth, uint32_t outputHeight, OCL_FC_KRN_RECT &targetROI)
{
    targetROI.left   = MOS_MAX(0, outputRcDst.left);
    targetROI.right  = MOS_MIN((uint64_t)outputRcDst.right, outputWidth);
    targetROI.top    = MOS_MAX(0, outputRcDst.top);
    targetROI.bottom = MOS_MIN((uint64_t)outputRcDst.bottom, outputHeight);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertChromaDownsampleToKrnParam(MOS_FORMAT format, uint32_t chromaSitingLoc, float *chromaSitingFactor, uint8_t &hitSecPlaneFactorX, uint8_t &hitSecPlaneFactorY)
{
    VP_PUBLIC_CHK_STATUS_RETURN(GetChromaSitingFactor(format, hitSecPlaneFactorX, hitSecPlaneFactorY));

    //Left Top
    chromaSitingFactor[0] = 1;
    chromaSitingFactor[1] = 0;
    chromaSitingFactor[2] = 0;
    chromaSitingFactor[3] = 0;

    if (chromaSitingLoc == CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            //Center Left
            chromaSitingFactor[0] = 0.5;
            chromaSitingFactor[1] = 0;
            chromaSitingFactor[2] = 0.5;
            chromaSitingFactor[3] = 0;
        }
    }
    else
    {
        // PL2, 6 positions are avalibale
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            // Horizontal Left
            if (chromaSitingLoc & CHROMA_SITING_HORZ_LEFT)
            {
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Left
                    chromaSitingFactor[0] = 1;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 0;
                    chromaSitingFactor[3] = 0;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Left
                    chromaSitingFactor[0] = 0.5;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 0.5;
                    chromaSitingFactor[3] = 0;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Left
                    chromaSitingFactor[0] = 0;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 1;
                    chromaSitingFactor[3] = 0;
                }
            }
            // Horizontal Center
            else if (chromaSitingLoc & CHROMA_SITING_HORZ_CENTER)
            {
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Center
                    chromaSitingFactor[0] = 0.5;
                    chromaSitingFactor[1] = 0.5;
                    chromaSitingFactor[2] = 0;
                    chromaSitingFactor[3] = 0;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Center
                    chromaSitingFactor[0] = 0.25;
                    chromaSitingFactor[1] = 0.25;
                    chromaSitingFactor[2] = 0.25;
                    chromaSitingFactor[3] = 0.25;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Center
                    chromaSitingFactor[0] = 0;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 0.5;
                    chromaSitingFactor[3] = 0.5;
                }
            }
        }
        else if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 1)
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (chromaSitingLoc & CHROMA_SITING_HORZ_CENTER)
            {
                //Top Center
                chromaSitingFactor[0] = 0.5;
                chromaSitingFactor[1] = 0.5;
                chromaSitingFactor[2] = 0;
                chromaSitingFactor[3] = 0;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertAlphaToKrnParam(bool bAlphaCalculateEnable, VPHAL_ALPHA_PARAMS &compAlpha, float colorFillAlpha, uint8_t &alphaLayerIndex, float &alpha)
{
    switch (compAlpha.AlphaMode)
    {
    case VPHAL_ALPHA_FILL_MODE_NONE:
        alpha           = compAlpha.fAlpha;
        alphaLayerIndex = 8;
        break;
    case VPHAL_ALPHA_FILL_MODE_OPAQUE:
        alpha           = 1.f;
        alphaLayerIndex = 8;
        break;
    case VPHAL_ALPHA_FILL_MODE_BACKGROUND:
        alpha           = colorFillAlpha;
        alphaLayerIndex = 8;
        break;
    case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
        alphaLayerIndex = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    if (!bAlphaCalculateEnable)
    {
        alpha           = 0;
        alphaLayerIndex = 8;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertColorFillToKrnParam(bool enableColorFill, VPHAL_COLORFILL_PARAMS &colorFillParams, MEDIA_CSPACE dstCspace, uint8_t &isColorFill, float *background)
{
    VP_FUNC_CALL();
    isColorFill = enableColorFill;
    if (enableColorFill)
    {
        VPHAL_COLOR_SAMPLE_8 srcColor = {};
        srcColor.dwValue              = colorFillParams.Color;
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetPixelWithCSCForColorFill(srcColor, background, colorFillParams.CSpace, dstCspace));
    }

    return MOS_STATUS_SUCCESS;
}
bool VpOclFcFilter::FastExpressConditionMeet(const OCL_FC_COMP_PARAM &compParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_pvpMhwInterface &&
        m_pvpMhwInterface->m_userFeatureControl &&
        m_pvpMhwInterface->m_userFeatureControl->DisableOclFcFp())
    {
        return false;
    }
#endif
    if (compParam.layerNumber > 1)
    {
        return false;
    }
    const OCL_FC_LAYER_PARAM &inputLayer  = compParam.inputLayersParam[0];
    const OCL_FC_LAYER_PARAM &outputLayer = compParam.outputLayerParam;
    VP_SURFACE               *inputSurf   = inputLayer.surf;
    VP_SURFACE               *outputSurf  = outputLayer.surf;
    if (!outputSurf ||
        !outputSurf->osSurface)
    {
        return false;
    }

    if (compParam.layerNumber > 0)
    {
        if (!inputSurf ||
            !inputSurf->osSurface)
        {
            return false;
        }
    }
    else
    {
        if (!compParam.enableColorFill)
        {
            //no input layer w/ no color fill means no feature for fast express
            return false;
        }
    }

    bool trgFormatSupport = outputSurf->osSurface->Format == Format_NV12 ||
                            outputSurf->osSurface->Format == Format_P010 ||
                            outputSurf->osSurface->Format == Format_P016;
    bool isAligned = false;
    if (compParam.enableColorFill)
    {
        isAligned = MOS_IS_ALIGNED(MOS_MAX(0, outputSurf->rcDst.left), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)outputSurf->rcDst.right), 2) &&
                    MOS_IS_ALIGNED(MOS_MAX(0, outputSurf->rcDst.top), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)outputSurf->rcDst.bottom), 2);
    }
    else
    {
        isAligned = MOS_IS_ALIGNED(MOS_MAX(0, inputSurf->rcDst.left), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)inputSurf->rcDst.right), 2) &&
                    MOS_IS_ALIGNED(MOS_MAX(0, inputSurf->rcDst.top), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)inputSurf->rcDst.bottom), 2);
    }
    if (!trgFormatSupport ||
        !isAligned)
    {
        return false;
    }

    if (compParam.layerNumber == 1 &&
       (inputLayer.blendingParams.BlendType != BLEND_NONE ||
        inputLayer.diParams.enabled ||
        inputLayer.lumaKey.enabled))
    {
        return false;
    }

    return true;
}

MOS_STATUS VpOclFcFilter::GenerateFcFastExpressKrnParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);

    param                                  = {};
    OCL_FC_FP_KRN_IMAGE_PARAM  imageParam  = {};
    OCL_FC_FP_KRN_TARGET_PARAM targetParam = {};
    VP_RENDER_CHK_STATUS_RETURN(GenerateFastExpressInputOutputParam(compParam, imageParam, targetParam));
    PrintFastExpressKrnParam(imageParam, targetParam);

    uint32_t                     localSize[3]        = {16, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     workItemNum         = targetParam.alignedTrgRectSize.width * targetParam.alignedTrgRectSize.height;
    uint32_t                     localWorkItemNum    = localSize[0] * localSize[1];
    uint32_t                     groupNumber         = workItemNum / localWorkItemNum + (workItemNum % localWorkItemNum != 0);
    uint32_t                     threadGroupWidth    = 8;
    uint32_t                     threadGroupHeight   = groupNumber / threadGroupWidth + (groupNumber % threadGroupWidth != 0);
    uint32_t                     globalSize[3]       = {localSize[0] * threadGroupWidth, localSize[1] * threadGroupHeight, 1};
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};

    auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find("FastExpress_fc_fp");
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_fcFastExpressKrnArgs.find(uIndex);
        if (argHandle == m_fcFastExpressKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_fcFastExpressKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_fcFastExpressKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcFastExpressKrnArg(compParam.layerNumber, imageParam, targetParam, localSize, globalSize, krnArg, bInit));

        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcFastExpressBti(uIndex, compParam, surfaceParam, bInit));

        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }

    param.kernelArgs             = krnArgs;
    param.kernelName             = "FastExpress_fc_fp";
    param.kernelId               = kernelOclFcFP;
    param.threadWidth            = threadGroupWidth;
    param.threadHeight           = threadGroupHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFcFastExpressBti(uint32_t uIndex, const OCL_FC_COMP_PARAM &compParam, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    switch (uIndex)
    {
    case FC_FP_FASTEXPRESS_INPUTPL0:
        if (compParam.layerNumber > 0)
        {
            surfaceParam.surfType = compParam.inputLayersParam[0].needIntermediaSurface ? SurfaceTypeFcIntermediaInput : SurfaceTypeFcInputLayer0;
            if (compParam.inputLayersParam[0].diParams.enabled &&
                compParam.inputLayersParam[0].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_FP_FASTEXPRESS_OUTPUTPL0:
        surfaceParam.surfType        = compParam.outputLayerParam.needIntermediaSurface ? SurfaceTypeFcIntermediaOutput : SurfaceTypeFcTarget0;
        surfaceParam.isOutput        = true;
        surfaceParam.combineChannelY = true;
        break;
    case FC_FP_FASTEXPRESS_INPUTPL1:
        if (compParam.layerNumber > 0 && compParam.inputLayersParam[0].needSepareateIntermediaSecPlane)
        {
            surfaceParam.surfType = SurfaceTypeFcSeparateIntermediaInputSecPlane;
            if (compParam.inputLayersParam[0].diParams.enabled &&
                compParam.inputLayersParam[0].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeSubPlane;
        }
        break;
    case FC_FP_FASTEXPRESS_OUTPUTPL1:
        surfaceParam.surfType = SurfaceTypeSubPlane;
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::SetupSingleFcFastExpressKrnArg(uint32_t layerNum, OCL_FC_FP_KRN_IMAGE_PARAM &imageParams, OCL_FC_FP_KRN_TARGET_PARAM &targetParam, uint32_t localSize[3], uint32_t globalSize[3], KRN_ARG &krnArg, bool &bInit)
{
    switch (krnArg.uIndex)
    {
    case FC_FP_FASTEXPRESS_LAYERNUMBER:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(layerNum));
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &layerNum, sizeof(layerNum));
        break;
    case FC_FP_FASTEXPRESS_IMAGEPARAM:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams));
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams, sizeof(imageParams));
        break;
    case FC_FP_FASTEXPRESS_OUTPUTPARAM:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(targetParam));
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &targetParam, sizeof(targetParam));
        break;
    case FC_FP_FASTEXPRESS_INLINE_SAMPLER_LINEAR_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_BILINEAR;
        break;
    case FC_FP_FASTEXPRESS_INLINE_SAMPLER_NEAREST_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_NEAREST;
        break;
    case FC_FP_FASTEXPRESS_GLOBAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = globalSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = globalSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = globalSize[2];
        break;
    case FC_FP_FASTEXPRESS_ENQUEUED_LOCAL_SIZE:
    case FC_FP_FASTEXPRESS_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::GenerateFastExpressInputOutputParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_FP_KRN_IMAGE_PARAM &imageParam, OCL_FC_FP_KRN_TARGET_PARAM &targetParam)
{
    VP_FUNC_CALL();
    OCL_FC_LAYER_PARAM &inputLayer  = compParam.inputLayersParam[0];
    OCL_FC_LAYER_PARAM &outputLayer = compParam.outputLayerParam;
    VP_PUBLIC_CHK_NULL_RETURN(outputLayer.surf);
    VP_PUBLIC_CHK_NULL_RETURN(outputLayer.surf->osSurface);
    MOS_FORMAT outputSurfOverwriteFormat = outputLayer.needIntermediaSurface ? outputLayer.intermediaFormat : outputLayer.surf->osSurface->Format;

    if (compParam.layerNumber == 1)
    {
        VP_PUBLIC_CHK_NULL_RETURN(inputLayer.surf);
        VP_PUBLIC_CHK_NULL_RETURN(inputLayer.surf->osSurface);
        MOS_FORMAT inputSurfOverwriteFormat = inputLayer.needIntermediaSurface ? inputLayer.intermediaFormat : inputLayer.surf->osSurface->Format;
        uint32_t   inputWidth               = MOS_MIN(static_cast<uint32_t>(inputLayer.surf->osSurface->dwWidth), static_cast<uint32_t>(inputLayer.surf->rcSrc.right));
        uint32_t   inputHeight              = MOS_MIN(static_cast<uint32_t>(inputLayer.surf->osSurface->dwHeight), static_cast<uint32_t>(inputLayer.surf->rcSrc.bottom));

        VP_PUBLIC_CHK_STATUS_RETURN(ConvertProcampAndCscToKrnParam(inputLayer.surf->ColorSpace, compParam.mainCSpace, imageParam.csc, inputLayer.procampParams));
        VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(inputSurfOverwriteFormat, inputLayer.separateIntermediaSecPlaneFormat, imageParam.inputChannelIndices));
        VP_PUBLIC_CHK_STATUS_RETURN(ConvertScalingRotToKrnParam(inputLayer.surf->rcSrc, inputLayer.surf->rcDst, inputLayer.scalingMode, inputWidth, inputHeight, inputLayer.rotation, imageParam.scaleParam, imageParam.controlSetting.samplerType, imageParam.coordShift));
        VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaUpsampleToKrnParam(inputLayer.surf->osSurface->Format, inputLayer.surf->ChromaSiting, inputLayer.scalingMode, inputWidth, inputHeight, imageParam.coordShift.chromaShiftX, imageParam.coordShift.chromaShiftY, imageParam.controlSetting.isChromaShift));
        VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(inputSurfOverwriteFormat, inputLayer.needSepareateIntermediaSecPlane, true, imageParam.inputPlaneNum));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(ConvertAlignedTrgRectToKrnParam(inputLayer.surf, outputLayer.surf, compParam.enableColorFill, targetParam));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(outputSurfOverwriteFormat, outputLayer.needSepareateIntermediaSecPlane, false, targetParam.planeNumber));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertOutputChannelIndicesToKrnParam(outputSurfOverwriteFormat, targetParam.dynamicChannelIndices));
    targetParam.combineChannelIndices[0] = 0;
    targetParam.combineChannelIndices[1] = 1;
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertTargetRoiToKrnParam(outputLayer.surf->rcDst, outputLayer.surf->osSurface->dwWidth, outputLayer.surf->osSurface->dwHeight, targetParam.targetROI));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaDownsampleToKrnParam(outputLayer.surf->osSurface->Format, outputLayer.surf->ChromaSiting, targetParam.chromaSitingFactor, targetParam.controlSetting.hitSecPlaneFactorX, targetParam.controlSetting.hitSecPlaneFactorY));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertColorFillToKrnParam(compParam.enableColorFill, compParam.colorFillParams, compParam.mainCSpace, targetParam.controlSetting.isColorFill, targetParam.background));
    //ConvertAlphaToKrnParam must be called after ConvertColorFillToKrnParam
    uint8_t alphaLayerIndex = 0;
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertAlphaToKrnParam(compParam.bAlphaCalculateEnable, compParam.compAlpha, targetParam.background[3], alphaLayerIndex, targetParam.alpha));
    targetParam.controlSetting.alphaLayerIndex = (alphaLayerIndex != 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpOclFcFilter::ConvertAlignedTrgRectToKrnParam(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, bool enableColorFill, OCL_FC_FP_KRN_TARGET_PARAM &targetParam)
{
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);

    RECT alignedRect = {};
    if (enableColorFill)
    {
        alignedRect.left   = MOS_ALIGN_FLOOR(MOS_MAX(0, outputSurf->rcDst.left), 8);
        alignedRect.right  = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)outputSurf->rcDst.right), 8);
        alignedRect.top    = MOS_ALIGN_FLOOR(MOS_MAX(0, outputSurf->rcDst.top), 4);
        alignedRect.bottom = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)outputSurf->rcDst.bottom), 4);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
        VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
        alignedRect.left   = MOS_ALIGN_FLOOR(MOS_MAX(0, inputSurf->rcDst.left), 8);
        alignedRect.right  = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)inputSurf->rcDst.right), 8);
        alignedRect.top    = MOS_ALIGN_FLOOR(MOS_MAX(0, inputSurf->rcDst.top), 4);
        alignedRect.bottom = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)inputSurf->rcDst.bottom), 4);
    }
    targetParam.alignedTrgRectStart.x     = static_cast<uint16_t>(alignedRect.left);
    targetParam.alignedTrgRectStart.y     = static_cast<uint16_t>(alignedRect.top);
    targetParam.alignedTrgRectSize.width  = static_cast<uint16_t>(alignedRect.right - alignedRect.left) / 2;
    targetParam.alignedTrgRectSize.height = static_cast<uint16_t>(alignedRect.bottom - alignedRect.top) / 2;

    return MOS_STATUS_SUCCESS;
}

void VpOclFcFilter::PrintFastExpressKrnParam(OCL_FC_FP_KRN_IMAGE_PARAM &imageParam, OCL_FC_FP_KRN_TARGET_PARAM &targetParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: CSC %f %f %f %f", imageParam.csc.s0123[0], imageParam.csc.s0123[1], imageParam.csc.s0123[2], imageParam.csc.s0123[3]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: CSC %f %f %f %f", imageParam.csc.s4567[0], imageParam.csc.s4567[1], imageParam.csc.s4567[2], imageParam.csc.s4567[3]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: CSC %f %f %f %f", imageParam.csc.s89AB[0], imageParam.csc.s89AB[1], imageParam.csc.s89AB[2], imageParam.csc.s89AB[3]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: CSC %f %f %f %f", imageParam.csc.sCDEF[0], imageParam.csc.sCDEF[1], imageParam.csc.sCDEF[2], imageParam.csc.sCDEF[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: inputChannelIndices %u %u %u %u",
        imageParam.inputChannelIndices[0],
        imageParam.inputChannelIndices[1],
        imageParam.inputChannelIndices[2],
        imageParam.inputChannelIndices[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: Scaling Src, startX %f, StartY %f, StrideX %f, StrideY %f",
        imageParam.scaleParam.src.startX,
        imageParam.scaleParam.src.startY,
        imageParam.scaleParam.src.strideX,
        imageParam.scaleParam.src.strideY);
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: Scaling Dst Rect, left %u, right %u, top %u, bottom %u",
        imageParam.scaleParam.trg.left,
        imageParam.scaleParam.trg.right,
        imageParam.scaleParam.trg.top,
        imageParam.scaleParam.trg.bottom);
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: Rotation, indices[0] %u, indices[1] %u",
        imageParam.scaleParam.rotateIndices[0],
        imageParam.scaleParam.rotateIndices[1]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: CoordShift isChromaShift %d, CommonShiftX %f, CommonShiftY %f,  ChromaShiftX %f, ChromaShiftY %f",
        imageParam.controlSetting.isChromaShift,
        imageParam.coordShift.commonShiftX,
        imageParam.coordShift.commonShiftY,
        imageParam.coordShift.chromaShiftX,
        imageParam.coordShift.chromaShiftY);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP ImageParam: inputPlaneNum %u, SamplerType %d, ignoreSrcPixelAlpha %d, ignoreDstPixelAlpha %d",
        imageParam.inputPlaneNum,
        imageParam.controlSetting.samplerType,
        imageParam.controlSetting.ignoreSrcPixelAlpha,
        imageParam.controlSetting.ignoreDstPixelAlpha);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: dynamicChannelIndices %u %u %u %u",
        targetParam.dynamicChannelIndices[0],
        targetParam.dynamicChannelIndices[1],
        targetParam.dynamicChannelIndices[2],
        targetParam.dynamicChannelIndices[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: Combined Channel Indices %u %u",
        targetParam.combineChannelIndices[0],
        targetParam.combineChannelIndices[1]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: Target ROI Rect, left %u, right %u, top %u, bottom %u",
        targetParam.targetROI.left,
        targetParam.targetROI.right,
        targetParam.targetROI.top,
        targetParam.targetROI.bottom);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: background %f %f %f %f",
        targetParam.background[0],
        targetParam.background[1],
        targetParam.background[2],
        targetParam.background[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: chromaSiting: secPlaneFactorX %d, secPlaneFactorY %d",
        targetParam.controlSetting.hitSecPlaneFactorX,
        targetParam.controlSetting.hitSecPlaneFactorY);
    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: chromaSiting: chromaSitingFactorLeftTop %f, chromaSitingFactorRightTop %f, chromaSitingFactorLeftBottom %f, chromaSitingFactorRightBottom %f",
        targetParam.chromaSitingFactor[0],
        targetParam.chromaSitingFactor[1],
        targetParam.chromaSitingFactor[2],
        targetParam.chromaSitingFactor[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: planeNum %u, enableColorFill %d, alphaLayerIndex %d, fAlpha %f",
        targetParam.planeNumber,
        targetParam.controlSetting.isColorFill,
        targetParam.controlSetting.alphaLayerIndex,
        targetParam.alpha);

    VP_PUBLIC_NORMALMESSAGE("OCL FC FP TargeParam: Aligned RECT x %d, y %d, width %d, height %d",
        targetParam.alignedTrgRectStart.x,
        targetParam.alignedTrgRectStart.y,
        targetParam.alignedTrgRectSize.width,
        targetParam.alignedTrgRectSize.height);
#endif
}

MOS_STATUS VpOclFcFilter::SetPerfTag(OCL_FC_COMP_PARAM &compParam, bool isFastExpress, VPHAL_PERFTAG &perfTag)
{
    bool rotation = false;
    bool primary  = false;

    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        OCL_FC_LAYER_PARAM &layer = compParam.inputLayersParam[i];
        if (layer.surf && layer.surf->SurfType == SURF_IN_PRIMARY)
        {
            primary = true;
        }
        if (layer.rotation != VPHAL_ROTATION_IDENTITY)
        {
            rotation = true;
        }
    }
    if (isFastExpress)
    {
        perfTag = rotation ? VPHAL_PERFTAG(VPHAL_OCL_FC_FP_ROT) : VPHAL_PERFTAG(VPHAL_OCL_FC_FP);
    }
    else if (rotation)
    {
        perfTag = VPHAL_PERFTAG(VPHAL_OCL_FC_ROT_1LAYER + compParam.layerNumber - 1);
    }
    else if (primary)
    {
        perfTag = VPHAL_PERFTAG(VPHAL_OCL_FC_PRI_1LAYER + compParam.layerNumber - 1);
    }
    else
    {
        perfTag = VPHAL_PERFTAG(VPHAL_OCL_FC_0LAYER + compParam.layerNumber);
    }

    return MOS_STATUS_SUCCESS;
}

void VpOclFcFilter::PrintCompParam(OCL_FC_COMP_PARAM &compParam)
{
    VP_FUNC_CALL();
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("OCL FC CompParam: Layer Number %u", compParam.layerNumber);
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        PrintCompLayerParam(i, true, compParam.inputLayersParam[i]);
    }
    PrintCompLayerParam(0, false, compParam.outputLayerParam);

    VP_PUBLIC_NORMALMESSAGE("OCL FC CompParam: mainCSpace %d", compParam.mainCSpace);
    VP_PUBLIC_NORMALMESSAGE("OCL FC CompParam: enableCalculateAlpha %d, alphaMode %d, alpha %f", compParam.bAlphaCalculateEnable, compParam.compAlpha.AlphaMode, compParam.compAlpha.fAlpha);

    VPHAL_COLOR_SAMPLE_8 color = {};
    color.dwValue              = compParam.colorFillParams.Color;
    VP_PUBLIC_NORMALMESSAGE("OCL FC CompParam: enableColorFill %d, CSpace %d", compParam.enableColorFill, compParam.colorFillParams.CSpace);
    VP_PUBLIC_NORMALMESSAGE("OCL FC CompParam: colorFill R %d, G %d, B %d, A %d", color.R, color.G, color.B, color.A);
    VP_PUBLIC_NORMALMESSAGE("OCL FC CompParam: colorFill Y %d, U %d, V %d, A %d", color.Y, color.U, color.V, color.a);
    VP_PUBLIC_NORMALMESSAGE("OCL FC CompParam: colorFill YY %d, Cr %d, Cb %d, A %d", color.YY, color.Cr, color.Cb, color.Alpha);
#endif
}

void VpOclFcFilter::PrintCompLayerParam(uint32_t index, bool isInput, OCL_FC_LAYER_PARAM &layerParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: isInput %d, layerIndex %u, layerID %u, layerOriginID %u", isInput, index, layerParam.layerID, layerParam.layerIDOrigin);
    VP_SURFACE *surf = layerParam.surf;
    if (surf)
    {
        if (surf->osSurface)
        {
            VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: Format %d, Width %lu, Height %lu", surf->osSurface->Format, surf->osSurface->dwWidth, surf->osSurface->dwHeight);
        }
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: CSpace %d, ChromaSiting 0x%x", surf->ColorSpace, surf->ChromaSiting);
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: Src Rect, left %ld, right %ld, top %ld, bottom %ld", surf->rcSrc.left, surf->rcSrc.right, surf->rcSrc.top, surf->rcSrc.bottom);
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: Dst Rect, left %ld, right %ld, top %ld, bottom %ld", surf->rcDst.left, surf->rcDst.right, surf->rcDst.top, surf->rcDst.bottom);
    }

    if (isInput)
    {
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: ScalingMode %d, RotationMir %d", layerParam.scalingMode, layerParam.rotation);
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: LumaKey enable %d, low %d, high %d", layerParam.lumaKey.enabled, layerParam.lumaKey.params.LumaLow, layerParam.lumaKey.params.LumaHigh);
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: Blending type %d, alpha %f", layerParam.blendingParams.BlendType, layerParam.blendingParams.fAlpha);
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: Deinterlace enabled %d, FMD %d, SCD %d, SingleField %d, mode %d",
            layerParam.diParams.enabled,
            layerParam.diParams.params.bEnableFMD,
            layerParam.diParams.params.bSCDEnable,
            layerParam.diParams.params.bSingleField,
            layerParam.diParams.params.DIMode);
        VP_PUBLIC_NORMALMESSAGE("OCL FC CompLayerParam: Procamp enabled %d, brightness %f, contrast %f, hue %f, saturation %f",
            layerParam.procampParams.bEnabled,
            layerParam.procampParams.fBrightness,
            layerParam.procampParams.fContrast,
            layerParam.procampParams.fHue,
            layerParam.procampParams.fSaturation);
    }

    VP_PUBLIC_NORMALMESSAGE("");
#endif
}

void VpOclFcFilter::PrintKrnParam(std::vector<OCL_FC_KRN_IMAGE_PARAM> &imageParams, OCL_FC_KRN_TARGET_PARAM &targetParam)
{
    VP_FUNC_CALL();
#if (_DEBUG || _RELEASE_INTERNAL)
    for (uint32_t i = 0; i < imageParams.size(); ++i)
    {
        PrintKrnImageParam(i, imageParams.at(i));
    }
    PrintKrnTargetParam(targetParam);
#endif
}

void VpOclFcFilter::PrintKrnImageParam(uint32_t index, OCL_FC_KRN_IMAGE_PARAM &imageParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam Layer Index %d", index);
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: CSC %f %f %f %f", imageParam.csc.s0123[0], imageParam.csc.s0123[1], imageParam.csc.s0123[2], imageParam.csc.s0123[3]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: CSC %f %f %f %f", imageParam.csc.s4567[0], imageParam.csc.s4567[1], imageParam.csc.s4567[2], imageParam.csc.s4567[3]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: CSC %f %f %f %f", imageParam.csc.s89AB[0], imageParam.csc.s89AB[1], imageParam.csc.s89AB[2], imageParam.csc.s89AB[3]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: CSC %f %f %f %f", imageParam.csc.sCDEF[0], imageParam.csc.sCDEF[1], imageParam.csc.sCDEF[2], imageParam.csc.sCDEF[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: inputChannelIndices %u %u %u %u",
        imageParam.inputChannelIndices[0],
        imageParam.inputChannelIndices[1],
        imageParam.inputChannelIndices[2],
        imageParam.inputChannelIndices[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: Scaling Src, startX %f, StartY %f, StrideX %f, StrideY %f",
        imageParam.scale.src.startX,
        imageParam.scale.src.startY,
        imageParam.scale.src.strideX,
        imageParam.scale.src.strideY);
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: Scaling Dst Rect, left %u, right %u, top %u, bottom %u",
        imageParam.scale.trg.left,
        imageParam.scale.trg.right,
        imageParam.scale.trg.top,
        imageParam.scale.trg.bottom);
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: Rotation, indices[0] %u, indices[1] %u",
        imageParam.scale.rotateIndices[0],
        imageParam.scale.rotateIndices[1]);
    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: CoordShift isChromaShift %d, CommonShiftX %f, CommonShiftY %f,  ChromaShiftX %f, ChromaShiftY %f",
        imageParam.controlSetting.isChromaShift,
        imageParam.coordShift.commonShiftX,
        imageParam.coordShift.commonShiftY,
        imageParam.coordShift.chromaShiftX,
        imageParam.coordShift.chromaShiftY);

    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: LumaKey min %f, max %f", imageParam.lumaKey.low, imageParam.lumaKey.high);

    VP_PUBLIC_NORMALMESSAGE("OCL FC ImageParam: inputPlaneNum %u, SamplerType %d, ignoreSrcPixelAlpha %d, ignoreDstPixelAlpha %d, constAlpha %f",
        imageParam.inputPlaneNum,
        imageParam.controlSetting.samplerType,
        imageParam.controlSetting.ignoreSrcPixelAlpha,
        imageParam.controlSetting.ignoreDstPixelAlpha,
        imageParam.constAlphs);

    VP_PUBLIC_NORMALMESSAGE("");
#endif
}

void VpOclFcFilter::PrintKrnTargetParam(OCL_FC_KRN_TARGET_PARAM &targetParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("OCL FC TargeParam: dynamicChannelIndices %u %u %u %u",
        targetParam.dynamicChannelIndices[0],
        targetParam.dynamicChannelIndices[1],
        targetParam.dynamicChannelIndices[2],
        targetParam.dynamicChannelIndices[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC TargeParam: Target ROI Rect, left %u, right %u, top %u, bottom %u",
        targetParam.targetROI.left,
        targetParam.targetROI.right,
        targetParam.targetROI.top,
        targetParam.targetROI.bottom);

    VP_PUBLIC_NORMALMESSAGE("OCL FC TargeParam: background %f %f %f %f",
        targetParam.background[0],
        targetParam.background[1],
        targetParam.background[2],
        targetParam.background[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC TargeParam: chromaSiting: secPlaneFactorX %d, secPlaneFactorY %d",
        targetParam.controlSetting.hitSecPlaneFactorX,
        targetParam.controlSetting.hitSecPlaneFactorY);
    VP_PUBLIC_NORMALMESSAGE("OCL FC TargeParam: chromaSiting: chromaSitingFactorLeftTop %f, chromaSitingFactorRightTop %f, chromaSitingFactorLeftBottom %f, chromaSitingFactorRightBottom %f",
        targetParam.chromaSitingFactor[0],
        targetParam.chromaSitingFactor[1],
        targetParam.chromaSitingFactor[2],
        targetParam.chromaSitingFactor[3]);

    VP_PUBLIC_NORMALMESSAGE("OCL FC TargeParam: planeNum %u, enableColorFill %d, alphaLayerIndex %d, fAlpha %f",
        targetParam.planeNumber,
        targetParam.controlSetting.isColorFill,
        targetParam.controlSetting.alphaLayerIndex,
        targetParam.alpha);
#endif
}

void VpOclFcFilter::ReportDiffLog(const OCL_FC_COMP_PARAM &compParam, bool isFastExpressSupported)
{
    VP_FUNC_CALL();
#if (_DEBUG || _RELEASE_INTERNAL)
    VpFeatureReport *vpFeatureReport = m_pvpMhwInterface->m_reporting;
    if (vpFeatureReport == nullptr)
    {
        VP_PUBLIC_ASSERTMESSAGE("vpFeatureReportExt is nullptr");
        return;
    }
    uint32_t &reportLog = vpFeatureReport->GetFeatures().diffLogOclFC;
    reportLog           = 0;
    //check 422 input
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        const OCL_FC_LAYER_PARAM &layer = compParam.inputLayersParam[i];
        VP_SURFACE               *surf  = layer.surf;
        if (surf)
        {
            if (surf->osSurface)
            {
                MOS_FORMAT format = surf->osSurface->Format;
                if (layer.scalingMode == VPHAL_SCALING_BILINEAR)
                {
                    // bilinear scaling shift difference
                    reportLog |= (1llu << int(OclFcDiffReportShift::BilinearScaling));
                    if (format == Format_NV12 ||
                        format == Format_P010 ||
                        format == Format_P016)
                    {
                        //1 plane and 2 plane surface state read difference
                        reportLog |= (1llu << int(OclFcDiffReportShift::MediaSpecificSampler));
                    }
                }

                if (format == Format_400P)
                {
                    //400P has issue on legacy FC even with nearest sampler
                    reportLog |= (1llu << int(OclFcDiffReportShift::Format400PRead));
                }
            }
        }
        if (layer.rotation != VPHAL_ROTATION_IDENTITY)
        {
            //rotation shift place is different with legacy FC
            reportLog |= (1llu << int(OclFcDiffReportShift::Rotation));
        }
        if (layer.diParams.enabled || layer.procampParams.bEnabled)
        {
            //di or procamp used
            reportLog |= (1llu << int(OclFcDiffReportShift::Procamp));
        }
        if (layer.lumaKey.enabled)
        {
            //luma key cases will have difference for float(OclFC) vs int(FC)
            reportLog |= (1llu << int(OclFcDiffReportShift::LumaKey));
        }
    }

    VP_SURFACE *targetSurf = compParam.outputLayerParam.surf;
    if (targetSurf)
    {
        if (targetSurf->osSurface)
        {
            MOS_FORMAT format = targetSurf->osSurface->Format;
            if ((format == Format_Y210 ||
                    format == Format_Y216 ||
                    format == Format_YUY2 ||
                    format == Format_YUYV ||
                    format == Format_YVYU ||
                    format == Format_UYVY ||
                    format == Format_VYUY) &&
                targetSurf->ChromaSiting & CHROMA_SITING_HORZ_CENTER)
            {
                //422 packed no chromasiting on legacy FC
                reportLog |= (1llu << int(OclFcDiffReportShift::ChromasittingOn422Packed));
            }

            if ((format == Format_YV12 ||
                 format == Format_IYUV ||
                 format == Format_I420 ||
                 format == Format_IMC3) &&
                targetSurf->ChromaSiting != (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP))
            {
                //legacy didn't support 3 plane chromasiting CDS. So legacy FC will only do left top for PL3 output
                reportLog |= (1llu << int(OclFcDiffReportShift::ChromaSitingOnPL3));
            }

            if ((format == Format_A8R8G8B8 ||
                    format == Format_A8B8G8R8 ||
                    format == Format_R10G10B10A2 ||
                    format == Format_B10G10R10A2 ||
                    format == Format_Y410) &&
                compParam.compAlpha.AlphaMode == VPHAL_ALPHA_FILL_MODE_OPAQUE)
            {
                //fixed alpha not used in legacy FC
                reportLog |= (1llu << int(OclFcDiffReportShift::FixedAlpha));
            }

            if (format == Format_R5G6B5)
            {
                //legacy FC will drop (16 - 5/6/5) of LSB
                reportLog |= (1llu << int(OclFcDiffReportShift::FormatRGB565Write));
            }
        }
        if (compParam.enableColorFill &&
            IS_COLOR_SPACE_BT2020(targetSurf->ColorSpace))
        {
            //legacy didn't support color fill w/ BT2020 as target color space. It will use black or green as background in legacy case
            reportLog |= (1llu << int(OclFcDiffReportShift::BT2020ColorFill));
        }
    }
    if (isFastExpressSupported)
    {
        reportLog |= (1llu << int(OclFcDiffReportShift::FastExpress));
    }

    //actually walked into OCL FC. Always set to 1 when OCL FC Filter take effect. "OCL FC Enabled" may be 1 but not walked into OCL FC, cause it may fall back in wrapper class
    reportLog |= (1llu << int(OclFcDiffReportShift::OclFcEnabled));

    VP_PUBLIC_NORMALMESSAGE("OclFC vs FC Difference Report Log: 0x%x", reportLog);
#endif
}

void VpOclFcFilter::ReportFeatureLog(const OCL_FC_COMP_PARAM& compParam)
{
    VP_FUNC_CALL();
#if (_DEBUG || _RELEASE_INTERNAL)
    VpFeatureReport   *vpFeatureReport = m_pvpMhwInterface->m_reporting;
    if (vpFeatureReport == nullptr)
    {
        VP_PUBLIC_ASSERTMESSAGE("vpFeatureReportExt is nullptr");
        return;
    }
    OclFcFeatureReport featureLog = {};
    featureLog.layerCount         = compParam.layerNumber;
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        const OCL_FC_LAYER_PARAM &layer = compParam.inputLayersParam[i];
        if (layer.procampParams.bEnabled)
        {
            featureLog.procamp += 1;
        }
        if (layer.rotation != VPHAL_ROTATION_IDENTITY)
        {
            featureLog.rotation += 1;
        }
        if (layer.diParams.enabled)
        {
            featureLog.deinterlace += 1;
        }
        if (layer.lumaKey.enabled)
        {
            featureLog.lumaKey += 1;
        }
        if (layer.surf && (!(layer.surf->ChromaSiting & CHROMA_SITING_VERT_CENTER) || !(layer.surf->ChromaSiting & CHROMA_SITING_HORZ_CENTER)))
        {
            featureLog.chromaUpSample += 1;
        }
    }
    if (compParam.enableColorFill)
    {
        featureLog.colorFill = 1;
    }
    if (compParam.bAlphaCalculateEnable)
    {
        featureLog.alpha = 1;
    }

    vpFeatureReport->GetFeatures().featureLogOclFC = featureLog.value;
#endif
}

MOS_STATUS VpOclFcFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bRender)
    {
        // create a filter Param buffer
        if (!m_renderOclFcParams)
        {
            m_renderOclFcParams = (PRENDER_OCL_FC_PARAMS)MOS_New(RENDER_OCL_FC_PARAMS);

            if (m_renderOclFcParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("render ocl fc Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            m_renderOclFcParams->Init();
        }

        InitKrnParams(m_renderOclFcParams->fc_kernelParams, *m_executingPipe);
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for Dn");
    }
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   HwFilter OCL Fc Parameter                                          */
/****************************************************************************************************/
HwFilterParameter *HwFilterOclFcParameter::Create(HW_FILTER_OCL_FC_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterOclFcParameter *p = MOS_New(HwFilterOclFcParameter, featureType);
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            MOS_Delete(p);
            return nullptr;
        }
    }
    return p;
}

HwFilterOclFcParameter::HwFilterOclFcParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterOclFcParameter::~HwFilterOclFcParameter()
{
}

MOS_STATUS HwFilterOclFcParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterOclFcParameter::Initialize(HW_FILTER_OCL_FC_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet OCL Fc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpRenderOclFcParameter::Create(HW_FILTER_OCL_FC_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderOclFcParameter *p = dynamic_cast<VpRenderOclFcParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            VpPacketParameter *pParam = p;
            param.pPacketParamFactory->ReturnPacketParameter(pParam);
            return nullptr;
        }
    }
    return p;
}

VpRenderOclFcParameter::VpRenderOclFcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_fcFilter(pHwInterface)
{
}
VpRenderOclFcParameter::~VpRenderOclFcParameter() {}

bool VpRenderOclFcParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    VpRenderCmdPacket *renderPacket = dynamic_cast<VpRenderCmdPacket *>(pPacket);
    if (nullptr == renderPacket)
    {
        return false;
    }

    PRENDER_OCL_FC_PARAMS params = m_fcFilter.GetFcParams();
    if (nullptr == params)
    {
        return false;
    }
    return MOS_SUCCEEDED(renderPacket->SetOclFcParams(params));
}

MOS_STATUS VpRenderOclFcParameter::Initialize(HW_FILTER_OCL_FC_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.SetExecuteEngineCaps(params.executingPipe, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy FC Feature Handler                                      */
/****************************************************************************************************/

MOS_STATUS PolicyOclFcFeatureHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    FeatureType type = feature.GetFeatureType();

    if (caps.bRenderHdr)
    {
        // HDR Kernel
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        // FC
        if (FeatureTypeLumakeyOnRender == type ||
            FeatureTypeBlendingOnRender == type ||
            FeatureTypeAlphaOnRender == type ||
            FeatureTypeCscOnRender == type ||
            FeatureTypeScalingOnRender == type ||
            FeatureTypeRotMirOnRender == type ||
            FeatureTypeDiOnRender == type ||
            FeatureTypeProcampOnRender == type)
        {
            return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
        }
        else if (FeatureTypeColorFillOnRender == type)
        {
            // Only apply color fill on 1st pass.
            VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.RemoveSwFilter(&feature));
            VP_PUBLIC_CHK_STATUS_RETURN(executePipe.AddSwFilterUnordered(&feature, isInputPipe, index));
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyOclFcFeatureHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    // feature.GetFilterEngineCaps().bEnabled should be used here instead of feature.IsFeatureEnabled(caps)
    // to ensure the feature does not be enabled.
    // feature.IsFeatureEnabled(caps) being false means the feature is not being used in current workload,
    // in which case, the feature itself may be enable and need be processed in following workloads.
    if (0 == caps.bOutputPipeFeatureInuse &&
        !feature.GetFilterEngineCaps().bEnabled &&
        (feature.GetFilterEngineCaps().forceEnableForSfc ||
            feature.GetFilterEngineCaps().forceEnableForFc))
    {
        // To avoid filter being destroyed in Policy::UpdateFeaturePipe.
        feature.GetFilterEngineCaps().usedForNextPass = 1;
    }
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy FC Handler                                              */
/****************************************************************************************************/
PolicyOclFcHandler::PolicyOclFcHandler(VP_HW_CAPS &hwCaps) : PolicyFcHandler(hwCaps)
{
    m_Type = FeatureTypeFc;
}
PolicyOclFcHandler::~PolicyOclFcHandler()
{
}

HwFilterParameter *PolicyOclFcHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        HW_FILTER_OCL_FC_PARAM param = {};
        param.type                   = m_Type;
        param.pHwInterface           = pHwInterface;
        param.vpExecuteCaps          = vpExecuteCaps;
        param.pPacketParamFactory    = &m_PacketOclParamFactory;
        param.executingPipe          = &swFilterPipe;
        param.pfnCreatePacketParam   = PolicyOclFcHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterOclFcParameter *)pHwFilterParam)->Initialize(param)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterOclFcParameter::Create(param, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyOclFcHandler::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps)
{
    layerIndexes.clear();
    int32_t resLayerCount = VP_COMP_MAX_LAYERS;

    VP_PUBLIC_CHK_STATUS_RETURN(RemoveTransparentLayers(featurePipe));

    bool        skip                      = false;
    VP_SURFACE *output                    = featurePipe.GetSurface(false, 0);
    bool        bilinearInUseFor3DSampler = false;
    VP_PUBLIC_CHK_NULL_RETURN(output);

    for (uint32_t i = 0; i < featurePipe.GetSurfaceCount(true); ++i)
    {
        VPHAL_SCALING_MODE scalingMode = VPHAL_SCALING_NEAREST;
        VP_SURFACE        *input       = featurePipe.GetSurface(true, i);
        SwFilterSubPipe   *subpipe     = featurePipe.GetSwFilterSubPipe(true, i);
        VP_PUBLIC_CHK_NULL_RETURN(input);
        VP_PUBLIC_CHK_NULL_RETURN(subpipe);
        VP_PUBLIC_CHK_STATUS_RETURN(AddInputLayerForProcess(skip, layerIndexes, scalingMode, i, *input, *subpipe, *output, caps, resLayerCount));
        if (skip)
        {
            break;
        }

        if (VPHAL_SCALING_BILINEAR == scalingMode)
        {
            bilinearInUseFor3DSampler = true;
        }
    }

    // Use bilinear for layers, which is using nearest.
    if (s_forceNearestToBilinearIfBilinearExists && bilinearInUseFor3DSampler)
    {
        for (uint32_t i = 0; i < layerIndexes.size(); ++i)
        {
            SwFilterSubPipe *subpipe = featurePipe.GetSwFilterSubPipe(true, layerIndexes[i]);
            VP_PUBLIC_CHK_NULL_RETURN(subpipe);
            SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(subpipe->GetSwFilter(FeatureType::FeatureTypeScaling));
            if (scaling && VPHAL_SCALING_NEAREST == scaling->GetSwFilterParams().scalingMode)
            {
                scaling->GetSwFilterParams().scalingMode = VPHAL_SCALING_BILINEAR;
                VP_PUBLIC_NORMALMESSAGE("Scaling Info: Force nearest to bilinear for layer %d (%d)", layerIndexes[i], i);
                MT_LOG3(MT_VP_HAL_FC_SCALINGINFO, MT_NORMAL, MT_VP_HAL_FC_LAYER, layerIndexes[i], MT_VP_HAL_SCALING_MODE, VPHAL_SCALING_NEAREST, MT_VP_HAL_SCALING_MODE_FORCE, VPHAL_SCALING_BILINEAR);
            }
        }
    }

    // No procamp in target being used.
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyOclFcHandler::AddInputLayerForProcess(bool &bSkip, std::vector<int> &layerIndexes, VPHAL_SCALING_MODE &scalingMode, int index, VP_SURFACE &input, SwFilterSubPipe &pipe, VP_SURFACE &output, VP_EXECUTE_CAPS &caps, int32_t &resLayerCount)
{
    //Legacy FC has lots of limitations.
    //Procamp Luma key layer limitation, sampler limitation
    //OCL FC doesn't have these limitations
    bSkip = false;
    --resLayerCount;

    SwFilterScaling     *scaling               = dynamic_cast<SwFilterScaling *>(pipe.GetSwFilter(FeatureType::FeatureTypeScaling));
    SwFilterDeinterlace *di                    = dynamic_cast<SwFilterDeinterlace *>(pipe.GetSwFilter(FeatureType::FeatureTypeDi));
    VPHAL_SAMPLE_TYPE    sampleType            = input.SampleType;

    VP_PUBLIC_CHK_NULL_RETURN(scaling);

    scalingMode = scaling->GetSwFilterParams().scalingMode;
    //Disable AVS scaling mode
    if (VPHAL_SCALING_AVS == scalingMode)
    {
        scalingMode = VPHAL_SCALING_BILINEAR;
    }

    if (!IsInterlacedInputSupported(input))
    {
        sampleType = SAMPLE_PROGRESSIVE;
        //Disable DI
        if (di && di->IsFeatureEnabled(caps))
        {
            di->GetFilterEngineCaps().bEnabled = false;
        }
        //Disable Iscaling
        if (scaling->IsFeatureEnabled(caps) &&
            ISCALING_NONE != scaling->GetSwFilterParams().interlacedScalingType)
        {
            scaling->GetSwFilterParams().interlacedScalingType = ISCALING_NONE;
        }
    }
    VP_PUBLIC_CHK_STATUS_RETURN(Get3DSamplerScalingMode(scalingMode, pipe, layerIndexes.size(), input, output));

    if (resLayerCount < 0)
    {
        //Multipass
        bSkip = true;
        VP_PUBLIC_NORMALMESSAGE("Scaling Info: layer %d is not selected. layers %d",
            index,
            resLayerCount);
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_NORMALMESSAGE("Scaling Info: scalingMode %d is selected for layer %d", scalingMode, index);
    MT_LOG2(MT_VP_HAL_FC_SCALINGINFO, MT_NORMAL, MT_VP_HAL_FC_LAYER, index, MT_VP_HAL_SCALING_MODE, scalingMode);

    // Append source to compositing operation
    scaling->GetSwFilterParams().scalingMode = scalingMode;

    if (di)
    {
        di->GetSwFilterParams().sampleTypeInput = sampleType;
    }

    input.SampleType = sampleType;
    layerIndexes.push_back(index);

    return MOS_STATUS_SUCCESS;
}

}  // namespace vp