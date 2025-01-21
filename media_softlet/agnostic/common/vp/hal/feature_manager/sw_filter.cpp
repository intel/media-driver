/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     sw_filter.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "sw_filter.h"
#include "vp_obj_factories.h"
#include "sw_filter_handle.h"
#include "vp_utils.h"
#include "vp_user_feature_control.h"
#include "vp_utils.h"

using namespace vp;

template <typename T>
inline void swap(T &a, T &b)
{
    T tmp = b;
    b     = a;
    a     = tmp;
}

/****************************************************************************************************/
/*                                      SwFilter                                                    */
/****************************************************************************************************/

SwFilter::SwFilter(VpInterface &vpInterface, FeatureType type) : m_vpInterface(vpInterface), m_type(type)
{
    if (m_EngineCaps.value != 0)
    {
        // Some complier may not work well for m_EngineCaps initialization by m_EngineCaps = {}. Force set to 0 here.
        VP_PUBLIC_NORMALMESSAGE("m_EngineCaps is not initialized correctly since complier issue, m_EngineCaps.value: %x. Force reset to 0.", m_EngineCaps.value);
        m_EngineCaps.value = 0;
    }
}

SwFilter::~SwFilter()
{
    Clean();
}

MOS_STATUS SwFilter::SetFeatureType(FeatureType type)
{
    VP_FUNC_CALL();

    if ((type & FEATURE_TYPE_MASK) != (m_type & FEATURE_TYPE_MASK))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_type = type;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilter::ResetFeatureType()
{
    VP_FUNC_CALL();

    m_type = (FeatureType)(m_type & FEATURE_TYPE_MASK);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilter::SetRenderTargetType(RenderTargetType type)
{
    VP_FUNC_CALL();

    m_renderTargetType = type;

    return MOS_STATUS_SUCCESS;
}

SwFilter* SwFilter::CreateSwFilter(FeatureType type)
{
    VP_FUNC_CALL();

    auto handle = m_vpInterface.GetSwFilterHandler(m_type);
    SwFilter* p = nullptr;
    if (handle)
    {
        p = handle->CreateSwFilter();
        if (nullptr == p)
        {
            return nullptr;
        }
        p->SetRenderTargetType(m_renderTargetType);
        p->GetFilterEngineCaps().value = 0;
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("SwFilter Handler didn't Init, return Fail");
        return nullptr;
    }

    return p;
}

void SwFilter::DestroySwFilter(SwFilter* p)
{
    VP_FUNC_CALL();

    auto handle = m_vpInterface.GetSwFilterHandler(m_type);

    if (handle)
    {
        handle->Destory(p);
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("SwFilter Handler didn't Init, return Fail");
        return;
    }
}

VP_MHWINTERFACE *SwFilter::GetHwInterface()
{
    VP_FUNC_CALL();

    return m_vpInterface.GetHwInterface();
}

/****************************************************************************************************/
/*                                      SwFilterCsc                                                 */
/****************************************************************************************************/

SwFilterCsc::SwFilterCsc(VpInterface &vpInterface) : SwFilter(vpInterface, FeatureTypeCsc)
{
    m_Params.type = m_type;
}

SwFilterCsc::~SwFilterCsc()
{
    Clean();
}

MOS_STATUS SwFilterCsc::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterCsc::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    // Parameter checking should be done in SwFilterCscHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.input.colorSpace       = surfInput->ColorSpace;
    m_Params.output.colorSpace      = surfOutput->ColorSpace;
    m_Params.pIEFParams             = surfInput->pIEFParams;
    m_Params.formatInput            = surfInput->Format;
    m_Params.formatOutput           = surfOutput->Format;
    m_Params.input.chromaSiting     = surfInput->ChromaSiting;
    m_Params.output.chromaSiting    = surfOutput->ChromaSiting;
    m_Params.input.tileMode         = surfInput->TileModeGMM;
    m_Params.output.tileMode        = surfOutput->TileModeGMM;
    m_Params.isFullRgbG10P709       = surfOutput->pGamutParams ? surfOutput->pGamutParams->GammaValue == GAMMA_1P0 : 0 && surfOutput->ColorSpace == CSpace_sRGB && IS_RGB64_FLOAT_FORMAT(surfOutput->Format);
    // Alpha should be handled in input pipe to avoid alpha data lost from image.
    m_Params.pAlphaParams           = params.pCompAlpha;
    // formatForCUS will be set on demand in Policy::GetCSCExecutionCapsBT2020ToRGB.
    m_Params.formatforCUS           = Format_None;

    VP_PUBLIC_NORMALMESSAGE("formatInput %d, formatOutput %d", m_Params.formatInput, m_Params.formatOutput);

    return MOS_STATUS_SUCCESS;
}

namespace vp
{
MOS_STATUS GetVeboxOutputParams(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, MOS_TILE_TYPE inputTileType, MOS_FORMAT outputFormat,
                                MOS_FORMAT &veboxOutputFormat, MOS_TILE_TYPE &veboxOutputTileType, VPHAL_CSPACE colorSpaceOutput);
}

MOS_STATUS SwFilterCsc::Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfInput->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
    VP_PUBLIC_CHK_NULL_RETURN(surfOutput->osSurface);

    if (caps.bSFC)
    {
        MOS_FORMAT      veboxOutputFormat   = surfInput->osSurface->Format;
        MOS_TILE_TYPE   veboxOutputTileType = surfInput->osSurface->TileType;

        GetVeboxOutputParams(caps, surfInput->osSurface->Format, surfInput->osSurface->TileType,
                            surfOutput->osSurface->Format, veboxOutputFormat, veboxOutputTileType, surfOutput->ColorSpace);
        m_Params.input.colorSpace = surfInput->ColorSpace;
        m_Params.output.colorSpace = surfInput->ColorSpace;

        m_Params.input.tileMode  = surfInput->osSurface->TileModeGMM;
        m_Params.output.tileMode = surfOutput->osSurface->TileModeGMM;
        m_Params.formatInput = surfInput->osSurface->Format;
        // formatForCUS will be set on demand in Policy::GetCSCExecutionCapsBT2020ToRGB.
        m_Params.formatforCUS = Format_None;
        m_Params.formatOutput = veboxOutputFormat;
        m_Params.input.chromaSiting = surfInput->ChromaSiting;
        m_Params.output.chromaSiting = surfOutput->ChromaSiting;

        m_Params.pAlphaParams = nullptr;
        m_Params.pIEFParams = nullptr;

        m_noNeedUpdate = true;

        return MOS_STATUS_SUCCESS;
    }
    else
    {
        // Skip CSC and only for chroma sitting purpose
        m_Params.input.colorSpace = m_Params.output.colorSpace = surfInput->ColorSpace;
        m_Params.formatInput = m_Params.formatOutput = surfInput->osSurface->Format;
        m_Params.input.tileMode                      = surfInput->osSurface->TileModeGMM;
        m_Params.output.tileMode                     = surfOutput->osSurface->TileModeGMM;
        // formatForCUS will be set on demand in Policy::GetCSCExecutionCapsBT2020ToRGB.
        m_Params.formatforCUS                        = Format_None;
        m_Params.input.chromaSiting                  = surfInput->ChromaSiting;
        m_Params.output.chromaSiting                 = surfOutput->ChromaSiting;
        m_Params.pAlphaParams                        = nullptr;
        m_Params.pIEFParams                          = nullptr;

        m_noNeedUpdate = true;

        return MOS_STATUS_SUCCESS;
    }
}

MOS_STATUS SwFilterCsc::Configure(FeatureParamCsc &params)
{
    // Skip CSC and only for chroma sitting purpose
    m_Params       = params;
    m_noNeedUpdate = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterCsc::Configure(VEBOX_SFC_PARAMS &params)
{
    VP_FUNC_CALL();

    if (m_noNeedUpdate)
    {
        return MOS_STATUS_SUCCESS;
    }
    m_Params.input.colorSpace       = params.input.colorSpace;
    m_Params.output.colorSpace      = params.output.colorSpace;
    m_Params.input.tileMode         = params.input.surface->TileModeGMM;
    m_Params.output.tileMode        = params.output.surface->TileModeGMM;
    m_Params.pIEFParams             = nullptr;
    m_Params.formatInput            = params.input.surface->Format;
    // formatForCUS will be set on demand in Policy::GetCSCExecutionCapsBT2020ToRGB.
    m_Params.formatforCUS           = Format_None;
    m_Params.formatOutput           = params.output.surface->Format;
    m_Params.input.chromaSiting     = params.input.chromaSiting;
    m_Params.output.chromaSiting    = params.output.chromaSiting;
    m_Params.pAlphaParams           = nullptr;
    return MOS_STATUS_SUCCESS;
}

FeatureParamCsc &SwFilterCsc::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterCsc::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterCsc *swFilter = dynamic_cast<SwFilterCsc *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterCsc::operator == (SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterCsc *p = dynamic_cast<SwFilterCsc *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamCsc));
}

MOS_STATUS SwFilterCsc::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    if (FeatureTypeCscOnVebox == m_type)
    {
        // BeCSC may be added for IECP/DI. No need update.
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);

    m_Params.formatInput        = inputSurf->osSurface->Format;
    m_Params.formatOutput       = outputSurf->osSurface->Format;
    m_Params.input.colorSpace   = inputSurf->ColorSpace;
    m_Params.output.colorSpace  = outputSurf->ColorSpace;
    m_Params.input.chromaSiting = inputSurf->ChromaSiting;
    m_Params.output.chromaSiting = outputSurf->ChromaSiting;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterCsc::SetFeatureType(FeatureType type)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::SetFeatureType(type));
    m_Params.type = m_type;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterScaling                                             */
/****************************************************************************************************/

SwFilterScaling::SwFilterScaling(VpInterface &vpInterface) : SwFilter(vpInterface, FeatureTypeScaling)
{
    m_Params.type = m_type;
}

SwFilterScaling::~SwFilterScaling()
{
    Clean();
}

MOS_STATUS SwFilterScaling::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterScaling::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    // Parameter checking should be done in SwFilterScalingHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.isPrimary              = params.uSrcCount == 1 || SURF_IN_PRIMARY == surfInput->SurfType;
    m_Params.scalingMode            = surfInput->ScalingMode;
    m_Params.scalingPreference      = surfInput->ScalingPreference;
    m_Params.bDirectionalScalar     = surfInput->bDirectionalScalar;
    m_Params.formatInput            = surfInput->Format;
    m_Params.input.rcSrc            = surfInput->rcSrc;

    m_Params.input.rcMaxSrc         = surfInput->rcMaxSrc;
    m_Params.input.dwWidth          = surfInput->dwWidth;
    m_Params.input.dwHeight         = surfInput->dwHeight;
    m_Params.formatOutput           = surfOutput->Format;
    m_Params.csc.colorSpaceOutput   = surfOutput->ColorSpace;
    // Will be assigned during PolicySfcColorFillHandler::UpdateFeaturePipe.
    m_Params.pColorFillParams       = nullptr;
    // Alpha should be handled in input pipe to avoid alpha data lost from image.
    m_Params.pCompAlpha             = params.pCompAlpha;

    if (!VpUtils::IsVerticalRotation(surfInput->Rotation))
    {
        m_Params.rotation.rotationNeeded    = false;
        m_Params.output.dwWidth             = surfOutput->dwWidth;
        m_Params.output.dwHeight            = surfOutput->dwHeight;
        m_Params.output.dwPitch             = surfOutput->dwPitch;
        m_Params.input.rcDst                = surfInput->rcDst;
        m_Params.output.rcSrc               = surfOutput->rcSrc;
        m_Params.output.rcDst               = surfOutput->rcDst;
        m_Params.output.rcMaxSrc            = surfOutput->rcMaxSrc;
    }
    else
    {
        m_Params.rotation.rotationNeeded    = true;
        m_Params.output.dwWidth             = surfOutput->dwHeight;
        m_Params.output.dwHeight            = surfOutput->dwWidth;
        m_Params.output.dwPitch             = surfOutput->dwPitch;
        RECT_ROTATE(m_Params.input.rcDst, surfInput->rcDst);
        RECT_ROTATE(m_Params.output.rcSrc, surfOutput->rcSrc);
        RECT_ROTATE(m_Params.output.rcDst, surfOutput->rcDst);
        RECT_ROTATE(m_Params.output.rcMaxSrc, surfOutput->rcMaxSrc);
    }

    m_Params.bTargetRectangle = !((surfOutput->rcSrc.left == 0) && (surfOutput->rcSrc.top == 0) && (surfOutput->rcSrc.bottom == surfOutput->dwHeight) && (surfOutput->rcSrc.right == surfOutput->dwWidth));
    VP_PUBLIC_NORMALMESSAGE("Target Rectangle is enabled: %d", m_Params.bTargetRectangle);

    if (surfInput->bInterlacedScaling)
    {
        m_Params.interlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
    }
    else if (surfInput->bFieldWeaving)
    {
        m_Params.interlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
    }
    else
    {
        m_Params.interlacedScalingType = surfInput->InterlacedScalingType;
    }

    m_Params.input.sampleType       = surfInput->SampleType;
    m_Params.output.sampleType      = surfOutput->SampleType;

    VP_PUBLIC_NORMALMESSAGE("interlacedScalingType %d", m_Params.interlacedScalingType);

    m_Params.input.tileMode         = surfInput->TileModeGMM;
    m_Params.output.tileMode        = surfOutput->TileModeGMM;

    VP_PUBLIC_NORMALMESSAGE("Input  Surface:  TileType %d, TileModeGMM %d", surfInput->TileType,  surfInput->TileModeGMM);
    VP_PUBLIC_NORMALMESSAGE("Output Surface:  TileType %d, TileModeGMM %d", surfOutput->TileType, surfOutput->TileModeGMM);

    // For field-to-interleaved scaling, the height of rcSrcInput is input field height,
    // the height of rcDstInput is output frame height, for scaling ratio calculation, the
    // bottom of rcDstInput need to divide 2.
    if(m_Params.interlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED)
    {
        m_Params.input.rcDst.bottom /= 2;
    }
    // For interleaved--to-field scaling, the height of rcSrcInput is input frame height,
    // the height of rcDstInput is output field height, for scaling ratio calculation, the
    // bottom of rcDstInput need to multiple 2.
    if(m_Params.interlacedScalingType == ISCALING_INTERLEAVED_TO_FIELD)
    {
        m_Params.input.rcDst.bottom *= 2;
        m_Params.output.dwHeight *= 2;
    }

    VP_PUBLIC_NORMALMESSAGE("Configure scaling parameters by VP_PIPELINE_PARAMS: intput %d x %d, output %d x %d, (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
        m_Params.input.dwWidth, m_Params.input.dwHeight, m_Params.output.dwWidth, m_Params.output.dwHeight,
        m_Params.input.rcSrc.left, m_Params.input.rcSrc.top, m_Params.input.rcSrc.right, m_Params.input.rcSrc.bottom,
        m_Params.input.rcDst.left, m_Params.input.rcDst.top, m_Params.input.rcDst.right, m_Params.input.rcDst.bottom);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterScaling::Configure(VEBOX_SFC_PARAMS &params)
{
    VP_FUNC_CALL();

    m_Params.isPrimary              = true;
    m_Params.scalingMode            = VPHAL_SCALING_AVS;
    m_Params.scalingPreference      = VPHAL_SCALING_PREFER_SFC;
    m_Params.bDirectionalScalar     = false;
    m_Params.formatInput            = params.input.surface->Format;
    m_Params.input.rcSrc            = params.input.rcSrc;
    m_Params.input.rcMaxSrc         = params.input.rcSrc;
    m_Params.input.dwWidth          = params.input.surface->dwWidth;
    m_Params.input.dwHeight         = params.input.surface->dwHeight;
    m_Params.formatOutput           = params.output.surface->Format;
    m_Params.csc.colorSpaceOutput   = params.output.colorSpace;
    m_Params.pColorFillParams       = nullptr;
    m_Params.pCompAlpha             = nullptr;

    m_Params.input.tileMode         = params.input.surface->TileModeGMM;
    m_Params.output.tileMode        = params.output.surface->TileModeGMM;

    VP_PUBLIC_NORMALMESSAGE("Input  Surface:  TileType %d, TileModeGMM %d", params.input.surface->TileType, params.input.surface->TileModeGMM);
    VP_PUBLIC_NORMALMESSAGE("Output Surface:  TileType %d, TileModeGMM %d", params.output.surface->TileType, params.output.surface->TileModeGMM);

    RECT recOutput = {0, 0, (int32_t)params.output.surface->dwWidth, (int32_t)params.output.surface->dwHeight};

    if (params.input.rotation == (MEDIA_ROTATION)VPHAL_ROTATION_IDENTITY    ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_ROTATION_180         ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_MIRROR_HORIZONTAL    ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_MIRROR_VERTICAL)
    {
        m_Params.rotation.rotationNeeded = false;
        m_Params.output.dwWidth     = params.output.surface->dwWidth;
        m_Params.output.dwHeight    = params.output.surface->dwHeight;
        m_Params.output.dwPitch     = params.output.surface->dwPitch;
        m_Params.input.rcDst        = params.output.rcDst;
        m_Params.output.rcSrc       = recOutput;
        m_Params.output.rcDst       = recOutput;
        m_Params.output.rcMaxSrc    = recOutput;
    }
    else
    {
        m_Params.rotation.rotationNeeded = true;
        m_Params.output.dwWidth     = params.output.surface->dwHeight;
        m_Params.output.dwHeight    = params.output.surface->dwWidth;
        m_Params.output.dwPitch     = params.output.surface->dwPitch;

        RECT_ROTATE(m_Params.input.rcDst, params.output.rcDst);
        RECT_ROTATE(m_Params.output.rcSrc, recOutput);
        RECT_ROTATE(m_Params.output.rcDst, recOutput);
        RECT_ROTATE(m_Params.output.rcMaxSrc, recOutput);
    }

    VP_PUBLIC_NORMALMESSAGE("Configure scaling parameters by VEBOX_SFC_PARAMS: intput %d x %d, output %d x %d, (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
        m_Params.input.dwWidth, m_Params.input.dwHeight, m_Params.output.dwWidth, m_Params.output.dwHeight,
        m_Params.input.rcSrc.left, m_Params.input.rcSrc.top, m_Params.input.rcSrc.right, m_Params.input.rcSrc.bottom,
        m_Params.input.rcDst.left, m_Params.input.rcDst.top, m_Params.input.rcDst.right, m_Params.input.rcDst.bottom);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterScaling::Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfInput->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
    VP_PUBLIC_CHK_NULL_RETURN(surfOutput->osSurface);

    m_Params.type                       = FeatureTypeScaling;
    m_Params.formatInput                = surfInput->osSurface->Format;
    m_Params.formatOutput               = surfOutput->osSurface->Format;

    m_Params.input.dwWidth              = surfInput->osSurface->dwWidth;
    m_Params.input.dwHeight             = surfInput->osSurface->dwHeight;
    m_Params.input.rcSrc                = surfInput->rcSrc;
    m_Params.input.rcDst                = surfInput->rcDst;
    m_Params.input.rcMaxSrc             = surfInput->rcMaxSrc;
    m_Params.input.sampleType           = surfInput->SampleType;

    m_Params.rotation.rotationNeeded    = false;
    m_Params.output.dwWidth             = surfOutput->osSurface->dwWidth;
    m_Params.output.dwHeight            = surfOutput->osSurface->dwHeight;
    m_Params.output.dwPitch             = surfOutput->osSurface->dwPitch;
    m_Params.output.rcSrc               = surfOutput->rcSrc;
    m_Params.output.rcDst               = surfOutput->rcDst;
    m_Params.output.rcMaxSrc            = surfOutput->rcMaxSrc;
    m_Params.output.sampleType          = surfOutput->SampleType;

    m_Params.isPrimary                  = SURF_IN_PRIMARY == surfInput->SurfType;
    m_Params.scalingMode                = VPHAL_SCALING_NEAREST;
    m_Params.scalingPreference          = VPHAL_SCALING_PREFER_SFC;

    m_Params.interlacedScalingType      = ISCALING_NONE;
    m_Params.pColorFillParams           = nullptr;
    m_Params.pCompAlpha                 = nullptr;
    m_Params.bDirectionalScalar         = false;

    m_Params.csc.colorSpaceOutput       = surfOutput->ColorSpace;
    m_Params.rotation.rotationNeeded    = false;

    m_Params.input.tileMode             = surfInput->osSurface->TileModeGMM;
    m_Params.output.tileMode            = surfOutput->osSurface->TileModeGMM;

    VP_PUBLIC_NORMALMESSAGE("Input  Surface:  TileType %d, TileModeGMM %d", surfInput->osSurface->TileType, surfInput->osSurface->TileModeGMM);
    VP_PUBLIC_NORMALMESSAGE("Output Surface:  TileType %d, TileModeGMM %d", surfOutput->osSurface->TileType, surfOutput->osSurface->TileModeGMM);

    VP_PUBLIC_NORMALMESSAGE("Configure scaling parameters by Surfaces: intput %d x %d, output %d x %d, (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
        m_Params.input.dwWidth, m_Params.input.dwHeight, m_Params.output.dwWidth, m_Params.output.dwHeight,
        m_Params.input.rcSrc.left, m_Params.input.rcSrc.top, m_Params.input.rcSrc.right, m_Params.input.rcSrc.bottom,
        m_Params.input.rcDst.left, m_Params.input.rcDst.top, m_Params.input.rcDst.right, m_Params.input.rcDst.bottom);

    return MOS_STATUS_SUCCESS;
}

FeatureParamScaling &SwFilterScaling::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterScaling::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterScaling *swFilter = dynamic_cast<SwFilterScaling *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterScaling::operator == (SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterScaling *p = dynamic_cast<SwFilterScaling *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamScaling));
}

MOS_STATUS SwFilterScaling::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    SwFilterRotMir *rotMir = dynamic_cast<SwFilterRotMir *>(pipe.GetSwFilter(FeatureTypeRotMir));

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);

    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    m_Params.csc.colorSpaceOutput   = outputSurf->ColorSpace;

    if (rotMir &&
        VpUtils::IsVerticalRotation(rotMir->GetSwFilterParams().rotation))
    {
        m_Params.rotation.rotationNeeded = true;

        // Update the rectangle on input surface. The input surface may be used as output in previous
        // step, in which case the rectangle may be different.
        // The rcDst in surface should be the one after rotation.
        inputSurf->rcSrc = m_Params.input.rcSrc;
        RECT_ROTATE(inputSurf->rcDst, m_Params.input.rcDst);
        inputSurf->rcMaxSrc = m_Params.input.rcMaxSrc;
    }
    else
    {
        // 90/270 rotation has been done in execute pipe. Update scaling parameters in swfilter.
        if (m_Params.rotation.rotationNeeded && !m_isInExePipe)
        {
            // width and height cannot be used to check whether rotation has been done,
            // as width and height may be same.
            if (m_Params.input.dwWidth != inputSurf->osSurface->dwHeight    ||
                m_Params.input.dwHeight != inputSurf->osSurface->dwWidth    ||
                m_Params.output.dwWidth != outputSurf->osSurface->dwHeight  ||
                m_Params.output.dwHeight != outputSurf->osSurface->dwWidth)
            {
                VP_PUBLIC_ASSERTMESSAGE("The Rotation not matching between input params and output params of scaling.");
                VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
            // For VE-SFC + FC case, scaling is done by sfc, but scaling filter is still needed
            // by render workload for composition position.
            VP_PUBLIC_NORMALMESSAGE("Rotation has been done in execute pipe. Update scaling parameters.");
            swap(m_Params.input.dwWidth, m_Params.input.dwHeight);
            RECT tmp = m_Params.input.rcSrc;
            RECT_ROTATE(m_Params.input.rcSrc, tmp);
            tmp = m_Params.input.rcDst;
            RECT_ROTATE(m_Params.input.rcDst, tmp);
            tmp = m_Params.input.rcMaxSrc;
            RECT_ROTATE(m_Params.input.rcMaxSrc, tmp);

            swap(m_Params.output.dwWidth, m_Params.output.dwHeight);
            tmp = m_Params.output.rcSrc;
            RECT_ROTATE(m_Params.output.rcSrc, tmp);
            tmp = m_Params.output.rcDst;
            RECT_ROTATE(m_Params.output.rcDst, tmp);
            tmp = m_Params.output.rcMaxSrc;
            RECT_ROTATE(m_Params.output.rcMaxSrc, tmp);
        }

        m_Params.rotation.rotationNeeded = false;

        // Update the rectangle on input surface. The input surface may be used as output in previous
        // step, in which case the rectangle may be different.
        inputSurf->rcSrc = m_Params.input.rcSrc;
        inputSurf->rcDst = m_Params.input.rcDst;
        inputSurf->rcMaxSrc = m_Params.input.rcMaxSrc;
    }

    // update source sample type for field to interleaved mode.
    m_Params.input.sampleType       = inputSurf->SampleType;

    VP_PUBLIC_NORMALMESSAGE("Update scaling parameters: intput %d x %d, output %d x %d, (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
        m_Params.input.dwWidth, m_Params.input.dwHeight, m_Params.output.dwWidth, m_Params.output.dwHeight,
        m_Params.input.rcSrc.left, m_Params.input.rcSrc.top, m_Params.input.rcSrc.right, m_Params.input.rcSrc.bottom,
        m_Params.input.rcDst.left, m_Params.input.rcDst.top, m_Params.input.rcDst.right, m_Params.input.rcDst.bottom);

    return MOS_STATUS_SUCCESS;
}
/****************************************************************************************************/
/*                                      SwFilter Rotation/Mirror                                    */
/****************************************************************************************************/

SwFilterRotMir::SwFilterRotMir(VpInterface &vpInterface) : SwFilter(vpInterface, FeatureTypeRotMir)
{
    m_Params.type = m_type;
}

SwFilterRotMir::~SwFilterRotMir()
{
    Clean();
}

MOS_STATUS SwFilterRotMir::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterRotMir::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    // Parameter checking should be done in SwFilterRotMirHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.rotation     = surfInput->Rotation;
    m_Params.surfInfo.tileOutput = surfOutput->TileType;
    m_Params.formatInput  = surfInput->Format;
    m_Params.formatOutput = surfOutput->Format;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterRotMir::Configure(VEBOX_SFC_PARAMS &params)
{
    VP_FUNC_CALL();

    // Parameter checking should be done in SwFilterRotMirHandler::IsFeatureEnabled.
    m_Params.rotation     = (VPHAL_ROTATION)params.input.rotation;
    m_Params.surfInfo.tileOutput = params.output.surface->TileType;
    m_Params.formatInput  = params.input.surface->Format;
    m_Params.formatOutput = params.output.surface->Format;
    return MOS_STATUS_SUCCESS;
}

FeatureParamRotMir &SwFilterRotMir::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterRotMir::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterRotMir *swFilter = dynamic_cast<SwFilterRotMir *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterRotMir::operator == (SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterRotMir *p = dynamic_cast<SwFilterRotMir *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamRotMir));
}

MOS_STATUS SwFilterRotMir::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    m_Params.surfInfo.tileOutput = outputSurf->osSurface->TileType;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterDenoise                                             */
/****************************************************************************************************/

SwFilterDenoise::SwFilterDenoise(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeDn)
{
    m_Params.type = m_type;
}

SwFilterDenoise::~SwFilterDenoise()
{
    Clean();
}

MOS_STATUS SwFilterDenoise::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterDenoise::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];

    m_Params.sampleTypeInput = surfInput->SampleType;
    m_Params.denoiseParams = *surfInput->pDenoiseParams;
    m_Params.formatInput   = surfInput->Format;
    m_Params.formatOutput  = surfInput->Format;// Denoise didn't change the original format;
    m_Params.heightInput   = surfInput->dwHeight;
    m_Params.srcBottom     = surfInput->rcSrc.bottom;

    m_Params.denoiseParams.bEnableChroma =
        m_Params.denoiseParams.bEnableChroma && m_Params.denoiseParams.bEnableLuma;
#if !EMUL
    GMM_RESOURCE_INFO* pSrcGmmResInfo    = surfInput->OsResource.pGmmResInfo;
    GMM_RESOURCE_INFO* pTargetGmmResInfo = params.pTarget[0]->OsResource.pGmmResInfo;
    VP_PUBLIC_CHK_NULL_RETURN(pSrcGmmResInfo);
    VP_PUBLIC_CHK_NULL_RETURN(pTargetGmmResInfo);

    bool inputProtected = pSrcGmmResInfo->GetSetCpSurfTag(0, 0);
    bool outputProtected = pTargetGmmResInfo->GetSetCpSurfTag(0, 0);

    if (inputProtected || outputProtected ||
       (m_vpInterface.GetHwInterface()->m_osInterface->osCpInterface &&
        m_vpInterface.GetHwInterface()->m_osInterface->osCpInterface->IsHMEnabled()))
    {
        m_Params.secureDnNeeded = true;
    }
#endif

    VP_PUBLIC_NORMALMESSAGE("denoiseLevel = %d,secureDn = %d, AutoDn = %d", m_Params.denoiseParams.NoiseLevel, m_Params.secureDnNeeded, m_Params.denoiseParams.bAutoDetect);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterDenoise::Configure(FeatureParamDenoise &params)
{
    VP_FUNC_CALL();

    m_Params.sampleTypeInput = params.sampleTypeInput;
    m_Params.denoiseParams   = params.denoiseParams;
    m_Params.formatInput     = params.formatInput;
    m_Params.formatOutput    = params.formatOutput;
    m_Params.heightInput     = params.heightInput;
    m_Params.secureDnNeeded  = params.secureDnNeeded;

    VP_PUBLIC_NORMALMESSAGE("denoiseLevel = %d,secureDn = %d, AutoDn = %d", m_Params.denoiseParams.NoiseLevel, m_Params.secureDnNeeded, m_Params.denoiseParams.bAutoDetect);
    return MOS_STATUS_SUCCESS;
}

FeatureParamDenoise& SwFilterDenoise::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterDenoise::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterDenoise *swFilter = dynamic_cast<SwFilterDenoise *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterDenoise::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterDenoise* p = dynamic_cast<SwFilterDenoise*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamDenoise));
}

MOS_STATUS SwFilterDenoise::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterDeinterlace                                         */
/****************************************************************************************************/

SwFilterDeinterlace::SwFilterDeinterlace(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeDi)
{
    m_Params.type = m_type;
}

SwFilterDeinterlace::~SwFilterDeinterlace()
{
    Clean();
}

MOS_STATUS SwFilterDeinterlace::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    MOS_ZeroMemory(&m_Params, sizeof(m_Params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterDeinterlace::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfInput->pDeinterlaceParams);

    MOS_ZeroMemory(&m_Params, sizeof(m_Params));

    m_Params.formatInput          = surfInput->Format;
    m_Params.formatOutput         = surfInput->Format;
    m_Params.sampleTypeInput      = surfInput->SampleType;
    m_Params.diParams             = surfInput->pDeinterlaceParams;
    m_Params.bHDContent           = MEDIA_IS_HDCONTENT(surfInput->dwWidth, surfInput->dwHeight);
    m_Params.bQueryVarianceEnable = false; // Feature is not supported in current filter, disable in current stage
    m_Params.heightInput          = surfInput->dwHeight;
    m_Params.rcSrc                = surfInput->rcSrc;

    return MOS_STATUS_SUCCESS;
}

FeatureParamDeinterlace& SwFilterDeinterlace::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterDeinterlace::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterDeinterlace *swFilter = dynamic_cast<SwFilterDeinterlace *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterDeinterlace::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterDeinterlace* p = dynamic_cast<SwFilterDeinterlace*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamDeinterlace));
}

MOS_STATUS vp::SwFilterDeinterlace::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterSte                                                 */
/****************************************************************************************************/

SwFilterSte::SwFilterSte(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeSte)
{
    m_Params.type = m_type;
}

SwFilterSte::~SwFilterSte()
{
    Clean();
}

MOS_STATUS SwFilterSte::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSte::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];

    m_Params.formatInput   = surfInput->Format;
    m_Params.formatOutput  = surfInput->Format; // STE didn't change the original format;

    if (surfInput->pColorPipeParams)
    {
        m_Params.bEnableSTE  =  surfInput->pColorPipeParams->bEnableSTE;
        m_Params.bEnableSTD  = surfInput->pColorPipeParams->bEnableSTD;
        m_Params.STDParam    = surfInput->pColorPipeParams->StdParams;
        m_Params.dwSTEFactor = surfInput->pColorPipeParams->SteParams.dwSTEFactor;
    }
    else
    {
        m_Params.bEnableSTE  = false;
        m_Params.bEnableSTD  = false;
        m_Params.STDParam    = {};
        m_Params.dwSTEFactor = 0;
    }

    return MOS_STATUS_SUCCESS;
}

FeatureParamSte& SwFilterSte::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter * SwFilterSte::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterSte *swFilter = dynamic_cast<SwFilterSte *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterSte::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterSte* p = dynamic_cast<SwFilterSte*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamSte));
}

MOS_STATUS vp::SwFilterSte::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterTcc                                             */
/****************************************************************************************************/

SwFilterTcc::SwFilterTcc(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeTcc)
{
    m_Params.type = m_type;
}

SwFilterTcc::~SwFilterTcc()
{
    Clean();
}

MOS_STATUS SwFilterTcc::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterTcc::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];

    m_Params.formatInput   = surfInput->Format;
    m_Params.formatOutput  = surfInput->Format;// TCC didn't change the original format;

    if (surfInput->pColorPipeParams)
    {
        m_Params.bEnableTCC = surfInput->pColorPipeParams->bEnableTCC;
        m_Params.Red = surfInput->pColorPipeParams->TccParams.Red;
        m_Params.Green = surfInput->pColorPipeParams->TccParams.Green;
        m_Params.Blue = surfInput->pColorPipeParams->TccParams.Blue;
        m_Params.Cyan = surfInput->pColorPipeParams->TccParams.Cyan;
        m_Params.Magenta = surfInput->pColorPipeParams->TccParams.Magenta;
        m_Params.Yellow = surfInput->pColorPipeParams->TccParams.Yellow;
    }
    else
    {
        m_Params.bEnableTCC = false;
        m_Params.Red = 0;
        m_Params.Green = 0;
        m_Params.Blue = 0;
        m_Params.Cyan = 0;
        m_Params.Magenta = 0;
        m_Params.Yellow = 0;
    }

    return MOS_STATUS_SUCCESS;
}

FeatureParamTcc& SwFilterTcc::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter * SwFilterTcc::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterTcc *swFilter = dynamic_cast<SwFilterTcc *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterTcc::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterTcc* p = dynamic_cast<SwFilterTcc*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamTcc));
}

MOS_STATUS vp::SwFilterTcc::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterProcamp                                             */
/****************************************************************************************************/

SwFilterProcamp::SwFilterProcamp(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeProcamp)
{
    m_Params.type = m_type;
}

SwFilterProcamp::~SwFilterProcamp()
{
    Clean();
}

MOS_STATUS SwFilterProcamp::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterProcamp::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];

    m_Params.formatInput   = surfInput->Format;
    m_Params.formatOutput  = surfInput->Format;// Procamp didn't change the original format;

    m_Params.procampParams = surfInput->pProcampParams;

    return MOS_STATUS_SUCCESS;
}

FeatureParamProcamp& SwFilterProcamp::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter * SwFilterProcamp::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterProcamp *swFilter = dynamic_cast<SwFilterProcamp *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterProcamp::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterProcamp* p = dynamic_cast<SwFilterProcamp*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamProcamp));
}

MOS_STATUS vp::SwFilterProcamp::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterHdr                                                 */
/****************************************************************************************************/
SwFilterHdr::SwFilterHdr(VpInterface &vpInterface) : SwFilter(vpInterface, FeatureTypeHdr)
{
    m_Params.type = m_type;
}

SwFilterHdr::~SwFilterHdr()
{
    Clean();
}

MOS_STATUS SwFilterHdr::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterHdr::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();
    PVPHAL_SURFACE surfInput  = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];
    MOS_STATUS     eStatus    = MOS_STATUS_SUCCESS;
    uint32_t       i          = 0;
    uint32_t     dwUpdateMask = 0;
    bool                        bSupported       = false;
    VPHAL_HDR_LUT_MODE          CurrentLUTMode      = VPHAL_HDR_LUT_MODE_NONE;
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_vpPlatformInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_reporting);

    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    auto vpPlatformInterface = m_vpInterface.GetHwInterface()->m_vpPlatformInterface;
    VpFeatureReport *vpFeatureReport  = dynamic_cast<VpFeatureReport *>(m_vpInterface.GetHwInterface()->m_reporting);
    m_Params.isOclKernelEnabled          = userFeatureControl->EnableOcl3DLut();
#if (_DEBUG || _RELEASE_INTERNAL)
    vpFeatureReport->GetFeatures().isOcl3DLut = m_Params.isOclKernelEnabled;
#endif

    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_osInterface);

    m_Params.formatInput  = surfInput->Format;
    m_Params.formatOutput = surfOutput->Format;
    m_Params.widthInput   = surfInput->dwWidth;
    m_Params.heightInput  = surfInput->dwHeight;
    if (surfInput->p3DLutParams)
    {
        m_Params.external3DLutParams = surfInput->p3DLutParams;
    }

    // For H2S, it is possible that there is no HDR params for render target.
    m_Params.uiMaxContentLevelLum = 4000;
    m_Params.srcColorSpace        = surfInput->ColorSpace;
    m_Params.dstColorSpace        = surfOutput->ColorSpace;

    if (surfInput->pHDRParams)
    {
        m_Params.uiMaxContentLevelLum = surfInput->pHDRParams->MaxCLL;
        if (surfInput->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084 ||
           (surfInput->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR && IS_RGB64_FLOAT_FORMAT(surfInput->Format))) // For FP16 HDR CSC typical usage
        {
            m_Params.hdrMode = VPHAL_HDR_MODE_TONE_MAPPING;
            if (surfOutput->pHDRParams)
            {
                m_Params.uiMaxDisplayLum = surfOutput->pHDRParams->max_display_mastering_luminance;
                if (surfOutput->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
                {
                    m_Params.hdrMode = VPHAL_HDR_MODE_H2H;
                }
            }
        }
        else if (surfInput->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR && surfOutput->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
        {
            m_Params.hdrMode = VPHAL_HDR_MODE_INVERSE_TONE_MAPPING;
        }
    }

    m_Params.pColorFillParams = params.pColorFillParams;

    if (surfInput->SurfType == SURF_IN_PRIMARY && m_Params.globalLutMode != VPHAL_HDR_LUT_MODE_3D)
    {
        CurrentLUTMode = VPHAL_HDR_LUT_MODE_2D;
    }
    else
    {
        CurrentLUTMode = VPHAL_HDR_LUT_MODE_3D;
    }

    // Neither 1D nor 3D LUT is needed in linear output case.
    if (IS_RGB64_FLOAT_FORMAT(surfOutput->Format))
    {
        CurrentLUTMode = VPHAL_HDR_LUT_MODE_NONE;
    }

    m_Params.lutMode = CurrentLUTMode;

    VP_PUBLIC_CHK_STATUS_RETURN(HdrIsInputFormatSupported(surfInput, &bSupported));

    if (!bSupported)
    {
        VP_RENDER_ASSERTMESSAGE("HDR Unsupported Source Format\n");
        return MOS_STATUS_SUCCESS;
    }

    if (surfInput && surfInput->pHDRParams)
    {
        MOS_SecureMemcpy(&m_Params.srcHDRParams, sizeof(HDR_PARAMS), (HDR_PARAMS *)surfInput->pHDRParams, sizeof(HDR_PARAMS));
    }
    else
    {
        MOS_ZeroMemory(&m_Params.srcHDRParams, sizeof(HDR_PARAMS));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(HdrIsOutputFormatSupported(surfOutput, &bSupported));

    if (!bSupported)
    {
        VP_RENDER_ASSERTMESSAGE("HDR Unsupported Target Format\n");
        return MOS_STATUS_SUCCESS;
    }

    if (surfOutput && surfOutput->pHDRParams)
    {
        MOS_SecureMemcpy(&m_Params.targetHDRParams, sizeof(HDR_PARAMS), (HDR_PARAMS *)surfOutput->pHDRParams, sizeof(HDR_PARAMS));
    }
    else
    {
        MOS_ZeroMemory(&m_Params.targetHDRParams, sizeof(HDR_PARAMS));
    }

     return MOS_STATUS_SUCCESS;
}

FeatureParamHdr &SwFilterHdr::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterHdr::Clone()
{
    VP_FUNC_CALL();

    SwFilter *p = CreateSwFilter(m_type);

    SwFilterHdr *swFilter = dynamic_cast<SwFilterHdr *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pSrcSurface
//!           [in] Pointer to source surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS SwFilterHdr::HdrIsInputFormatSupported(
    PVPHAL_SURFACE pSrcSurface,
    bool          *pbSupported)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_PUBLIC_CHK_NULL(pSrcSurface);
    VP_PUBLIC_CHK_NULL(pbSupported);

    // HDR supported formats
    if (pSrcSurface->Format == Format_A8R8G8B8 ||
        pSrcSurface->Format == Format_X8R8G8B8 ||
        pSrcSurface->Format == Format_A8B8G8R8 ||
        pSrcSurface->Format == Format_X8B8G8R8 ||
        pSrcSurface->Format == Format_R10G10B10A2 ||
        pSrcSurface->Format == Format_B10G10R10A2 ||
        pSrcSurface->Format == Format_A16B16G16R16 ||
        pSrcSurface->Format == Format_A16R16G16B16 ||
        pSrcSurface->Format == Format_A16B16G16R16F ||
        pSrcSurface->Format == Format_A16R16G16B16F ||
        pSrcSurface->Format == Format_P016 ||
        pSrcSurface->Format == Format_NV12 ||
        pSrcSurface->Format == Format_P010 ||
        pSrcSurface->Format == Format_YUY2 ||
        pSrcSurface->Format == Format_AYUV ||
        pSrcSurface->Format == Format_Y410 ||
        pSrcSurface->Format == Format_Y416 ||
        pSrcSurface->Format == Format_Y210 ||
        pSrcSurface->Format == Format_Y216)
    {
        *pbSupported = true;
        goto finish;
    }
    else
    {
        VP_RENDER_ASSERTMESSAGE(
            "HDR Unsupported Source Format: '0x%08x'\n",
            pSrcSurface->Format);
        *pbSupported = false;
    }

finish:
    return eStatus;
}

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pTargetSurface
//!           [in] Pointer to target surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS SwFilterHdr::HdrIsOutputFormatSupported(
    PVPHAL_SURFACE pTargetSurface,
    bool          *pbSupported)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_PUBLIC_CHK_NULL(pTargetSurface);
    VP_PUBLIC_CHK_NULL(pbSupported);

    // HDR supported formats
    if (pTargetSurface->Format == Format_A8R8G8B8 ||
        pTargetSurface->Format == Format_X8R8G8B8 ||
        pTargetSurface->Format == Format_A8B8G8R8 ||
        pTargetSurface->Format == Format_X8B8G8R8 ||
        pTargetSurface->Format == Format_R10G10B10A2 ||
        pTargetSurface->Format == Format_B10G10R10A2 ||
        pTargetSurface->Format == Format_A16B16G16R16 ||
        pTargetSurface->Format == Format_A16R16G16B16 ||
        pTargetSurface->Format == Format_YUY2 ||
        pTargetSurface->Format == Format_P016 ||
        pTargetSurface->Format == Format_NV12 ||
        pTargetSurface->Format == Format_P010 ||
        pTargetSurface->Format == Format_P016 ||
        pTargetSurface->Format == Format_A16R16G16B16F ||
        pTargetSurface->Format == Format_A16B16G16R16F)
    {
        *pbSupported = true;
        goto finish;
    }
    else
    {
        VP_RENDER_ASSERTMESSAGE(
            "HDR Unsupported Target Format: '0x%08x'\n",
            pTargetSurface->Format);
        *pbSupported = false;
    }

finish:
    return eStatus;
}

bool vp::SwFilterHdr::operator==(SwFilter &swFilter)
{
    VP_FUNC_CALL();

    SwFilterHdr *p = dynamic_cast<SwFilterHdr *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamHdr));
}

MOS_STATUS vp::SwFilterHdr::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    if (m_Params.stage == HDR_STAGE_VEBOX_3DLUT_UPDATE)
    {
        VP_PUBLIC_NORMALMESSAGE("HDR 3DLUT Kernel path already update format, skip further update.");
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput  = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                SwFilterLumakey                                                   */
/****************************************************************************************************/

SwFilterLumakey::SwFilterLumakey(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeLumakey)
{
    m_Params.type = m_type;
}

SwFilterLumakey::~SwFilterLumakey()
{
    Clean();
}

MOS_STATUS SwFilterLumakey::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterLumakey::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    if (!isInputSurf ||
        nullptr == params.pSrc[surfIndex]->pLumaKeyParams)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    auto surfInput = params.pSrc[surfIndex];

    m_Params.formatInput    = surfInput->Format;
    m_Params.formatOutput   = surfInput->Format;
    m_Params.lumaKeyParams  = surfInput->pLumaKeyParams;

    return MOS_STATUS_SUCCESS;
}

FeatureParamLumakey& SwFilterLumakey::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterLumakey::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterLumakey *swFilter = dynamic_cast<SwFilterLumakey *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterLumakey::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterLumakey* p = dynamic_cast<SwFilterLumakey*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamLumakey));
}

MOS_STATUS vp::SwFilterLumakey::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = inputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                SwFilterBlending                                                  */
/****************************************************************************************************/

SwFilterBlending::SwFilterBlending(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeBlending)
{
    m_Params.type = m_type;
}

SwFilterBlending::~SwFilterBlending()
{
    Clean();
}

MOS_STATUS SwFilterBlending::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterBlending::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    if (!isInputSurf ||
        nullptr == params.pSrc[surfIndex]->pBlendingParams)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    auto surfInput = params.pSrc[surfIndex];

    m_Params.formatInput    = surfInput->Format;
    m_Params.formatOutput   = surfInput->Format;
    m_Params.blendingParams = surfInput->pBlendingParams;

    //Skip Blend PARTIAL for alpha input non alpha output
    if (m_Params.blendingParams && m_Params.blendingParams->BlendType == BLEND_PARTIAL)
    {
        auto surfOutput = params.pTarget[0];
        if (surfOutput)
        {
            if (IS_ALPHA_FORMAT(m_Params.formatInput) &&
                !IS_ALPHA_FORMAT(surfOutput->Format))
            {
                VP_PUBLIC_NORMALMESSAGE("Force to use Blend Source instead of Blend Partial");
                m_Params.blendingParams->BlendType = BLEND_SOURCE;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

FeatureParamBlending& SwFilterBlending::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterBlending::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterBlending *swFilter = dynamic_cast<SwFilterBlending *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterBlending::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterBlending* p = dynamic_cast<SwFilterBlending*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamBlending));
}

MOS_STATUS vp::SwFilterBlending::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = inputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterColorFill                                           */
/****************************************************************************************************/

SwFilterColorFill::SwFilterColorFill(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeColorFill)
{
    m_Params.type = m_type;
}

SwFilterColorFill::~SwFilterColorFill()
{
    Clean();
}

MOS_STATUS SwFilterColorFill::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterColorFill::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    if (isInputSurf ||
        nullptr == params.pColorFillParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto surfOutput = params.pTarget[0];

    m_Params.formatInput    = surfOutput->Format;
    m_Params.formatOutput   = surfOutput->Format;
    m_Params.colorFillParams = params.pColorFillParams;

    if (m_Params.colorFillParams)
    {
        VP_PUBLIC_NORMALMESSAGE("Color 0x%x, CSpace %d", m_Params.colorFillParams->Color, m_Params.colorFillParams->CSpace);
    }
    else
    {
        VP_PUBLIC_NORMALMESSAGE("nullptr == m_Params.colorFillParams");
    }

    return MOS_STATUS_SUCCESS;
}

FeatureParamColorFill& SwFilterColorFill::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterColorFill::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterColorFill *swFilter = dynamic_cast<SwFilterColorFill *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterColorFill::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterColorFill* p = dynamic_cast<SwFilterColorFill*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamColorFill));
}

MOS_STATUS vp::SwFilterColorFill::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    if (nullptr == inputSurf)
    {
        VP_PUBLIC_NORMALMESSAGE("ColorFill does not have input surface!");
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    }
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = outputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

VP_EngineEntry SwFilterColorFill::GetCombinedFilterEngineCaps(SwFilterSubPipe *inputPipeSelected)
{
    if (nullptr == inputPipeSelected)
    {
        return m_EngineCaps;
    }
    else
    {
        VP_EngineEntry engineCaps = m_EngineCaps;

        SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(inputPipeSelected->GetSwFilter(FeatureTypeScaling));

        if (nullptr == scaling)
        {
            // return default one for no scaling filter case.
            VP_PUBLIC_ASSERTMESSAGE("No scaling filter exists");
            return engineCaps;
        }

        FeatureParamScaling &scalingParams = scaling->GetSwFilterParams();
        VP_EngineEntry &scalingCaps = scaling->GetFilterEngineCaps();

        bool isColorFill = (m_Params.colorFillParams &&
                    (!m_Params.colorFillParams->bDisableColorfillinSFC) &&
                    (m_Params.colorFillParams->bOnePixelBiasinSFC ? 
                    (!RECT1_CONTAINS_RECT2_ONEPIXELBIAS(scalingParams.input.rcDst, scalingParams.output.rcDst)):
                    (!RECT1_CONTAINS_RECT2(scalingParams.input.rcDst, scalingParams.output.rcDst))))
                    ? true
                    : false;

        if (!scalingCaps.SfcNeeded || !isColorFill && (engineCaps.VeboxNeeded || engineCaps.SfcNeeded))
        {
            engineCaps.VeboxNeeded = 0;
            engineCaps.SfcNeeded = 0;
            engineCaps.bypassIfVeboxSfcInUse = 1;
            VP_PUBLIC_NORMALMESSAGE("engineCaps updated. value 0x%x (bEnabled %d, VeboxNeeded %d, SfcNeeded %d, RenderNeeded %d, fcSupported %d, isolated %d)",
                engineCaps.value, engineCaps.bEnabled, engineCaps.VeboxNeeded, engineCaps.SfcNeeded,
                engineCaps.RenderNeeded, engineCaps.fcSupported, engineCaps.isolated);
        }

        return engineCaps;
    }
}

/****************************************************************************************************/
/*                                        SwFilterAlpha                                             */
/****************************************************************************************************/

SwFilterAlpha::SwFilterAlpha(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeAlpha)
{
    m_Params.type = m_type;
}

SwFilterAlpha::~SwFilterAlpha()
{
    Clean();
}

MOS_STATUS SwFilterAlpha::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterAlpha::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    if (isInputSurf ||
        nullptr == params.pCompAlpha)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto surfOutput = params.pTarget[0];

    m_Params.formatInput    = surfOutput->Format;
    m_Params.formatOutput   = surfOutput->Format;
    m_Params.compAlpha      = params.pCompAlpha;
    m_Params.calculatingAlpha = params.bCalculatingAlpha;

    if (m_Params.compAlpha)
    {
        VP_PUBLIC_NORMALMESSAGE("AlphaMode %d, fAlpha %f", m_Params.compAlpha->AlphaMode, m_Params.compAlpha->fAlpha);
    }
    else
    {
        VP_PUBLIC_NORMALMESSAGE("nullptr == m_Params.compAlpha");
    }

    return MOS_STATUS_SUCCESS;
}

FeatureParamAlpha& SwFilterAlpha::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterAlpha::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterAlpha *swFilter = dynamic_cast<SwFilterAlpha *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool vp::SwFilterAlpha::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterAlpha* p = dynamic_cast<SwFilterAlpha*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamAlpha));
}

MOS_STATUS vp::SwFilterAlpha::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    if (nullptr == inputSurf)
    {
        VP_PUBLIC_NORMALMESSAGE("Alpha does not have input surface!");
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    }
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = outputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterCgc                                                 */
/****************************************************************************************************/

SwFilterCgc::SwFilterCgc(VpInterface& vpInterface) : SwFilter(vpInterface, FeatureTypeCgc)
{
    m_Params.type = m_type;
}

SwFilterCgc::~SwFilterCgc()
{
    Clean();
}

MOS_STATUS SwFilterCgc::Clean()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterCgc::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = static_cast<PVPHAL_SURFACE>(isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0]);
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];
    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfOutput);

    m_Params.formatInput   = surfInput->Format;
    m_Params.formatOutput  = surfInput->Format; // CGC didn't change the original format;

    if (IsBt2020ToRGB(params, isInputSurf, surfIndex))
    {
        m_Params.formatOutput     = Format_A8B8G8R8;
        m_Params.bBt2020ToRGB     = true;
        m_Params.colorSpace       = surfInput->ColorSpace;
        m_Params.GCompMode        = GAMUT_MODE_NONE;
        m_Params.bExtendedSrcGamut = false;
        m_Params.bExtendedDstGamut = false;
        m_Params.dwAttenuation = 0;
        MOS_ZeroMemory(m_Params.displayRGBW_x, sizeof(m_Params.displayRGBW_x));
        MOS_ZeroMemory(m_Params.displayRGBW_y, sizeof(m_Params.displayRGBW_y));
    }
    else
    {
        m_Params.GCompMode = GAMUT_MODE_NONE;
        m_Params.colorSpace = CSpace_Any;
        m_Params.bExtendedSrcGamut = false;
        m_Params.bExtendedDstGamut = false;
        m_Params.dwAttenuation = 0;
        MOS_ZeroMemory(m_Params.displayRGBW_x, sizeof(m_Params.displayRGBW_x));
        MOS_ZeroMemory(m_Params.displayRGBW_y, sizeof(m_Params.displayRGBW_y));
    }

    return MOS_STATUS_SUCCESS;
}

FeatureParamCgc& SwFilterCgc::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter * SwFilterCgc::Clone()
{
    VP_FUNC_CALL();

    SwFilter* p = CreateSwFilter(m_type);

    SwFilterCgc *swFilter = dynamic_cast<SwFilterCgc *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterCgc::operator==(SwFilter& swFilter)
{
    VP_FUNC_CALL();

    SwFilterCgc* p = dynamic_cast<SwFilterCgc*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamCgc));
}

MOS_STATUS SwFilterCgc::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

bool SwFilterCgc::IsBt2020ToRGB(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = static_cast<PVPHAL_SURFACE>(isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0]);
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    if (surfInput && surfOutput &&
        IS_COLOR_SPACE_BT2020_YUV(surfInput->ColorSpace))
    {
        if ((surfOutput->ColorSpace == CSpace_BT601) ||
            (surfOutput->ColorSpace == CSpace_BT709) ||
            (surfOutput->ColorSpace == CSpace_BT601_FullRange) ||
            (surfOutput->ColorSpace == CSpace_BT709_FullRange) ||
            (surfOutput->ColorSpace == CSpace_stRGB) ||
            (surfOutput->ColorSpace == CSpace_sRGB))
        {
            return true;
        }
    }

    return false;
}

/****************************************************************************************************/
/*                                      SwFilterAiBase                                              */
/****************************************************************************************************/

SwFilterAiBase::SwFilterAiBase(VpInterface &vpInterface, FeatureType featureType) : SwFilter(vpInterface, featureType)
{
    m_Params.type = m_type;
}

SwFilterAiBase::~SwFilterAiBase()
{
    Clean();
}

MOS_STATUS SwFilterAiBase::Clean()
{
    VP_FUNC_CALL();

    m_Params.kernelSettings.clear();
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterAiBase::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    m_Params.kernelSettings.clear();
    VP_PUBLIC_CHK_STATUS_RETURN(RegisterAiSettingPipe(m_Params.kernelSettings, m_Params.kernelSplitGroupIndex));

    return MOS_STATUS_SUCCESS;
}

FeatureParamAi &SwFilterAiBase::GetSwFilterParams()
{
    VP_FUNC_CALL();

    return m_Params;
}

SwFilter *SwFilterAiBase::Clone()
{
    VP_FUNC_CALL();

    SwFilter *p = CreateSwFilter(m_type);

    SwFilterAiBase *swFilter = dynamic_cast<SwFilterAiBase *>(p);
    if (nullptr == swFilter)
    {
        DestroySwFilter(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterAiBase::operator==(SwFilter &swFilter)
{
    VP_FUNC_CALL();

    SwFilterAiBase *p = dynamic_cast<SwFilterAiBase *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamAi));
}

MOS_STATUS SwFilterAiBase::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput  = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterSet                                                 */
/****************************************************************************************************/

SwFilterSet::SwFilterSet()
{}
SwFilterSet::~SwFilterSet()
{
    Clean();
}

MOS_STATUS SwFilterSet::AddSwFilter(SwFilter *swFilter)
{
    VP_FUNC_CALL();

    auto it = m_swFilters.find(swFilter->GetFeatureType());
    if (m_swFilters.end() != it)
    {
        VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! SwFilter for feature %d has already been exists in swFilterSet!", swFilter->GetFeatureType());
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_swFilters.insert(std::make_pair(swFilter->GetFeatureType(), swFilter));
    swFilter->SetLocation(this);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSet::RemoveSwFilter(SwFilter *swFilter)
{
    VP_FUNC_CALL();

    auto it = m_swFilters.find(swFilter->GetFeatureType());
    if (m_swFilters.end() == it)
    {
        // The feature does not exist in current swFilterSet.
        return MOS_STATUS_SUCCESS;
    }

    if (it->second != swFilter)
    {
        // swFilter does not belong to current swFilterSet.
        return MOS_STATUS_SUCCESS;
    }

    m_swFilters.erase(it);
    swFilter->SetLocation(nullptr);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSet::Clean()
{
    VP_FUNC_CALL();

    while (!m_swFilters.empty())
    {
        auto it = m_swFilters.begin();
        auto swFilter = (*it).second;
        m_swFilters.erase(it);
        if (swFilter)
        {
            VpInterface &vpIntf = swFilter->GetVpInterface();
            SwFilterFeatureHandler *swFilterHandler = vpIntf.GetSwFilterHandler(swFilter->GetFeatureType());
            VP_PUBLIC_CHK_NULL_RETURN(swFilterHandler);
            swFilterHandler->Destory(swFilter);
        }
    }
    return MOS_STATUS_SUCCESS;
}

SwFilter *SwFilterSet::GetSwFilter(FeatureType type)
{
    VP_FUNC_CALL();

    auto it = m_swFilters.find(type);
    if (m_swFilters.end() == it)
    {
        // The feature does not exist in current swFilterSet.
        return nullptr;
    }

    return it->second;
}

std::vector<SwFilterSet *> *SwFilterSet::GetLocation()
{
    VP_FUNC_CALL();

    return m_location;
}
void SwFilterSet::SetLocation(std::vector<SwFilterSet *> *location)
{
    VP_FUNC_CALL();

    m_location = location;
}

MOS_STATUS SwFilterSet::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe)
{
    VP_FUNC_CALL();

    for (auto swFilter : m_swFilters)
    {
        VP_PUBLIC_CHK_NULL_RETURN(swFilter.second);
        VP_PUBLIC_CHK_STATUS_RETURN(swFilter.second->Update(inputSurf, outputSurf, pipe));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSet::AddFeatureGraphRTLog()
{
    VP_FUNC_CALL();

    for (auto swFilter : m_swFilters)
    {
        if (swFilter.second)
        {
            swFilter.second->AddFeatureGraphRTLog();
        }
    }
    return MOS_STATUS_SUCCESS;
}

RenderTargetType SwFilterSet::GetRenderTargetType()
{
    VP_FUNC_CALL();

    for (auto swFilter : m_swFilters)
    {
        if (swFilter.second)
        {
            RenderTargetType renderTargetType = swFilter.second->GetRenderTargetType();
            if (renderTargetType == RenderTargetTypeSurface)
            {
                return RenderTargetTypeSurface;
            }
        }
    }
    return RenderTargetTypeParameter;
}

MOS_STATUS SwFilterSet::GetAiSwFilter(SwFilterAiBase*& swAiFilter)
{
    swAiFilter = nullptr;
    for (auto& handle : m_swFilters)
    {
        SwFilterAiBase *filter = dynamic_cast<SwFilterAiBase *>(handle.second);
        if (filter)
        {
            if (swAiFilter)
            {
                VP_PUBLIC_ASSERTMESSAGE("Only one AI Sw Filter is allowed in one SwFilterSet. More than one is found. Feature Types: %d and %d", swAiFilter->GetFeatureType(), filter->GetFeatureType());
                VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
            swAiFilter = filter;
        }
    }
    return MOS_STATUS_SUCCESS;
}