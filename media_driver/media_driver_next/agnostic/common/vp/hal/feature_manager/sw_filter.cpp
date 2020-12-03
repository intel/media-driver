/*
* Copyright (c) 2019-2020, Intel Corporation
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
    if ((type & FEATURE_TYPE_MASK) != (m_type & FEATURE_TYPE_MASK))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_type = type;

    return MOS_STATUS_SUCCESS;
}

SwFilter* SwFilter::CreateSwFilter(FeatureType type)
{
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterCsc::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    // Parameter checking should be done in SwFilterCscHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.colorSpaceInput    = surfInput->ColorSpace;
    m_Params.colorSpaceOutput   = surfOutput->ColorSpace;
    m_Params.pIEFParams         = surfInput->pIEFParams;
    m_Params.formatInput        = surfInput->Format;
    m_Params.formatOutput       = surfOutput->Format;
    m_Params.chromaSitingInput  = surfInput->ChromaSiting;
    m_Params.chromaSitingOutput = surfOutput->ChromaSiting;
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
        m_Params.colorSpaceInput = surfInput->ColorSpace;
        m_Params.colorSpaceOutput = surfInput->ColorSpace;

        m_Params.formatInput = surfInput->osSurface->Format;
        m_Params.formatOutput = veboxOutputFormat;
        m_Params.chromaSitingInput = surfInput->ChromaSiting;
        m_Params.chromaSitingOutput = surfOutput->ChromaSiting;

        m_Params.pAlphaParams = nullptr;
        m_Params.pIEFParams = nullptr;

        m_noNeedUpdate = true;

        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS SwFilterCsc::Configure(VEBOX_SFC_PARAMS &params)
{
    if (m_noNeedUpdate)
    {
        return MOS_STATUS_SUCCESS;
    }
    m_Params.colorSpaceInput    = params.input.colorSpace;
    m_Params.colorSpaceOutput   = params.output.colorSpace;
    m_Params.pIEFParams         = nullptr;
    m_Params.formatInput        = params.input.surface->Format;
    m_Params.formatOutput       = params.output.surface->Format;
    m_Params.chromaSitingInput  = params.input.chromaSiting;
    m_Params.chromaSitingOutput = params.output.chromaSiting;
    m_Params.pAlphaParams       = nullptr;
    return MOS_STATUS_SUCCESS;
}

FeatureParamCsc &SwFilterCsc::GetSwFilterParams()
{
    return m_Params;
}

SwFilter *SwFilterCsc::Clone()
{
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
    SwFilterCsc *p = dynamic_cast<SwFilterCsc *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamCsc));
}

MOS_STATUS SwFilterCsc::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    if (FeatureTypeCscOnVebox == m_type)
    {
        // BeCSC may be added for IECP/DI. No need update.
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);

    m_Params.colorSpaceInput    = inputSurf->ColorSpace;
    m_Params.colorSpaceOutput   = outputSurf->ColorSpace;
    m_Params.formatInput        = inputSurf->osSurface->Format;
    m_Params.formatOutput       = outputSurf->osSurface->Format;
    m_Params.chromaSitingInput  = inputSurf->ChromaSiting;
    m_Params.chromaSitingOutput = outputSurf->ChromaSiting;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterCsc::SetFeatureType(FeatureType type)
{
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterScaling::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    // Parameter checking should be done in SwFilterScalingHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.scalingMode            = surfInput->ScalingMode;
    m_Params.scalingPreference      = surfInput->ScalingPreference;
    m_Params.bDirectionalScalar     = surfInput->bDirectionalScalar;
    m_Params.formatInput            = surfInput->Format;
    m_Params.rcSrcInput             = surfInput->rcSrc;

    m_Params.rcMaxSrcInput          = surfInput->rcMaxSrc;
    m_Params.dwWidthInput           = surfInput->dwWidth;
    m_Params.dwHeightInput          = surfInput->dwHeight;
    m_Params.formatOutput           = surfOutput->Format;
    m_Params.colorSpaceOutput       = surfOutput->ColorSpace;
    m_Params.pColorFillParams       = params.pColorFillParams;
    m_Params.pCompAlpha             = params.pColorFillParams ? params.pCompAlpha : nullptr;

    if (surfInput->Rotation == VPHAL_ROTATION_IDENTITY ||
        surfInput->Rotation == VPHAL_ROTATION_180 ||
        surfInput->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        surfInput->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        m_Params.bRotateNeeded  = false;
        m_Params.dwWidthOutput  = surfOutput->dwWidth;
        m_Params.dwHeightOutput = surfOutput->dwHeight;

        m_Params.rcDstInput     = surfInput->rcDst;
        m_Params.rcSrcOutput    = surfOutput->rcSrc;
        m_Params.rcDstOutput    = surfOutput->rcDst;
        m_Params.rcMaxSrcOutput = surfOutput->rcMaxSrc;
    }
    else
    {
        m_Params.bRotateNeeded      = true;
        m_Params.dwWidthOutput      = surfOutput->dwHeight;
        m_Params.dwHeightOutput     = surfOutput->dwWidth;

        RECT_ROTATE(m_Params.rcDstInput, surfInput->rcDst);
        RECT_ROTATE(m_Params.rcSrcOutput, surfOutput->rcSrc);
        RECT_ROTATE(m_Params.rcDstOutput, surfOutput->rcDst);
        RECT_ROTATE(m_Params.rcMaxSrcOutput, surfOutput->rcMaxSrc);
    }

    m_Params.interlacedScalingType = surfInput->InterlacedScalingType;
    m_Params.srcSampleType         = surfInput->SampleType;
    m_Params.dstSampleType         = surfOutput->SampleType;

    // For field-to-interleaved scaling, the height of rcSrcInput is input field height,
    // the height of rcDstInput is output frame height, for scaling ratio calculation, the
    // bottom of rcDstInput need to divide 2.
    if(m_Params.interlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED)
    {
        m_Params.rcDstInput.bottom /= 2;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterScaling::Configure(VEBOX_SFC_PARAMS &params)
{
    m_Params.scalingMode            = VPHAL_SCALING_AVS;
    m_Params.scalingPreference      = VPHAL_SCALING_PREFER_SFC;
    m_Params.bDirectionalScalar     = false;
    m_Params.formatInput            = params.input.surface->Format;
    m_Params.rcSrcInput             = params.input.rcSrc;
    m_Params.rcMaxSrcInput          = params.input.rcSrc;
    m_Params.dwWidthInput           = params.input.surface->dwWidth;
    m_Params.dwHeightInput          = params.input.surface->dwHeight;
    m_Params.formatOutput           = params.output.surface->Format;
    m_Params.colorSpaceOutput       = params.output.colorSpace;
    m_Params.pColorFillParams       = nullptr;
    m_Params.pCompAlpha             = nullptr;

    RECT recOutput = {0, 0, (int32_t)params.output.surface->dwWidth, (int32_t)params.output.surface->dwHeight};

    if (params.input.rotation == (MEDIA_ROTATION)VPHAL_ROTATION_IDENTITY    ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_ROTATION_180         ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_MIRROR_HORIZONTAL    ||
        params.input.rotation == (MEDIA_ROTATION)VPHAL_MIRROR_VERTICAL)
    {
        m_Params.dwWidthOutput  = params.output.surface->dwWidth;
        m_Params.dwHeightOutput = params.output.surface->dwHeight;

        m_Params.rcDstInput     = params.output.rcDst;
        m_Params.rcSrcOutput    = recOutput;
        m_Params.rcDstOutput    = recOutput;
        m_Params.rcMaxSrcOutput = recOutput;
    }
    else
    {
        m_Params.dwWidthOutput      = params.output.surface->dwHeight;
        m_Params.dwHeightOutput     = params.output.surface->dwWidth;

        RECT_ROTATE(m_Params.rcDstInput, params.output.rcDst);
        RECT_ROTATE(m_Params.rcSrcOutput, recOutput);
        RECT_ROTATE(m_Params.rcDstOutput, recOutput);
        RECT_ROTATE(m_Params.rcMaxSrcOutput, recOutput);
    }

    return MOS_STATUS_SUCCESS;
}

FeatureParamScaling &SwFilterScaling::GetSwFilterParams()
{
    return m_Params;
}

SwFilter *SwFilterScaling::Clone()
{
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
    SwFilterScaling *p = dynamic_cast<SwFilterScaling *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamScaling));
}

MOS_STATUS SwFilterScaling::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);

    m_Params.colorSpaceOutput   = outputSurf->ColorSpace;
    // update source sample type for field to interleaved mode.
    m_Params.srcSampleType      = inputSurf->SampleType;

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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterRotMir::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    // Parameter checking should be done in SwFilterRotMirHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.rotation     = surfInput->Rotation;
    m_Params.tileOutput   = surfOutput->TileType;
    m_Params.formatInput  = surfInput->Format;
    m_Params.formatOutput = surfOutput->Format;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterRotMir::Configure(VEBOX_SFC_PARAMS &params)
{
    // Parameter checking should be done in SwFilterRotMirHandler::IsFeatureEnabled.
    m_Params.rotation     = (VPHAL_ROTATION)params.input.rotation;
    m_Params.tileOutput   = params.output.surface->TileType;
    m_Params.formatInput  = params.input.surface->Format;
    m_Params.formatOutput = params.output.surface->Format;
    return MOS_STATUS_SUCCESS;
}

FeatureParamRotMir &SwFilterRotMir::GetSwFilterParams()
{
    return m_Params;
}

SwFilter *SwFilterRotMir::Clone()
{
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
    SwFilterRotMir *p = dynamic_cast<SwFilterRotMir *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamRotMir));
}

MOS_STATUS SwFilterRotMir::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    m_Params.tileOutput = outputSurf->osSurface->TileType;
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterDenoise::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
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
    return m_Params;
}

SwFilter *SwFilterDenoise::Clone()
{
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

bool vp::SwFilterDenoise::operator==(SwFilter& swFilter)
{
    SwFilterDenoise* p = dynamic_cast<SwFilterDenoise*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamDenoise));
}

MOS_STATUS vp::SwFilterDenoise::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf)
{
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterDeinterlace::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    VP_PUBLIC_CHK_NULL_RETURN(surfInput);
    VP_PUBLIC_CHK_NULL_RETURN(surfInput->pDeinterlaceParams);

    m_Params.formatInput        = surfInput->Format;
    m_Params.formatOutput       = surfInput->Format;
    m_Params.sampleTypeInput    = surfInput->SampleType;
    m_Params.DIMode             = surfInput->pDeinterlaceParams->DIMode;            //!< DeInterlacing mode
    m_Params.bEnableFMD         = surfInput->pDeinterlaceParams->bEnableFMD;        //!< FMD
    m_Params.b60fpsDi           = !surfInput->pDeinterlaceParams->bSingleField;      //!< Used in frame Recon - if 30fps (one call per sample pair)
    m_Params.bSCDEnable         = surfInput->pDeinterlaceParams->bSCDEnable;        //!< Scene change detection
    m_Params.bHDContent         = MEDIA_IS_HDCONTENT(surfInput->dwWidth, surfInput->dwHeight);

    return MOS_STATUS_SUCCESS;
}

FeatureParamDeinterlace& SwFilterDeinterlace::GetSwFilterParams()
{
    return m_Params;
}

SwFilter *SwFilterDeinterlace::Clone()
{
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
    SwFilterDeinterlace* p = dynamic_cast<SwFilterDeinterlace*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamDeinterlace));
}

MOS_STATUS vp::SwFilterDeinterlace::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf)
{
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSte::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
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
    return m_Params;
}

SwFilter * SwFilterSte::Clone()
{
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
    SwFilterSte* p = dynamic_cast<SwFilterSte*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamSte));
}

MOS_STATUS vp::SwFilterSte::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf)
{
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterTcc::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
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
    return m_Params;
}

SwFilter * SwFilterTcc::Clone()
{
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
    SwFilterTcc* p = dynamic_cast<SwFilterTcc*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamTcc));
}

MOS_STATUS vp::SwFilterTcc::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf)
{
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterProcamp::Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex)
{
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
    return m_Params;
}

SwFilter * SwFilterProcamp::Clone()
{
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
    SwFilterProcamp* p = dynamic_cast<SwFilterProcamp*>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamProcamp));
}

MOS_STATUS vp::SwFilterProcamp::Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf)
{
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
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterHdr::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
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
    return m_Params;
}

SwFilter *SwFilterHdr::Clone()
{
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
    SwFilterHdr *p = dynamic_cast<SwFilterHdr *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamHdr));
}

MOS_STATUS vp::SwFilterHdr::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
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
    return m_location;
}
void SwFilterSet::SetLocation(std::vector<SwFilterSet *> *location)
{
    m_location = location;
}

MOS_STATUS SwFilterSet::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    for (auto swFilter : m_swFilters)
    {
        VP_PUBLIC_CHK_NULL_RETURN(swFilter.second);
        VP_PUBLIC_CHK_STATUS_RETURN(swFilter.second->Update(inputSurf, outputSurf));
    }
    return MOS_STATUS_SUCCESS;
}
