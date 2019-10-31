/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vphal_render_composite_g12.cpp
//! \brief    Composite related VPHAL functions
//! \details  Unified VP HAL Composite module including render initialization,
//!           resource allocation/free and rendering
//!
#include "vphal_render_composite_g12.h"

void CompositeStateG12::SubmitStatesFillGenSpecificStaticData(
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
            pStatic->DW12.TopBottomDelta = (float)(1.0 / (pSurface->rcDst.bottom - pSurface->rcDst.top) - 1.0 / (pSurface->rcSrc.bottom - pSurface->rcSrc.top));
        }
        else
        {
            pStatic->DW12.TopBottomDelta = (float)(-(1.0 / (pSurface->rcDst.bottom - pSurface->rcDst.top) - 1.0 / (pSurface->rcSrc.bottom - pSurface->rcSrc.top)));
        }
    }

    // Set ChromaSitting
    pStatic->DW10.ObjKa2Gen9.ChromaSitingLocation = GetOutputChromaSitting(pTarget);

    if (pRenderingData->iLayers > 0)
    {
        // “IEF Bypass” bit is changed to “MBZ” bit for Gen12 in HW interface,  so driver should always set “Bypass IEF” to be 0 in CURBE.
        pStatic->DW09.ObjKa2Gen9.IEFByPassEnable = false;
    }

    // Set alpha calculation flag. The bit definitions are different for GEN8 and GEN9+.
    // Set Bit-18
    pStatic->DW09.ObjKa2Gen9.AlphaChannelCalculation = pRenderingData->bAlphaCalculateEnable ? true : false;
}
