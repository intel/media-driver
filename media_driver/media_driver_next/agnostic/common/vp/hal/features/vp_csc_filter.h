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
//! \file     vp_csc_filter.h
//! \brief    Defines the common interface for CSC
//!           this file is for the base interface which is shared by all CSC in driver.
//!
#ifndef __VP_CSC_FILTER_H__
#define __VP_CSC_FILTER_H__

#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpCscFilter : public VpFilter
{
public:

    VpCscFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpCscFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamCsc         &cscParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    SFC_CSC_PARAMS *GetSfcParams()
    {
        return m_sfcCSCParams;
    }

protected:

    //!
    //! \brief    Setup Chroma sitting parameters
    //! \details  Setup Chroma sitting parameters
    //! \param    [in] vpExecuteCaps
    //!           Pointer to Vebox Render Execution Caps
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetChromaParams(
        VP_EXECUTE_CAPS         vpExecuteCaps);

    //!
    //! \brief    Setup Chroma sitting parameters
    //! \details  Setup Chroma sitting parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetSubSampling();

    //!
    //! \brief    Check whether Chroma Up Sampling Needed
    //! \details  Check whether Chroma Up Sampling Needed
    //! \return   bool
    //!
    bool IsChromaUpSamplingNeeded();

protected:
    FeatureParamCsc     m_cscParams = {};
    PSFC_CSC_PARAMS     m_sfcCSCParams  = nullptr;
};


struct HW_FILTER_CSC_PARAM
{
    FeatureType             type;
    PVP_MHWINTERFACE        pHwInterface;
    VP_EXECUTE_CAPS         vpExecuteCaps;
    PacketParamFactoryBase *pPacketParamFactory;
    FeatureParamCsc         cscParams;
};

class HwFilterCscParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_CSC_PARAM &param, FeatureType featureType);
    HwFilterCscParameter(FeatureType featureType);
    virtual ~HwFilterCscParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_CSC_PARAM &param);

private:
    HW_FILTER_CSC_PARAM m_Params = {};
};

class VpSfcCscParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_CSC_PARAM &param);
    VpSfcCscParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpSfcCscParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_CSC_PARAM &params);

    VpCscFilter m_CscFilter;
};

class PolicySfcCscHandler : public PolicyFeatureHandler
{
public:
    PolicySfcCscHandler();
    virtual ~PolicySfcCscHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

private:
    PacketParamFactory<VpSfcCscParameter> m_PacketParamFactory;
};

}
#endif // !__VP_CSC_FILTER_H__