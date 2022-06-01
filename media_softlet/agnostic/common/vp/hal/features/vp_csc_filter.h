/*
* Copyright (c) 2018-2022, Intel Corporation
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

    virtual ~VpCscFilter()
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

    MOS_STATUS CalculateSfcEngineParams();

    MOS_STATUS CalculateVeboxEngineParams();

    SFC_CSC_PARAMS *GetSfcParams()
    {
        return m_sfcCSCParams;
    }

    VEBOX_CSC_PARAMS* GetVeboxParams()
    {
        return m_veboxCSCParams;
    }

protected:

    //!
    //! \brief    Setup Chroma sitting parameters
    //! \details  Setup Chroma sitting parameters
    //! \param    [in] vpExecuteCaps
    //!           Pointer to Vebox Render Execution Caps
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetSfcChromaParams(
        VP_EXECUTE_CAPS         vpExecuteCaps);

    //!
    //! \brief    Setup Vebox Chroma up sampling parameters
    //! \details  Setup Chroma sitting parameters
    //! \param    [in] vpExecuteCaps
    //!           Pointer to Vebox Render Execution Caps
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetVeboxCUSChromaParams(
        VP_EXECUTE_CAPS         vpExecuteCaps);

    //!
//! \brief    Setup Vebox Chroma down sampling parameters
//! \details  Setup Chroma sitting parameters
//! \param    [in] vpExecuteCaps
//!           Pointer to Vebox Render Execution Caps
//! \return   MOS_STATUS
//!
    MOS_STATUS SetVeboxCDSChromaParams(
        VP_EXECUTE_CAPS         vpExecuteCaps);

    //!
    //! \brief    Setup Chroma sitting parameters
    //! \details  Setup Chroma sitting parameters
    //! \param    [in] vpExecuteCaps
    //!           Pointer to Vebox Render Execution Caps
    //! \return   MOS_STATUS
    //!
    MOS_STATUS UpdateChromaSiting(
        VP_EXECUTE_CAPS         vpExecuteCaps);

    //!
    //! \brief    Check whether Chroma Up Sampling Needed
    //! \details  Check whether Chroma Up Sampling Needed
    //! \return   bool
    //!
    bool IsChromaUpSamplingNeeded();

    //!
    //! \brief    Check whether dithering Needed
    //! \details  Check whether dithering Needed
    //! \param    [in] formatInput
    //!           The input format
    //!           [in] formatOutput
    //!           The output format
    //! \return   bool
    //!
    bool IsDitheringNeeded(MOS_FORMAT formatInput, MOS_FORMAT formatOutput);

protected:
    FeatureParamCsc     m_cscParams = {};
    PSFC_CSC_PARAMS     m_sfcCSCParams   = nullptr;
    PVEBOX_CSC_PARAMS   m_veboxCSCParams = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpCscFilter)
};


struct HW_FILTER_CSC_PARAM : public HW_FILTER_PARAM
{
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

MEDIA_CLASS_DEFINE_END(vp__HwFilterCscParameter)
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

MEDIA_CLASS_DEFINE_END(vp__VpSfcCscParameter)
};

class PolicySfcCscHandler : public PolicyFeatureHandler
{
public:
    PolicySfcCscHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicySfcCscHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual MOS_STATUS UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeCscOnSfc)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid Parameter for SFC CSC!");
            return nullptr;
        }

        HW_FILTER_CSC_PARAM* cscParam = (HW_FILTER_CSC_PARAM*)(&param);
        return VpSfcCscParameter::Create(*cscParam);
    }

private:
    PacketParamFactory<VpSfcCscParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicySfcCscHandler)
};

class VpVeboxCscParameter : public VpPacketParameter
{
public:
    static VpPacketParameter* Create(HW_FILTER_CSC_PARAM& param);
    VpVeboxCscParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase* packetParamFactory);
    virtual ~VpVeboxCscParameter();

    virtual bool SetPacketParam(VpCmdPacket* pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_CSC_PARAM& params);

    VpCscFilter m_CscFilter;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxCscParameter)
};

class PolicyVeboxCscHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxCscHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxCscHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter* CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual MOS_STATUS         UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeCscOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid Parameter for Vebox CSC!");
            return nullptr;
        }

        HW_FILTER_CSC_PARAM* cscParam = (HW_FILTER_CSC_PARAM*)(&param);
        return VpVeboxCscParameter::Create(*cscParam);
    }

private:
    PacketParamFactory<VpVeboxCscParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyVeboxCscHandler)
};
}
#endif // !__VP_CSC_FILTER_H__
