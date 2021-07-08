/*
* Copyright (c) 2019-2021, Intel Corporation
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
using namespace vp;

/****************************************************************************************************/
/*                                      SwFilter                                                    */
/****************************************************************************************************/

SwFilter::SwFilter(VpInterface &vpInterface, FeatureType type) : m_vpInterface(vpInterface), m_type(type)
{
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

    m_Params.input.colorSpace    = surfInput->ColorSpace;
    m_Params.output.colorSpace   = surfOutput->ColorSpace;
    m_Params.pIEFParams         = surfInput->pIEFParams;
    m_Params.formatInput        = surfInput->Format;
    m_Params.formatOutput       = surfOutput->Format;
    m_Params.input.chromaSiting  = surfInput->ChromaSiting;
    m_Params.output.chromaSiting = surfOutput->ChromaSiting;
    m_Params.pAlphaParams       = params.pCompAlpha;

    return MOS_STATUS_SUCCESS;
}

namespace vp
{
MOS_STATUS GetVeboxOutputParams(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, MOS_TILE_TYPE inputTileType, MOS_FORMAT outputFormat,
                                MOS_FORMAT &veboxOutputFormat, MOS_TILE_TYPE &veboxOutputTileType);
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
                            surfOutput->osSurface->Format, veboxOutputFormat, veboxOutputTileType);
        m_Params.input.colorSpace = surfInput->ColorSpace;
        m_Params.output.colorSpace = surfInput->ColorSpace;

        m_Params.formatInput = surfInput->osSurface->Format;
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
        m_Params.input.chromaSiting                  = surfInput->ChromaSiting;
        m_Params.output.chromaSiting                 = surfOutput->ChromaSiting;
        m_Params.pAlphaParams                        = nullptr;
        m_Params.pIEFParams                          = nullptr;

        m_noNeedUpdate = true;

        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_UNIMPLEMENTED;
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
    m_Params.pIEFParams             = nullptr;
    m_Params.formatInput            = params.input.surface->Format;
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
    m_Params.pColorFillParams       = params.pColorFillParams;
    m_Params.pCompAlpha             = params.pColorFillParams ? params.pCompAlpha : nullptr;

    if (surfInput->Rotation == VPHAL_ROTATION_IDENTITY ||
        surfInput->Rotation == VPHAL_ROTATION_180 ||
        surfInput->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        surfInput->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        m_Params.rotation.rotationNeeded    = false;
        m_Params.output.dwWidth             = surfOutput->dwWidth;
        m_Params.output.dwHeight            = surfOutput->dwHeight;
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
        RECT_ROTATE(m_Params.input.rcDst, surfInput->rcDst);
        RECT_ROTATE(m_Params.output.rcSrc, surfOutput->rcSrc);
        RECT_ROTATE(m_Params.output.rcDst, surfOutput->rcDst);
        RECT_ROTATE(m_Params.output.rcMaxSrc, surfOutput->rcMaxSrc);
    }

    m_Params.interlacedScalingType  = surfInput->InterlacedScalingType;
    m_Params.input.sampleType       = surfInput->SampleType;
    m_Params.output.sampleType      = surfOutput->SampleType;

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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterScaling::Configure(VEBOX_SFC_PARAMS &params)
{
    VP_FUNC_CALL();

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

    RECT recOutput = {0, 0, (int32_t)params.output.surface->dwWidth, (int32_t)params.output.surface->dwHeight};

    if (params.input.rotation == (MEDIA_ROTATION)VPHAL_ROTATION_IDENTITY    ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_ROTATION_180         ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_MIRROR_HORIZONTAL    ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_MIRROR_VERTICAL)
    {
        m_Params.output.dwWidth     = params.output.surface->dwWidth;
        m_Params.output.dwHeight    = params.output.surface->dwHeight;
        m_Params.input.rcDst        = params.output.rcDst;
        m_Params.output.rcSrc       = recOutput;
        m_Params.output.rcDst       = recOutput;
        m_Params.output.rcMaxSrc    = recOutput;
    }
    else
    {
        m_Params.output.dwWidth     = params.output.surface->dwHeight;
        m_Params.output.dwHeight    = params.output.surface->dwWidth;

        RECT_ROTATE(m_Params.input.rcDst, params.output.rcDst);
        RECT_ROTATE(m_Params.output.rcSrc, recOutput);
        RECT_ROTATE(m_Params.output.rcDst, recOutput);
        RECT_ROTATE(m_Params.output.rcMaxSrc, recOutput);
    }

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
        (rotMir->GetSwFilterParams().rotation == VPHAL_ROTATION_90 ||
        rotMir->GetSwFilterParams().rotation == VPHAL_ROTATION_270 ||
        rotMir->GetSwFilterParams().rotation == VPHAL_ROTATE_90_MIRROR_VERTICAL ||
        rotMir->GetSwFilterParams().rotation == VPHAL_ROTATE_90_MIRROR_HORIZONTAL))
    {
        m_Params.rotation.rotationNeeded = true;
    }
    else
    {
        m_Params.rotation.rotationNeeded = false;
    }

    // update source sample type for field to interleaved mode.
    m_Params.input.sampleType       = inputSurf->SampleType;

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

    m_Params.denoiseParams.bEnableChroma =
        m_Params.denoiseParams.bEnableChroma && m_Params.denoiseParams.bEnableLuma;

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
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterDeinterlace::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    VP_FUNC_CALL();

    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfInput->pDeinterlaceParams);

    m_Params.formatInput          = surfInput->Format;
    m_Params.formatOutput         = surfInput->Format;
    m_Params.sampleTypeInput      = surfInput->SampleType;
    m_Params.DIMode               = surfInput->pDeinterlaceParams->DIMode;            //!< DeInterlacing mode
    m_Params.bEnableFMD           = surfInput->pDeinterlaceParams->bEnableFMD;        //!< FMD
    m_Params.b60fpsDi             = !surfInput->pDeinterlaceParams->bSingleField;      //!< Used in frame Recon - if 30fps (one call per sample pair)
    m_Params.bSCDEnable           = surfInput->pDeinterlaceParams->bSCDEnable;        //!< Scene change detection
    m_Params.bHDContent           = MEDIA_IS_HDCONTENT(surfInput->dwWidth, surfInput->dwHeight);
    m_Params.bQueryVarianceEnable = false; // Feature is not supported in current filter, disable in current stage

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
        m_Params.bEnableSTE =  surfInput->pColorPipeParams->bEnableSTE;
        m_Params.dwSTEFactor = surfInput->pColorPipeParams->SteParams.dwSTEFactor;
    }
    else
    {
        m_Params.bEnableSTE = false;
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

    if (surfInput->pProcampParams)
    {
        m_Params.bEnableProcamp = surfInput->pProcampParams->bEnabled;
        m_Params.fBrightness = surfInput->pProcampParams->fBrightness;
        m_Params.fContrast = surfInput->pProcampParams->fContrast;
        m_Params.fHue = surfInput->pProcampParams->fHue;
        m_Params.fSaturation = surfInput->pProcampParams->fSaturation;
    }
    else
    {
        m_Params.bEnableProcamp = false;
        m_Params.fBrightness = 0;
        m_Params.fContrast = 0;
        m_Params.fHue = 0;
        m_Params.fSaturation = 0;
    }

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

    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
    VP_PUBLIC_CHK_NULL_RETURN(surfInput->pHDRParams);

    m_Params.formatInput  = surfInput->Format;
    m_Params.formatOutput = surfOutput->Format;

    // For H2S, it is possible that there is no HDR params for render target.
    m_Params.uiMaxContentLevelLum = surfInput->pHDRParams->MaxCLL;
    m_Params.srcColorSpace        = surfInput->ColorSpace;
    m_Params.dstColorSpace        = surfOutput->ColorSpace;
    if (surfInput->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
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
    if (m_Params.hdrMode == VPHAL_HDR_MODE_NONE)
    {
        VP_PUBLIC_ASSERTMESSAGE("HDR Mode is NONE");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
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

bool vp::SwFilterHdr::operator==(SwFilter &swFilter)
{
    VP_FUNC_CALL();

    SwFilterHdr *p = dynamic_cast<SwFilterHdr *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamHdr));
}

MOS_STATUS vp::SwFilterHdr::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe)
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
