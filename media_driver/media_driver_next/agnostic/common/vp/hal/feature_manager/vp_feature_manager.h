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
//! \file     vp_feature_manager.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __VP_FEATURE_MANAGER_H__
#define __VP_FEATURE_MANAGER_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include "vp_obj_factories.h"

namespace vp
{

class VpResourceManager;

class VPFeatureManager : public MediaFeatureManager
{
public:
    //!
    //! \brief  VPFeatureManager constructor
    //! \param  [in] hwInterface
    //!         Pointer to VP_MHWINTERFACE
    //!
     VPFeatureManager(PVP_MHWINTERFACE hwInterface);

    //!
    //! \brief  VPFeatureManager deconstructor
    //!
    virtual ~VPFeatureManager() {}

    //!
    //! \brief  Check the conflict between features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckFeatures(void *params, bool &bapgFuncSupported);

    //!
    //! \brief  Check the conflict between features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckFeatures(void *params);

protected:
    //!
    //! \brief    Check if HDR are needed
    //! \param    [in] pSrc
    //!           Pointer to input surface of Vebox
    //! \param    [in] pRenderTarget
    //!           Pointer to Render targe surface of VPP BLT
    //! \return   bool
    //!           return true if 2 Passes CSC is needed, otherwise false
    //!
    virtual bool IsHdrNeeded(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget);

    //!
    //! \brief    Check if 2 passes CSC are needed
    //! \param    [in] pSrc
    //!           Pointer to input surface of Vebox
    //! \param    [in] pRenderTarget
    //!           Pointer to Render targe surface of VPP BLT
    //! \return   bool
    //!           return true if 2 Passes CSC is needed, otherwise false
    //!
    virtual bool Is2PassesCSCNeeded(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget);

    virtual bool IsVeboxOutFeasible(PVP_PIPELINE_PARAMS params);

    virtual bool IsVeboxInputFormatSupport(PVPHAL_SURFACE pSrcSurface);

    virtual bool IsVeboxRTFormatSupport(
        PVPHAL_SURFACE pSrcSurface,
        PVPHAL_SURFACE pRTSurface);

    virtual bool IsSfcOutputFeasible(PVP_PIPELINE_PARAMS params);

    virtual bool IsOutputFormatSupported(
        PVPHAL_SURFACE              outSurface);

    //!
    //! \brief    Get the aligned the surface height and width unit
    //! \details  According to the format of the surface, get the aligned unit for the surface
    //!           width and height
    //! \param    [in,out] pwWidthAlignUnit
    //!           Pointer to the surface width alignment unit
    //! \param    [in,out] pwHeightAlignUnit
    //!           Pointer to the surface height alignment unit
    //! \param    [in] format
    //!           The format of the surface
    //! \return   void
    //!
    virtual void GetAlignUnit(
        uint16_t        &wWidthAlignUnit,
        uint16_t        &wHeightAlignUnit,
        MOS_FORMAT      format);

    //!
    //! \brief    Align the src/dst surface rectangle and surface width/height
    //! \details  The surface rects and width/height need to be aligned according to the surface format
    //! \param    [in,out] pSurface
    //!           Pointer to the surface
    //! \param    [in] formatForDstRect
    //!           Format for Dst Rect
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS RectSurfaceAlignment(
        PVPHAL_SURFACE       pSurface,
        MOS_FORMAT           formatForDstRect);

protected:
    PVP_MHWINTERFACE        m_hwInterface       = nullptr;
};



class VpFeatureManagerNext : public MediaFeatureManager
{
public:
    VpFeatureManagerNext(VpAllocator &allocator, VpResourceManager *resourceManager, PVP_MHWINTERFACE pHwInterface);
    virtual ~VpFeatureManagerNext();

    virtual MOS_STATUS Initialize();
    virtual MOS_STATUS InitPacketPipe(VP_PIPELINE_PARAMS &params,
                    PacketPipe &packetPipe);

protected:
    MOS_STATUS CreateHwFilterPipe(VP_PIPELINE_PARAMS &params, HwFilterPipe *&pHwFilterPipe);
    MOS_STATUS UpdateResources(HwFilterPipe &hwFilterPipe);

    VpInterface         m_vpInterface;
    Policy              m_Policy;

private:
    MOS_STATUS Init(void *settings) { return MOS_STATUS_UNIMPLEMENTED; }
};

}
#endif // !__VP_FEATURE_MANAGER_H__
