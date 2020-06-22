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
//! \file     vp_rot_mir_filter.h
//! \brief    Defines the common interface for rotation/mirror
//!           this file is for the base interface which is shared by rotation/mirror in driver.
//!

#ifndef __VP_ROT_MIR_FILTER_H__
#define __VP_ROT_MIR_FILTER_H__

#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {

class VpRotMirFilter : public VpFilter
{
public:

    VpRotMirFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpRotMirFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamRotMir      &rotMirParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    SFC_ROT_MIR_PARAMS *GetSfcParams()
    {
        return m_sfcRotMirParams;
    }
protected:

    //!
    //! \brief    Setup Rotation and Mirrow params
    //! \details  Setup Rotation and Mirrow params
    //! \param    [in] Rotation
    //!           Pointer to surface rotation params
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetRotationAndMirrowParams(
        VPHAL_ROTATION     Rotation);

    //!
    //! \brief    Setup Rotation and Mirrow params
    //! \details  Setup Rotation and Mirrow params
    //! \param    [in] Rotation
    //!           surface rotation params
    //! \return   VPHAL_ROTATION
    //!
    VPHAL_ROTATION GetRotationParam(
        VPHAL_ROTATION Rotation);

protected:
    FeatureParamRotMir      m_rotMirParams = {};
    PSFC_ROT_MIR_PARAMS     m_sfcRotMirParams = nullptr;
};

struct HW_FILTER_ROT_MIR_PARAM
{
    FeatureType             type;
    PVP_MHWINTERFACE        pHwInterface;
    VP_EXECUTE_CAPS         vpExecuteCaps;
    PacketParamFactoryBase *pPacketParamFactory;
    FeatureParamRotMir      rotMirParams;
};

class HwFilterRotMirParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_ROT_MIR_PARAM &param, FeatureType featureType);
    HwFilterRotMirParameter(FeatureType featureType);
    virtual ~HwFilterRotMirParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);
    MOS_STATUS Initialize(HW_FILTER_ROT_MIR_PARAM &param);

private:
    HW_FILTER_ROT_MIR_PARAM m_Params = {};
};

class VpSfcRotMirParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_ROT_MIR_PARAM &param);
    VpSfcRotMirParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpSfcRotMirParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_ROT_MIR_PARAM & params);

    VpRotMirFilter m_RotMirFilter;
};

class PolicySfcRotMirHandler : public PolicyFeatureHandler
{
public:
    PolicySfcRotMirHandler();
    virtual ~PolicySfcRotMirHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
private:
    PacketParamFactory<VpSfcRotMirParameter> m_PacketParamFactory;
};
}
#endif//__VP_ROT_MIR_FILTER_H__