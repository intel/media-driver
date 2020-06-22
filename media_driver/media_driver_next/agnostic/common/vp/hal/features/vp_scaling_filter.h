/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     vp_scaling_filter.h
//! \brief    Defines the common interface for scaling
//!           this file is for the base interface which is shared by all scaling in driver.
//!
#ifndef __VP_SCALING_FILTER_H__
#define __VP_SCALING_FILTER_H__

#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpScalingFilter : public VpFilter
{
public:

    VpScalingFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpScalingFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamScaling     &scalingParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    SFC_SCALING_PARAMS *GetSfcParams()
    {
        return m_sfcScalingParams;
    }

protected:
    //!
    //! \brief      Gen specific function for SFC adjust boundary
    //! \details    Adjust the width and height of the input surface for SFC
    //! \param      [out] pdwSurfaceWidth
    //!             Pointer to adjusted surface width
    //! \param      [out] pdwSurfaceHeight
    //!             Pointer to adjusted surface height
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SfcAdjustBoundary(
        uint32_t                   *pdwSurfaceWidth,
        uint32_t                   *pdwSurfaceHeight);

    //!
    //! \brief    Get width and height align unit of input format
    //! \param    [in] inputFormat
    //!           input format
    //! \param    [out] widthAlignUnit
    //!           width align unit
    //! \param    [out] heightAlignUnit
    //!           height align unit
    //! \return   void
    //!
    virtual void GetFormatWidthHeightAlignUnit(
        MOS_FORMAT              inputFormat,
        uint16_t                &widthAlignUnit,
        uint16_t                &heightAlignUnit);

    //!
    //! \brief    check whether colorfill enable or not
    //! \return   MOS_STATUS
    //!  MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsColorfillEnable();

    //!
    //! \brief    set colorfill Params
    //! \return   MOS_STATUS
    //!  MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetColorFillParams();

    //!
    //! \brief    set colorfill YUV/RGB Pixel values
    //! \return   MOS_STATUS
    //!  MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetYUVRGBPixel();

    //!
    //! \brief    set Alpha values
    //! \return   MOS_STATUS
    //!  MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetAlphaPixelParams();

    //!
    //! \brief    set Src/Dst rect values
    //! \return   MOS_STATUS
    //!  MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRectSurfaceAlignment(MOS_FORMAT format, bool isOutputSurf, uint32_t &width, uint32_t &height, RECT &rcSrc, RECT &rcDst);

protected:
    FeatureParamScaling          m_scalingParams;
    SFC_SCALING_PARAMS          *m_sfcScalingParams  = nullptr;
    float                        fScaleX             = 0.0f;
    float                        fScaleY             = 0.0f;

    // colorfill params
    bool                         m_bColorfillEnable  = false;
    VPHAL_COLOR_SAMPLE_8         m_colorFillColorSrc = {};                 //!< ColorFill Sample from DDI
    VPHAL_COLOR_SAMPLE_8         m_colorFillColorDst = {};                 //!< ColorFill Sample programmed in Sfc State
    VPHAL_CSPACE                 m_colorFillSrcCspace = {};                //!< Cspace of the source ColorFill Color
    VPHAL_CSPACE                 m_colorFillRTCspace = {};                 //!< Cspace of the Render Target

};

struct HW_FILTER_SCALING_PARAM
{
    FeatureType             type;
    PVP_MHWINTERFACE        pHwInterface;
    VP_EXECUTE_CAPS         vpExecuteCaps;
    PacketParamFactoryBase *pPacketParamFactory;
    FeatureParamScaling     scalingParams;
};

class HwFilterScalingParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_SCALING_PARAM &param, FeatureType featureType);
    HwFilterScalingParameter(FeatureType featureType);
    virtual ~HwFilterScalingParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);
    MOS_STATUS Initialize(HW_FILTER_SCALING_PARAM &param);

private:

    HW_FILTER_SCALING_PARAM         m_Params = {};
};

class VpSfcScalingParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_SCALING_PARAM &param);
    VpSfcScalingParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpSfcScalingParameter();
    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_SCALING_PARAM &params);

    VpScalingFilter m_ScalingFilter;
};

class PolicySfcScalingHandler : public PolicyFeatureHandler
{
public:
    PolicySfcScalingHandler();
    virtual ~PolicySfcScalingHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

private:
    PacketParamFactory<VpSfcScalingParameter> m_PacketParamFactory;
};
}
#endif // !__VP_SCALING_FILTER_H__