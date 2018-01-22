/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     vphal_render_composite_g8.cpp
//! \brief    Composite related VPHAL functions
//! \details  Unified VP HAL Composite module including render initialization,
//!           resource allocation/free and rendering
//!
#include "vphal_render_composite_g8.h"

#define VPHAL_COMP_WA_BDW_GT2_IEF_THREAD_LIMIT  96

void CompositeStateG8::SubmitStatesFillGenSpecificStaticData(
    PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
    PVPHAL_SURFACE                      pTarget,
    MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic)
{
    PVPHAL_SURFACE                      pSurface;

    //Set shift offset for interlace scaling
    //Vertical Frame Origin for Layer 0 - Layer 7
    //Format = Single precision floating point
    pSurface = pRenderingData->pLayers[0]; // only using primary layer [0]
    if (nullptr != pSurface && pSurface->bInterlacedScaling)
    {
        if (pSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD || pSurface->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD)
        {
            //use the cropping size, not the surface size
            pStatic->DW11.TopBottomDelta = (float)(1.0 / (pSurface->rcDst.bottom - pSurface->rcDst.top) - 1.0 / (pSurface->rcSrc.bottom - pSurface->rcSrc.top));
        }
        else
        {
            pStatic->DW11.TopBottomDelta = (float)(-(1.0 / (pSurface->rcDst.bottom - pSurface->rcDst.top) - 1.0 / (pSurface->rcSrc.bottom - pSurface->rcSrc.top)));
        }
    }

    // Set ChromaSitting
    pStatic->DW10.ChromaSitingLocation = GetOutputChromaSitting(pTarget);

    if (pRenderingData->iLayers > 0)
    {
        pStatic->DW09.ObjKa2Gen8.IEFByPassEnable = pRenderingData->pLayers[0]->bIEF ? false : true;
    }

    // Set alpha calculation flag. The bit definitions are different for GEN8 and GEN9+.
    // Set Bit-17
    pStatic->DW09.ObjKa2Gen8.AlphaChannelCalculation = pRenderingData->bAlphaCalculateEnable ? true : false;
}

PRENDERHAL_OFFSET_OVERRIDE CompositeStateG8::GetPlaneOffsetOverrideParam(
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams,
    PRENDERHAL_OFFSET_OVERRIDE      pOverride)
{
    uint32_t                        uBytesPerPixelShift  = 0;
    uint32_t                        uYPlaneTopLvlIndexY  = 0;
    uint32_t                        uYPlane2ndLvlIndexY  = 0;
    uint32_t                        uYPlaneTopLvlIndexX  = 0;
    uint32_t                        uYPlane2ndLvlIndexX  = 0;
    uint32_t                        uUVPlaneTopLvlIndexY = 0;
    uint32_t                        uUVPlane2ndLvlIndexY = 0;
    uint32_t                        uUVPlaneTopLvlIndexX = 0;
    uint32_t                        uUVPlane2ndLvlIndexX = 0;
    uint32_t                        uSurfPitch           = 0;
    PMOS_SURFACE                    pSurface;
    RECT                            tempRect;
    PRENDERHAL_OFFSET_OVERRIDE      returnOverride = nullptr;

    if ((pRenderHalSurface == nullptr)          ||
        (pParams == nullptr)                    ||
        (pOverride == nullptr))
    {
        return nullptr;
    }

    pSurface = &pRenderHalSurface->OsSurface;

    if (!pParams->b32MWColorFillKern)
    {
        return nullptr;
    }

    // Due to lacking of native 32x32 block support, we use 2x2 x (16x16) instead,
    // so only the left-top block can perform none zero X/Y origin correctly.
    // Due to limitation of X/Y offset in Surface State, it can be programmed
    // within a tile only to make left/top coordinate falling into (0, 0) block.
    // Adjust Surface Base Address to tile alignment.
    // The same approach below is applied to both X and Y direction.
    //
    // |   32 DWORDs    |   32 DWORDs    | 16 pxls| 16 pxls|
    // |----------------|----------------|--------|--------|-----
    // |      tile      |      tile      |        |   left |
    // |                |                |        |     |  |
    //                              base address  |     |
    //                                adjustment  |     |
    //                                            |     |
    //                                     X/Y Offset   |
    //                                  in Surface State|
    //                                                  |
    //                                       DestHorizontalBlockOrigin

    if (pParams->b32MWColorFillKern == true)
    {
        uint32_t uiOld_YplaneHeight   = pSurface->dwHeight;
        tempRect                      = pRenderHalSurface->rcDst;
        // Backup Original Surface Pitch for surface offset tuning solution later
        uSurfPitch                    = pSurface->dwPitch;

        pSurface->YPlaneOffset.iXOffset = tempRect.left;
        pSurface->YPlaneOffset.iYOffset = tempRect.top;

        // This is to preserve lowest 4 bits of left/top value for DW69.block original
        // i.e. pRenderHalSurface->rcDst.left    = tempRect.left % VPHAL_MACROBLOCK_SIZE
        //      pRenderHalSurface->rcDst.top     = tempRect.top  % VPHAL_MACROBLOCK_SIZE
        pRenderHalSurface->rcDst.left    = tempRect.left & (VPHAL_MACROBLOCK_SIZE - 1);
        pRenderHalSurface->rcDst.top     = tempRect.top  & (VPHAL_MACROBLOCK_SIZE - 1);

        // Due to we offset/shifted surface base address so that we need update width/height and right/bottom
        pSurface->dwWidth       = pRenderHalSurface->rcDst.right     = tempRect.right  - MOS_ALIGN_FLOOR(tempRect.left, VPHAL_MACROBLOCK_SIZE);
        pSurface->dwHeight      = pRenderHalSurface->rcDst.bottom    = tempRect.bottom - MOS_ALIGN_FLOOR(tempRect.top, VPHAL_MACROBLOCK_SIZE);

        switch (pSurface->Format)
        {
            case Format_A8B8G8R8:
            case Format_X8B8G8R8:
            case Format_A8R8G8B8:
            case Format_X8R8G8B8:
            case Format_R10G10B10A2:
                    uBytesPerPixelShift = 2;    // 4 bytes per pixel
               break;
            // packeted format
            case Format_YUY2:
            case Format_YUYV:
            case Format_YVYU:
            case Format_UYVY:
            case Format_VYUY:
                    uBytesPerPixelShift = 1;    // 2 bytes per pixel
                break;
            // planar format
            case Format_P010:
            case Format_P016:
                    uBytesPerPixelShift = 1;    // 2 bytes per pixel
                break;
            // planar format
            case Format_NV12:
                    uBytesPerPixelShift = 0;    // 1 bytes per pixel
                break;
            default:
                    uBytesPerPixelShift = 0;
                break;
        }

        // # of tiles in Y direction for base address adjustment
        uYPlaneTopLvlIndexY     = tempRect.top >> VPHAL_YTILE_H_SHIFTBITS;

        // Yoffset within tile and above 16x16 block
        // It's to retrieve 2ndLvlIndex bits field through bitwise operations.
        // The intersection part of two AND MASKS would be data bits what we need.
        uYPlane2ndLvlIndexY     = (tempRect.top & (VPHAL_YTILE_H_ALIGNMENT - 1)) &
                ~(VPHAL_MACROBLOCK_SIZE - 1);

        // # of tiles in X direction for base address adjustment
        // it's to simply shifting left VPHAL_YTILE_W_SHIFTBITS and
        // then right shifting uBytesPerPixelShift number of bits
        uYPlaneTopLvlIndexX     = tempRect.left >> (VPHAL_YTILE_W_SHIFTBITS - uBytesPerPixelShift);

        // Xoffset within tile and above 16x16 block, in DWORD
        uYPlane2ndLvlIndexX     = ((tempRect.left &
                ((VPHAL_YTILE_W_ALIGNMENT >> uBytesPerPixelShift)-1)) &
                ~(VPHAL_MACROBLOCK_SIZE - 1)) >> (2 - uBytesPerPixelShift);

        // NV12/P010/P016 is using two planes so that we have to caculate TopLvl/2ndLvl index for UV plane
        if (pSurface->Format == Format_NV12 ||
            pSurface->Format == Format_P010 ||
            pSurface->Format == Format_P016)
        {
            //      (original) old Y plane base ->| +------------------+     |
            //                                    | |                  |     |uiOld_YplaneHeight
            //                                    | |                  |     |
            //                                   ......   Y-plane     ...   ...
            //        (rebased) new Y plane base->| |(tile-aligned)    |     |  |
            //                                    | |                  |     |  |
            // uiOld_YplaneBase_to_New_UVplaneBase| |////colorfill/////|     |  |
            //                                    | |//////area////////|     |  |uiNew_YplaneHeight
            //                old UV plane base ->| +------------------+     |  |
            //                                    | |                  |  |
            //                 new UV plane base->| |(tile-aligned)    |  |
            //                                  |   |                  |  |
            //          32 > YOffsetFor_UV_Plane|   |     UV-plane     |  | uiOldUVplaneBaseToBottom
            //     16 > new UV plane rect.top|  |   |                  |  | =tempRect.bottom/2
            //                                    | |                  |  |
            //                 uiNew_UVplaneHeight| |//colorfill area//|  |
            //               =uiNew_YplaneHeight/2| +------------------+  |<-UV plane rect.bottom (old == new)
            //

            uint32_t uiOldUVplaneBaseToBottom            = tempRect.bottom / 2;
            uint32_t uiNew_UVplaneHeight                 = pSurface->dwHeight / 2; // (= uiNew_YplaneHeight / 2);
            uint32_t uiOld_YplaneBase_to_UVplaneBottom   = uiOld_YplaneHeight + uiOldUVplaneBaseToBottom;
            uint32_t uiOldYBaseToNewUVBase_Unaligned     = uiOld_YplaneBase_to_UVplaneBottom - uiNew_UVplaneHeight;

            // uiOld_YplaneBase_to_New_UVplaneBase is diff_in_rows(new UV plane base, old Y plane base), which is tile (32-rows) alignment.
            uint32_t uiOld_YplaneBase_to_New_UVplaneBase = MOS_ALIGN_FLOOR(uiOldYBaseToNewUVBase_Unaligned, VPHAL_YTILE_H_ALIGNMENT);

            uUVPlaneTopLvlIndexY = tempRect.top >> (VPHAL_YTILE_H_SHIFTBITS + 1);
            // uUVPlane2ndLvlIndexY is YOffsetFor_UV_Plane, which is 16-rows alignment offset + new UV plane rect.top
            uUVPlane2ndLvlIndexY = uiOldYBaseToNewUVBase_Unaligned - uiOld_YplaneBase_to_New_UVplaneBase;
            uUVPlaneTopLvlIndexX = uYPlaneTopLvlIndexX;
            uUVPlane2ndLvlIndexX = uYPlane2ndLvlIndexX;
        }

        // Y plane adjustments/overrides
        pOverride->iYOffsetAdjust  = uYPlaneTopLvlIndexY * (uSurfPitch / VPHAL_YTILE_W_ALIGNMENT) * MHW_PAGE_SIZE +
                                    uYPlaneTopLvlIndexX * MHW_PAGE_SIZE;
        pOverride->iYOffsetX       = uYPlane2ndLvlIndexX;
        pOverride->iYOffsetY       = uYPlane2ndLvlIndexY;

        // UV plane adjustments/overrides
        pOverride->iUVOffsetAdjust = uUVPlaneTopLvlIndexY * (uSurfPitch >> VPHAL_YTILE_W_SHIFTBITS) * MHW_PAGE_SIZE +
                                    uUVPlaneTopLvlIndexX * MHW_PAGE_SIZE;
        pOverride->iUVOffsetX      = uUVPlane2ndLvlIndexX;
        pOverride->iUVOffsetY      = uUVPlane2ndLvlIndexY;

        // calculation is done, assign return value to the pointer to Override Data
        returnOverride = pOverride;
    }

    return returnOverride;
}

int32_t CompositeStateG8::GetThreadCountForVfeState(
    PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
    PVPHAL_SURFACE                      pTarget)
{
    // Solution for BDW GT2 IEF performance issue
    if (!m_pPerfData->CompMaxThreads.bEnabled           &&
        MEDIA_IS_SKU(m_pRenderHal->pSkuTable, FtrGT2)                &&
        pRenderingData->pLayers[0]                      &&
        pRenderingData->pLayers[0]->bIEF)
    {
        return waBdwGt2ThreadLimit;
    }
    else
    {
        return CompositeState::GetThreadCountForVfeState(pRenderingData, pTarget);
    }
}
