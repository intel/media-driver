/*
* Copyright (c) 2018, Intel Corporation
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
//! \file      cm_surface_state.cpp
//! \brief     Contains Class CmSurfaceState  definitions
//!

#include "cm_surface_state.h"
#include "cm_surface_state_manager.h"
#include "renderhal_platform_interface.h"

uint16_t RenderHal_CalculateYOffset(PMOS_INTERFACE pOsInterface, PMOS_RESOURCE pOsResource);

extern const MHW_SURFACE_PLANES g_cRenderHal_SurfacePlanes[RENDERHAL_PLANES_DEFINITION_COUNT];

const uint32_t g_cLookup_RotationMode[8] =
{
    ROTATION_IDENTITY,  // 0 - MHW_ROTATION_IDENTITY
    ROTATION_90,        // 1 - MHW_ROTATION_90
    ROTATION_180,       // 2 - MHW_ROTATION_180
    ROTATION_270,       // 3 - MHW_ROTATION_270
    ROTATION_IDENTITY,  // 4 - MHW_MIRROR_HORIZONTAL
    ROTATION_180,       // 5 - MHW_MIRROR_VERTICAL
    ROTATION_270,       // 6 - MHW_ROTATE_90_MIRROR_VERTICAL
    ROTATION_90         // 7 - MHW_ROTATE_90_MIRROR_HORIZONTAL
};

CmSurfaceState::CmSurfaceState(CM_HAL_STATE *cmhal):
    m_cmhal(cmhal),
    m_renderhal(nullptr),
    m_resource(nullptr),
    m_memoryObjectControl(0),
    m_ssh(nullptr),
    m_btIdx(-1),
    m_bteIdx(-1),
    m_ssIdx(-1)
{
    m_resourceData = {0};
}

CM_RETURN_CODE CmSurfaceState::Initialize(MOS_RESOURCE *resource)
{
    if (m_cmhal)
    {
        m_renderhal = m_cmhal->renderHal;
    }
    if (!m_renderhal)
    {
        return CM_NULL_POINTER;
    }
    m_resource = resource;
    return CM_SUCCESS;
}

uint32_t CmSurfaceState::GetCacheabilityControl()
{
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));
    m_cmhal->cmHalInterface->HwSetSurfaceMemoryObjectControl(m_memoryObjectControl, &surfaceParam);
    return surfaceParam.MemObjCtl;
}

CmSurfaceState2Dor3D::CmSurfaceState2Dor3D(CM_HAL_STATE *cmhal):
    //m_device(device),
    CmSurfaceState(cmhal),
    m_format(Format_Invalid),
    m_width(0),
    m_height(0),
    m_depth(0),
    m_pitch(0),
    m_qPitch(0),
    m_tile(0),
    m_tileModeGMM(MOS_TILE_LINEAR_GMM),
    m_bGMMTileEnabled(0),
    m_isCompressed(0),
    m_compressionMode(0),
    m_mmcState(MOS_MEMCOMP_DISABLED),
    m_compressionFormat(0),
    m_rotation(0),
    m_chromaSitting(0),
    m_surfaceXOffset(0),
    m_surfaceYOffset(0),
    m_frameType(CM_FRAME),
    m_isRenderTarget(true),
    m_paletteID(0),
    m_userWidth(0),
    m_userHeight(0),
    m_userDepth(0),
    m_maxStateSize(0),
    m_avsUsed(0),
    m_numPlane(0),
    m_pixelPitch(false),
    m_isWidthInDWord(false),
    m_isVme(false),
    m_direction(0),
    m_isHalfPitchChroma(false),
    m_isInterleaveChrome(false),
    m_uXOffset(0),
    m_uYOffset(0),
    m_vXOffset(0),
    m_vYOffset(0),
    m_isVaSurface(false)
{
    MOS_ZeroMemory(m_surfOffsets, sizeof(m_surfOffsets));
    MOS_ZeroMemory(m_xOffsets, sizeof(m_xOffsets));
    MOS_ZeroMemory(m_yOffsets, sizeof(m_yOffsets));
    MOS_ZeroMemory(m_lockOffsets, sizeof(m_lockOffsets));
    MOS_ZeroMemory(m_planeParams, sizeof(m_planeParams));
    MOS_ZeroMemory(m_cmds, sizeof(m_cmds));
}

CM_RETURN_CODE CmSurfaceState2Dor3D::Initialize(MOS_RESOURCE *resource, bool isAvs, bool isSampler)
{
    CmSurfaceState::Initialize(resource);

    m_avsUsed = isAvs;
    m_pixelPitch = (!isAvs)&&isSampler;
    m_isVme = isAvs&&(!isSampler);

    m_isWidthInDWord = m_avsUsed?false:(m_pixelPitch?false:true);
    m_maxStateSize = m_renderhal->pRenderHalPltInterface->GetSurfaceStateCmdSize();
    return CM_SUCCESS;
}

MOS_STATUS CmSurfaceState2Dor3D::GenerateSurfaceState(CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param)
{
    RefreshSurfaceInfo(param);

    UpdateSurfaceState();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmSurfaceState2Dor3D::RefreshSurfaceInfo(CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param)
{
    MOS_SURFACE         ResDetails;

    MHW_RENDERHAL_ASSERT(!Mos_ResourceIsNull(m_resource));
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));

    ResDetails.Format = (param && param->format)?(MOS_FORMAT)param->format:m_format;

    m_cmhal->osInterface->pfnGetResourceInfo(m_cmhal->osInterface, m_resource, &ResDetails);

    m_width = ResDetails.dwWidth;
    m_height = ResDetails.dwHeight;
    m_depth = ResDetails.dwDepth;
    m_pitch = ResDetails.dwPitch;
    m_qPitch = ResDetails.dwQPitch;
    m_format = ResDetails.Format;
    m_tile = ResDetails.TileType;
    m_tileModeGMM = ResDetails.TileModeGMM;
    m_bGMMTileEnabled = ResDetails.bGMMTileEnabled;
    m_isCompressed = ResDetails.bIsCompressed;
    m_compressionMode = ResDetails.CompressionMode;

    m_cmhal->osInterface->pfnGetMemoryCompressionMode(m_cmhal->osInterface,
        m_resource, &m_mmcState);
    m_cmhal->osInterface->pfnGetMemoryCompressionFormat(m_cmhal->osInterface,
        m_resource, &m_compressionFormat);

#ifdef __GNUC__
    // calculate the offsets
    switch (m_format)
    {
        case Format_NV12:
            m_surfOffsets[1] = ResDetails.RenderOffset.YUV.U.BaseOffset;
            m_yOffsets[1]    = ResDetails.RenderOffset.YUV.U.YOffset;
            break;
        case Format_P010:
        case Format_P016:
        case Format_P208:
            m_surfOffsets[1] = (m_height - m_height % 32) * m_pitch;
            m_yOffsets[1]    = m_height % 32;
            break;
        case Format_NV21:
            m_surfOffsets[1] = m_height * m_pitch;
            break;
        case Format_YV12:
            m_surfOffsets[2] = m_height * m_pitch;
            m_yOffsets[2]    = 0;
            m_surfOffsets[1] = m_height * m_pitch * 5 / 4;
            m_yOffsets[1]    = 0;
            break;
        case Format_422H:
            // Calculation methods are derived from Gmm's result.
            m_surfOffsets[1] = (m_height - m_height % 32) * m_pitch;
            m_yOffsets[1]       = m_height % 32;
            m_surfOffsets[2] = (m_height * 2 - (m_height * 2) % 32) * m_pitch;
            m_yOffsets[2]      = (m_height * 2) % 32;
            break;
        case Format_IMC3:
        case Format_422V:
            // Calculation methods are derived from Gmm's result.
            m_surfOffsets[1] = (m_height - m_height % 32) * m_pitch;
            m_yOffsets[1]       = m_height % 32;
            m_surfOffsets[2] = (m_height * 3 / 2 - (m_height * 3 / 2) % 32) * m_pitch;
            m_yOffsets[2]       = (m_height * 3 / 2) % 32;
            break;
        case Format_IMC4:
            m_surfOffsets[1] = m_height * m_pitch;
            m_yOffsets[1]       = m_height;
            m_yOffsets[2]       = m_height * 3 / 2;
            break;
        case Format_411P:
            m_surfOffsets[1] = m_height * m_pitch;
            m_yOffsets[1]      = 0;
            m_surfOffsets[2] = m_height * m_pitch * 2;
            m_yOffsets[2]       = 0;
            break;
        case Format_444P:
        case Format_RGBP:
            m_surfOffsets[1] = m_height * m_pitch;
            m_yOffsets[1]       = 0;
            m_surfOffsets[2] = m_height * m_pitch * 2;
            m_yOffsets[2]       = 0;
            break;

        default:
            break;
    }
#else
    if (IS_RGB32_FORMAT(m_format) ||
        IS_RGB16_FORMAT(m_format) ||
        IS_RGB128_FORMAT(m_format)||
        m_format == Format_RGB    ||
        m_format == Format_Y410)
    {
        m_surfOffsets[0] = ResDetails.RenderOffset.RGB.BaseOffset;
        m_xOffsets[0] = ResDetails.RenderOffset.RGB.XOffset;
        m_yOffsets[0] = ResDetails.RenderOffset.RGB.YOffset;
    }
    else // YUV or PL3_RGB
    {
        // Get Y plane information (plane offset, X/Y offset)
        m_surfOffsets[0] = ResDetails.RenderOffset.YUV.Y.BaseOffset;
        m_xOffsets[0] = ResDetails.RenderOffset.YUV.Y.XOffset;
        m_yOffsets[0] = ResDetails.RenderOffset.YUV.Y.YOffset;
        m_lockOffsets[0] = ResDetails.LockOffset.YUV.Y;

        // Get U/UV plane information (plane offset, X/Y offset)
        m_surfOffsets[1] = ResDetails.RenderOffset.YUV.U.BaseOffset;
        m_xOffsets[1] = ResDetails.RenderOffset.YUV.U.XOffset;
        m_yOffsets[1] = ResDetails.RenderOffset.YUV.U.YOffset;
        m_lockOffsets[1] = ResDetails.LockOffset.YUV.U;

        // Get V plane information (plane offset, X/Y offset)
        m_surfOffsets[2] = ResDetails.RenderOffset.YUV.V.BaseOffset;
        m_xOffsets[2] = ResDetails.RenderOffset.YUV.V.XOffset;
        m_yOffsets[2] = ResDetails.RenderOffset.YUV.V.YOffset;
        m_lockOffsets[2] = ResDetails.LockOffset.YUV.V;
    }
#endif

    // set User-defined dimension
    m_width = m_userWidth?m_userWidth:m_width;
    m_height = m_userHeight?m_userHeight:m_height;
    m_depth = m_userDepth?m_userDepth:m_depth;

    // set paramaters for alias
    if (param)
    {
        m_format = param->format?(MOS_FORMAT)param->format:m_format;
        m_width = param->width?param->width:m_width;
        m_height = param->height?param->height:m_height;
        m_depth = param->depth?param->depth:m_depth;
        m_pitch = param->pitch?param->pitch:m_pitch;
        m_surfaceXOffset = param->surfaceXOffset?param->surfaceXOffset:m_surfaceXOffset;
        m_surfaceYOffset = param->surfaceYOffset?param->surfaceYOffset:m_surfaceYOffset;
        m_memoryObjectControl = param->memoryObjectControl?param->memoryObjectControl:m_memoryObjectControl;
    }

    return MOS_STATUS_SUCCESS;
}

void CmSurfaceState2Dor3D::GetDIUVOffSet()
{
    uint32_t dwNumRowsFromTopV = 0;
    uint32_t dwNumRowsFromTopU = 0;
    uint32_t dwNumColsFromLeftV = 0;
    uint32_t dwNumColsFromLeftU = 0;

    switch (m_format)
    {
        //  YY
        //  YY
        //  V-
        //  U-
        case Format_IMC1:
            dwNumRowsFromTopV = m_height;
            dwNumRowsFromTopU = m_height + (m_height >> 1);
            break;

        //  YY
        //  YY
        //  VU
        case Format_IMC2:
            dwNumRowsFromTopV = dwNumRowsFromTopU = m_height;
            dwNumColsFromLeftU = m_pitch >> 1;
            break;

        //  YY
        //  YY
        //  U-
        //  V-
        case Format_IMC3:
            dwNumRowsFromTopU = m_height;
            dwNumRowsFromTopV = m_height + (m_height >> 1);
            break;

        //  YY
        //  YY
        //  UV
        case Format_IMC4:
            dwNumRowsFromTopU = dwNumRowsFromTopV = m_height;
            dwNumColsFromLeftV = m_pitch >> 1;
            break;

        //  YY
        //  YY
        //  U
        //  V
        case Format_I420:
        case Format_IYUV:
            dwNumRowsFromTopU = m_height;
            dwNumRowsFromTopV = m_height + (m_height >> 1);
            break;

        //  YY
        //  YY
        //  V
        //  U
        case Format_YV12:
            dwNumRowsFromTopU = m_height + (m_height >> 1);
            dwNumRowsFromTopV = m_height;
            break;

        //  YYYY
        //  YYYY
        //  YYYY
        //  YYYY
        //  V
        //  U
        case Format_YVU9:
            dwNumRowsFromTopU = m_height;
            dwNumRowsFromTopV = m_height + (m_height >> 2);
            break;

        //  NV12                P208
        //  ----                ----
        //  YY                  YY
        //  YY                  UV (interleaved)
        //  UV (interleaved)
        case Format_NV12:
        case Format_P208:
        case Format_P016:
        case Format_P010:
            dwNumRowsFromTopU = dwNumRowsFromTopV = m_height;
            break;

        //  NV11
        //  ----
        //  YYYY
        //  UV (interleaved)
        case Format_NV11:
            dwNumRowsFromTopU = dwNumRowsFromTopV = m_height;
            break;

       default:
           MHW_RENDERHAL_ASSERTMESSAGE("called with Packed or Unknown format.");
           break;
    }

    // the Offsets must be even numbers so we round down
    dwNumRowsFromTopU = MOS_ALIGN_FLOOR(dwNumRowsFromTopU, 2);
    dwNumColsFromLeftU = MOS_ALIGN_FLOOR(dwNumColsFromLeftU, 2);
    dwNumRowsFromTopV = MOS_ALIGN_FLOOR(dwNumRowsFromTopV, 2);
    dwNumColsFromLeftV = MOS_ALIGN_FLOOR(dwNumColsFromLeftV, 2);

    m_vYOffset = (uint16_t)dwNumRowsFromTopV;
    m_uYOffset = (uint16_t)dwNumRowsFromTopU;
    m_vXOffset = (uint16_t)dwNumColsFromLeftV;
    m_uXOffset = (uint16_t)dwNumColsFromLeftU;
}

bool CmSurfaceState2Dor3D::IsFormatMMCSupported(MOS_FORMAT format)
{
    // Check if Sample Format is supported
    if ((format != Format_YUY2)        &&
        (format != Format_Y410)        &&
        (format != Format_Y216)        &&
        (format != Format_Y210)        &&
        (format != Format_Y416)        &&
        (format != Format_P010)        &&
        (format != Format_P016)        &&
        (format != Format_AYUV)        &&
        (format != Format_NV21)        &&
        (format != Format_NV12)        &&
        (format != Format_UYVY)        &&
        (format != Format_YUYV)        &&
        (format != Format_A8B8G8R8)    &&
        (format != Format_X8B8G8R8)    &&
        (format != Format_A8R8G8B8)    &&
        (format != Format_X8R8G8B8)    &&
        (format != Format_B10G10R10A2) &&
        (format != Format_R10G10B10A2) &&
        (format != Format_A16R16G16B16F))
    {
        return false;
    }

    return true;
}

int CmSurfaceState2Dor3D::GetPlaneDefinitionMedia()
{
    int planeIndex = -1;
    bool isRenderOutTarget = false;
    if ( (m_format == Format_NV12 || m_format == Format_YV12 || m_format == Format_Y216)
          && (!m_pixelPitch))
    {
        isRenderOutTarget = true;
    }

    uint8_t direction = GetDirection();

    switch (m_format)
    {
        case Format_NV12:
        {
            // On G8, NV12 format needs the width and Height to be a multiple
            // of 4 for both 3D sampler and 8x8 sampler; G75 needs the width
            // of NV12 input surface to be a multiple of 4 for 3D sampler;
            // G9+ does not has such restriction; to simplify the implementation,
            // we enable 2 plane NV12 for all of the platform when the width
            // or Height is not a multiple of 4
            bool is2PlaneNeeded = false;
            if (!GFX_IS_GEN_10_OR_LATER(m_renderhal->Platform))
            {
                is2PlaneNeeded = (!MOS_IS_ALIGNED(m_height, 4) || !MOS_IS_ALIGNED(m_width, 4));
            }
            else
            {
                is2PlaneNeeded = (!MOS_IS_ALIGNED(m_height, 2) || !MOS_IS_ALIGNED(m_width, 2));
            }
            if (is2PlaneNeeded)
            {
                planeIndex = RENDERHAL_PLANES_NV12_2PLANES_ADV;
            }
            else
            {
                planeIndex = RENDERHAL_PLANES_NV12_ADV;
                m_uYOffset = RenderHal_CalculateYOffset(m_renderhal->pOsInterface, m_resource);
            }

            m_isHalfPitchChroma = false;
            m_isInterleaveChrome = true;

            // Set up chroma direction
            m_direction = direction;
            break;
        }
        case Format_P016:
            if (m_isVme)
            {
                planeIndex = RENDERHAL_PLANES_P010_1PLANE_ADV;
            }
            else
            {
                planeIndex = RENDERHAL_PLANES_P016_2PLANES_ADV;
            }
            break;

        case Format_P208:
            planeIndex = RENDERHAL_PLANES_P208_1PLANE_ADV;
            break;

        case Format_IMC1:
        case Format_IMC2:
        case Format_IMC3:
        case Format_IMC4:
        case Format_I420:
        case Format_IYUV:
        case Format_YV12:
        case Format_YVU9:
            planeIndex     = RENDERHAL_PLANES_PL3_ADV;
            m_isHalfPitchChroma = false;

            if (m_format == Format_I420 ||
                m_format == Format_IYUV ||
                m_format == Format_YV12 ||
                m_format == Format_NV11)
            {
                m_isHalfPitchChroma = true;
            }

            // Set up chroma direction
            m_direction = direction;

            if (true) //if (!pParams->bAVS)
            {
                // Get U/V offset for PL3 DNDI
                RENDERHAL_SURFACE surface;
                MOS_ZeroMemory(&surface, sizeof(surface));
                GetDIUVOffSet();
                planeIndex = RENDERHAL_PLANES_NV12_ADV;
            }
            break;

        case Format_400P:
            // Single Y plane here is treated like a NV12 surface.
            // U and V offsets fall inside this Y plane. Eventhough false UV pixels are
            // picked by the kernel, CSC coeffecients are such that the effect of these
            // are nullified.
            planeIndex     = RENDERHAL_PLANES_NV12_ADV;
            break;

        case Format_411P:
            planeIndex     = RENDERHAL_PLANES_411P_ADV;
            break;

        case Format_411R:
            planeIndex     = RENDERHAL_PLANES_411R_ADV;
            break;

        case Format_422H:
            planeIndex     = RENDERHAL_PLANES_422H_ADV;
            break;

        case Format_422V:
            planeIndex     = RENDERHAL_PLANES_422V_ADV;
            break;

        case Format_444P:
            planeIndex     = RENDERHAL_PLANES_444P_ADV;
            break;

        case Format_RGBP:
            planeIndex     = RENDERHAL_PLANES_RGBP_ADV;
            break;

        case Format_BGRP:
            planeIndex     = RENDERHAL_PLANES_BGRP_ADV;
            break;

        case Format_AYUV:
            planeIndex = RENDERHAL_PLANES_AYUV_ADV;
            break;

        case Format_YUYV:
        case Format_YUY2:
            if (m_isVme)
            {
                //Since 422 planar is not supported on application side.
                //App is using 422 packed as WA with w=w/2 and h=h*2
                m_width = m_width * 2;
                m_height = m_height / 2;
                planeIndex = RENDERHAL_PLANES_YUY2_ADV;

                // Set up chroma direction
                m_direction = direction;
            }
            else
            {
                planeIndex = RENDERHAL_PLANES_YUY2_ADV;

                // Set up chroma direction
                m_direction = direction;
            }
            break;

        case Format_UYVY:
            planeIndex     = RENDERHAL_PLANES_UYVY_ADV;

            // Set up chroma direction
            m_direction = direction;
            break;

        case Format_YVYU:
            planeIndex     = RENDERHAL_PLANES_YVYU_ADV;

            // Set up chroma direction
            m_direction = direction;
            break;

        case Format_VYUY:
            planeIndex     = RENDERHAL_PLANES_VYUY_ADV;

            // Set up chroma direction
            m_direction = direction;
            break;

        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
            if (m_isVaSurface)
            {
                m_width *= 32;
                planeIndex     = RENDERHAL_PLANES_Y1;
            }
            else
            {
                planeIndex     = RENDERHAL_PLANES_ARGB_ADV;
            }
            break;

        case Format_R8G8SN:
            if ( m_isVaSurface )
            {
                planeIndex = RENDERHAL_PLANES_Y16S;
            }
            break;

        case Format_V8U8:
            if ( m_isVaSurface )
            {
                planeIndex = RENDERHAL_PLANES_Y16U;
            }
            break;

        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
            planeIndex     = RENDERHAL_PLANES_ABGR_ADV;
            break;

        case Format_STMM:
            planeIndex     = RENDERHAL_PLANES_STMM_ADV;
            break;

        case Format_A8:
        case Format_Buffer_2D:
            if (m_isVaSurface)
            {
                planeIndex = RENDERHAL_PLANES_Y8;
            }
            break;

        case Format_L8:
                planeIndex     = RENDERHAL_PLANES_L8_ADV;
            break;

        case Format_A16B16G16R16:
        case Format_Y416:
            planeIndex        = RENDERHAL_PLANES_A16B16G16R16_ADV;
            break;

        case Format_R10G10B10A2:
        case Format_Y410:
            planeIndex        = RENDERHAL_PLANES_R10G10B10A2_ADV;
            break;

        case Format_L16:
        case Format_R16S:
            if (m_isVaSurface)
            {
                planeIndex = RENDERHAL_PLANES_Y16S;
            }
            break;

        case Format_D16:
        case Format_R16U:
            if (m_isVaSurface)
            {
                planeIndex = RENDERHAL_PLANES_Y16U;
            }
            break;

        case Format_P010:
            if (m_isVme)
            {
                planeIndex = RENDERHAL_PLANES_P010_1PLANE_ADV;
            }
            else if (m_cmhal->cmHalInterface->IsP010SinglePassSupported() && (!isRenderOutTarget))
            {
                planeIndex = RENDERHAL_PLANES_P010_1PLANE_ADV;
                m_isHalfPitchChroma = false;
                m_isInterleaveChrome = true;
                m_uYOffset = RenderHal_CalculateYOffset(m_renderhal->pOsInterface, m_resource);

                // Set up chroma direction
                m_direction = direction;
            }
            else
            {
                // Format not supported with AVS - use regular format
                MHW_RENDERHAL_NORMALMESSAGE("Format not supported with AVS.");
                m_avsUsed = false;
            }
            break;
        case Format_Y210:
        case Format_Y216:
            if (m_isVme)
            {
                //Since 422 planar is not supported on application side.
                //App is using 422 packed as WA with w=w/2 and h=h*2
                m_width = m_width * 2;
                m_height = m_height / 2;
                planeIndex = RENDERHAL_PLANES_Y210_1PLANE_ADV;
            }
            else
            {
                planeIndex = RENDERHAL_PLANES_Y210_ADV;
            }
            break;
        default:
            // Format not supported with AVS - use regular format
            MHW_RENDERHAL_NORMALMESSAGE("Format not supported with AVS.");

            m_avsUsed = false;

            break;
    }
    MHW_RENDERHAL_ASSERT(planeIndex < RENDERHAL_PLANES_DEFINITION_COUNT);
    return (int)planeIndex;
}

int CmSurfaceState2Dor3D::GetPlaneDefinitionRender()
{

    int planeIndex = -1;
    bool isRenderOutTarget = false;
    if ( (m_format == Format_NV12 || m_format == Format_YV12 || m_format == Format_Y210 || m_format == Format_Y216)
          && (!m_pixelPitch))
    {
        isRenderOutTarget = true;
    }

    switch (m_format)
    {
        case Format_IMC1:
        case Format_IMC2:
        case Format_IMC3:
        case Format_IMC4:
        case Format_I420:
        case Format_IYUV:
        case Format_YVU9:
            planeIndex = RENDERHAL_PLANES_PL3;
            break;

        case Format_YV12:
            m_isHalfPitchChroma = true;

            // Y_Uoffset(Height*2 + Height/2) of RENDERHAL_PLANES_YV12 define Bitfield_Range(0, 13) on gen9+.
            // The max value is 16383. So use PL3 kernel to avoid out of range when Y_Uoffset is larger than 16383.
            // Use PL3 plane to avoid YV12 U channel shift issue with not 4-aligned height
            planeIndex = (m_renderhal->bEnableYV12SinglePass &&
                               (!isRenderOutTarget) &&
                               MOS_IS_ALIGNED(m_height, 4) &&
                               (m_height * 2 + m_height / 2) < RENDERHAL_MAX_YV12_PLANE_Y_U_OFFSET_G9)?
                               RENDERHAL_PLANES_YV12 : RENDERHAL_PLANES_PL3;
            break;

        case Format_400P:
            // Single Y plane here is treated like a NV12 surface.
            // U and V offsets fall inside this Y plane. Eventhough false UV pixels are
            // picked by the kernel, CSC coeffecients are such that the effect of these
            // are nullified.
            planeIndex = RENDERHAL_PLANES_NV12;
            break;

        case Format_P016:
            planeIndex = RENDERHAL_PLANES_P016;
            break;

        case Format_P208:
            planeIndex = RENDERHAL_PLANES_P208;
            break;

        case Format_P010:
            if (m_renderhal->bEnableP010SinglePass &&
                (!isRenderOutTarget))
            {
                planeIndex = RENDERHAL_PLANES_P010_1PLANE;
            }
            else
            {
                planeIndex = RENDERHAL_PLANES_P010;
            }
            break;

        case Format_411P:
            planeIndex = RENDERHAL_PLANES_411P;
            break;

        case Format_411R:
            planeIndex = RENDERHAL_PLANES_411R;
            break;

        case Format_422H:
            planeIndex = RENDERHAL_PLANES_422H;
            break;

        case Format_422V:
            planeIndex = RENDERHAL_PLANES_422V;
            break;

        case Format_444P:
            planeIndex = RENDERHAL_PLANES_444P;
            break;

        case Format_RGBP:
            planeIndex = RENDERHAL_PLANES_RGBP;
            break;

        case Format_BGRP:
            planeIndex = RENDERHAL_PLANES_BGRP;
            break;

        case Format_NV12:
            // On Gen7.5 (Haswell) NV12 format needs a single plane instead
            // of two (listed in renderhal_g75.c for RENDERHAL_PLANES_NV12),  and
            // is also expected by the Sampler or Media Kernels. Yet, the
            // Data Port works with two planes instead. Besides, the Sampler
            // uses it for input only (as there is no output) while the Data
            // Port uses it for input as well as output or both for the same
            // surface. Hence the check added for bWidthInDword_Y &&
            // bWidthInDword_UV, which are set in vphal_render_3P.c for the
            // above reason. Two plane NV12 can also be explicitly spcified.

            // On G8, NV12 format needs the width and Height to be a multiple
            // of 4 for both 3D sampler and 8x8 sampler; G75 needs the width
            // of NV12 input surface to be a multiple of 4 for 3D sampler;
            // G9+ does not has such restriction; to simplify the implementation,
            // we enable 2 plane NV12 for all of the platform when the width
            // or Height is not a multiple of 4
            {
                bool is2PlaneNeeded = false;
                if (!GFX_IS_GEN_10_OR_LATER(m_renderhal->Platform))
                {
                    is2PlaneNeeded = (!MOS_IS_ALIGNED(m_height, 4) || !MOS_IS_ALIGNED(m_width, 4));
                }
                else
                {
                    is2PlaneNeeded = (!MOS_IS_ALIGNED(m_height, 4) || !MOS_IS_ALIGNED(m_width, 2));
                }
                if ( isRenderOutTarget ||
                     m_isWidthInDWord ||
                     is2PlaneNeeded)
                {
                    planeIndex = RENDERHAL_PLANES_NV12_2PLANES;
                }
                else
                {
                    planeIndex = RENDERHAL_PLANES_NV12;
                }
            }
            break;

        case Format_YUYV    :
        case Format_YUY2    :
            planeIndex = RENDERHAL_PLANES_YUY2;
            break;

        case Format_G8R8_G8B8:
        case Format_UYVY     :
            planeIndex = RENDERHAL_PLANES_UYVY;
            break;

        case Format_YVYU:
            planeIndex = RENDERHAL_PLANES_YVYU;
            break;

        case Format_VYUY:
            planeIndex = RENDERHAL_PLANES_VYUY;
            break;

        case Format_A8R8G8B8:
            planeIndex = RENDERHAL_PLANES_ARGB;
            break;

        case Format_R32U:
            planeIndex = RENDERHAL_PLANES_R32U;
            break;

        case Format_R32S:
            planeIndex = RENDERHAL_PLANES_R32S;
            break;

        case Format_R32F:
        case Format_D32F:
        case Format_R32:
            planeIndex = RENDERHAL_PLANES_R32F;
            break;

        case Format_Y8:
            planeIndex = RENDERHAL_PLANES_Y8;
            break;

        case Format_Y1:
            planeIndex = RENDERHAL_PLANES_Y1;
            break;

        case Format_Y16U:
            planeIndex = RENDERHAL_PLANES_Y16U;
            break;

        case Format_Y16S:
            planeIndex = RENDERHAL_PLANES_Y16S;
            break;

        case Format_R8G8SN:
        case Format_V8U8:
            planeIndex = RENDERHAL_PLANES_V8U8;
            break;

        case Format_R16U:
            planeIndex = RENDERHAL_PLANES_R16U;
            break;

        case Format_R16S:
            planeIndex = RENDERHAL_PLANES_R16S;
            break;

        case Format_R8G8UN:
            planeIndex = RENDERHAL_PLANES_R8G8_UNORM;
            break;

        case Format_X8R8G8B8:
            // h/w doesn't support XRGB render target
            planeIndex =
                (m_isRenderTarget) ? RENDERHAL_PLANES_ARGB : RENDERHAL_PLANES_XRGB;
            break;

        case Format_A8B8G8R8:
            planeIndex = RENDERHAL_PLANES_ABGR;
            break;

        case Format_X8B8G8R8:
            // h/w doesn't support XBGR render target
            planeIndex =
                (m_isRenderTarget) ? RENDERHAL_PLANES_ABGR : RENDERHAL_PLANES_XBGR;
            break;

        case Format_R5G6B5:
            planeIndex = RENDERHAL_PLANES_RGB16;
            break;

        case Format_R8G8B8:
            planeIndex = RENDERHAL_PLANES_RGB24;
            break;

        case Format_AYUV    :
            planeIndex = RENDERHAL_PLANES_AYUV;
            break;

        case Format_AI44    :
            planeIndex = (m_paletteID == 0) ?
                                            RENDERHAL_PLANES_AI44_PALLETE_0 :
                                            RENDERHAL_PLANES_AI44_PALLETE_1;
            break;

        case Format_IA44:
            planeIndex = (m_paletteID == 0) ?
                                            RENDERHAL_PLANES_IA44_PALLETE_0 :
                                            RENDERHAL_PLANES_IA44_PALLETE_1;
            break;

        case Format_P8:
            planeIndex = (m_paletteID == 0) ?
                                            RENDERHAL_PLANES_P8_PALLETE_0 :
                                            RENDERHAL_PLANES_P8_PALLETE_1;
            break;

        case Format_A8P8:
            planeIndex = (m_paletteID == 0) ?
                                            RENDERHAL_PLANES_A8P8_PALLETE_0 :
                                            RENDERHAL_PLANES_A8P8_PALLETE_1;
            break;

        case Format_STMM:
            planeIndex = RENDERHAL_PLANES_STMM;
            break;

        case Format_L8:
            planeIndex = RENDERHAL_PLANES_L8;
            break;

        case Format_A8:
        case Format_Buffer_2D:
            planeIndex = RENDERHAL_PLANES_A8;
            break;

        case Format_R8U:
        case Format_R8UN:
            planeIndex = RENDERHAL_PLANES_R8;
            break;

        case Format_R16UN:
        case Format_D16:
        case Format_R16:
            planeIndex = RENDERHAL_PLANES_R16_UNORM;
            break;

        case Format_A16B16G16R16:
            planeIndex = RENDERHAL_PLANES_A16B16G16R16;
            break;
        case Format_Y416:
            if (isRenderOutTarget)
            {
                planeIndex = RENDERHAL_PLANES_Y416_RT;
            }
            else
            {
                planeIndex = RENDERHAL_PLANES_A16B16G16R16;
            }
            break;
        case Format_A16B16G16R16F:
            planeIndex = RENDERHAL_PLANES_A16B16G16R16F;
            break;
        case Format_A16R16G16B16F:
            planeIndex = RENDERHAL_PLANES_A16R16G16B16F;
            break;
        case Format_R32G32B32A32F:
            planeIndex = RENDERHAL_PLANES_R32G32B32A32F;
            break;
        case Format_NV21:
            planeIndex = RENDERHAL_PLANES_NV21;
            break;
        case Format_L16:
            planeIndex = RENDERHAL_PLANES_L16;
            break;

        case Format_R10G10B10A2:
        case Format_Y410:
            planeIndex = RENDERHAL_PLANES_R10G10B10A2;
            break;

        case Format_Y210:
        case Format_Y216:
        {
            RENDERHAL_PLANE_DEFINITION temp;
            m_renderhal->pRenderHalPltInterface->GetPlaneDefForFormatY216(
                isRenderOutTarget,
                m_renderhal,
                temp);
            planeIndex = temp;
        }
            break;

        case Format_B10G10R10A2:
            planeIndex = RENDERHAL_PLANES_B10G10R10A2;
            break;

        case Format_IRW0:
            planeIndex = RENDERHAL_PLANES_IRW0;
            break;

        case Format_IRW1:
            planeIndex = RENDERHAL_PLANES_IRW1;
            break;

        case Format_IRW2:
            planeIndex = RENDERHAL_PLANES_IRW2;
            break;

        case Format_IRW3:
            planeIndex = RENDERHAL_PLANES_IRW3;
            break;

        case Format_R16G16UN:
            planeIndex = RENDERHAL_PLANES_R16G16_UNORM;
            break;

        case Format_R16G16S:
            planeIndex = RENDERHAL_PLANES_R16G16_SINT;
            break;

        case Format_R16F:
            planeIndex = RENDERHAL_PLANES_R16_FLOAT;
            break;

        case Format_R24G8:
        case Format_D24S8UN:
            planeIndex = RENDERHAL_PLANES_R24_UNORM_X8_TYPELESS;
            break;

        case Format_R32G8X24:
        case Format_D32S8X24_FLOAT:
            planeIndex = RENDERHAL_PLANES_R32_FLOAT_X8X24_TYPELESS;
            break;

        default:
            return -1;
    }

    // Get plane definitions
    MHW_RENDERHAL_ASSERT(planeIndex < RENDERHAL_PLANES_DEFINITION_COUNT);
    return (int)planeIndex;
}

uint8_t CmSurfaceState2Dor3D::GetDirection()
{
    if(GFX_IS_GEN_9_OR_LATER(m_renderhal->Platform))
    {
        // Gen9+
        uint8_t direction;
        if (m_chromaSitting & MHW_CHROMA_SITING_HORZ_CENTER)
        {
            direction = CHROMA_SITING_UDIRECTION_CENTER;
        }
        else
        {
            direction = CHROMA_SITING_UDIRECTION_LEFT;
        }
        direction = direction << 3;

        if (m_chromaSitting & MHW_CHROMA_SITING_VERT_TOP)
        {
            direction |= CHROMA_SITING_VDIRECTION_0;
        }
        else if (m_chromaSitting & MHW_CHROMA_SITING_VERT_BOTTOM)
        {
            direction |= CHROMA_SITING_VDIRECTION_1;
        }
        else
        {
            direction |= CHROMA_SITING_VDIRECTION_1_2;
        }
        return direction;
    }
    else
    {
        return MEDIASTATE_VDIRECTION_FULL_FRAME;
    }
}

MOS_STATUS CmSurfaceState2Dor3D::SetPerPlaneParam()
{
    int planeIndex = -1;
    if (m_avsUsed)
    {
        planeIndex = GetPlaneDefinitionMedia();
    }
    else
    {
        planeIndex = GetPlaneDefinitionRender();
    }
    CM_CHK_COND_RETURN((planeIndex == -1), MOS_STATUS_UNKNOWN, "Cannot find the plane definition.");

    bool isWidthInDword = m_avsUsed?false:(m_pixelPitch?false:true);
    m_numPlane = g_cRenderHal_SurfacePlanes[planeIndex].dwNumPlanes;
    PCMHW_PLANE_SETTING plane = g_cRenderHal_SurfacePlanes[planeIndex].Plane;

    uint32_t alignUnitWidth = 1;
    uint32_t alignUnitHeight = 1;
    if (m_format == Format_YUY2
        || m_format == Format_UYVY
        || m_format == Format_YVYU
        || m_format == Format_VYUY
        || m_format == Format_P208)
    {
        alignUnitHeight = 2;
    }

    for (uint32_t idx = 0; idx < m_numPlane; idx ++, plane ++)
    {
        uint32_t adjustedWidth = MOS_ALIGN_CEIL(m_width, alignUnitWidth);
        uint32_t adjustedHeight = MOS_ALIGN_CEIL(m_height, alignUnitHeight);
        adjustedHeight = (adjustedHeight + plane->ui8ScaleHeight - 1) / plane->ui8ScaleHeight;
        adjustedWidth  = adjustedWidth  / plane->ui8ScaleWidth;

        if (m_isWidthInDWord)
        {
            if (planeIndex == RENDERHAL_PLANES_R32G32B32A32F)
            {
                adjustedWidth = adjustedWidth << 2;
            }
            else if(planeIndex == RENDERHAL_PLANES_A16B16G16R16 ||
                planeIndex == RENDERHAL_PLANES_A16B16G16R16_ADV ||
                planeIndex == RENDERHAL_PLANES_A16B16G16R16F    ||
                planeIndex == RENDERHAL_PLANES_A16R16G16B16F    ||
                planeIndex == RENDERHAL_PLANES_Y210_RT          ||
                planeIndex == RENDERHAL_PLANES_Y416_RT          ||
                planeIndex == RENDERHAL_PLANES_R32_FLOAT_X8X24_TYPELESS)
            {
                adjustedWidth = adjustedWidth << 1;
            }
            else
            {
                adjustedWidth = (adjustedWidth + plane->ui8PixelsPerDword - 1) /
                                                        plane->ui8PixelsPerDword;
            }
        }

        if ((!m_isVme) && m_frameType != CM_FRAME)
        {
            adjustedHeight /= 2;
            adjustedHeight = MOS_MAX(adjustedHeight, 1);
        }

        adjustedHeight = MOS_ALIGN_FLOOR(adjustedHeight, plane->ui8AlignHeight);
        adjustedWidth  = MOS_ALIGN_FLOOR(adjustedWidth, plane->ui8AlignWidth);

        m_planeParams[idx].planeID = plane->ui8PlaneID;
        m_planeParams[idx].format = plane->dwFormat;
        m_planeParams[idx].width = adjustedWidth;
        m_planeParams[idx].height= adjustedHeight;
        if (plane->ui8PlaneID == MHW_U_PLANE ||
            plane->ui8PlaneID == MHW_V_PLANE)
        {
            if (m_format == Format_I420 ||
                m_format == Format_IYUV ||
                m_format == Format_YV12 ||
                m_format == Format_NV11)
            {
                m_planeParams[idx].pitch = (m_pitch>>1);
            }
            else if (m_format == Format_YVU9)
            {
                m_planeParams[idx].pitch = (m_pitch>>2);
            }
            else
            {
                m_planeParams[idx].pitch = m_pitch;
            }
        }
        else
        {
            m_planeParams[idx].pitch = m_pitch;
        }
        m_planeParams[idx].isAvs = plane->bAdvanced;
        m_planeParams[idx].xoffset = m_xOffsets[idx] + m_surfaceXOffset;
        if (m_format == Format_NV12 && idx == 1)
        {
            m_planeParams[idx].yoffset = m_yOffsets[idx] + m_surfaceYOffset/2;
        }
        else
        {
            m_planeParams[idx].yoffset = m_yOffsets[idx] + m_surfaceYOffset;
        }
    }

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CmSurfaceState2Dor3D::UpdateSurfaceState()
{
    // get the number of planes and plane types: y, u, v
    SetPerPlaneParam();

    // Set Surface States
    MHW_SURFACE_STATE_PARAMS SurfStateParams;
    for (uint32_t idx = 0; idx < m_numPlane; idx ++)
    {
        MOS_ZeroMemory(&SurfStateParams, sizeof(SurfStateParams));

        SurfStateParams.pSurfaceState         = m_cmds + idx * m_maxStateSize;
        SurfStateParams.bUseAdvState          = m_planeParams[idx].isAvs;
        SurfStateParams.dwWidth               = m_planeParams[idx].width;
        SurfStateParams.dwHeight              = m_planeParams[idx].height;
        SurfStateParams.dwFormat              = m_planeParams[idx].format;
        SurfStateParams.dwPitch               = m_planeParams[idx].pitch;
        SurfStateParams.dwQPitch              = m_qPitch;
        SurfStateParams.bTiledSurface         = (m_tile != MOS_TILE_LINEAR)?1:0;
        SurfStateParams.bTileWalk             = IS_Y_MAJOR_TILE_FORMAT(m_tile)
                                                ? GFX3DSTATE_TILEWALK_YMAJOR
                                                : GFX3DSTATE_TILEWALK_XMAJOR;;
        SurfStateParams.TileModeGMM           = m_tileModeGMM;
        SurfStateParams.bGMMTileEnabled       = m_bGMMTileEnabled;
        SurfStateParams.dwCacheabilityControl = GetCacheabilityControl();

        if (m_cmhal->platform.eRenderCoreFamily <= IGFX_GEN11_CORE)
        {
            SurfStateParams.bCompressionEnabled   = m_isCompressed;
            SurfStateParams.bCompressionMode      = (m_compressionMode == MOS_MMC_VERTICAL) ? 1 : 0;
        }
        else
        {
            if (IsFormatMMCSupported(m_format) && m_renderhal->isMMCEnabled)
            {
                // bCompressionEnabled/bCompressionMode is deprecated, use MmcState instead.
                if ((!m_cmhal->cmHalInterface->SupportCompressedOutput()) &&
                    (m_mmcState == MOS_MEMCOMP_RC && m_isRenderTarget))
                {
                    // RC compression mode is not supported on render output surface.
                    SurfStateParams.MmcState = MOS_MEMCOMP_DISABLED;
                    m_mmcState = MOS_MEMCOMP_DISABLED;
                    SurfStateParams.dwCompressionFormat = 0;
                }
                else if (m_mmcState == MOS_MEMCOMP_MC ||
                     m_mmcState == MOS_MEMCOMP_RC)
                {
                    SurfStateParams.MmcState = m_mmcState;

                    if (m_planeParams[idx].planeID == MHW_U_PLANE && 
                       (m_format == Format_NV12 || m_format == Format_P010 || m_format == Format_P016))
                    {
                        SurfStateParams.dwCompressionFormat = (uint32_t)(0x00000010) | (m_compressionFormat & 0x0f);
                    }
                    else if ((m_format == Format_R8G8UN) && (m_mmcState == MOS_MEMCOMP_MC))
                    {
                        /* it will be an issue if the R8G8UN surface with MC enable
                           is not chroma plane from NV12 surface, so far there is no
                           such case
                        */
                        SurfStateParams.dwCompressionFormat = (uint32_t)(0x00000010) | (m_compressionFormat & 0x0f);
                    }
                    else
                    {
                        SurfStateParams.dwCompressionFormat = m_compressionFormat & 0x1f;
                    }
                }
                else
                {
                    MHW_RENDERHAL_NORMALMESSAGE("Unsupported Compression Mode for Render Engine.");
                    SurfStateParams.MmcState            = MOS_MEMCOMP_DISABLED;
                    m_mmcState = MOS_MEMCOMP_DISABLED;
                    SurfStateParams.dwCompressionFormat = 0;
                }
            }
        }

        // update if is sampler
        if (m_pixelPitch || (m_avsUsed && (!m_isVme)))
        {
            SurfStateParams.RotationMode          = g_cLookup_RotationMode[m_rotation];
        }

        if (m_planeParams[idx].isAvs)
        {

            SurfStateParams.bHalfPitchChroma        = m_isHalfPitchChroma;
            SurfStateParams.bInterleaveChroma       = m_isInterleaveChrome;
            SurfStateParams.UVPixelOffsetUDirection = m_direction >> 0x3;
            SurfStateParams.UVPixelOffsetVDirection = m_direction & 0x7;

            if (m_planeParams[idx].planeID == MHW_U_PLANE)         // AVS U plane
            {
                // Lockoffset is the offset from base address of Y plane to the origin of U/V plane.
                // So, We can get XOffsetforU by Lockoffset % pSurface->dwPitch, and get YOffsetForU by Lockoffset / pSurface->dwPitch
                SurfStateParams.dwXOffsetForU = (uint32_t)m_lockOffsets[1] % m_pitch;
                SurfStateParams.dwYOffsetForU = (uint32_t)m_lockOffsets[1] / m_pitch;
                SurfStateParams.dwXOffsetForV = 0;
                SurfStateParams.dwYOffsetForV = 0;
                SurfStateParams.iXOffset      = m_xOffsets[1];
                SurfStateParams.iYOffset      = m_yOffsets[1];
            }
            else if (m_planeParams[idx].planeID == MHW_V_PLANE)    // AVS V plane
            {
                SurfStateParams.dwXOffsetForU = 0;
                SurfStateParams.dwYOffsetForU = 0;
                SurfStateParams.dwXOffsetForV = (uint32_t)m_lockOffsets[2] % m_pitch;
                SurfStateParams.dwYOffsetForV = (uint32_t)m_lockOffsets[2] / m_pitch;
                SurfStateParams.iXOffset      = m_xOffsets[2];
                SurfStateParams.iYOffset      = m_yOffsets[2];
            }
            else // AVS/DNDI Y plane
            {
                SurfStateParams.dwXOffsetForU = m_uXOffset;
                SurfStateParams.dwYOffsetForU = m_uYOffset;
                SurfStateParams.dwXOffsetForV = m_vXOffset;
                SurfStateParams.dwYOffsetForV = m_vYOffset;
                SurfStateParams.iXOffset      = 0;
                SurfStateParams.iYOffset      = m_yOffsets[0];
            }
        }
        else
        {
            SurfStateParams.SurfaceType3D             = (m_depth > 1) ?
                                                           GFX3DSTATE_SURFACETYPE_3D :
                                                           GFX3DSTATE_SURFACETYPE_2D;
            SurfStateParams.dwDepth                   = MOS_MAX(1, m_depth);
            SurfStateParams.bVerticalLineStrideOffset = (m_frameType == CM_BOTTOM_FIELD)?1:0;
            SurfStateParams.bVerticalLineStride       = (m_frameType == CM_FRAME)?0:1;
            SurfStateParams.bHalfPitchChroma          = m_isHalfPitchChroma;

            // Setup surface g9 surface state
            if (m_planeParams[idx].planeID == MHW_U_PLANE ||
                m_planeParams[idx].planeID == MHW_V_PLANE)
            {
                uint32_t pixelPerSampleUV = 1;
                if (m_isWidthInDWord)
                {
                    RenderHal_GetPixelsPerSample(m_format, &pixelPerSampleUV);
                }
                if(pixelPerSampleUV == 1)
                {
                    SurfStateParams.iXOffset = m_planeParams[idx].xoffset;
                }
                else
                {
                    SurfStateParams.iXOffset = m_planeParams[idx].xoffset/sizeof(uint32_t);
                }
                SurfStateParams.iYOffset = m_planeParams[idx].yoffset;
            }
            else // Y plane
            {
                SurfStateParams.iXOffset = m_planeParams[idx].xoffset/sizeof(uint32_t);
                SurfStateParams.iYOffset = m_planeParams[idx].yoffset;

                if(m_planeParams[idx].format == MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8)
                {
                    if (m_format == Format_YV12)
                    {
                        SurfStateParams.bSeperateUVPlane = true;
                        SurfStateParams.dwXOffsetForU    = 0;
                        SurfStateParams.dwYOffsetForU    = m_height * 2 + m_height/2;
                        SurfStateParams.dwXOffsetForV    = 0;
                        SurfStateParams.dwYOffsetForV    = m_height * 2;
                    }
                    else
                    {
                        SurfStateParams.bSeperateUVPlane = false;
                        SurfStateParams.dwXOffsetForU    = 0;
                        SurfStateParams.dwYOffsetForU    = m_surfOffsets[1]/m_pitch + m_yOffsets[1];
                        SurfStateParams.dwXOffsetForV    = 0;
                        SurfStateParams.dwYOffsetForV    = 0;
                    }
                }
            }
        }
        m_renderhal->pMhwStateHeap->SetSurfaceStateEntry(&SurfStateParams);
    }

    return MOS_STATUS_SUCCESS;
}

CmSurfaceStateBuffer::CmSurfaceStateBuffer(CM_HAL_STATE *cmhal):
    CmSurfaceState(cmhal),
    m_size(0),
    m_offset(0)
{
    MOS_ZeroMemory(m_cmds, sizeof(m_cmds));
}

CM_RETURN_CODE CmSurfaceStateBuffer::Initialize(MOS_RESOURCE *resource, uint32_t size)
{
    CmSurfaceState::Initialize(resource);
    m_size = size;
    return CM_SUCCESS;
}

MOS_STATUS CmSurfaceStateBuffer::GenerateSurfaceState(CM_HAL_BUFFER_SURFACE_STATE_ENTRY *param)
{
    if (param)
    {
        m_size = param->surfaceStateSize?param->surfaceStateSize:m_size;
        m_offset = param->surfaceStateOffset?param->surfaceStateOffset:m_offset;
        m_memoryObjectControl = param->surfaceStateMOCS?param->surfaceStateMOCS:m_memoryObjectControl;
    }

    MHW_SURFACE_STATE_PARAMS params;
    MOS_ZeroMemory(&params, sizeof(params));

    uint32_t bufferSize = m_size - 1;
    params.SurfaceType3D = GFX3DSTATE_SURFACETYPE_BUFFER;
    // Width  contains bits [ 6:0] of the number of entries in the buffer
    params.dwWidth = (uint8_t)(bufferSize & MOS_MASKBITS32(0, 6));
    // Height contains bits [20:7] of the number of entries in the buffer
    params.dwHeight = (uint16_t)((bufferSize & MOS_MASKBITS32(7, 20)) >> 7);
    // For SURFTYPE_BUFFER, this field indicates the size of the structure
    params.dwPitch = 0;

    uint32_t depthMaskRawBuffer = m_renderhal->pRenderHalPltInterface->GetDepthBitMaskForRawBuffer();
    params.dwFormat = MHW_GFX3DSTATE_SURFACEFORMAT_RAW;
    params.dwDepth = (uint16_t)((bufferSize & depthMaskRawBuffer) >> 21);
    params.dwCacheabilityControl = GetCacheabilityControl();
    params.pSurfaceState = m_cmds;

    // Default tile mode of surface state buffer is linear
    params.bGMMTileEnabled = true;

    m_renderhal->pMhwStateHeap->SetSurfaceStateEntry(&params);

    return MOS_STATUS_SUCCESS;
}


CmSurfaceStateVME::CmSurfaceStateVME(CM_HAL_STATE *cmhal):
    CmSurfaceState(cmhal),
    m_numBte(0),
    m_forwardCount(0),
    m_backwardCount(0),
    m_curIndex(CM_NULL_SURFACE),
    m_forwardIndexes(nullptr),
    m_backwardIndexes(nullptr)
{
    MOS_ZeroMemory(&m_surf2DParam, sizeof(m_surf2DParam));
    MOS_ZeroMemory(m_offsets, sizeof(m_offsets));
    MOS_ZeroMemory(m_mmcStates, sizeof(m_mmcStates));
}

CM_RETURN_CODE CmSurfaceStateVME::Initialize(CM_HAL_VME_ARG_VALUE *vmeArg)
{
    CmSurfaceState::Initialize(nullptr);
    m_forwardCount = vmeArg->fwRefNum;
    m_backwardCount = vmeArg->bwRefNum;
    m_curIndex = vmeArg->curSurface;
    m_forwardIndexes = findFwRefInVmeArg(vmeArg);
    m_backwardIndexes = findBwRefInVmeArg(vmeArg);
    uint32_t numPair = m_forwardCount>m_backwardCount?m_forwardCount:m_backwardCount;
    m_numBte = 2*numPair + 1;

    m_surf2DParam.width = vmeArg->surfStateParam.surfaceStateWidth;
    m_surf2DParam.height= vmeArg->surfStateParam.surfaceStateHeight;

    return CM_SUCCESS;
}

uint8_t *CmSurfaceStateVME::GetSurfaceState(int index)
{
    int surfIndex = GetCmHalSurfaceIndex(index);
    if (surfIndex == -1)
        return nullptr;

    CmSurfaceState2Dor3DMgr *surfStateMgr = m_cmhal->umdSurf2DTable[surfIndex].surfStateMgr;
    CmSurfaceState *surfState = nullptr;
    if (m_surf2DParam.width == 0 && m_surf2DParam.height == 0)
    {
        surfState = surfStateMgr->GetSurfaceState(1);
    }
    else
    {
        surfState = surfStateMgr->GetSurfaceState(1, 0, &m_surf2DParam);
    }
    CM_CHK_NULL_RETURN(surfState, nullptr);
    m_offsets[index]=surfState->GetSurfaceOffset(0);
    m_mmcStates[index]=surfState->GetMmcState(0);
    return surfState->GetSurfaceState(0); //in vme every surface has only one plane
}

MOS_RESOURCE *CmSurfaceStateVME::GetResource(uint32_t index)
{
    int surfIndex = GetCmHalSurfaceIndex(index);
    if (surfIndex == -1)
        return nullptr;
    return m_cmhal->umdSurf2DTable[surfIndex].surfStateMgr->GetResource();
}

int CmSurfaceStateVME::GetCmHalSurfaceIndex(uint32_t index)
{
    uint32_t surfIndex;
    if (index == 0) // current surface
    {
        surfIndex = m_curIndex;
    }
    else if (index%2 == 1) // forward references
    {
        if ((index - 1)/2 >= m_forwardCount)
        {
            return -1;
        }
        surfIndex = m_forwardIndexes[(index - 1)/2];
    }
    else // backward references
    {
        if ((index - 1)/2 >= m_backwardCount)
        {
            return -1;
        }
        surfIndex = m_backwardIndexes[(index - 2)/2];
    }
    return (int)surfIndex;

}

