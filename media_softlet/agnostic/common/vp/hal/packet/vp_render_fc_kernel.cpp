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
//! \file     vp_render_vebox_update_cmd_packet.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!
#include "vp_render_fc_kernel.h"
#include "vp_render_kernel_obj.h"
#include "hal_kerneldll.h"
#include "hal_oca_interface.h"
#include "vp_user_feature_control.h"

using namespace vp;

#define VP_COMP_SOURCE_DEPTH          16
#define VP_COMP_P010_DEPTH            0

// Compositing surface binding table index
#define VP_COMP_BTINDEX_LAYER0          0
#define VP_COMP_BTINDEX_LAYER0_FIELD0   0
#define VP_COMP_BTINDEX_LAYER1          3
#define VP_COMP_BTINDEX_LAYER2          6
#define VP_COMP_BTINDEX_LAYER3          9
#define VP_COMP_BTINDEX_LAYER4         12
#define VP_COMP_BTINDEX_LAYER5         15
#define VP_COMP_BTINDEX_LAYER6         18
#define VP_COMP_BTINDEX_LAYER7         21
#define VP_COMP_BTINDEX_RENDERTARGET   24
#define VP_COMP_BTINDEX_RT_SECOND      27    // Pre-SKL
#define VP_COMP_BTINDEX_L0_FIELD1_DUAL 48    // Pre-SKL

// CMFC macro
#define VP_COMP_BTINDEX_CSC_COEFF      34

const int32_t VpRenderFcKernel::s_bindingTableIndex[] =
{
    VP_COMP_BTINDEX_LAYER0,
    VP_COMP_BTINDEX_LAYER1,
    VP_COMP_BTINDEX_LAYER2,
    VP_COMP_BTINDEX_LAYER3,
    VP_COMP_BTINDEX_LAYER4,
    VP_COMP_BTINDEX_LAYER5,
    VP_COMP_BTINDEX_LAYER6,
    VP_COMP_BTINDEX_LAYER7
};

VpRenderFcKernel::VpRenderFcKernel(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    VpRenderKernelObj(hwInterface, allocator)
{
    m_kernelBinaryID = IDR_VP_EOT;
    m_kernelId       = kernelCombinedFc;

    if (m_hwInterface && m_hwInterface->m_vpPlatformInterface &&
        m_hwInterface->m_vpPlatformInterface->GetKernelPool().end() != m_hwInterface->m_vpPlatformInterface->GetKernelPool().find(VpRenderKernel::s_kernelNameNonAdvKernels))
    {
        m_kernelDllState = m_hwInterface->m_vpPlatformInterface->GetKernelPool().find(VpRenderKernel::s_kernelNameNonAdvKernels)->second.GetKdllState();

        // Setup Procamp Parameters
        KernelDll_SetupProcampParameters(m_kernelDllState,
                                         m_Procamp,
                                         m_maxProcampEntries);
    }
    else
    {
        VP_RENDER_ASSERTMESSAGE("VpRenderFcKernel::VpRenderFcKernel, m_kernelDllState is nullptr!");
    }

    m_renderHal = m_hwInterface ? m_hwInterface->m_renderHal : nullptr;

    if (m_renderHal && m_renderHal->pRenderHalPltInterface && m_hwInterface->m_vpPlatformInterface)
    {
        m_cscCoeffPatchModeEnabled = m_hwInterface->m_vpPlatformInterface->GetKernelConfig().IsFcCscCoeffPatchModeEnabled();

        MOS_USER_FEATURE_VALUE_DATA UserFeatureData = {};
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE_ID,
            &UserFeatureData,
            m_hwInterface->m_osInterface->pOsContext));
        m_cscCoeffPatchModeEnabled = UserFeatureData.bData ? false : true;

        m_computeWalkerEnabled = true;
    }
}

MOS_STATUS VpRenderFcKernel::SetCacheCntl(PVP_RENDER_CACHE_CNTL surfMemCacheCtl)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(surfMemCacheCtl);

    if (!surfMemCacheCtl->bCompositing)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    m_surfMemCacheCtl = surfMemCacheCtl->Composite;

    return MOS_STATUS_SUCCESS;
}

bool IsNV12SamplerLumakeyNeeded()
{
    return false;
}

MOS_STATUS VpRenderFcKernel::SetSurfaceParams(KERNEL_SURFACE_STATE_PARAM &surfParam, VP_FC_LAYER &layer, bool is32MWColorFillKern)
{
    VP_FUNC_CALL();

    auto &renderSurfParams = surfParam.surfaceOverwriteParams.renderSurfaceParams;
    MOS_ZeroMemory(&renderSurfParams, sizeof(renderSurfParams));

    surfParam.surfaceOverwriteParams.updatedRenderSurfaces = true;

    // Render target or private surface
    if (layer.surf->SurfType == SURF_OUT_RENDERTARGET)
    {
        // Disable AVS, IEF
        layer.scalingMode   = VPHAL_SCALING_BILINEAR;
        layer.iefEnabled    = false;

        // Set flags for RT
        surfParam.renderTarget              = true;
        renderSurfParams.bRenderTarget      = true;
        renderSurfParams.bWidthInDword_Y    = true;
        renderSurfParams.bWidthInDword_UV   = true;
        renderSurfParams.Boundary           = RENDERHAL_SS_BOUNDARY_DSTRECT;
    }
    // other surfaces
    else
    {
        surfParam.renderTarget              = false;
        renderSurfParams.bRenderTarget      = false;
        renderSurfParams.bWidthInDword_Y    = false;
        renderSurfParams.bWidthInDword_UV   = false;
        renderSurfParams.Boundary           = RENDERHAL_SS_BOUNDARY_SRCRECT;
    }

    renderSurfParams.b32MWColorFillKern = is32MWColorFillKern;

    // Set surface type based on scaling mode
    if (layer.scalingMode == VPHAL_SCALING_AVS)
    {
        renderSurfParams.Type = m_renderHal->SurfaceTypeAdvanced;
        renderSurfParams.bAVS = true;
    }
    else
    {
        renderSurfParams.Type = m_renderHal->SurfaceTypeDefault;
        renderSurfParams.bAVS = false;
    }

    // Set interlacing flags
    switch (layer.surf->SampleType)
    {
        case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
        case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
            renderSurfParams.bVertStride     = true;
            renderSurfParams.bVertStrideOffs = 0;
            break;
        case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
        case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
            renderSurfParams.bVertStride     = true;
            renderSurfParams.bVertStrideOffs = 1;
            break;
        default:
            renderSurfParams.bVertStride     = false;
            renderSurfParams.bVertStrideOffs = 0;
            break;
    }

    if (layer.layerID && IsNV12SamplerLumakeyNeeded())
    {
        renderSurfParams.b2PlaneNV12NeededByKernel = true;
    }

    surfParam.surfaceEntries = layer.surfaceEntries;
    surfParam.sizeOfSurfaceEntries = &layer.numOfSurfaceEntries;

    VP_RENDER_NORMALMESSAGE("SurfaceTYpe %d, bAVS %d, b2PlaneNV12NeededByKernel %d",
        renderSurfParams.Type,
        renderSurfParams.bAVS,
        renderSurfParams.b2PlaneNV12NeededByKernel);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_renderHal);

    uint32_t i = 0;
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;

    m_surfaceState.clear();

    for (i = 0; i < compParams.sourceCount; ++i)
    {
        KERNEL_SURFACE_STATE_PARAM surfParam = {};
        VP_FC_LAYER *layer = &compParams.source[i];

        surfParam.surfaceOverwriteParams.updatedSurfaceParams = true;

        // Only need to specify binding index in surface parameters.
        surfParam.surfaceOverwriteParams.bindedKernel = true;
        surfParam.surfaceOverwriteParams.bindIndex = s_bindingTableIndex[layer->layerID];

        SetSurfaceParams(surfParam, *layer, false);
        surfParam.surfaceOverwriteParams.renderSurfaceParams.bChromasiting = layer->calculatedParams.chromaSitingEnabled;

        surfParam.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl = (layer->surf->SurfType == SURF_IN_PRIMARY) ?
                                    m_surfMemCacheCtl.PrimaryInputSurfMemObjCtl :
                                    m_surfMemCacheCtl.InputSurfMemObjCtl;

        m_surfaceState.insert(std::make_pair(SurfaceType(SurfaceTypeFcInputLayer0 + layer->layerID), surfParam));

        //update render GMM resource usage type
        m_allocator->UpdateResourceUsageType(&layer->surf->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER);

        // Ensure the input is ready to be read
        // Currently, mos RegisterResourcere cannot sync the 3d resource.
        // Temporaly, call sync resource to do the sync explicitly.
        // Sync need be done after switching context.
#if MOS_MEDIASOLO_SUPPORTED
        if (!m_hwInterface->m_osInterface->bSoloInUse)
#endif
        {
            m_allocator->SyncOnResource(
                &layer->surf->osSurface->OsResource,
                false);
        }
    }

    // Used for 32x32 Media walker kernel + Color fill kernel
    // Not valid for media object.
    bool is32MWColorFillKern =
        (compParams.pColorFillParams != nullptr &&
         compParams.sourceCount == 0 &&
         m_renderHal->pHwSizes->dwSizeMediaWalkerBlock == 32);

    for (i = 0; i < compParams.targetCount; ++i)
    {
        KERNEL_SURFACE_STATE_PARAM surfParam = {};

        surfParam.surfaceOverwriteParams.updatedSurfaceParams = true;

        // Only need to specify binding index in surface parameters.
        surfParam.surfaceOverwriteParams.bindedKernel = true;
        if (compParams.targetCount > 1 && 0 == i)
        {
            surfParam.surfaceOverwriteParams.bindIndex = VP_COMP_BTINDEX_RT_SECOND;
        }
        else
        {
            surfParam.surfaceOverwriteParams.bindIndex = VP_COMP_BTINDEX_RENDERTARGET;
        }

        SetSurfaceParams(surfParam, compParams.target[i], is32MWColorFillKern);

        surfParam.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl = m_surfMemCacheCtl.TargetSurfMemObjCtl;

        m_surfaceState.insert(std::make_pair(SurfaceType(SurfaceTypeFcTarget0 + i), surfParam));

        //update render GMM resource usage type
        m_allocator->UpdateResourceUsageType(&compParams.target[i].surf->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_RENDER);

        // Ensure the output is ready to be written.
#if MOS_MEDIASOLO_SUPPORTED
        if (!m_hwInterface->m_osInterface->bSoloInUse)
#endif
        {
            m_allocator->SyncOnResource(
                &compParams.target[i].surf->osSurface->OsResource,
                true);
        }
    }

    if (m_kernelDllState->bEnableCMFC && m_cscCoeffPatchModeEnabled)
    {
        KERNEL_SURFACE_STATE_PARAM surfParam = {};

        surfParam.surfaceOverwriteParams.updatedSurfaceParams = true;
        // Only need to specify binding index in surface parameters.
        surfParam.surfaceOverwriteParams.bindedKernel = true;
        surfParam.surfaceOverwriteParams.bindIndex = VP_COMP_BTINDEX_CSC_COEFF;

        surfParam.surfaceOverwriteParams.updatedRenderSurfaces             = true;
        surfParam.surfaceOverwriteParams.renderSurfaceParams.Type          = RENDERHAL_SURFACE_TYPE_G10;
        surfParam.surfaceOverwriteParams.renderSurfaceParams.bRenderTarget = false;
        surfParam.surfaceOverwriteParams.renderSurfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        surfParam.surfaceOverwriteParams.renderSurfaceParams.bWidth16Align = false;
        surfParam.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl     = m_surfMemCacheCtl.InputSurfMemObjCtl;
        m_surfaceState.insert(std::make_pair(SurfaceTypeFcCscCoeff, surfParam));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::PrintSearchFilter(Kdll_FilterEntry *filter, int32_t filterSize)
{
    VP_RENDER_CHK_NULL_RETURN(filter);
    // Log for debug
    for (int32_t i = 0; i < filterSize; i++)
    {
        VPHAL_RENDER_NORMALMESSAGE("Kernel Search Filter %d: layer %d, format %d, cspace %d, \
                                   bEnableDscale %d, bIsDitherNeeded %d, chromasiting %d, colorfill %d, dualout %d, \
                                   lumakey %d, procamp %d, RenderMethod %d, sampler %d, samplerlumakey %d ",
                                   i, filter[i].layer, filter[i].format, filter[i].cspace,
                                   filter[i].bEnableDscale, filter[i].bIsDitherNeeded,
                                   filter[i].chromasiting, filter[i].colorfill,  filter[i].dualout,
                                   filter[i].lumakey, filter[i].procamp, filter[i].RenderMethod, filter[i].sampler, filter[i].samplerlumakey);
    }

    return MOS_STATUS_SUCCESS;
}

const Kdll_Layer g_surfaceType_Layer[] =
{
    Layer_None        ,    //!< SURF_NONE
    Layer_Background  ,    //!< SURF_IN_BACKGROUND
    Layer_MainVideo   ,    //!< SURF_IN_PRIMARY
    Layer_SubVideo    ,    //!< SURF_IN_SECONDARY
    Layer_SubPicture1 ,    //!< SURF_IN_SUBSTREAM
    Layer_Graphics    ,    //!< SURF_IN_GRAPHICS
    Layer_Invalid     ,    //!< SURF_IN_REFERENCE
    Layer_RenderTarget     //!< SURF_OUT_RENDERTARGET
};

static inline RENDERHAL_SURFACE_TYPE ConvertVpSurfaceTypeToRenderSurfType(VPHAL_SURFACE_TYPE vpSurfType)
{
    switch (vpSurfType)
    {
        case SURF_IN_BACKGROUND:
            return RENDERHAL_SURF_IN_BACKGROUND;

        case SURF_IN_PRIMARY:
            return RENDERHAL_SURF_IN_PRIMARY;

        case SURF_IN_SUBSTREAM:
            return RENDERHAL_SURF_IN_SUBSTREAM;

        case SURF_IN_REFERENCE:
            return RENDERHAL_SURF_IN_REFERENCE;

        case SURF_OUT_RENDERTARGET:
            return RENDERHAL_SURF_OUT_RENDERTARGET;

        case SURF_NONE:
        default:
            return RENDERHAL_SURF_NONE;
    }
}

static inline RENDERHAL_SCALING_MODE ConvertVpScalingModeToRenderScalingMode(VPHAL_SCALING_MODE vpScalingMode)
{
    switch (vpScalingMode)
    {
        case VPHAL_SCALING_NEAREST:
            return RENDERHAL_SCALING_NEAREST;

        case VPHAL_SCALING_BILINEAR:
            return RENDERHAL_SCALING_BILINEAR;

        case VPHAL_SCALING_AVS:
            return RENDERHAL_SCALING_AVS;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid VPHAL_SCALING_MODE %d, force to nearest mode.", vpScalingMode);
            return RENDERHAL_SCALING_NEAREST;
    }
}

static inline RENDERHAL_SAMPLE_TYPE ConvertVpSampleTypeToRenderSampleType(VPHAL_SAMPLE_TYPE SampleType)
{
    switch (SampleType)
    {
        case SAMPLE_PROGRESSIVE:
            return RENDERHAL_SAMPLE_PROGRESSIVE;

        case SAMPLE_SINGLE_TOP_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_TOP_FIELD;

        case SAMPLE_SINGLE_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;

        case SAMPLE_INVALID:
        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid VPHAL_SAMPLE_TYPE %d.\n", SampleType);
            return RENDERHAL_SAMPLE_INVALID;
    }
}

static inline MHW_ROTATION VpRotationModeToRenderRotationMode(VPHAL_ROTATION Rotation)
{
    MHW_ROTATION    Mode = MHW_ROTATION_IDENTITY;

    switch (Rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            Mode = MHW_ROTATION_IDENTITY;
            break;

        case VPHAL_ROTATION_90:
            Mode = MHW_ROTATION_90;
            break;

        case VPHAL_ROTATION_180:
            Mode = MHW_ROTATION_180;
            break;

        case VPHAL_ROTATION_270:
            Mode = MHW_ROTATION_270;
            break;

        case VPHAL_MIRROR_HORIZONTAL:
            Mode = MHW_MIRROR_HORIZONTAL;
            break;

        case VPHAL_MIRROR_VERTICAL:
            Mode = MHW_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            Mode = MHW_ROTATE_90_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
            Mode = MHW_ROTATE_90_MIRROR_HORIZONTAL;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid Rotation Angle.");
            break;
    }

    return Mode;
}

MOS_STATUS VpRenderFcKernel::InitRenderHalSurface(
    VP_FC_LAYER             *src,
    PRENDERHAL_SURFACE      renderHalSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    //---------------------------------------
    VP_RENDER_CHK_NULL_RETURN(src);
    VP_RENDER_CHK_NULL_RETURN(src->surf);
    VP_RENDER_CHK_NULL_RETURN(renderHalSurface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    //---------------------------------------

    auto osInterface = m_hwInterface->m_osInterface;

    VPHAL_RENDER_CHK_NULL_RETURN(osInterface);
    VPHAL_RENDER_CHK_NULL_RETURN(osInterface->pfnGetMemoryCompressionMode);
    VPHAL_RENDER_CHK_NULL_RETURN(osInterface->pfnGetMemoryCompressionFormat);

    MOS_ZeroMemory(renderHalSurface, sizeof(*renderHalSurface));

    renderHalSurface->OsSurface  = *src->surf->osSurface;

    if (0 == renderHalSurface->OsSurface.dwQPitch)
    {
        renderHalSurface->OsSurface.dwQPitch = renderHalSurface->OsSurface.dwHeight;
    }

    VP_RENDER_CHK_STATUS_RETURN(osInterface->pfnGetMemoryCompressionMode(osInterface,
        &src->surf->osSurface->OsResource, &renderHalSurface->OsSurface.MmcState));

    VP_RENDER_CHK_STATUS_RETURN(osInterface->pfnGetMemoryCompressionFormat(osInterface,
        &src->surf->osSurface->OsResource, &renderHalSurface->OsSurface.CompressionFormat));

    renderHalSurface->rcSrc                        = src->surf->rcSrc;
    renderHalSurface->rcDst                        = src->surf->rcDst;
    renderHalSurface->rcMaxSrc                     = src->surf->rcMaxSrc;
    renderHalSurface->SurfType                     =
                    ConvertVpSurfaceTypeToRenderSurfType(src->surf->SurfType);
    renderHalSurface->ScalingMode                  =
                    ConvertVpScalingModeToRenderScalingMode(src->scalingMode);
    renderHalSurface->ChromaSiting                 = src->surf->ChromaSiting;

    if (src->diParams != nullptr)
    {
        renderHalSurface->bDeinterlaceEnable       = true;
    }
    else
    {
        renderHalSurface->bDeinterlaceEnable       = false;
    }

    renderHalSurface->iPaletteID                   = src->paletteID;
    renderHalSurface->bQueryVariance               = src->queryVariance;
    renderHalSurface->bInterlacedScaling           = src->iscalingEnabled;
    renderHalSurface->pDeinterlaceParams           = (void *)src->diParams;
    renderHalSurface->SampleType                   =
                    ConvertVpSampleTypeToRenderSampleType(src->surf->SampleType);

    renderHalSurface->Rotation                     =
                    VpRotationModeToRenderRotationMode(src->rotation);

    return eStatus;
}

MOS_STATUS VpRenderFcKernel::InitRenderHalSurface(
    SurfaceType             type,
    VP_SURFACE              *surf,
    PRENDERHAL_SURFACE      renderHalSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL_RETURN(surf);
    VPHAL_RENDER_CHK_NULL_RETURN(m_fcParams);

    auto &compParams = m_fcParams->compParams;

    if (type >= SurfaceTypeFcInputLayer0 && type <= SurfaceTypeFcInputLayerMax)
    {
        int32_t layerID = (int32_t)type - (int32_t)SurfaceTypeFcInputLayer0;
        for (int32_t i = 0; i < (int32_t)compParams.sourceCount; ++i)
        {
            if (layerID != compParams.source[i].layerID)
            {
                continue;
            }
            VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurface(&compParams.source[i], renderHalSurface));
            return MOS_STATUS_SUCCESS;
        }
    }
    else if (SurfaceTypeFcTarget0 == type)
    {
        VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurface(&compParams.target[0], renderHalSurface));
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_UNIMPLEMENTED;
}

void VpRenderFcKernel::OcaDumpKernelInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext)
{
    HalOcaInterface::DumpVpKernelInfo(cmdBuffer, mosContext, m_kernelId, m_kernelSearch.KernelCount, m_kernelSearch.KernelID);
}

bool IsRenderAlignmentWANeeded(VP_SURFACE *surface)
{
    if (nullptr == surface || nullptr == surface->osSurface)
    {
        return false;
    }

    if (!(MOS_IS_ALIGNED(MOS_MIN((uint32_t)surface->osSurface->dwHeight, (uint32_t)surface->rcMaxSrc.bottom), 4)) &&
        (surface->osSurface->Format == Format_NV12))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsBobDiEnabled(VP_FC_LAYER *layer)
{
    if (nullptr == layer || nullptr == layer->surf || nullptr == layer->surf->osSurface)
    {
        return false;
    }

    // Kernel don't support inderlaced Y410/Y210 as input format
    return (layer->diParams     &&
           (layer->surf->osSurface->Format != Format_Y410  &&
            layer->surf->osSurface->Format != Format_Y210  &&
            layer->surf->osSurface->Format != Format_Y216  &&
            layer->surf->osSurface->Format != Format_Y416) &&
            !IsRenderAlignmentWANeeded(layer->surf));
}

void CalculateScale(float &scaleX, float &scaleY, RECT &rcSrc, RECT &rcDst, VPHAL_ROTATION rot)
{
    scaleX = 1.0;
    scaleY = 1.0;

    if (rcSrc.bottom  - rcSrc.top == 0 || rcSrc.right - rcSrc.left == 0)
    {
        return;
    }

    // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (rot == VPHAL_ROTATION_IDENTITY    ||
        rot == VPHAL_ROTATION_180         ||
        rot == VPHAL_MIRROR_HORIZONTAL    ||
        rot == VPHAL_MIRROR_VERTICAL)
    {
        scaleX      = (float)(rcDst.right  - rcDst.left) /
                       (float)(rcSrc.right  - rcSrc.left);
        scaleY      = (float)(rcDst.bottom - rcDst.top) /
                       (float)(rcSrc.bottom - rcSrc.top);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        scaleX      = (float)(rcDst.right  - rcDst.left) /
                       (float)(rcSrc.bottom  - rcSrc.top);
        scaleY      = (float)(rcDst.bottom - rcDst.top) /
                       (float)(rcSrc.right - rcSrc.left);
    }
}

MOS_STATUS VpRenderFcKernel::BuildFilter(
    VP_COMPOSITE_PARAMS             *compParams,
    PKdll_FilterEntry               pFilter,
    int32_t*                        piFilterSize)
{
    VP_FC_LAYER                 *src = nullptr;
    VPHAL_CSPACE                cspace_main = CSpace_sRGB;
    int32_t                     iMaxFilterSize = 0;
    bool                        bColorFill = false, bLumaKey = false;
    int32_t                     i = 0;
    PRECT                       pTargetRect = nullptr;
    RENDERHAL_SURFACE           RenderHalSurface = {};
    bool                        bNeed = false;
    int32_t                     procampCount = 0;
    float scaleX = 1.0;
    float scaleY = 1.0;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_waTable);

    VP_RENDER_CHK_NULL_RETURN(compParams);
    VP_RENDER_CHK_NULL_RETURN(pFilter);
    VP_RENDER_CHK_NULL_RETURN(piFilterSize);

    cspace_main    = CSpace_sRGB;                   // Default colorspace
    *piFilterSize  = 0;
    iMaxFilterSize = DL_MAX_SEARCH_FILTER_SIZE - 1; // Save one entry for Render Target
    pTargetRect    = &(compParams->target[0].surf->rcDst);

    // Initialize ColorFill flag
    bColorFill = (compParams->pColorFillParams != nullptr);

    for (i = 0; (i < (int)compParams->sourceCount) && (iMaxFilterSize > 0); i++)
    {
        src = &compParams->source[i];

        //--------------------------------
        // Skip non-visible layers
        //--------------------------------
        if (src->layerID < 0)
        {
            continue;
        }

        //--------------------------------
        // Composition path does not support conversion from BT2020 RGB to BT2020 YUV, BT2020->BT601/BT709, BT601/BT709 -> BT2020
        //--------------------------------
        if (IS_COLOR_SPACE_BT2020_RGB(src->surf->ColorSpace)   &&
            IS_COLOR_SPACE_BT2020_YUV(compParams->target[0].surf->ColorSpace))  //BT2020 RGB->BT2020 YUV
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
        else if (IS_COLOR_SPACE_BT2020(src->surf->ColorSpace) &&
                 !IS_COLOR_SPACE_BT2020(compParams->target[0].surf->ColorSpace)) //BT2020->BT601/BT709
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
        else if (!IS_COLOR_SPACE_BT2020(src->surf->ColorSpace) &&
                 IS_COLOR_SPACE_BT2020(compParams->target[0].surf->ColorSpace))  //BT601/BT709 -> BT2020
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }

        //--------------------------------
        // Set render method
        //--------------------------------
        pFilter->RenderMethod = RenderMethod_MediaObjectWalker;

        //--------------------------------
        // Set CSC coefficient setting method for CoeffID_0
        //--------------------------------
        pFilter->SetCSCCoeffMode = m_cscCoeffPatchModeEnabled ? SetCSCCoeffMethod_Patch : SetCSCCoeffMethod_Curbe;

        //--------------------------------
        // Set current layer
        //--------------------------------
        pFilter->layer = g_surfaceType_Layer[src->surf->SurfType];

        //--------------------------------
        // Set layer format
        //--------------------------------
        pFilter->format = src->surf->osSurface->Format;

        // On G8, NV12 format needs the width and Height to be a multiple of 4 for both
        // 3D sampler and 8x8 sampler; G75 needs the width of NV12 input surface to be
        // a multiple of 4 for 3D sampler; G9 does not has such restriction; to simplify the
        // implementation, we enable 2 plane NV12 for all of the platform when the width
        // or Height is not a multiple of 4. Here to set the filter format in order to select
        // the PL2 kernel when building the combined kernel.
        VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurface(src, &RenderHalSurface));
        bNeed = m_renderHal->pfnIs2PlaneNV12Needed(
                m_renderHal,
                &RenderHalSurface,
                RENDERHAL_SS_BOUNDARY_SRCRECT) ? true : false;

        if (bNeed)
        {
            if (pFilter->format == Format_NV12)
            {
                pFilter->format = Format_NV12_UnAligned;
            }
            else if (pFilter->format == Format_P208)
            {
                pFilter->format = Format_P208_UnAligned;
            }
            else if (pFilter->format == Format_NV11)
            {
                pFilter->format = Format_NV11_UnAligned;
            }
            else if (pFilter->format == Format_PL2)
            {
                pFilter->format = Format_PL2_UnAligned;
            }
        }

        // Y_Uoffset(Height*2 + Height/2) of RENDERHAL_PLANES_YV12 define Bitfield_Range(0, 13) on gen9+.
        // The max value is 16383. So use PL3 kernel to avoid out of range when Y_Uoffset is larger than 16383.
        // Use PL3 plane to avoid YV12 blending issue with DI enabled and U channel shift issue with not 4-aligned height
        if ((pFilter->format   == Format_YV12)           &&
            (src->scalingMode != VPHAL_SCALING_AVS)     &&
            (src->iefEnabled != true)                  &&
            (src->surf->SurfType != SURF_OUT_RENDERTARGET) &&
            m_renderHal->bEnableYV12SinglePass          &&
            !src->diParams                    &&
            !src->iscalingEnabled                    &&
            MOS_IS_ALIGNED(src->surf->osSurface->dwHeight, 4)            &&
            ((src->surf->osSurface->dwHeight * 2 + src->surf->osSurface->dwHeight / 2) < RENDERHAL_MAX_YV12_PLANE_Y_U_OFFSET_G9))
        {
            pFilter->format = Format_YV12_Planar;
        }

        if (pFilter->format == Format_A8R8G8B8 ||
            pFilter->format == Format_X8R8G8B8 ||
            pFilter->format == Format_A8B8G8R8 ||
            pFilter->format == Format_X8B8G8R8 ||
            pFilter->format == Format_R5G6B5)
        {
            pFilter->format = Format_RGB;
        }

        //--------------------------------
        // Set layer rotation
        //--------------------------------
        pFilter->rotation = src->rotation;

        //--------------------------------
        // Set layer color space
        //--------------------------------
        // Source is palletized, leave CSC to software (driver)
        if (IS_PAL_FORMAT(pFilter->format))
        {
            pFilter->cspace = CSpace_Any;
        }
        // Source is YUV or RGB, set primaries
        else
        {
            pFilter->cspace = src->surf->ColorSpace;
        }

        // Save color space of main video
        if (src->surf->SurfType == SURF_IN_PRIMARY)
        {
            cspace_main = pFilter->cspace;
        }

        //--------------------------------
        // Set sampling mode
        //--------------------------------
        bLumaKey = (src->lumaKeyParams != nullptr);

        // Progressive main video (except RGB format) or for RGB10, use AVS
        if (src->useSampleUnorm)
        {
            pFilter->sampler = (src->iscalingEnabled || src->fieldWeaving) ? Sample_iScaling : Sample_Scaling;
        }
        else
        {
            pFilter->sampler = (src->scalingMode == VPHAL_SCALING_AVS && !IsBobDiEnabled(src)) ?
                (src->iscalingEnabled ? Sample_iScaling_AVS : Sample_Scaling_AVS) :
                (src->iscalingEnabled || src->fieldWeaving) ? Sample_iScaling_034x : Sample_Scaling_034x;
        }

        // When input format is Format_R10G10B10A2/Format_B10G10R10A2/Y410(kernel regards Y410 as Format_R10G10B10A2)
        // Dscale kernel should be used
        if (src->surf->osSurface->Format == Format_R10G10B10A2 ||
            src->surf->osSurface->Format == Format_B10G10R10A2 ||
            src->surf->osSurface->Format == Format_Y410        ||
            src->surf->osSurface->Format == Format_Y416)
        {
            pFilter->bEnableDscale = true;
        }
        else
        {
            pFilter->bEnableDscale = false;
        }

        if (m_computeWalkerEnabled)
        {
            pFilter->bWaEnableDscale = true;
        }
        else
        {
            pFilter->bWaEnableDscale = MEDIA_IS_WA(m_hwInterface->m_waTable, WaEnableDscale);
        }

        //--------------------------------
        // Set Luma key
        //--------------------------------
        if (bLumaKey)
        {
            pFilter->lumakey = LumaKey_True;
            pFilter->samplerlumakey = src->useSamplerLumakey ? LumaKey_True : LumaKey_False;
        }
        else
        {
            pFilter->lumakey = LumaKey_False;
            pFilter->samplerlumakey = LumaKey_False;
        }

        //--------------------------------
        // Select function
        //--------------------------------
        if (src->blendingParams != nullptr)
        {
            switch (src->blendingParams->BlendType)
            {
                case BLEND_SOURCE:
                    if (IS_ALPHA4_FORMAT(src->surf->osSurface->Format))
                    {
                        pFilter->process = Process_SBlend_4bits;
                    }
                    else
                    {
                        pFilter->process = Process_SBlend;
                    }
                    break;

                case BLEND_PARTIAL:
                    pFilter->process = Process_PBlend;
                    break;

                case BLEND_CONSTANT:
                    pFilter->process = Process_CBlend;
                    break;

                case BLEND_CONSTANT_SOURCE:
                    pFilter->process = Process_CSBlend;
                    break;

                case BLEND_CONSTANT_PARTIAL:
                    pFilter->process = Process_CPBlend;
                    break;

                case BLEND_XOR_MONO:
                    pFilter->process = Process_XORComposite;
                    break;

                case BLEND_NONE:
                default:
                    pFilter->process = Process_Composite;
                    break;
            }
        }
        else
        {
            pFilter->process = Process_Composite;
        }

        if (pFilter->samplerlumakey && pFilter->process != Process_Composite)
        {
            VP_RENDER_ASSERTMESSAGE("Invalid kll processing for sampler lumakey! Sampler lumakey can only work with composition.");
            pFilter->samplerlumakey = LumaKey_False;
        }

        //--------------------------------
        // Set color fill
        //--------------------------------
        if (*piFilterSize == 0 &&
            (bLumaKey ||
             (bColorFill && (!RECT1_CONTAINS_RECT2(src->surf->rcDst, compParams->target[0].surf->rcDst))) ||
             ((pFilter->process == Process_PBlend) ||
              (pFilter->process == Process_CBlend) ||
              (pFilter->process == Process_SBlend) ||
              (pFilter->process == Process_CSBlend))))
        {
            pFilter->colorfill = ColorFill_True;
        }
        else
        {
            pFilter->colorfill = ColorFill_False;
        }

        //--------------------------------
        // Set Procamp parameters
        //--------------------------------
        if (src->procampParams && src->procampParams->bEnabled)
        {
            pFilter->procamp = 0;

            if (procampCount < VP_COMP_MAX_PROCAMP)
            {
                Kdll_Procamp procamp = {};
                procamp.iProcampVersion = m_Procamp[procampCount].iProcampVersion;
                procamp.bEnabled    =  true;
                procamp.fBrightness =  src->procampParams->fBrightness;
                procamp.fContrast   =  src->procampParams->fContrast  ;
                procamp.fHue        =  src->procampParams->fHue       ;
                procamp.fSaturation =  src->procampParams->fSaturation;

                // Update procamp version and values only if changed
                if (memcmp(&m_Procamp[procampCount], &procamp, sizeof(procamp)))
                {
                    ++procamp.iProcampVersion;
                    m_Procamp[procampCount] = procamp;
                }

                procampCount++;
            }
            else
            {
                VP_RENDER_ASSERTMESSAGE("procamp count exceed limit!");
                VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
        }
        else
        {
            pFilter->procamp = DL_PROCAMP_DISABLED;
        }

        //--------------------------------
        // Set chromasiting parameters
        //--------------------------------
        pFilter->chromasiting = DL_CHROMASITING_DISABLE;
        if (src->calculatedParams.chromaSitingEnabled)
        {
            pFilter->chromasiting = 0;
        }

        //--------------------------------
        // reset CSC
        //--------------------------------
        pFilter->matrix = DL_CSC_DISABLED;

        if (0 == src->layerID)
        {
            //set first layer's scalingRatio
            CalculateScale(scaleX, scaleY, src->surf->rcSrc, src->surf->rcDst, src->rotation);
            pFilter->ScalingRatio = m_hwInterface->m_vpPlatformInterface->GetKernelConfig().GetFilterScalingRatio(scaleX, scaleY);
        }

        // Update filter
        pFilter++;
        (*piFilterSize)++;
        iMaxFilterSize--;
    }

    //-----------------------------------------
    // Set Render Target parameters
    //-----------------------------------------
    if (compParams->targetCount == 2)
    {
        pFilter->dualout = true;
    }
    auto &target = compParams->target[0];
    pFilter->RenderMethod    = RenderMethod_MediaObjectWalker;
    pFilter->SetCSCCoeffMode = m_cscCoeffPatchModeEnabled ? SetCSCCoeffMethod_Patch : SetCSCCoeffMethod_Curbe;
    pFilter->layer    = Layer_RenderTarget;
    pFilter->format   = target.surf->osSurface->Format;
    pFilter->tiletype = target.surf->osSurface->TileType;
    pFilter->sampler  = Sample_None;
    pFilter->process  = Process_None;
    pFilter->procamp  = DL_PROCAMP_DISABLED;
    pFilter->matrix   = DL_CSC_DISABLED;
    pFilter->bFillOutputAlphaWithConstant = true;

    //set rendertarget's scalingRatio
    CalculateScale(scaleX, scaleY, target.surf->rcSrc, target.surf->rcDst, target.rotation);
    pFilter->ScalingRatio = m_hwInterface->m_vpPlatformInterface->GetKernelConfig().GetFilterScalingRatio(scaleX, scaleY);

    if (compParams->sourceCount > 0                                     &&
       compParams->source[0].surf->osSurface->Format == Format_R5G6B5   &&
       compParams->target[0].surf->osSurface->Format == Format_R5G6B5)
    {
        pFilter->bIsDitherNeeded = false;
    }else
    {
        pFilter->bIsDitherNeeded = true;
    }

    if (pFilter->format == Format_A8R8G8B8    ||
        pFilter->format == Format_A8B8G8R8    ||
        pFilter->format == Format_R10G10B10A2 ||
        pFilter->format == Format_B10G10R10A2 ||
        pFilter->format == Format_AYUV        ||
        pFilter->format == Format_Y416)
    {
        if (compParams->pCompAlpha != nullptr && compParams->sourceCount > 0 &&
            (compParams->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_NONE ||
             compParams->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM))
        {
            // When layer 0 does not have alpha channel, Save_RGB will be linked instead of
            // Save_ARGB, to avoid output alpha value corruption.
            switch (compParams->source[0].surf->osSurface->Format)
            {
                case Format_AYUV:
                case Format_AUYV:
                case Format_AI44:
                case Format_IA44:
                case Format_A8R8G8B8:
                case Format_A8B8G8R8:
                case Format_R10G10B10A2:
                case Format_B10G10R10A2:
                case Format_A8P8:
                case Format_A8:
                case Format_Y416:
                case Format_Y410:
                    pFilter->bFillOutputAlphaWithConstant = false;
                    break;

                default:
                    break;
            }
        }
    }

    //-------------------------------------------------------
    // Set color fill for RT. Valid for colorfill only cases
    //-------------------------------------------------------
    // If filter size is zero i.e. number of layers is zero, set colorfill to true.
    if (*piFilterSize == 0)
    {
        if(bColorFill)
        {
            pFilter->colorfill = ColorFill_True;
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
    }
    else
    {
        pFilter->colorfill = ColorFill_False;
    }

    // Get App supplied RT format
    pFilter->cspace = target.surf->ColorSpace;

    // Update filter
    (*piFilterSize)++;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::GetKernelEntry(Kdll_CacheEntry &entry)
{
    bool kernelEntryUpdate = false;
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;

    //============================
    // Create search filter for Dynamic Linking
    //============================
    MOS_ZeroMemory(m_searchFilter, sizeof(m_searchFilter));

    // Init search filter.
    Kdll_FilterEntry *filter = m_searchFilter;
    int32_t filterSize = 0;

    VP_RENDER_CHK_STATUS_RETURN(BuildFilter(
             &compParams,
             m_searchFilter,
             &filterSize));

    PrintSearchFilter(m_searchFilter, filterSize);

    //============================
    // KERNEL SEARCH
    //============================
    Kdll_State *kernelDllState = m_kernelDllState;

    VP_RENDER_CHK_NULL_RETURN(kernelDllState);

    uint32_t kernelHash = KernelDll_SimpleHash(filter, filterSize * sizeof(Kdll_FilterEntry));
    Kdll_CacheEntry *kernelEntry = KernelDll_GetCombinedKernel(kernelDllState, filter, filterSize, kernelHash);

    if (kernelEntry)
    {
        auto pCscParams = kernelEntry->pCscParams;
        // CoeffID_0 may not be used if no csc needed for both main video and RT.
        auto matrixId = (uint8_t)DL_CSC_DISABLED == pCscParams->MatrixID[CoeffID_0] ?
            pCscParams->MatrixID[CoeffID_1] : pCscParams->MatrixID[CoeffID_0];

        if ((uint8_t)DL_CSC_DISABLED != matrixId)
        {
            auto pMatrix    = &pCscParams->Matrix[matrixId];
            kernelDllState->colorfill_cspace = kernelEntry->colorfill_cspace;

            if ((pMatrix->iProcampID != DL_PROCAMP_DISABLED) &&
                (pMatrix->iProcampID < VP_MAX_PROCAMP))
            {
                kernelEntryUpdate = (m_Procamp[pMatrix->iProcampID].iProcampVersion != pMatrix->iProcampVersion) ? true : false;
            }
        }
    }

    if (!kernelEntry || kernelEntryUpdate)
    {
        Kdll_SearchState *pSearchState = &m_kernelSearch;

        // Remove kernel entry from kernel caches
        if (kernelEntryUpdate)
        {
            KernelDll_ReleaseHashEntry(&(kernelDllState->KernelHashTable), kernelEntry->wHashEntry);
            KernelDll_ReleaseCacheEntry(&(kernelDllState->KernelCache), kernelEntry);
        }

        // Setup kernel search
        kernelDllState->pfnStartKernelSearch(
            kernelDllState,
            pSearchState,
            m_searchFilter,
            filterSize,
            1);

        // Search kernel
        if (!kernelDllState->pfnSearchKernel(kernelDllState, pSearchState))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to find a kernel.");
            return MOS_STATUS_UNKNOWN;
        }

        // Build kernel
        if (!kernelDllState->pfnBuildKernel(kernelDllState, pSearchState))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to build kernel.");
            return MOS_STATUS_UNKNOWN;
        }

        // Load resulting kernel into kernel cache
        kernelEntry = KernelDll_AddKernel(
                           kernelDllState,
                           pSearchState,
                           m_searchFilter,
                           filterSize,
                           kernelHash);

        if (!kernelEntry)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to store kernel in local cache.");
            return MOS_STATUS_UNKNOWN;
        }
    }
    else
    {
        VPHAL_RENDER_NORMALMESSAGE("Use previous kernel list.");
    }
    m_kernelEntry = kernelEntry;
    entry = *kernelEntry;

    return MOS_STATUS_SUCCESS;
}

VPHAL_CHROMA_SUBSAMPLING VpRenderFcKernel::GetChromaSitting(VP_SURFACE &surf)
{
    VPHAL_CHROMA_SUBSAMPLING chromaSitingLocation = CHROMA_SUBSAMPLING_TOP_LEFT;

    if (nullptr == surf.osSurface)
    {
        return chromaSitingLocation;
    }

    // If there is no DDI setting, we use the Horizontal Left Vertical Center as default for PL2 surface.
    if (surf.ChromaSiting == CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (IS_PL2_FORMAT(surf.osSurface->Format) || IS_PL2_FORMAT_UnAligned(surf.osSurface->Format))
        {
            chromaSitingLocation = CHROMA_SUBSAMPLING_CENTER_LEFT;
        }
    }
    else
    {
        // PL2, 6 positions are avalibale
        if (IS_PL2_FORMAT(surf.osSurface->Format) || IS_PL2_FORMAT_UnAligned(surf.osSurface->Format))
        {
            // Horizontal Left
            if (surf.ChromaSiting & CHROMA_SITING_HORZ_LEFT)
            {
                if (surf.ChromaSiting & CHROMA_SITING_VERT_TOP)
                {
                    chromaSitingLocation = CHROMA_SUBSAMPLING_TOP_LEFT;
                }
                else if (surf.ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    chromaSitingLocation = CHROMA_SUBSAMPLING_CENTER_LEFT;
                }
                else if (surf.ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    chromaSitingLocation = CHROMA_SUBSAMPLING_BOTTOM_LEFT;
                }
            }
            // Horizontal Center
            else if (surf.ChromaSiting & CHROMA_SITING_HORZ_CENTER)
            {
                if (surf.ChromaSiting & CHROMA_SITING_VERT_TOP)
                {
                    chromaSitingLocation = CHROMA_SUBSAMPLING_TOP_CENTER;
                }
                else if (surf.ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    chromaSitingLocation = CHROMA_SUBSAMPLING_CENTER_CENTER;
                }
                else if (surf.ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    chromaSitingLocation = CHROMA_SUBSAMPLING_BOTTOM_CENTER;
                }
            }
        }
        else if (IS_PA_FORMAT(surf.osSurface->Format))
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (surf.ChromaSiting & (CHROMA_SITING_HORZ_CENTER))
            {
                chromaSitingLocation = CHROMA_SUBSAMPLING_TOP_CENTER;
            }
        }
    }

    return chromaSitingLocation;
}

//!
//! \brief    calculate the Horiz Gap and Vert Gap with different sample location
//! \param    [in] pTarget
//!           Pointer to Source Surface
//! \Param    [in] pHorzGap
//!           Pointer to Horzontal Gap
//! \Param    [in] pVertGap
//!           Pointer to Vertital Gap
//!
static MOS_STATUS GetOffsetChromasiting(
    VP_SURFACE                          &source,
    float                               &horizGap,
    float                               &vertGap
    )
{
    horizGap = 0.0f;
    vertGap  = 0.0f;

    // If there is no DDI setting, we use the Horizontal Left Vertical Center as default for PL2 surface.
    if (source.ChromaSiting == CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (IS_PL2_FORMAT(source.osSurface->Format) || IS_PL2_FORMAT_UnAligned(source.osSurface->Format))
        {
            vertGap = (float)(0.5f / source.osSurface->dwHeight);
        }
    }
    else
    {
        // PL2, 6 positions are available
        if (IS_PL2_FORMAT(source.osSurface->Format) || IS_PL2_FORMAT_UnAligned(source.osSurface->Format))
        {
            // Horizontal Left
            if (source.ChromaSiting & CHROMA_SITING_HORZ_LEFT)
            {
                if (source.ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    vertGap = (float)(0.5f / source.osSurface->dwHeight);
                }
                else if (source.ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    vertGap = (float)(1.0f / source.osSurface->dwHeight);
                }
            }
            // Horizontal Center
            else if (source.ChromaSiting & CHROMA_SITING_HORZ_CENTER)
            {
                horizGap = (float)(0.5f / source.osSurface->dwWidth);
                if (source.ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    vertGap = (float)(0.5f / source.osSurface->dwHeight);
                }
                else if (source.ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    vertGap = (float)(1.0f / source.osSurface->dwHeight);
                }
            }
        }
        else if (IS_PA_FORMAT(source.osSurface->Format))
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (source.ChromaSiting & (CHROMA_SITING_HORZ_CENTER))
            {
                horizGap = (float)(0.5f / source.osSurface->dwWidth);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Set3DSamplerStatus(
    VP_FC_LAYER                    &layer,
    VP_FC_CURBE_DATA               &curbeData)
{
    if (layer.layerID > 7)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid parameter.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (VPHAL_SCALING_AVS == layer.scalingMode)
    {                                              
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // When the bit being set to 1, the items of samplerIndex 4, 5, 6 will be used, otherwise
    // the items of samplerIndex 1, 2, 3 will be used.
    if (VPHAL_SCALING_BILINEAR == layer.scalingMode)
    {
        curbeData.DW14.Sampler3DStateSetSelection |= (1 << layer.layerID);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::InitCscInCurbeData()
{
    Kdll_CSC_Matrix *matrix        = nullptr;
    for (uint32_t i = 0; i < DL_CSC_MAX; i++)
    {
        if (m_kernelEntry->pCscParams->Matrix[i].iCoeffID == CoeffID_0)
        {
            matrix = &m_kernelEntry->pCscParams->Matrix[i];
            break;
        }
    }

    // Load CSC matrix
    if (matrix && matrix->bInUse && !m_cscCoeffPatchModeEnabled)
    {
        // Procamp is present
        if (matrix->iProcampID != DL_PROCAMP_DISABLED &&
            matrix->iProcampID < m_maxProcampEntries)
        {
            // Get Procamp parameter - update matrix only if Procamp is changed
            auto procamp = &m_Procamp[matrix->iProcampID];
            if (matrix->iProcampVersion != procamp->iProcampVersion)
            {
                KernelDll_UpdateCscCoefficients(m_kernelDllState, matrix);
            }
        }

        // CSC coeff from static parameter only applies to primary layer
        if (matrix->iCoeffID == CoeffID_0)
        {
            int16_t* pCoeff = matrix->Coeff;

            m_curbeData.DW00.CscConstantC0  = *(pCoeff++);
            m_curbeData.DW00.CscConstantC1  = *(pCoeff++);
            m_curbeData.DW01.CscConstantC2  = *(pCoeff++);
            m_curbeData.DW01.CscConstantC3  = *(pCoeff++);
            m_curbeData.DW02.CscConstantC4  = *(pCoeff++);
            m_curbeData.DW02.CscConstantC5  = *(pCoeff++);
            m_curbeData.DW03.CscConstantC6  = *(pCoeff++);
            m_curbeData.DW03.CscConstantC7  = *(pCoeff++);
            m_curbeData.DW04.CscConstantC8  = *(pCoeff++);
            m_curbeData.DW04.CscConstantC9  = *(pCoeff++);
            m_curbeData.DW05.CscConstantC10 = *(pCoeff++);
            m_curbeData.DW05.CscConstantC11 = *pCoeff;
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("CSC matrix coefficient id is non-zero.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::InitLayerInCurbeData(VP_FC_LAYER *layer)
{
    VP_PUBLIC_CHK_NULL_RETURN(layer);
    VP_RENDER_CHK_NULL_RETURN(layer->surfaceEntries[0]);

    float   horizgap = 0;
    float   vertgap = 0;
    uint32_t bitDepth = 0;
    uint16_t &alpha = layer->calculatedParams.alpha;
    float   &fStepX = layer->calculatedParams2.fStepX;
    float   &fStepY = layer->calculatedParams2.fStepY;
    float   &fOriginX = layer->calculatedParams2.fOriginX;
    float   &fOriginY = layer->calculatedParams2.fOriginY;
    RECT    &clipedDstRect = layer->calculatedParams.clipedDstRect; // Clipped dest rectangle

    if (layer->lumaKeyParams != nullptr)
    {
        VP_RENDER_NORMALMESSAGE("LumaLow %d, LumaHigh %d",
            layer->lumaKeyParams->LumaLow,
            layer->lumaKeyParams->LumaHigh);

        m_curbeData.DW14.LumakeyLowThreshold  = layer->lumaKeyParams->LumaLow;
        m_curbeData.DW14.LumakeyHighThreshold = layer->lumaKeyParams->LumaHigh;

    }

    if (layer->xorComp)
    {
        // set mono-chroma XOR composite specific curbe data. re-calculate fStep due to 1 bit = 1 pixel.
        m_curbeData.DW10.MonoXORCompositeMask = layer->surf->rcDst.left & 0x7;
    }

    VP_RENDER_NORMALMESSAGE("Scaling Info: layer %d, width %d, height, %d, rotation %d, alpha %d, shiftX %f, shiftY %f, scaleX %f, scaleY %f, offsetX %f, offsetY %f, stepX %f, stepY %f, originX %f, originY %f",
        layer->layerID, layer->surfaceEntries[0]->dwWidth, layer->surfaceEntries[0]->dwHeight, layer->rotation, alpha, layer->calculatedParams.fShiftX, layer->calculatedParams.fShiftY,
         layer->calculatedParams.fScaleX, layer->calculatedParams.fScaleY, layer->calculatedParams.fOffsetX, layer->calculatedParams.fOffsetY, fStepX, fStepY, fOriginX, fOriginY);

    VP_RENDER_NORMALMESSAGE("Scaling Info: layer %d, DestXTopLeft %d, DestYTopLeft %d, DestXBottomRight %d, DestYBottomRight %d",
        layer->layerID, clipedDstRect.left, clipedDstRect.top, clipedDstRect.right - 1, clipedDstRect.bottom - 1);

    VP_RENDER_NORMALMESSAGE("Scaling Info: layer %d, chromaSitingEnabled %d, isChromaUpSamplingNeeded %d, isChromaDownSamplingNeeded %d",
        layer->layerID, layer->calculatedParams.chromaSitingEnabled, layer->calculatedParams.isChromaUpSamplingNeeded, layer->calculatedParams.isChromaDownSamplingNeeded);

    switch (layer->layerID)
    {
    case 0:
        // Gen9+ uses HW based Rotation
        m_curbeData.DW10.RotationAngleofLayer0              = layer->rotation;
        m_curbeData.DW13.ColorFill_A                        = alpha;
        m_curbeData.DW16.HorizontalScalingStepRatioLayer0   = fStepX;
        m_curbeData.DW24.VerticalScalingStepRatioLayer0     = fStepY;
        m_curbeData.DW40.HorizontalFrameOriginLayer0        = fOriginX;
        m_curbeData.DW32.VerticalFrameOriginLayer0          = fOriginY;
        // GRF7.0
        m_curbeData.DW48.DestXTopLeftLayer0                 = clipedDstRect.left;
        m_curbeData.DW48.DestYTopLeftLayer0                 = clipedDstRect.top;
        // GRF8.0
        m_curbeData.DW56.DestXBottomRightLayer0             = clipedDstRect.right - 1;
        m_curbeData.DW56.DestYBottomRightLayer0             = clipedDstRect.bottom - 1;
        // GRF9.0
        m_curbeData.DW64.MainVideoXScalingStepLeft          = fStepX;

        // ChromasitingUOffset and ChromasitingVOffset are only for 3D Sampler use case
        horizgap = 0;
        vertgap = 0;
        GetOffsetChromasiting(*layer->surf, horizgap, vertgap);
        if (IS_PL2_FORMAT(layer->surf->osSurface->Format))
        {

            m_curbeData.DW11.ChromasitingUOffset = (float)((0.5f / (layer->surf->osSurface->dwWidth)) - horizgap);
            m_curbeData.DW12.ChromasitingVOffset = (float)((1.0f / (layer->surf->osSurface->dwHeight)) - vertgap);
        }
        else if (layer->surf->osSurface->Format == Format_YUY2)
        {
            m_curbeData.DW11.ChromasitingUOffset = (float)((1.0f / (layer->surf->osSurface->dwWidth)) - horizgap);
            m_curbeData.DW12.ChromasitingVOffset = (float)((0.5f / (layer->surf->osSurface->dwHeight)) - vertgap);
        }

        VP_RENDER_NORMALMESSAGE("Scaling Info: layer 0, ChromasitingUOffset %f, ChromasitingVOffset %f",
            m_curbeData.DW11.ChromasitingUOffset, m_curbeData.DW12.ChromasitingVOffset);

        // Set output depth.
        bitDepth = VpHal_GetSurfaceBitDepth(layer->surf->osSurface->Format);
        m_curbeData.DW07.OutputDepth    = VP_COMP_P010_DEPTH;
        if (bitDepth && !(layer->surf->osSurface->Format == Format_P010 || layer->surf->osSurface->Format == Format_Y210))
        {
            m_curbeData.DW07.OutputDepth = VP_COMP_SOURCE_DEPTH - bitDepth;
        }

        if (layer->iscalingEnabled)
        {
            m_curbeData.DW12.TopBottomDelta = (float)(1.0 / (layer->surf->rcSrc.bottom - layer->surf->rcSrc.top) -
                                                    1.0 / (layer->surf->rcDst.bottom - layer->surf->rcDst.top));
            if (layer->surf->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD ||
                layer->surf->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD)
            {
                //use the cropping size, not the surface size
                m_curbeData.DW12.TopBottomDelta = 0 - m_curbeData.DW12.TopBottomDelta;
            }
        }
        break;
    case 1:
        m_curbeData.DW10.RotationAngleofLayer1              = layer->rotation;
        m_curbeData.DW06.ConstantBlendingAlphaLayer1        = alpha;
        m_curbeData.DW17.HorizontalScalingStepRatioLayer1   = fStepX;
        m_curbeData.DW25.VerticalScalingStepRatioLayer1     = fStepY;
        m_curbeData.DW41.HorizontalFrameOriginLayer1        = fOriginX;
        m_curbeData.DW33.VerticalFrameOriginLayer1          = fOriginY;
        // GRF7.1
        m_curbeData.DW49.DestXTopLeftLayer1                 = clipedDstRect.left;
        m_curbeData.DW49.DestYTopLeftLayer1                 = clipedDstRect.top;
        // GRF8.1
        m_curbeData.DW57.DestXBottomRightLayer1             = clipedDstRect.right - 1;
        m_curbeData.DW57.DestYBottomRightLayer1             = clipedDstRect.bottom - 1;
        break;
    case 2:
        m_curbeData.DW10.RotationAngleofLayer2              = layer->rotation;
        m_curbeData.DW06.ConstantBlendingAlphaLayer2        = alpha;
        m_curbeData.DW18.HorizontalScalingStepRatioLayer2   = fStepX;
        m_curbeData.DW26.VerticalScalingStepRatioLayer2     = fStepY;
        m_curbeData.DW42.HorizontalFrameOriginLayer2        = fOriginX;
        m_curbeData.DW34.VerticalFrameOriginLayer2          = fOriginY;
        // GRF7.2
        m_curbeData.DW50.DestXTopLeftLayer2                 = clipedDstRect.left;
        m_curbeData.DW50.DestYTopLeftLayer2                 = clipedDstRect.top;
        // GRF8.2
        m_curbeData.DW58.DestXBottomRightLayer2             = clipedDstRect.right - 1;
        m_curbeData.DW58.DestYBottomRightLayer2             = clipedDstRect.bottom - 1;
        break;
    case 3:
        m_curbeData.DW10.RotationAngleofLayer3              = layer->rotation;
        m_curbeData.DW06.ConstantBlendingAlphaLayer3        = alpha;
        m_curbeData.DW19.HorizontalScalingStepRatioLayer3   = fStepX;
        m_curbeData.DW27.VerticalScalingStepRatioLayer3     = fStepY;
        m_curbeData.DW43.HorizontalFrameOriginLayer3        = fOriginX;
        m_curbeData.DW35.VerticalFrameOriginLayer3          = fOriginY;
        // GRF7.3
        m_curbeData.DW51.DestXTopLeftLayer3                 = clipedDstRect.left;
        m_curbeData.DW51.DestYTopLeftLayer3                 = clipedDstRect.top;
        // GRF8.3
        m_curbeData.DW59.DestXBottomRightLayer3             = clipedDstRect.right - 1;
        m_curbeData.DW59.DestYBottomRightLayer3             = clipedDstRect.bottom - 1;
        break;
    case 4:
        m_curbeData.DW10.RotationAngleofLayer4              = layer->rotation;
        m_curbeData.DW06.ConstantBlendingAlphaLayer4        = alpha;
        m_curbeData.DW20.HorizontalScalingStepRatioLayer4   = fStepX;
        m_curbeData.DW28.VerticalScalingStepRatioLayer4     = fStepY;
        m_curbeData.DW44.HorizontalFrameOriginLayer4        = fOriginX;
        m_curbeData.DW36.VerticalFrameOriginLayer4          = fOriginY;
        // GRF7.4
        m_curbeData.DW52.DestXTopLeftLayer4                 = clipedDstRect.left;
        m_curbeData.DW52.DestYTopLeftLayer4                 = clipedDstRect.top;
        // GRF8.4
        m_curbeData.DW60.DestXBottomRightLayer4             = clipedDstRect.right - 1;
        m_curbeData.DW60.DestYBottomRightLayer4             = clipedDstRect.bottom - 1;
        break;
    case 5:
        m_curbeData.DW10.RotationAngleofLayer5              = layer->rotation;
        m_curbeData.DW07.ConstantBlendingAlphaLayer5        = alpha;
        m_curbeData.DW21.HorizontalScalingStepRatioLayer5   = fStepX;
        m_curbeData.DW29.VerticalScalingStepRatioLayer5     = fStepY;
        m_curbeData.DW45.HorizontalFrameOriginLayer5        = fOriginX;
        m_curbeData.DW37.VerticalFrameOriginLayer5          = fOriginY;
        // GRF7.5
        m_curbeData.DW53.DestXTopLeftLayer5                 = clipedDstRect.left;
        m_curbeData.DW53.DestYTopLeftLayer5                 = clipedDstRect.top;
        // GRF8.5
        m_curbeData.DW61.DestXBottomRightLayer5             = clipedDstRect.right - 1;
        m_curbeData.DW61.DestYBottomRightLayer5             = clipedDstRect.bottom - 1;
        break;
    case 6:
        m_curbeData.DW10.RotationAngleofLayer6              = layer->rotation;
        m_curbeData.DW07.ConstantBlendingAlphaLayer6        = alpha;
        m_curbeData.DW22.HorizontalScalingStepRatioLayer6   = fStepX;
        m_curbeData.DW30.VerticalScalingStepRatioLayer6     = fStepY;
        m_curbeData.DW46.HorizontalFrameOriginLayer6        = fOriginX;
        m_curbeData.DW38.VerticalFrameOriginLayer6          = fOriginY;
        // GRF7.6
        m_curbeData.DW54.DestXTopLeftLayer6                 = clipedDstRect.left;
        m_curbeData.DW54.DestYTopLeftLayer6                 = clipedDstRect.top;
        // GRF8.6
        m_curbeData.DW62.DestXBottomRightLayer6             = clipedDstRect.right - 1;
        m_curbeData.DW62.DestYBottomRightLayer6             = clipedDstRect.bottom - 1;
        break;
    case 7:
        m_curbeData.DW10.RotationAngleofLayer7              = layer->rotation;
        m_curbeData.DW07.ConstantBlendingAlphaLayer7        = alpha;
        m_curbeData.DW23.HorizontalScalingStepRatioLayer7   = fStepX;
        m_curbeData.DW31.VerticalScalingStepRatioLayer7     = fStepY;
        m_curbeData.DW47.HorizontalFrameOriginLayer7        = fOriginX;
        m_curbeData.DW39.VerticalFrameOriginLayer7          = fOriginY;
        // GRF7.7
        m_curbeData.DW55.DestXTopLeftLayer7                 = clipedDstRect.left;
        m_curbeData.DW55.DestYTopLeftLayer7                 = clipedDstRect.top;
        // GRF8.7
        m_curbeData.DW63.DestXBottomRightLayer7             = clipedDstRect.right - 1;
        m_curbeData.DW63.DestYBottomRightLayer7             = clipedDstRect.bottom - 1;
        break;
    default:
        VPHAL_RENDER_ASSERTMESSAGE("Invalid layer.");
        break;
    }

    Set3DSamplerStatus(*layer, m_curbeData);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::InitFcCurbeData()
{
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;

    MOS_ZeroMemory(&m_curbeData, sizeof(m_curbeData));
    m_curbeData.DW07.PointerToInlineParameters = 7;

    uint32_t destRectWidth = 0, destRectHeight = 0;

    // set destination width and height
    destRectWidth  = compParams.target[0].surf->osSurface->dwWidth;
    destRectHeight = compParams.target[0].surf->osSurface->dwHeight;

    m_curbeData.DW08.DestinationRectangleWidth  = destRectWidth;
    m_curbeData.DW08.DestinationRectangleHeight = destRectHeight;

    uint32_t layerCount = 0;
    for (uint32_t i = 0; i < compParams.sourceCount; ++i)
    {
        auto &layer = compParams.source[i];

        if (-1 == layer.layerID)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        VP_PUBLIC_CHK_STATUS_RETURN(InitLayerInCurbeData(&layer));
    }

    // Normalize scaling factors for all layers
    // Ratio of Horizontal Scaling Step to Video X Scaling Step
    // Since NLAS is ZBBed, CM FC kernels simplified scaling factor calculation, no need to normalize here
    if (!m_kernelDllState->bEnableCMFC)
    {
        float stepXLayer0 = m_curbeData.DW16.HorizontalScalingStepRatioLayer0;
        m_curbeData.DW16.HorizontalScalingStepRatioLayer0 = 1;
        m_curbeData.DW17.HorizontalScalingStepRatioLayer1 /= stepXLayer0;
        m_curbeData.DW18.HorizontalScalingStepRatioLayer2 /= stepXLayer0;
        m_curbeData.DW19.HorizontalScalingStepRatioLayer3 /= stepXLayer0;
        m_curbeData.DW20.HorizontalScalingStepRatioLayer4 /= stepXLayer0;
        m_curbeData.DW21.HorizontalScalingStepRatioLayer5 /= stepXLayer0;
        m_curbeData.DW22.HorizontalScalingStepRatioLayer6 /= stepXLayer0;
        m_curbeData.DW23.HorizontalScalingStepRatioLayer7 /= stepXLayer0;
    }

    auto &target = compParams.target[0];

    // Set ChromaSitting
    m_curbeData.DW10.ChromaSitingLocation = GetChromaSitting(*target.surf);

    if (compParams.sourceCount > 0)
    {
        // "IEF Bypass" bit is changed to "MBZ" bit for Gen12 in HW interface,  so driver should always set "Bypass IEF" to be 0 in CURBE.
        m_curbeData.DW09.IEFByPassEnable = false;
    }

    // Set alpha calculation flag. The bit definitions are different for GEN8 and GEN9+.
    // Set Bit-18
    m_curbeData.DW09.AlphaChannelCalculation = compParams.bAlphaCalculateEnable ? true : false;
    // Set flag to swap R and B in Save_RGB/ARGB if target format is Format_A8B8G8R8/Format_X8B8G8R8/Format_B10G10R10A2.
    // No need for RGBP/BGRP, since they are 3 plane format, kenel change the RB channel by different plane order
    m_curbeData.DW09.ChannelSwap = ((target.surf->osSurface->Format == Format_A8B8G8R8) ||
                                 (target.surf->osSurface->Format == Format_X8B8G8R8) ||
                                 (target.surf->osSurface->Format == Format_B10G10R10A2)) ? 1 : 0;

    VP_PUBLIC_CHK_STATUS_RETURN(InitCscInCurbeData());
    VP_PUBLIC_CHK_STATUS_RETURN(InitColorFillInCurbeData());
    VP_PUBLIC_CHK_STATUS_RETURN(InitOutputFormatInCurbeData());

    if (compParams.targetCount > 1)
    {
        m_curbeData.DW09.DualOutputMode = 1;
    }

    // GRF 9.1-4
    m_curbeData.DW65.VideoStepDeltaForNonLinearRegion            = 0;
    m_curbeData.DW66.StartofLinearScalingInPixelPositionC0       = 0;
    m_curbeData.DW66.StartofRHSNonLinearScalingInPixelPositionC1 = 0;
    m_curbeData.DW67.MainVideoXScalingStepCenter                 = 0;
    m_curbeData.DW68.MainVideoXScalingStepRight                  = 0;

    if (compParams.targetCount > 1)
    {
        // Horizontal and Vertical base on non-rotated in case of dual output
        m_curbeData.DW69.DestHorizontalBlockOrigin               =
            (uint16_t)compParams.target[1].surf->rcDst.left;
        m_curbeData.DW69.DestVerticalBlockOrigin                 =
            (uint16_t)compParams.target[1].surf->rcDst.top;
    }
    else
    {
        m_curbeData.DW69.DestHorizontalBlockOrigin               =
                (uint16_t)compParams.target[0].surf->rcDst.left;
        m_curbeData.DW69.DestVerticalBlockOrigin                 =
                (uint16_t)compParams.target[0].surf->rcDst.top;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::InitColorFillInCurbeData()
{
    Kdll_FilterEntry        *filter     = m_kernelEntry->pFilter;
    int32_t                 filterSize  = m_kernelEntry->iFilterSize;
    int32_t                 i           = 0;
    VPHAL_COLOR_SAMPLE_8    srcColor    = {};
    MEDIA_CSPACE            srcCspace   = CSpace_None;
    MEDIA_CSPACE            dstCspace   = CSpace_None;
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;
    // Set Background color (use cspace of first layer)
    if (compParams.pColorFillParams)
    {
        srcColor.dwValue = compParams.pColorFillParams->Color;

        // get src and dst colorspaces
        srcCspace = compParams.pColorFillParams->CSpace;

        // if iscale enabled, set colorspace to render target color space
        if (filter->sampler == Sample_iScaling || filter->sampler == Sample_iScaling_034x || filter->sampler == Sample_iScaling_AVS)
        {
            dstCspace = CSpace_None;
            // find the filter of render target and set dstCspace to render target color space
            if (Layer_RenderTarget != filter[filterSize - 1].layer)
            {
                VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
            dstCspace = filter[filterSize - 1].cspace;

            if (dstCspace == CSpace_None) // if color space is invlaid return false
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to assign dst color spcae for iScale case.");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else // use selected cspace by kdll
        {
            dstCspace = m_kernelDllState->colorfill_cspace;
        }

        // Convert BG color only if not done before. CSC is expensive!
        if ((m_srcColor.dwValue != srcColor.dwValue) ||
            (m_srcCspace     != srcCspace)  ||
            (m_dstCspace     != dstCspace))
        {
            VpHal_CSC_8(&m_dstColor, &srcColor, srcCspace, dstCspace);

            // store the values for next iteration
            m_srcColor     = srcColor;
            m_srcCspace = srcCspace;
            m_dstCspace = dstCspace;
        }

        // Set BG color
        if (KernelDll_IsCspace(dstCspace, CSpace_RGB))
        {
            m_curbeData.DW13.ColorFill_R = m_dstColor.R;
            m_curbeData.DW13.ColorFill_G = m_dstColor.G;
            m_curbeData.DW13.ColorFill_B = m_dstColor.B;
        }
        else
        {
            m_curbeData.DW13.ColorFill_Y = m_dstColor.Y;
            m_curbeData.DW13.ColorFill_U = m_dstColor.U;
            m_curbeData.DW13.ColorFill_V = m_dstColor.V;
        }
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Set packed YUV component offsets
//! \details  Accoring to the format of the surface, set packed YUV component offsets
//! \param    [in] format
//!           The format of the surface
//! \param    [in,out] pOffsetY
//!           The offset of Y
//! \param    [in,out] pOffsetU
//!           The offset of U
//! \param    [in,out] pOffsetV
//!           The offset of V
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpSetYUVComponents(
    MOS_FORMAT      format,
    uint8_t*        pOffsetY,
    uint8_t*        pOffsetU,
    uint8_t*        pOffsetV)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    switch (format)
    {
        case Format_PA:
        case Format_YUY2:
        case Format_YUYV:
            *pOffsetY = 0;
            *pOffsetU = 1;
            *pOffsetV = 3;
            break;

        case Format_UYVY:
            *pOffsetY = 1;
            *pOffsetU = 0;
            *pOffsetV = 2;
            break;

        case Format_YVYU:
            *pOffsetY = 0;
            *pOffsetU = 3;
            *pOffsetV = 1;
            break;

        case Format_VYUY:
            *pOffsetY = 1;
            *pOffsetU = 2;
            *pOffsetV = 0;
            break;

        case Format_Y210:
            *pOffsetY = 0;
            *pOffsetU = 2;
            *pOffsetV = 6;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Unknown Packed YUV Format.");
            eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

/*
|    |---------------------------------------------------------------------|
|    |                      Alpha fill mode table                          |
|    |---------------------------------------------------------------------|
|    |                      ALPHA_FILL_MODE_NONE                           |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |      Save_ARGB        |
|    |      No Alpha        |        Has Alpha     |Save_RGB(ALpha frm app)|
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
|    |                    ALPHA_FILL_MODE_OPAQUE                           |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |    Save_RGB(0xff)     |
|    |      No Alpha        |        Has Alpha     |    Save_RGB(0xff)     |
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
|    |                   ALPHA_FILL_MODE_BACKGROUND                        |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |  Save_RGB(BG Alpha)   |
|    |      No Alpha        |        Has Alpha     |  Save_RGB(BG Alpha)   |
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
|    |                  ALPHA_FILL_MODE_SOURCE_STREAM                      |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |      Save_ARGB        |
|    |      No Alpha        |        Has Alpha     |    Save_RGB(0xff)     |
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
*/

MOS_STATUS VpRenderFcKernel::InitOutputFormatInCurbeData()
{
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;
    // Set output format
    MOS_FORMAT              outputFormat = compParams.target[0].surf->osSurface->Format;
    Kdll_FilterEntry        *filter     = m_kernelEntry->pFilter;
    int32_t                 filterSize  = m_kernelEntry->iFilterSize;

    filter = filter + filterSize - 1;

    if (Layer_RenderTarget != filter->layer)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    if (IS_PA_FORMAT(outputFormat)  &&
        outputFormat != Format_Y410 &&
        outputFormat != Format_Y416)
    {
        VpSetYUVComponents(
            outputFormat,
            &(m_curbeData.DW15.DestinationPackedYOffset),
            &(m_curbeData.DW15.DestinationPackedUOffset),
            &(m_curbeData.DW15.DestinationPackedVOffset));
    }
    else if (filter->bFillOutputAlphaWithConstant && compParams.pCompAlpha != nullptr)
    {
        switch (compParams.pCompAlpha->AlphaMode)
        {
            case VPHAL_ALPHA_FILL_MODE_NONE:
                if (filter->format == Format_A8R8G8B8    ||
                    filter->format == Format_A8B8G8R8    ||
                    filter->format == Format_R10G10B10A2 ||
                    filter->format == Format_B10G10R10A2 ||
                    filter->format == Format_AYUV        ||
                    filter->format == Format_Y410        ||
                    filter->format == Format_Y416)
                {
                    m_curbeData.DW15.DestinationRGBFormat = (uint8_t)(0xff * compParams.pCompAlpha->fAlpha);
                }
                else
                {
                    m_curbeData.DW15.DestinationRGBFormat = 0xff;
                }
                // For color fill only case, pass through alpha value
                if (compParams.pColorFillParams && 0 == compParams.sourceCount)
                {
                    m_curbeData.DW15.DestinationRGBFormat = m_dstColor.A;
                }
                break;

            case VPHAL_ALPHA_FILL_MODE_BACKGROUND:
                m_curbeData.DW15.DestinationRGBFormat = m_dstColor.A;
                break;

            // VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM case is hit when the input does not have alpha
            // So we set Opaque alpha channel.
            case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
            case VPHAL_ALPHA_FILL_MODE_OPAQUE:
            default:
                m_curbeData.DW15.DestinationRGBFormat = 0xff;
                break;
        }
    }
    else
    {
        m_curbeData.DW15.DestinationRGBFormat = 0xff;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::InitCscInDpCurbeData()
{
    Kdll_CSC_Matrix *matrix        = nullptr;
    for (uint32_t i = 0; i < DL_CSC_MAX; i++)
    {
        if (m_kernelEntry->pCscParams->Matrix[i].iCoeffID == CoeffID_0)
        {
            matrix = &m_kernelEntry->pCscParams->Matrix[i];
            break;
        }
    }

    // Load CSC matrix
    if (matrix && matrix->bInUse && !m_cscCoeffPatchModeEnabled)
    {
        // Procamp is present
        if (matrix->iProcampID != DL_PROCAMP_DISABLED &&
            matrix->iProcampID < m_maxProcampEntries)
        {
            // Get Procamp parameter - update matrix only if Procamp is changed
            auto procamp = &m_Procamp[matrix->iProcampID];
            if (matrix->iProcampVersion != procamp->iProcampVersion)
            {
                KernelDll_UpdateCscCoefficients(m_kernelDllState, matrix);
            }
        }

        // CSC coeff from static parameter only applies to primary layer
        if (matrix->iCoeffID == CoeffID_0)
        {
            int16_t* pCoeff = matrix->Coeff;

            m_curbeDataDp.DW0.CscConstantC0  = *(pCoeff++);
            m_curbeDataDp.DW0.CscConstantC1  = *(pCoeff++);

            m_curbeDataDp.DW1.CscConstantC2  = *(pCoeff++);
            m_curbeDataDp.DW1.CscConstantC3  = *(pCoeff++);
            m_curbeDataDp.DW2.CscConstantC4  = *(pCoeff++);
            m_curbeDataDp.DW2.CscConstantC5  = *(pCoeff++);
            m_curbeDataDp.DW3.CscConstantC6  = *(pCoeff++);
            m_curbeDataDp.DW3.CscConstantC7  = *(pCoeff++);
            m_curbeDataDp.DW4.CscConstantC8  = *(pCoeff++);
            m_curbeDataDp.DW4.CscConstantC9  = *(pCoeff++);
            m_curbeDataDp.DW5.CscConstantC10 = *(pCoeff++);
            m_curbeDataDp.DW5.CscConstantC11 = *pCoeff;
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("CSC matrix coefficient id is non-zero.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::InitFcDpBasedCurbeData()
{
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;

    if (compParams.sourceCount != 1 || compParams.targetCount != 1)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    MOS_ZeroMemory(&m_curbeDataDp, sizeof(m_curbeDataDp));

    uint32_t destRectWidth = 0, destRectHeight = 0;

    // set destination width and height
    destRectWidth  = compParams.target[0].surf->osSurface->dwWidth;
    destRectHeight = compParams.target[0].surf->osSurface->dwHeight;

    auto &layer = compParams.source[0];
    layer.layerID = 0;

    VP_RENDER_CHK_NULL_RETURN(layer.surfaceEntries[0]);

    if (layer.numOfSurfaceEntries > 0)
    {
        m_curbeDataDp.DW6.InputPictureWidth  = layer.surfaceEntries[0]->dwWidth -1;
        m_curbeDataDp.DW6.InputPictureHeight = layer.surfaceEntries[0]->dwHeight -1;
    }

    float   &fStepX = layer.calculatedParams2.fStepX;
    float   &fStepY = layer.calculatedParams2.fStepY;
    float   &fOriginX = layer.calculatedParams2.fOriginX;
    float   &fOriginY = layer.calculatedParams2.fOriginY;
    RECT    &clipedDstRect = layer.calculatedParams.clipedDstRect; // Clipped dest rectangle

    m_curbeDataDp.DW7.DestinationRectangleWidth = destRectWidth;
    m_curbeDataDp.DW7.DestinationRectangleHeight = destRectHeight;
    m_curbeDataDp.DW9.HorizontalScalingStepRatioLayer0 = m_kernelDllState->bEnableCMFC ? fStepX : 1;
    m_curbeDataDp.DW10.VerticalScalingStepRatioLayer0 = fStepY;
    m_curbeDataDp.DW11.HorizontalFrameOriginLayer0 = fOriginX;
    m_curbeDataDp.DW12.VerticalFrameOriginLayer0 = fOriginY;
    // GRF7.0
    m_curbeDataDp.DW13.DestXTopLeftLayer0       = clipedDstRect.left;
    m_curbeDataDp.DW13.DestYTopLeftLayer0       = clipedDstRect.top;
    // GRF8.0
    m_curbeDataDp.DW14.DestXBottomRightLayer0   = clipedDstRect.right - 1;
    m_curbeDataDp.DW14.DestYBottomRightLayer0   = clipedDstRect.bottom - 1;

    // Load CSC matrix
    VP_RENDER_CHK_STATUS_RETURN(InitCscInDpCurbeData());

    auto target = compParams.target[0];

    if (target.surf->osSurface->Format == Format_A8R8G8B8)
    {
       m_curbeDataDp.DW15.waFlag = 1;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::GetCurbeState(void*& curbe, uint32_t& curbeLength)
{
    VP_FUNC_CALL();

    if (m_hwInterface->m_vpPlatformInterface->GetKernelConfig().IsDpFcKernelEnabled())
    {
        VP_RENDER_CHK_STATUS_RETURN(InitFcDpBasedCurbeData());
        // DataPort used FC kernel case
        curbe       = &m_curbeDataDp;
        curbeLength = sizeof(VP_FC_DP_BASED_CURBE_DATA);
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(InitFcCurbeData());
        curbe       = &m_curbeData;
        curbeLength = sizeof(VP_FC_CURBE_DATA);
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t VpRenderFcKernel::GetInlineDataSize()
{
    VP_FUNC_CALL();
    int32_t inlineDataSize = 0;
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;

    // Set Inline Data Size
    if (compParams.sourceCount <= 3)
    {
        // compParams.sourceCount == 0 case is only for colorfill only cases.
        // Colorfill uses inverted layer 0 block mask to determine colorfill region.
        inlineDataSize = 8 * sizeof(uint32_t);
    }
    else if (compParams.sourceCount <= 8)
    {
        inlineDataSize = (compParams.sourceCount + 5) * sizeof(uint32_t);
    }
    else
    {
        VP_RENDER_ASSERTMESSAGE("%s, Invalid Number of Layers.");
    }

    return inlineDataSize;
}

MOS_STATUS VpRenderFcKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS& walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{
    VP_FUNC_CALL();

    MOS_ZeroMemory(&walkerParam, sizeof(walkerParam));

    walkerParam.iBindingTable   = renderData.bindingTable;
    walkerParam.iMediaID        = renderData.mediaID;
    walkerParam.iCurbeOffset    = renderData.iCurbeOffset;
    walkerParam.iCurbeLength    = renderData.iCurbeLength;
    // iBlocksX/iBlocksY will be calculated during prepare walker parameters in RenderCmdPacket
    walkerParam.calculateBlockXYByAlignedRect = true;

    if (0 == m_fcParams->compParams.targetCount || m_fcParams->compParams.targetCount > 1)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    walkerParam.alignedRect = m_fcParams->compParams.target[0].surf->rcDst;

    if (m_fcParams->compParams.sourceCount == 1                                         &&
        m_fcParams->compParams.source[0].surf->osSurface->TileType == MOS_TILE_LINEAR   &&
       (m_fcParams->compParams.source[0].rotation == VPHAL_ROTATION_90                  ||
        m_fcParams->compParams.source[0].rotation == VPHAL_ROTATION_270))
    {
        walkerParam.isVerticalPattern = true;
    }

    walkerParam.bSyncFlag = 0;
    walkerParam.isGroupStartInvolvedInGroupSize = true;

    return MOS_STATUS_SUCCESS;
}

bool VpRenderFcKernel::IsEufusionBypassed()
{
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;
    VpUserFeatureControl *userFeatureControl = m_hwInterface->m_userFeatureControl;

    if (nullptr == userFeatureControl || !userFeatureControl->IsEufusionBypassWaEnabled())
    {
        return false;
    }

    if (compParams.sourceCount > 1)
    {
        return true;
    }
    else if (1 == compParams.sourceCount)
    {
        VP_FC_LAYER &layer = compParams.source[0];
        bool bColorFill = false;
        bool bRotation  = false;
        if (compParams.pColorFillParams != nullptr)
        {
            // To avoid colorfill + rotation output cropution when Eu fusion is on.
            bColorFill =  (!RECT1_CONTAINS_RECT2(layer.surf->rcSrc, compParams.target->surf->rcDst)) ? true : false;
            bRotation = (layer.rotation != VPHAL_ROTATION_IDENTITY) ? true : false;
            m_renderHal->eufusionBypass = bColorFill && bRotation;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return false;
}

MOS_STATUS VpRenderFcKernel::UpdateCompParams()
{
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;

    m_renderHal->eufusionBypass = IsEufusionBypassed();

    VP_RENDER_NORMALMESSAGE("eufusionBypass = %d", m_renderHal->eufusionBypass ? 1 : 0);

    for (uint32_t i = 0; i < compParams.sourceCount; ++i)
    {
        VP_FC_LAYER &layer = compParams.source[i];
        auto &params = layer.calculatedParams;
        auto &params2 = layer.calculatedParams2;

        if (nullptr == layer.surfaceEntries[0])
        {
            VP_RENDER_ASSERTMESSAGE("layer.surfaceEntries[0] == nullptr!");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
        }

        if (0 == layer.surfaceEntries[0]->dwWidth || 0 == layer.surfaceEntries[0]->dwWidth)
        {
            VP_RENDER_ASSERTMESSAGE("width or height in surface entry is 0!");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        // Frame origins for the current layer
        params2.fOriginX = (params.fOffsetX + params.fShiftX * params2.fStepX) / layer.surfaceEntries[0]->dwWidth;
        params2.fOriginY = (params.fOffsetY + params.fShiftY * params2.fStepY) / layer.surfaceEntries[0]->dwHeight;

        // Normalized block step for the current layer (block increment)
        params2.fStepX /= layer.surfaceEntries[0]->dwWidth;
        params2.fStepY /= layer.surfaceEntries[0]->dwHeight;

        if (layer.xorComp)
        {
            // set mono-chroma XOR composite specific curbe data. re-calculate fStep due to 1 bit = 1 pixel.
            params2.fStepX /= 8;
            params2.fOriginX /= 8;
        }
    }

    return MOS_STATUS_SUCCESS;
}


#define VP_SAMPLER_INDEX_Y_NEAREST                  1
#define VP_SAMPLER_INDEX_U_NEAREST                  2
#define VP_SAMPLER_INDEX_V_NEAREST                  3

#define VP_SAMPLER_INDEX_Y_BILINEAR                 4
#define VP_SAMPLER_INDEX_U_BILINEAR                 5
#define VP_SAMPLER_INDEX_V_BILINEAR                 6

MOS_STATUS VpRenderFcKernel::GetSamplerIndex(
    VPHAL_SCALING_MODE                  scalingMode,
    uint32_t                            yuvPlane,
    int32_t                             &samplerIndex,
    MHW_SAMPLER_TYPE                    &samplerType)
{
    const int32_t samplerindex[2][3] = { {VP_SAMPLER_INDEX_Y_NEAREST, VP_SAMPLER_INDEX_U_NEAREST, VP_SAMPLER_INDEX_V_NEAREST },
                                         {VP_SAMPLER_INDEX_Y_BILINEAR, VP_SAMPLER_INDEX_U_BILINEAR, VP_SAMPLER_INDEX_V_BILINEAR}};

    if (scalingMode == VPHAL_SCALING_AVS)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // if Scalingmode is BILINEAR, use the 4,5,6. if NEAREST, use 1,2,3
    samplerType = MHW_SAMPLER_TYPE_3D;
    samplerIndex = samplerindex[scalingMode][yuvPlane];

    return MOS_STATUS_SUCCESS;
}

// Need be called after SetupSurfaceState.
MOS_STATUS VpRenderFcKernel::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup)
{
    VP_COMPOSITE_PARAMS &compParams = m_fcParams->compParams;

    samplerStateGroup.clear();

    for (uint32_t i = 0; i < compParams.sourceCount; ++i)
    {
        auto &layer = compParams.source[i];

        if (0 == layer.numOfSurfaceEntries)
        {
            VP_RENDER_ASSERTMESSAGE("0 == layer.numOfSurfaceEntries!");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        VP_RENDER_CHK_NULL_RETURN(layer.surf);
        VP_RENDER_CHK_NULL_RETURN(layer.surf->osSurface);

        for (uint32_t entryIndex = 0; entryIndex < layer.numOfSurfaceEntries; ++entryIndex)
        {
            int32_t                         samplerIndex        = 0;
            MHW_SAMPLER_TYPE                samplerType         = MHW_SAMPLER_TYPE_INVALID;
            PRENDERHAL_SURFACE_STATE_ENTRY  entry               = layer.surfaceEntries[entryIndex];
            MHW_SAMPLER_STATE_PARAM         samplerStateParam   = {};

            VP_RENDER_CHK_NULL_RETURN(entry);

            // Obtain Sampler ID and Type
            VP_RENDER_CHK_STATUS_RETURN(GetSamplerIndex(layer.scalingMode,
                                      entry->YUVPlane,
                                      samplerIndex,
                                      samplerType));

            if (samplerType != MHW_SAMPLER_TYPE_3D)
            {
                VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }

            samplerStateParam.bInUse                    = true;
            samplerStateParam.SamplerType               = samplerType;
            samplerStateParam.Unorm.SamplerFilterMode   = layer.calculatedParams.samplerFilterMode;
            samplerStateParam.Unorm.AddressU            = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            samplerStateParam.Unorm.AddressV            = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            samplerStateParam.Unorm.AddressW            = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;

            samplerStateGroup.insert(std::make_pair(samplerIndex, samplerStateParam));

            VP_RENDER_NORMALMESSAGE("Scaling Info: layer %d, layerOrigin %d, entry %d, format %d, scalingMode %d, samplerType %d, samplerFilterMode %d, samplerIndex %d, yuvPlane %d",
                layer.layerID, layer.layerIDOrigin, entryIndex, layer.surf->osSurface->Format, layer.scalingMode, samplerType, samplerStateParam.Unorm.SamplerFilterMode, samplerIndex, entry->YUVPlane);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderFcKernel::SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs)
{
    VP_FUNC_CALL();

    if (m_fcParams == nullptr)
    {
        m_fcParams = (PRENDER_FC_PARAMS)MOS_AllocAndZeroMemory(sizeof(RENDER_FC_PARAMS));
    }

    PRENDER_FC_PARAMS fcParams = nullptr;
    if (kernelConfigs.find(m_kernelId) != kernelConfigs.end())
    {
        fcParams = (PRENDER_FC_PARAMS)kernelConfigs.find(m_kernelId)->second;
    }

    VP_RENDER_CHK_NULL_RETURN(fcParams);

    MOS_SecureMemcpy(m_fcParams, sizeof(RENDER_FC_PARAMS), fcParams, sizeof(RENDER_FC_PARAMS));

    return MOS_STATUS_SUCCESS;
}
