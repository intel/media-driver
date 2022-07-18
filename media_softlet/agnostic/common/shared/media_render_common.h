/*
* Copyright (c) 2022, Intel Corporation
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
//! \file      media_render_common.h 
//! \brief         Unified media shared rendering definitions 
//!
//!
//! \file     media_render_common.h
//! \brief    The header file of common struct/macro definitions shared by low level renderers
//! \details  Common struct/macro for different renderers, e.g. DNDI or Comp
//!
#ifndef __MEDIA_RENDER_COMMON_H__
#define __MEDIA_RENDER_COMMON_H__

#include "renderhal.h"
#include "vp_common.h"
#include "vp_utils.h"
//!
//! \class    MediaRenderCommon
//! \brief    Media Render Common Interface
//!
class MediaRenderCommon
{
public:
    //!
    //! \brief    Initialized RenderHal Surface according to incoming VPHAL Surface
    //! \param    [in] pVpSurface
    //!           Pointer to the VPHAL surface
    //! \param    [out] pRenderHalSurface
    //!           Pointer to the RenderHal surface
    //! \return   MOS_STATUS
    //!
    static MOS_STATUS RndrCommonInitRenderHalSurface(
        PVPHAL_SURFACE          pVpSurface,
        PRENDERHAL_SURFACE      pRenderHalSurface);

    //!
    //! \brief    Get output RenderHal Surface parameters back to VPHAL Surface
    //! \param    [in] pRenderHalSurface
    //!           Pointer to the RenderHal surface
    //! \param    [in,out] pVpSurface
    //!           Pointer to the VPHAL surface
    //! \return   MOS_STATUS
    //!
    static MOS_STATUS RndrCommonGetBackVpSurfaceParams(
        PRENDERHAL_SURFACE      pRenderHalSurface,
        PVPHAL_SURFACE          pVpSurface);

    //!
    //! \brief    Set Surface for HW Access for CP HM
    //! \details  Common Function for setting up surface state, need to use this function
    //!           if render would use CP HM
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in] pSurface
    //!           Pointer to Surface
    //! \param    [in] pRenderSurface
    //!           Pointer to Render Surface
    //! \param    [in] pSurfaceParams
    //!           Pointer to RenderHal Surface Params
    //! \param    [in] iBindingTable
    //!           Binding Table to bind surface
    //! \param    [in] iBTEntry
    //!           Binding Table Entry index
    //! \param    [in] bWrite
    //!           Write mode flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    static MOS_STATUS CommonSetSurfaceForHwAccess(
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_SURFACE                      pSurface,
        PRENDERHAL_SURFACE                  pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
        int32_t                             iBindingTable,
        int32_t                             iBTEntry,
        bool                                bWrite);

    //!
    //! \brief    Set Buffer Surface for HW Access for CP HM
    //! \details  Common Function for setting up buffer surface state, need to use this function
    //!           if render would use CP HM
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in] pSurface
    //!           Pointer to Surface
    //! \param    [in] pRenderSurface
    //!           Pointer to Render Surface
    //! \param    [in,out] pSurfaceParams
    //!           Pointer to RenderHal Surface Params
    //! \param    [in] iBindingTable
    //!           Binding Table to Bind Surface
    //! \param    [in] iBTEntry
    //!           Binding Table Entry index
    //! \param    [in] bWrite
    //!           Write mode flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    static MOS_STATUS CommonSetBufferSurfaceForHwAccess(
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_SURFACE                      pSurface,
        PRENDERHAL_SURFACE                  pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
        int32_t                             iBindingTable,
        int32_t                             iBTEntry,
        bool                                bWrite);

MEDIA_CLASS_DEFINE_END(MediaRenderCommon)
};

#endif  // __MEDIA_RENDER_COMMON_H__
