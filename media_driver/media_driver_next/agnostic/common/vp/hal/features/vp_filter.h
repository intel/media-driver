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
//! \file     vp_filter.h
//! \brief    Defines the common interface for vp filters
//!           this file is for the base interface which is shared by all features.
//!
#ifndef __VP_FILTER_H__
#define __VP_FILTER_H__

#include <map>
#include "mos_defs.h"
#include "vp_pipeline_common.h"
#include "vp_sfc_common.h"
#include "vp_utils.h"
#include "sw_filter.h"

namespace vp {
class VpCmdPacket;

class VpFilter
{
public:
    VpFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    virtual ~VpFilter() {};

    //!
    //! \brief  Initialize the media filter, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() = 0;

    //!
    //! \brief  Prepare the parameters for filter generation
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() = 0;

    //!
    //! \brief  Destroy the media Filter and release the resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() = 0;

    //!
    //! \brief  Get execute caps for this filter
    //! \return VP_EXECUTE_CAPS
    //!         return the caps of filters
    //!
    VP_EXECUTE_CAPS GetExecuteCaps()
    {
        return m_executeCaps;
    }

    //!
    //! \brief  Get current associated media Packet
    //! \return MediaTask*
    //!         return the media task pointer
    //!
    VpCmdPacket * GetActivePacket()
    {
        return m_packet;
    }

    //!
    //! \brief  Set current associated media Packet
    //! \return MediaTask*
    //!         return the media task pointer
    //!
    void SetPacket(VpCmdPacket* packet)
    {
        m_packet = packet;
    }

protected:

    PVP_MHWINTERFACE      m_pvpMhwInterface = nullptr;   // vp HW interfaces
    VP_EXECUTE_CAPS       m_executeCaps = {};        // Filter executed caps
    PVPHAL_SURFACE        m_tempSurface = nullptr;   // Inter-Media surface for Filter temp output

    VpCmdPacket         * m_packet = nullptr;

};

struct _SFC_SCALING_PARAMS
{
    // Scaling parameters

    uint32_t                        dwOutputFrameHeight;                        // Output Frame Height
    uint32_t                        dwOutputFrameWidth;                         // Output Frame Width
    uint32_t                        dwInputFrameHeight;                         // Input Frame Height
    uint32_t                        dwInputFrameWidth;                          // Input Frame Width

    uint32_t                        dwAVSFilterMode;                            // Bilinear, 5x5 or 8x8
    uint32_t                        dwSourceRegionHeight;                       // Source/Crop region height
    uint32_t                        dwSourceRegionWidth;                        // Source/Crop region width
    uint32_t                        dwSourceRegionVerticalOffset;               // Source/Crop region vertical offset
    uint32_t                        dwSourceRegionHorizontalOffset;             // Source/Crop region horizontal offset
    uint32_t                        dwScaledRegionHeight;                       // Scaled region height
    uint32_t                        dwScaledRegionWidth;                        // Scaled region width
    uint32_t                        dwScaledRegionVerticalOffset;               // Scaled region vertical offset
    uint32_t                        dwScaledRegionHorizontalOffset;             // Scaled region horizontal offset
    float                           fAVSXScalingRatio;                          // X Scaling Ratio
    float                           fAVSYScalingRatio;                          // Y Scaling Ratio

    SFC_COLORFILL_PARAMS            sfcColorfillParams;                         // Colorfill Params

};

struct _SFC_CSC_PARAMS
{
    bool                            bCSCEnabled;                                 // CSC Enabled
    bool                            bInputColorSpace;                            // 0: YUV color space, 1:RGB color space
    bool                            bIEFEnable;                                  // IEF Enabled
    bool                            bChromaUpSamplingEnable;                     // ChromaUpSampling
    bool                            b8tapChromafiltering;                        // Enables 8 tap filtering for Chroma Channels
    VPHAL_CSPACE                    inputColorSpcase;                            // Input Color Space
    MOS_FORMAT                      inputFormat;                                 // SFC Input Format
    MOS_FORMAT                      outputFormat;                                // SFC Output Format
    PVPHAL_IEF_PARAMS               iefParams;                                   // Vphal Params
    uint32_t                        inputChromaSubSampling;                      // Chroma subsampling at SFC input
    uint32_t                        sfcSrcChromaSiting;                          // SFC Source Chroma Siting location
    uint32_t                        chromaDownSamplingVerticalCoef;              // Chroma DownSampling Vertical Coeff
    uint32_t                        chromaDownSamplingHorizontalCoef;            // Chroma DownSampling Horizontal Coeff
};

struct _SFC_ROT_MIR_PARAMS
{
    VPHAL_ROTATION                  rotationMode;                               // Rotation mode -- 0, 90, 180 or 270
    uint32_t                        mirrorType;                               // Mirror Type -- vert/horiz
    bool                            bMirrorEnable;                              // Mirror mode -- enable/disable
};

// Vebox obligatory parameters when enabling vebox features.
struct VEBOX_OBLI_PARAMS
{
    PVPHAL_SURFACE                  pCurrInput;
    PVPHAL_SURFACE                  pCurrOutput;    // will be removed from here, this is not a obligate parameter. for example SFC scaling only
    PVPHAL_SURFACE                  pStatisticsOutput;
    PVPHAL_SURFACE                  pLaceOrAceOrRgbHistogram;
};

struct _VEBOX_DN_PARAMS
{
    VEBOX_OBLI_PARAMS               VeboxObliParams;
    PVPHAL_SURFACE                  pPrevInput;
    PVPHAL_SURFACE                  pSTMMInput;
    PVPHAL_SURFACE                  pSTMMOutput;
    PVPHAL_SURFACE                  pDenoisedCurrOutput;
    bool                            bRefValid;
};

struct _VEBOX_DI_PARAMS
{
    VEBOX_OBLI_PARAMS               VeboxObliParams;
    PVPHAL_SURFACE                  pPrevOutput;
    PVPHAL_SURFACE                  pSTMMInput;
    PVPHAL_SURFACE                  pSTMMOutput;
    uint32_t                        DIOutputFrames;
};

enum _FilterType
{
    VP_FILTER_SFC_SCALING = 0,
    VP_FILTER_SFC_CSC,
    VP_FILTER_SFC_ROTMIR,
    VP_FILTER_MAX
};

using SFC_SCALING_PARAMS    = _SFC_SCALING_PARAMS;
using PSFC_SCALING_PARAMS   = SFC_SCALING_PARAMS * ;
using SFC_CSC_PARAMS        = _SFC_CSC_PARAMS;
using PSFC_CSC_PARAMS       = SFC_CSC_PARAMS * ;
using SFC_ROT_MIR_PARAMS    = _SFC_ROT_MIR_PARAMS;
using PSFC_ROT_MIR_PARAMS   = SFC_ROT_MIR_PARAMS * ;
using VEBOX_DN_PARAMS       = _VEBOX_DN_PARAMS;
using PVEBOX_DN_PARAMS      = VEBOX_DN_PARAMS *;
using FilterType            = _FilterType;

class SwFilterPipe;
class HwFilter;
class PacketParamFactoryBase;

/////////////////////////////HwFilter Parameters///////////////////////////////////
class HwFilterParameter
{
public:
    HwFilterParameter(FeatureType featureType);
    virtual ~HwFilterParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter) = 0;

    FeatureType GetFeatureType()
    {
        return m_FeatureType;
    }

private:
    FeatureType m_FeatureType = FeatureTypeInvalid;
};

/////////////////////////////Packet Parameters///////////////////////////////////

class VpPacketParameter
{
public:
    VpPacketParameter(PacketParamFactoryBase *packetParamFactory);
    virtual ~VpPacketParameter();

    static void Destory(VpPacketParameter *&p);

    virtual bool SetPacketParam(VpCmdPacket *pPacket) = 0;

private:
    PacketParamFactoryBase *m_packetParamFactory = nullptr;
};

/////////////////////////////Policy Feature Handler//////////////////////////////

class PolicyFeatureHandler
{
public:
    PolicyFeatureHandler();
    virtual ~PolicyFeatureHandler();
    virtual bool IsFeatureEnabled(SwFilterPipe &swFilterPipe);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    FeatureType GetType();
    HwFilterParameter *GetHwFeatureParameterFromPool();
    MOS_STATUS ReleaseHwFeatureParameter(HwFilterParameter *&pParam);
protected:
    FeatureType m_Type = FeatureTypeInvalid;
    std::vector<HwFilterParameter *> m_Pool;
};

class PacketParamFactoryBase
{
public:
    PacketParamFactoryBase();
    virtual ~PacketParamFactoryBase();
    virtual VpPacketParameter *GetPacketParameter(PVP_MHWINTERFACE pHwInterface) = 0;
    void ReturnPacketParameter(VpPacketParameter *&p);
protected:
    std::vector<VpPacketParameter *> m_Pool;
};

template<class T>
class PacketParamFactory : public PacketParamFactoryBase
{
public:
    PacketParamFactory()
    {
    }
    virtual ~PacketParamFactory()
    {
    }
    virtual VpPacketParameter *GetPacketParameter(PVP_MHWINTERFACE pHwInterface)
    {
        if (nullptr == pHwInterface)
        {
            return nullptr;
        }
        if (m_Pool.empty())
        {
            T *p = MOS_New(T, pHwInterface, this);
            if (nullptr == p)
            {
                return nullptr;
            }

            VpPacketParameter *pBase = dynamic_cast<VpPacketParameter *>(p);

            if (nullptr == pBase)
            {
                MOS_Delete(p);
            }
            return pBase;
        }
        else
        {
            VpPacketParameter *p = m_Pool.back();
            m_Pool.pop_back();
            return p;
        }
    }
};
}



#endif // !__VP_FILTER_H__