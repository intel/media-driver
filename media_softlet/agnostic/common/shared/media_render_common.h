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
//! \details  eu kernel related funcitons and structs.
//!
#ifndef __MEDIA_RENDER_COMMON_H__
#define __MEDIA_RENDER_COMMON_H__

#include "renderhal.h"

//!
//! \brief VPHAL SS/EU setting
//!
struct euSetting
{
    uint8_t   numSlices;
    uint8_t   numSubSlices;
    uint8_t   numEUs;
    uint8_t   reserved;       // Place holder for frequency setting
};
const euSetting defaultSSEUTable[8] =
{// Slice    Sub-Slice       EU      Rsvd(freq)
    {2, 3, 8, 0},
};

//!
//! \class    MediaRenderCommon
//! \brief    Media Render Common Interface
//!
class MediaRenderCommon
{
public:

    //!
    //! \brief    Set 2D Surface for HW Access
    //! \details  Common Function for setting up buffer surface state, need to use this function
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
    static MOS_STATUS Set2DSurfaceForHwAccess(
        PRENDERHAL_INTERFACE                pRenderHal,
        PMOS_SURFACE                        pSurface,
        PRENDERHAL_SURFACE                  pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
        int32_t                             iBindingTable,
        int32_t                             iBTEntry,
        bool                                bWrite);

    //!
    //! \brief    Set the numbers of Slice, Sub-slice, EUs for power mode
    //! \details  Set the numbers of Slice, Sub-slice, EUs recommended for
    //!           the given kernel type for power mode
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in] KernelID
    //!            render Kernel ID
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    static MOS_STATUS SetPowerMode(
        PRENDERHAL_INTERFACE              pRenderHal,
        uint32_t                          KernelID);

    //!
    //! \brief    Set 1D Surface for HW Access
    //! \details  Common Function for setting up buffer surface state, need to use this function
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
    static MOS_STATUS Set1DSurfaceForHwAccess(
        PRENDERHAL_INTERFACE                pRenderHal,
        PMOS_SURFACE                        pSurface,
        PRENDERHAL_SURFACE                  pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
        int32_t                             iBindingTable,
        int32_t                             iBTEntry,
        bool                                bWrite);

    //!
    //! \brief      Submit commands for rendering
    //! \details    Submit commands for rendering. The KMD related fields in pGenericPrologParam might be modified by this
    //!             function in order to maintain the synchronization mechanism for resource.
    //! \param      [in] pRenderHal
    //!             Pointer to RenderHal Interface Structure
    //! \param      [in] pBatchBuffer
    //!             Pointer to batch buffer
    //! \param      [in] bNullRendering
    //!             Indicate whether is Null rendering
    //! \param      [in] pWalkerParams
    //!             Pointer to walker parameters
    //! \param      [in] pGpGpuWalkerParams
    //!             Pointer to GPGPU walker parameters
    //! \param      [in] KernelID
    //!             VP Kernel ID
    //! \param      [in] bLastSubmission
    //!             Is last submission
    //! \return     MOS_STATUS
    //!
    static MOS_STATUS EukernelSubmitCommands(
        PRENDERHAL_INTERFACE                pRenderHal,
        PMHW_BATCH_BUFFER                   pBatchBuffer,
        bool                                bNullRendering,
        PMHW_WALKER_PARAMS                  pWalkerParams,
        PMHW_GPGPU_WALKER_PARAMS            pGpGpuWalkerParams,
        uint32_t                            KernelID,
        bool                                bLastSubmission);

MEDIA_CLASS_DEFINE_END(MediaRenderCommon)
};

#endif  // __MEDIA_RENDER_COMMON_H__
