/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     vp_fc_filter.cpp
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all fc in driver.
//!
#include "vp_fc_filter.h"
#include "vp_render_cmd_packet.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_hal_ddi_utils.h"
#include <vector>

namespace vp {

bool PolicyFcHandler::s_forceNearestToBilinearIfBilinearExists = false;

VpFcFilter::VpFcFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpFcFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_renderFcParams)
    {
        MOS_FreeMemory(m_renderFcParams);
        m_renderFcParams = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::SetExecuteEngineCaps(
    SwFilterPipe *executedPipe,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executedPipe  = executedPipe;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::InitLayer(VP_FC_LAYER &layer, bool isInputPipe, int index, SwFilterPipe &executedPipe, VPHAL_SCALING_MODE defaultScalingMode)
{
    auto &surfGroup             = executedPipe.GetSurfacesSetting().surfGroup;

    SurfaceType surfId          = isInputPipe ? (SurfaceType)(SurfaceTypeFcInputLayer0 + index) : SurfaceTypeFcTarget0;
    auto        surfHandle = surfGroup.find(surfId);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfHandle, &surfGroup);
    layer.surf = surfHandle->second;

    VP_PUBLIC_CHK_NULL_RETURN(layer.surf);

    layer.layerID               = index;
    layer.layerIDOrigin         = index;

    SwFilterScaling *scaling    = dynamic_cast<SwFilterScaling *>(executedPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeScaling));
    layer.scalingMode           = scaling ? scaling->GetSwFilterParams().scalingMode : defaultScalingMode;
    layer.iscalingEnabled       = scaling ? ISCALING_INTERLEAVED_TO_INTERLEAVED == scaling->GetSwFilterParams().interlacedScalingType : false;
    layer.fieldWeaving          = scaling ? ISCALING_FIELD_TO_INTERLEAVED == scaling->GetSwFilterParams().interlacedScalingType : false;

    SwFilterRotMir *rotation    = dynamic_cast<SwFilterRotMir *>(executedPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeRotMir));
    layer.rotation              = rotation ? rotation->GetSwFilterParams().rotation : VPHAL_ROTATION_IDENTITY;

    layer.useSampleUnorm        = false;    // Force using sampler16 (dscaler) when compute walker in use.
    layer.useSamplerLumakey     = false;    // Only available on AVS sampler.
    layer.iefEnabled            = false;    // IEF not supported by 3D sampler.

    layer.paletteID             = -1;       //!<Palette Allocation
    layer.queryVariance         = layer.surf->bQueryVariance;

    SwFilterDeinterlace *di     = dynamic_cast<SwFilterDeinterlace *>(executedPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeDi));
    layer.diParams              = di ? di->GetSwFilterParams().diParams : nullptr;

    SwFilterLumakey *lumakey    = dynamic_cast<SwFilterLumakey *>(executedPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeLumakey));
    layer.lumaKeyParams         = lumakey ? lumakey->GetSwFilterParams().lumaKeyParams : nullptr;

    SwFilterBlending *blending  = dynamic_cast<SwFilterBlending *>(executedPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeBlending));
    layer.blendingParams        = blending ? blending->GetSwFilterParams().blendingParams : nullptr;
    layer.xorComp               = blending ? BLEND_XOR_MONO == blending->GetSwFilterParams().blendingParams->BlendType : false;

    SwFilterProcamp *procamp    = dynamic_cast<SwFilterProcamp *>(executedPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeProcamp));
    layer.procampParams         = procamp ? procamp->GetSwFilterParams().procampParams : nullptr;

    surfId                      = (SurfaceType)(SurfaceTypeFcInputLayer0Field1Dual + index);

    auto it = surfGroup.find(surfId);
    if (surfGroup.end() == it)
    {
        layer.surfField = nullptr;
    }
    else
    {
        layer.surfField = surfGroup.find(surfId)->second;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::GetDefaultScalingMode(VPHAL_SCALING_MODE& defaultScalingMode,SwFilterPipe &executedPipe)
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
        SwFilterScaling *scaling    = dynamic_cast<SwFilterScaling *>(executedPipe.GetSwFilter(true, i, FeatureType::FeatureTypeScaling));
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
                isInited = true;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::InitCompParams(VP_COMPOSITE_PARAMS &compParams, SwFilterPipe &executedPipe)
{
    MOS_ZeroMemory(&compParams, sizeof(compParams));
    auto &surfGroup = executedPipe.GetSurfacesSetting().surfGroup;
    compParams.sourceCount = executedPipe.GetSurfaceCount(true);

    if (SurfaceTypeFcInputLayer0 + compParams.sourceCount - 1 > SurfaceTypeFcInputLayerMax)
    {
        VP_RENDER_ASSERTMESSAGE("Invalid source count (%d)!", compParams.sourceCount);
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // Select default scaling mode for 3D sampler.
    VPHAL_SCALING_MODE defaultScalingMode = VPHAL_SCALING_NEAREST;
    VP_PUBLIC_CHK_STATUS_RETURN(GetDefaultScalingMode(defaultScalingMode, executedPipe));

    for (uint32_t i = 0; i < executedPipe.GetSurfaceCount(true); ++i)
    {
        VP_RENDER_CHK_STATUS_RETURN(InitLayer(compParams.source[i], true, i, executedPipe, defaultScalingMode));
    }

    compParams.targetCount = 1;
    VP_RENDER_CHK_STATUS_RETURN(InitLayer(compParams.target[0], false, 0, executedPipe, defaultScalingMode));

    SwFilterColorFill *colorFill        = dynamic_cast<SwFilterColorFill *>(executedPipe.GetSwFilter(false, 0, FeatureType::FeatureTypeColorFill));
    compParams.pColorFillParams         = colorFill ? colorFill->GetSwFilterParams().colorFillParams : nullptr;

    SwFilterAlpha *alpha                = dynamic_cast<SwFilterAlpha *>(executedPipe.GetSwFilter(false, 0, FeatureType::FeatureTypeAlpha));
    compParams.pCompAlpha               = alpha ? alpha->GetSwFilterParams().compAlpha : nullptr;
    // Enable alpha calculating
    compParams.bAlphaCalculateEnable    = alpha ? alpha->GetSwFilterParams().calculatingAlpha : false;

    VP_RENDER_CHK_STATUS_RETURN(CalculateCompParams(compParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::AdjustParamsBasedOnFcLimit(VP_COMPOSITE_PARAMS &compParams)
{
    //The kernel is using the rectangle data to calculate mask. If the rectangle configuration does not comply to kernel requirement, the mask calculation will be incorrect and will see corruption.
    if (compParams.pColorFillParams == nullptr &&
        compParams.sourceCount == 1 &&
        compParams.targetCount == 1 &&
        compParams.target[0].surf != nullptr &&
        compParams.source[0].surf != nullptr)
    {
        if (compParams.target[0].surf->rcDst.top <= compParams.source[0].surf->rcDst.top &&
            compParams.target[0].surf->rcDst.left <= compParams.source[0].surf->rcDst.left &&
            compParams.target[0].surf->rcDst.right >= compParams.source[0].surf->rcDst.right &&
            compParams.target[0].surf->rcDst.bottom >= compParams.source[0].surf->rcDst.bottom)
        {
            VP_RENDER_NORMALMESSAGE("Render Path : 1 Surface to 1 Surface FC Composition. ColorFill is Disabled. Output Dst is bigger than Input Dst. Will make Output Dst become Input Dst to Avoid FC Corruption. (%d %d %d %d) -> (%d %d %d %d)",
                compParams.target[0].surf->rcDst.left,
                compParams.target[0].surf->rcDst.top,
                compParams.target[0].surf->rcDst.right,
                compParams.target[0].surf->rcDst.bottom,
                compParams.source[0].surf->rcDst.left,
                compParams.source[0].surf->rcDst.top,
                compParams.source[0].surf->rcDst.right,
                compParams.source[0].surf->rcDst.bottom);
            compParams.target[0].surf->rcSrc = compParams.source[0].surf->rcDst;
            compParams.target[0].surf->rcDst = compParams.source[0].surf->rcDst;
        }
    }

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS VpFcFilter::CalculateCompParams(VP_COMPOSITE_PARAMS &compParams)
{
    int layerCount = 0;

    VP_RENDER_CHK_STATUS_RETURN(AdjustParamsBasedOnFcLimit(compParams));

    for (uint32_t i = 0; i < compParams.sourceCount; ++i)
    {
        VP_FC_LAYER *layer = &compParams.source[i];
        auto params = &layer->calculatedParams;
        auto params2 = &layer->calculatedParams2;

        VP_RENDER_CHK_STATUS_RETURN(CalculateConstantAlpha(params->alpha, *layer));

        if (layerCount != i)
        {
            compParams.source[layerCount] = *layer;
            layer = &compParams.source[layerCount];
            params = &layer->calculatedParams;
            params2 = &layer->calculatedParams2;
        }
        layer->layerIDOrigin = i;
        layer->layerID = layerCount++;

        VP_RENDER_CHK_STATUS_RETURN(CalculateScalingParams(layer, &compParams.target[0], params->fScaleX, params->fScaleY,
            params->fOffsetX, params->fOffsetY, params->fShiftX, params->fShiftY, params->clipedDstRect,
            params->isChromaUpSamplingNeeded, params->isChromaDownSamplingNeeded, params->samplerFilterMode,
            params2->fStepX, params2->fStepY));

        if (params->isChromaUpSamplingNeeded || params->isChromaDownSamplingNeeded)
        {
            if (!MEDIA_IS_WA(m_pvpMhwInterface->m_waTable, WaEnableDscale)                      ||
                (MEDIA_IS_WA(m_pvpMhwInterface->m_waTable, WaEnableDscale)                      &&
                 layer->scalingMode == VPHAL_SCALING_BILINEAR  &&
                 params->fScaleX >= (float)(1.0/3.0)                     &&
                 params->fScaleY >= (float)(1.0/3.0)))
            {
                params->chromaSitingEnabled = true;
            }
            else
            {
                params->chromaSitingEnabled = false;
            }
        }
        else
        {
            params->chromaSitingEnabled = false;
        }
    }

    compParams.sourceCount = layerCount;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFcFilter::CalculateConstantAlpha(uint16_t &alpha, VP_FC_LAYER &layer)
{
    alpha = 255;
    //-----------------------------------
    // Alpha blending optimization.
    // If Constant blending and one of the following is true, disable blending.
    // If Src+Constant blending and one of the following is true, fall back to Src blending.
    // Condition; alpha <= 0. Layer is 100% transparent.
    // Condition; alpha >= 1. Layer is 100% opaque.
    //-----------------------------------
    if (layer.blendingParams &&
        ((layer.blendingParams->BlendType == BLEND_CONSTANT) ||
         (layer.blendingParams->BlendType == BLEND_CONSTANT_SOURCE) ||
         (layer.blendingParams->BlendType == BLEND_CONSTANT_PARTIAL)))
    {
        float fAlpha = layer.blendingParams->fAlpha;

        // Don't render layer with alpha <= 0.0f
        if (fAlpha <= 0.0f)
        {
            // layer is not visible. Should not come to here as the transparent should
            // have been removed during PolicyFcHandler::LayerSelectForProcess.
            VP_RENDER_ASSERTMESSAGE("Transparent layer found, which is not expected in current function!");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        else
        {
            alpha  = (uint16_t) (255.0f * fAlpha);
        }

        VP_RENDER_NORMALMESSAGE("Layer %d: BlendType %d, fAlpha %d",
            layer.layerID,
            layer.blendingParams->BlendType,
            layer.blendingParams->fAlpha);

        if (fAlpha >= 1.0f || alpha >= 255)
        {
            if (layer.blendingParams->BlendType == BLEND_CONSTANT)
            {
                layer.blendingParams->BlendType = BLEND_NONE;
            }
            else // for BlendType == BLEND_CONSTANT_SOURCE
            {
                layer.blendingParams->BlendType = BLEND_SOURCE;
            }

            layer.blendingParams->fAlpha    = 1.0f;
            alpha = 255;
        }
    }
    return MOS_STATUS_SUCCESS;
}

// x,y scaling steps
// cropping
MOS_STATUS VpFcFilter::CalculateScalingParams(VP_FC_LAYER *layer, VP_FC_LAYER *target, float &fScaleX, float &fScaleY,
    float &fOffsetX, float &fOffsetY, float &fShiftX , float &fShiftY, RECT &clipedDstRect,
    bool &isChromaUpSamplingNeeded, bool &isChromaDownSamplingNeeded, MHW_SAMPLER_FILTER_MODE &samplerFilterMode,
    float &fStepX, float &fStepY)
{
    float       fDiScaleY = 1.0f;                          // BOB scaling factor for Y
    float       fCropX = 0, fCropY = 0;
    float samplerLinearBiasX = VP_SAMPLER_BIAS;     //!< Linear sampler bias X
    float samplerLinearBiasY = VP_SAMPLER_BIAS;     //!< Linear sampler bias Y

    VP_RENDER_CHK_NULL_RETURN(layer);
    VP_RENDER_CHK_NULL_RETURN(target);

    // x,y scaling factor
    fScaleX = 1.0;
    fScaleY = 1.0;

    fOffsetX = samplerLinearBiasX;
    fOffsetY = samplerLinearBiasY;

    // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (layer->rotation == VPHAL_ROTATION_IDENTITY    ||
        layer->rotation == VPHAL_ROTATION_180         ||
        layer->rotation == VPHAL_MIRROR_HORIZONTAL    ||
        layer->rotation == VPHAL_MIRROR_VERTICAL)
    {
        fScaleX      = (float)(layer->surf->rcDst.right  - layer->surf->rcDst.left) /
                        (float)(layer->surf->rcSrc.right  - layer->surf->rcSrc.left);
        fScaleY      = (float)(layer->surf->rcDst.bottom - layer->surf->rcDst.top) /
                        (float)(layer->surf->rcSrc.bottom - layer->surf->rcSrc.top);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        fScaleX      = (float)(layer->surf->rcDst.right  - layer->surf->rcDst.left) /
                        (float)(layer->surf->rcSrc.bottom  - layer->surf->rcSrc.top);
        fScaleY      = (float)(layer->surf->rcDst.bottom - layer->surf->rcDst.top) /
                        (float)(layer->surf->rcSrc.right - layer->surf->rcSrc.left);
    }


    // if 1:1 scaling and interlaced scaling or field weaving
    // do not adjust offsets since it uses Nearest sampling
    if (fScaleX == 1.0F &&
        fScaleY == 1.0F &&
        (layer->iscalingEnabled || layer->fieldWeaving))
    {
        fDiScaleY = 0.5f;
    }
    else
    {
        switch (layer->surf->SampleType)
        {
            case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
            case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
                fDiScaleY = 0.5f;
                // don't break
            case SAMPLE_SINGLE_TOP_FIELD:
                fOffsetY += 0.25f;
                break;

            case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
            case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
                fDiScaleY = 0.5f;
                // don't break
            case SAMPLE_SINGLE_BOTTOM_FIELD:
                fOffsetY -= 0.25f;
                break;

            case SAMPLE_PROGRESSIVE:
            default:
                fDiScaleY = 1.0f;
                break;
        }
    }

    // Normalize source co-ordinates using the width and height programmed
    // in surface state. step X, Y pre-rotated
    // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (layer->rotation == VPHAL_ROTATION_IDENTITY    ||
        layer->rotation == VPHAL_ROTATION_180         ||
        layer->rotation == VPHAL_MIRROR_HORIZONTAL    ||
        layer->rotation == VPHAL_MIRROR_VERTICAL)
    {
        fStepX = ((layer->surf->rcSrc.right - layer->surf->rcSrc.left - fCropX) * 1.0f) /
                    ((layer->surf->rcDst.right - layer->surf->rcDst.left) > 0 ?
                    (layer->surf->rcDst.right - layer->surf->rcDst.left) : 1);
        fStepY = ((layer->surf->rcSrc.bottom - layer->surf->rcSrc.top - fCropY) * fDiScaleY) /
                    ((layer->surf->rcDst.bottom - layer->surf->rcDst.top) > 0 ?
                    (layer->surf->rcDst.bottom - layer->surf->rcDst.top) : 1);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        fStepX = ((layer->surf->rcSrc.right - layer->surf->rcSrc.left - fCropX) * 1.0f) /
                    ((layer->surf->rcDst.bottom - layer->surf->rcDst.top) > 0 ?
                    (layer->surf->rcDst.bottom - layer->surf->rcDst.top) : 1);
        fStepY = ((layer->surf->rcSrc.bottom - layer->surf->rcSrc.top - fCropY) * fDiScaleY) /
                    ((layer->surf->rcDst.right - layer->surf->rcDst.left) > 0 ?
                    (layer->surf->rcDst.right - layer->surf->rcDst.left) : 1);
    }

    // Source sampling coordinates based on rcSrc
    fOffsetX += (layer->surf->rcSrc.left + fCropX / 2);
    fOffsetY += (layer->surf->rcSrc.top + fCropY / 2) * fDiScaleY;

    isChromaUpSamplingNeeded = false;
    isChromaDownSamplingNeeded = false;
    VP_PUBLIC_CHK_STATUS_RETURN(PolicyFcHandler::IsChromaSamplingNeeded(isChromaUpSamplingNeeded, isChromaDownSamplingNeeded,
                                                    layer->surf->SurfType, layer->layerID,
                                                    layer->surf->osSurface->Format, target->surf->osSurface->Format));

    if (VPHAL_SCALING_NEAREST == layer->scalingMode && (isChromaUpSamplingNeeded || isChromaDownSamplingNeeded))
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Info: Nearest scaling with isChromaUpSamplingNeeded (%d) and isChromaDownSamplingNeeded (%d)",
            isChromaUpSamplingNeeded, isChromaDownSamplingNeeded);
    }

    // Use 3D Nearest Mode only for 1x Scaling in both directions and only if the input is Progressive or interlaced scaling is used
    // In case of two or more layers, set Sampler State to Bilinear if any layer requires Bilinear
    // When primary surface needs chroma upsampling,
    // force to use 3D Bilinear Mode for 1x scaling for better quality
    samplerFilterMode = Get3DSamperFilterMode(layer->scalingMode);

    if (samplerFilterMode == MHW_SAMPLER_FILTER_NEAREST)
    {
        fShiftX  = 0.0f;
        fShiftY  = 0.0f;
    }
    else if (samplerFilterMode == MHW_SAMPLER_FILTER_BILINEAR)
    {
        fShiftX = VP_HW_LINEAR_SHIFT;  // Bilinear scaling shift
        fShiftY = VP_HW_LINEAR_SHIFT;
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    clipedDstRect = layer->surf->rcDst;
    uint32_t dwDestRectWidth  = target->surf->osSurface->dwWidth;
    uint32_t dwDestRectHeight = target->surf->osSurface->dwHeight;
    switch (layer->rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            // Coordinate adjustment for render target coordinates (0,0)
            fShiftX  -= layer->surf->rcDst.left;
            fShiftY  -= layer->surf->rcDst.top;
            break;
        case VPHAL_ROTATION_90:
            // Coordinate adjustment for 90 degree rotation
            fShiftX  -= (float)layer->surf->rcDst.top;
            fShiftY  -= (float)dwDestRectWidth -
                        (float)(layer->surf->rcSrc.bottom - layer->surf->rcSrc.top) * fScaleX -
                        (float)layer->surf->rcDst.left;
            break;
        case VPHAL_ROTATION_180:
            // Coordinate adjustment for 180 degree rotation
            fShiftX  -= (float)dwDestRectWidth -
                        (float)(layer->surf->rcSrc.right - layer->surf->rcSrc.left) * fScaleX -
                        (float)layer->surf->rcDst.left;
            fShiftY  -= (float)dwDestRectHeight -
                        (float)(layer->surf->rcSrc.bottom - layer->surf->rcSrc.top) * fScaleY -
                        (float)layer->surf->rcDst.top;
            break;
        case VPHAL_ROTATION_270:
            // Coordinate adjustment for 270 degree rotation
            fShiftX  -= (float)dwDestRectHeight -
                        (float)(layer->surf->rcSrc.right - layer->surf->rcSrc.left) * fScaleY -
                        (float)layer->surf->rcDst.top;
            fShiftY  -= (float)layer->surf->rcDst.left;
            break;
        case VPHAL_MIRROR_HORIZONTAL:
            // Coordinate adjustment for horizontal mirroring
            fShiftX  -= (float)dwDestRectWidth -
                        (float)(layer->surf->rcSrc.right - layer->surf->rcSrc.left) * fScaleX -
                        (float)layer->surf->rcDst.left;
            fShiftY  -= layer->surf->rcDst.top;
            break;
        case VPHAL_MIRROR_VERTICAL:
            // Coordinate adjustment for vertical mirroring
            fShiftX  -= layer->surf->rcDst.left;
            fShiftY  -= (float)dwDestRectHeight -
                        (float)(layer->surf->rcSrc.bottom - layer->surf->rcSrc.top) * fScaleY -
                        (float)layer->surf->rcDst.top;
            break;
        case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
            // Coordinate adjustment for rotating 90 and horizontal mirroring
            fShiftX  -= (float)layer->surf->rcDst.top;
            fShiftY  -= (float)layer->surf->rcDst.left;
            break;
        case VPHAL_ROTATE_90_MIRROR_VERTICAL:
        default:
            // Coordinate adjustment for rotating 90 and vertical mirroring
            fShiftX  -= (float)dwDestRectHeight -
                        (float)(layer->surf->rcSrc.right - layer->surf->rcSrc.left) * fScaleY -
                        (float)layer->surf->rcDst.top;
            fShiftY  -= (float)dwDestRectWidth -
                        (float)(layer->surf->rcSrc.bottom - layer->surf->rcSrc.top) * fScaleX -
                        (float)layer->surf->rcDst.left;
            break;
    } // switch

    if (layer->xorComp)
    {
        // for cursor layer, every bit indicate 1 pixel. should extend the width as real output pixel.
        clipedDstRect.right =
            clipedDstRect.left + (clipedDstRect.right - clipedDstRect.left) * 8;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SAMPLER_FILTER_MODE VpFcFilter::Get3DSamperFilterMode(VPHAL_SCALING_MODE scalingMode)
{
    return (VPHAL_SCALING_NEAREST == scalingMode) ? MHW_SAMPLER_FILTER_NEAREST : MHW_SAMPLER_FILTER_BILINEAR;
}

MOS_STATUS VpFcFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bRender)
    {
        // create a filter Param buffer
        if (!m_renderFcParams)
        {
            m_renderFcParams = (PRENDER_FC_PARAMS)MOS_AllocAndZeroMemory(sizeof(RENDER_FC_PARAMS));

            if (m_renderFcParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("render fc Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_renderFcParams, sizeof(RENDER_FC_PARAMS));
        }

        m_renderFcParams->kernelId           = kernelCombinedFc;
        InitCompParams(m_renderFcParams->compParams, *m_executedPipe);
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for Dn");
    }
    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   HwFilter Fc Parameter                                          */
/****************************************************************************************************/
HwFilterParameter *HwFilterFcParameter::Create(HW_FILTER_FC_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterFcParameter *p = MOS_New(HwFilterFcParameter, featureType);
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

HwFilterFcParameter::HwFilterFcParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterFcParameter::~HwFilterFcParameter()
{
}

MOS_STATUS HwFilterFcParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterFcParameter::Initialize(HW_FILTER_FC_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Fc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpRenderFcParameter::Create(HW_FILTER_FC_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderFcParameter *p = dynamic_cast<VpRenderFcParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpRenderFcParameter::VpRenderFcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_fcFilter(pHwInterface)
{
}
VpRenderFcParameter::~VpRenderFcParameter() {}

bool VpRenderFcParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    VpRenderCmdPacket *renderPacket = dynamic_cast<VpRenderCmdPacket *>(pPacket);
    if (nullptr == renderPacket)
    {
        return false;
    }

    PRENDER_FC_PARAMS params = m_fcFilter.GetFcParams();
    if (nullptr == params)
    {
        return false;
    }
    return MOS_SUCCEEDED(renderPacket->SetFcParams(params));
}

MOS_STATUS VpRenderFcParameter::Initialize(HW_FILTER_FC_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.SetExecuteEngineCaps(params.executedPipe, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy FC Feature Handler                                      */
/****************************************************************************************************/

MOS_STATUS PolicyFcFeatureHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
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
        if (FeatureTypeLumakeyOnRender      == type ||
            FeatureTypeBlendingOnRender     == type ||
            FeatureTypeAlphaOnRender        == type ||
            FeatureTypeCscOnRender          == type ||
            FeatureTypeScalingOnRender      == type ||
            FeatureTypeRotMirOnRender       == type ||
            FeatureTypeDiOnRender           == type ||
            FeatureTypeProcampOnRender      == type)
        {
            return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
        }
        else if(FeatureTypeColorFillOnRender == type)
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

MOS_STATUS PolicyFcFeatureHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
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
PolicyFcHandler::PolicyFcHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeFc;
}
PolicyFcHandler::~PolicyFcHandler()
{
}

bool PolicyFcHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bComposite;
}

HwFilterParameter* PolicyFcHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        HW_FILTER_FC_PARAM param = {};
        param.type = m_Type;
        param.pHwInterface = pHwInterface;
        param.vpExecuteCaps = vpExecuteCaps;
        param.pPacketParamFactory = &m_PacketParamFactory;
        param.executedPipe = &swFilterPipe;
        param.pfnCreatePacketParam = PolicyFcHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterFcParameter*)pHwFilterParam)->Initialize(param)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterFcParameter::Create(param, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyFcHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();
    VP_PUBLIC_ASSERTMESSAGE("Should not coming here!");
    return MOS_STATUS_SUCCESS;
}

bool PolicyFcHandler::IsInterlacedInputSupported(VP_SURFACE &input)
{
    // The parameter YOffset of surface state should be
    // a multiple of 4 when the input is accessed in field mode.For interlaced NV12
    // input, if its height is not a multiple of 4, the YOffset of UV plane will not
    // be a multiple of 4.So under this condition, we treat it as progressive input.
    return MOS_IS_ALIGNED(MOS_MIN((uint32_t)input.osSurface->dwHeight, (uint32_t)input.rcMaxSrc.bottom), 4) || input.osSurface->Format != Format_NV12;
}

bool PolicyFcHandler::IsBobDiEnabled(SwFilterDeinterlace *di, VP_SURFACE &input)
{
    if (nullptr == di || di->GetFilterEngineCaps().bEnabled == false)
    {
        return false;
    }

    return IsInterlacedInputSupported(input);
}

MOS_STATUS PolicyFcHandler::IsChromaSamplingNeeded(bool &isChromaUpSamplingNeeded, bool &isChromaDownSamplingNeeded,
                                VPHAL_SURFACE_TYPE surfType, int layerIndex,
                                MOS_FORMAT inputFormat, MOS_FORMAT outputFormat)
{
    VPHAL_COLORPACK srcColorPack = VpHalDDIUtils::GetSurfaceColorPack(inputFormat);
    VPHAL_COLORPACK dstColorPack = VpHalDDIUtils::GetSurfaceColorPack(outputFormat);

    if (SURF_IN_PRIMARY == surfType                         &&
        // when 3D sampler been used, PL2 chromasitting kernel does not support sub-layer chromasitting
        ((IS_PL2_FORMAT(inputFormat) && 0 == layerIndex)    ||
        inputFormat == Format_YUY2))
    {
        isChromaUpSamplingNeeded   = ((srcColorPack == VPHAL_COLORPACK_420 &&
                                    (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_444)) ||
                                    (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_444));
        isChromaDownSamplingNeeded = ((srcColorPack == VPHAL_COLORPACK_444 &&
                                    (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_420)) ||
                                    (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_420));
    }
    else
    {
        isChromaUpSamplingNeeded   = false;
        isChromaDownSamplingNeeded = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyFcHandler::Get3DSamplerScalingMode(VPHAL_SCALING_MODE &scalingMode, SwFilterSubPipe &pipe, int layerIndex, VP_SURFACE &input, VP_SURFACE &output)
{
    bool isChromaUpSamplingNeeded = false;
    bool isChromaDownSamplingNeeded = false;
    VP_PUBLIC_CHK_STATUS_RETURN(IsChromaSamplingNeeded(isChromaUpSamplingNeeded, isChromaDownSamplingNeeded,
                                input.SurfType, layerIndex,
                                input.osSurface->Format, output.osSurface->Format));

    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(pipe.GetSwFilter(FeatureType::FeatureTypeScaling));

    bool iscalingEnabled       = scaling ? ISCALING_INTERLEAVED_TO_INTERLEAVED == scaling->GetSwFilterParams().interlacedScalingType : false;
    bool fieldWeaving          = scaling ? ISCALING_FIELD_TO_INTERLEAVED == scaling->GetSwFilterParams().interlacedScalingType : false;

    // The rectangle in VP_SURFACE contains the rotation information.
    // The rectangle in ScalingFilter has been adjusted based on the rotation,
    // which can be used directly here.
    bool isScalingNeeded = false;
    if (scaling)
    {
        auto &scalingParamsInput = scaling->GetSwFilterParams().input;
        isScalingNeeded = (scalingParamsInput.rcDst.right - scalingParamsInput.rcDst.left)
        != (scalingParamsInput.rcSrc.right - scalingParamsInput.rcSrc.left) ||
        (scalingParamsInput.rcDst.bottom - scalingParamsInput.rcDst.top)
        != (scalingParamsInput.rcSrc.bottom - scalingParamsInput.rcSrc.top);
    }
    else
    {
        isScalingNeeded = false;
    }

    if (!isScalingNeeded                                                                    &&
        !isChromaUpSamplingNeeded                                                           &&
        !isChromaDownSamplingNeeded                                                         &&
        (input.SampleType == SAMPLE_PROGRESSIVE || iscalingEnabled || fieldWeaving))
    {
        scalingMode = VPHAL_SCALING_NEAREST;
    }
    else
    {
        scalingMode = VPHAL_SCALING_BILINEAR;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyFcHandler::AddInputLayerForProcess(bool &bSkip, std::vector<int> &layerIndexes, VPHAL_SCALING_MODE &scalingMode,
    int index, VP_SURFACE &input, SwFilterSubPipe& pipe, VP_SURFACE &output, VP_EXECUTE_CAPS& caps)
{
    bSkip = false;
    --m_resCounter.layers;

    if (VPHAL_PALETTE_NONE != input.Palette.PaletteType)
    {
        --m_resCounter.palettes;
    }
    
    SwFilterProcamp *procamp = dynamic_cast<SwFilterProcamp *>(pipe.GetSwFilter(FeatureType::FeatureTypeProcamp));
    if (procamp && procamp->IsFeatureEnabled(caps)  &&
        procamp->GetSwFilterParams().procampParams  &&
        procamp->GetSwFilterParams().procampParams->bEnabled)
    {
        --m_resCounter.procamp;
    }

    SwFilterLumakey *lumakey = dynamic_cast<SwFilterLumakey *>(pipe.GetSwFilter(FeatureType::FeatureTypeLumakey));
    if (lumakey)
    {
        --m_resCounter.lumaKeys;
        if (m_resCounter.lumaKeys < 0 || layerIndexes.size() > 1)
        {
            bSkip = true;
            VP_PUBLIC_NORMALMESSAGE("Scaling Info: layer %d is not selected. lumaKeys %d, layerIndexes.size() %d",
                index, m_resCounter.lumaKeys, layerIndexes.size());
            return MOS_STATUS_SUCCESS;
        }
        if (layerIndexes.size() == 1)
        {
            m_resCounter.sampler = VP_COMP_MAX_SAMPLER;
        }
    }

    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(pipe.GetSwFilter(FeatureType::FeatureTypeScaling));
    SwFilterDeinterlace *di = dynamic_cast<SwFilterDeinterlace *>(pipe.GetSwFilter(FeatureType::FeatureTypeDi));
    VPHAL_SAMPLE_TYPE sampleType = input.SampleType;
    bool samplerLumakeyEnabled = m_hwCaps.m_rules.isAvsSamplerSupported;

    if (nullptr == scaling)
    {
        VP_PUBLIC_ASSERTMESSAGE("Scaling Info: Scaling filter does not exist on layer %d!", index);
        VP_PUBLIC_CHK_NULL_RETURN(scaling);
    }

    scalingMode = scaling->GetSwFilterParams().scalingMode;

    // Disable AVS scaling mode
    if (!m_hwCaps.m_rules.isAvsSamplerSupported)
    {
        if (VPHAL_SCALING_AVS == scalingMode)
        {
            scalingMode = VPHAL_SCALING_BILINEAR;
        }
    }

    if (!IsInterlacedInputSupported(input))
    {
        sampleType = SAMPLE_PROGRESSIVE;
        // Disable DI
        if (di && di->IsFeatureEnabled(caps))
        {
            di->GetFilterEngineCaps().bEnabled = false;
        }
        // Disable Iscaling
        if (scaling->IsFeatureEnabled(caps) &&
            ISCALING_NONE != scaling->GetSwFilterParams().interlacedScalingType)
        {
            scaling->GetSwFilterParams().interlacedScalingType = ISCALING_NONE;
        }
    }

    // Number of AVS, but lumaKey and BOB DI needs 3D sampler instead of AVS sampler.
    if (VPHAL_SCALING_AVS == scalingMode &&
        nullptr == lumakey && !IsBobDiEnabled(di, input))
    {
        --m_resCounter.avs;
    }
    // Number of Sampler filter mode, we had better only support Nearest or Bilinear filter in one phase
    // If two filters are used together, the later filter overwrite the first and cause output quality issue.
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(Get3DSamplerScalingMode(scalingMode, pipe, layerIndexes.size(), input, output));

        // If bilinear needed for one layer, it will also be used by other layers.
        // nearest only be used if it is used by all layers.
        int32_t samplerMask = (VP_COMP_SAMPLER_BILINEAR | VP_COMP_SAMPLER_NEAREST);

        // Use sampler luma key feature only if this is not the bottom most layer
        if (samplerLumakeyEnabled && lumakey && layerIndexes.size() > 0 && !IS_PL3_FORMAT(input.osSurface->Format))
        {
            m_resCounter.sampler &= VP_COMP_SAMPLER_LUMAKEY;
        }
        else if (m_resCounter.sampler & samplerMask)
        {
            m_resCounter.sampler &= samplerMask;
        }
        else
        {
            // switch to AVS if AVS sampler is not used, decrease the count of comp phase
            // For isAvsSamplerSupported == false case, curent layer will be rejected, since m_resCounter.avs == 0.
            scalingMode = VPHAL_SCALING_AVS;
            --m_resCounter.avs;
        }
    }

    // Fails if any of the limits are reached
    // Output structure has reason why failed :-)
    // multi-passes if rotation is not the same as Layer 0 rotation
    // single pass if Primary layer needs rotation and remaining layer does not need rotation
    if (m_resCounter.layers   < 0 ||
        m_resCounter.palettes < 0 ||
        m_resCounter.procamp  < 0 ||
        m_resCounter.lumaKeys < 0 ||
        m_resCounter.avs      < 0 ||
        m_resCounter.sampler == 0)
    {
        //Multipass
        bSkip = true;
        VP_PUBLIC_NORMALMESSAGE("Scaling Info: layer %d is not selected. layers %d, palettes %d, procamp %d, lumaKeys %d, avs %d, sampler %d",
            index, m_resCounter.layers, m_resCounter.palettes, m_resCounter.procamp, m_resCounter.lumaKeys, m_resCounter.avs, m_resCounter.sampler);
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

MOS_STATUS PolicyFcHandler::RemoveTransparentLayers(SwFilterPipe& featurePipe)
{
    for (uint32_t i = 0; i < featurePipe.GetSurfaceCount(true); ++i)
    {
        SwFilterSubPipe *subpipe = featurePipe.GetSwFilterSubPipe(true, i);

        auto blending = dynamic_cast<SwFilterBlending *>(featurePipe.GetSwFilter(true, i, FeatureTypeBlending));
        if (nullptr == blending)
        {
            continue;
        }

        auto &param = blending->GetSwFilterParams();

        //-----------------------------------
        // Alpha blending optimization.
        // If Constant blending and one of the following is true, disable blending.
        // If Src+Constant blending and one of the following is true, fall back to Src blending.
        // Condition; alpha <= 0. Layer is 100% transparent.
        // Condition; alpha >= 1. Layer is 100% opaque.
        //-----------------------------------
        if (param.blendingParams &&
            ((param.blendingParams->BlendType == BLEND_CONSTANT) ||
             (param.blendingParams->BlendType == BLEND_CONSTANT_SOURCE) ||
             (param.blendingParams->BlendType == BLEND_CONSTANT_PARTIAL)))
        {
            float fAlpha = param.blendingParams->fAlpha;

            // Don't render layer with alpha <= 0.0f
            if (fAlpha <= 0.0f)
            {
                VP_PUBLIC_NORMALMESSAGE("Layer %d skipped: BlendType %d, fAlpha %d",
                    i,
                    param.blendingParams->BlendType,
                    param.blendingParams->fAlpha);
                VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.DestroySurface(true, i));
            }
        }
    }
    featurePipe.Update();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyFcHandler::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps)
{
    layerIndexes.clear();
    m_resCounter.Reset(m_hwCaps.m_rules.isAvsSamplerSupported);

    VP_PUBLIC_CHK_STATUS_RETURN(RemoveTransparentLayers(featurePipe));

    bool skip = false;
    VP_SURFACE *output = featurePipe.GetSurface(false, 0);
    bool bilinearInUseFor3DSampler = false;
    VP_PUBLIC_CHK_NULL_RETURN(output);

    for (uint32_t i = 0; i < featurePipe.GetSurfaceCount(true); ++i)
    {
        VPHAL_SCALING_MODE scalingMode = VPHAL_SCALING_NEAREST;
        VP_SURFACE *input = featurePipe.GetSurface(true, i);
        SwFilterSubPipe *subpipe = featurePipe.GetSwFilterSubPipe(true, i);
        VP_PUBLIC_CHK_NULL_RETURN(input);
        VP_PUBLIC_CHK_NULL_RETURN(subpipe);
        VP_PUBLIC_CHK_STATUS_RETURN(AddInputLayerForProcess(skip, layerIndexes, scalingMode, i, *input, *subpipe, *output, caps));
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
}
