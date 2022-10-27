/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_avc_vdenc_roi_interface.h
//! \brief    Define interface of the ROI features of AVC VDENC
//!

#ifndef __CODECHAL_AVC_VDENC_ROI_INTERFACE_H__
#define __CODECHAL_AVC_VDENC_ROI_INTERFACE_H__

#include "encode_avc_brc.h"
#include "encode_avc_vdenc_stream_in_feature.h"

namespace encode
{

//!
//! \class    AvcVdencRoiInterface
//!
//! \brief    AvcVdencRoiInterface is unified interface of VDEnc ROI and Dirty ROI.
//!
//! \detail   This class is an feature interface for AVC native and non-native ROI
//!           in both ForceQP and DeltaQP modes and Dirty ROI.
class AvcVdencRoiInterface : public MediaFeature
{
public:

    union SupportedModes
    {
        struct
        {
            uint32_t ROI_Native    : MOS_BITFIELD_RANGE(0, 0);
            uint32_t ROI_NonNative : MOS_BITFIELD_RANGE(1, 1);
            uint32_t MBQP_ForceQP  : MOS_BITFIELD_RANGE(2, 2);
            uint32_t MBQP_DeltaQP  : MOS_BITFIELD_RANGE(3, 3);
            uint32_t DirtyROI      : MOS_BITFIELD_RANGE(4, 4);
            uint32_t ArbROI        : MOS_BITFIELD_RANGE(5, 5);
            uint32_t Reserved26    : MOS_BITFIELD_RANGE(6, 31);
        };
        uint32_t Value;
        SupportedModes() : Value(0) {}
    };

    AvcVdencRoiInterface(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings,
        SupportedModes& supportedModes);

    virtual ~AvcVdencRoiInterface() {}

    //!
    //! \brief  Init encode parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

protected:

    //!
    //! \brief    Sort and set distinct delta QPs
    //!
    //! \return   bool
    //!           true if native ROI, otherwise false
    //!
    bool ProcessRoiDeltaQp();

    virtual MOS_STATUS SetupROI();

    virtual MOS_STATUS SetupDirtyROI();

    virtual MOS_STATUS SetupMBQP();

    virtual MOS_STATUS SetupForceSkip();

    virtual MOS_STATUS SetupArbROI();

    MOS_STATUS IsModesSupported(uint32_t modes, std::string featureName);

    MOS_STATUS GetDeltaQPIndex(uint32_t maxNumRoi, int32_t dqp, int32_t& dqpIdx);

    static constexpr uint8_t m_maxNumRoi       = 16;  //!< VDEnc maximum number of ROI supported (uncluding non-ROI zone0)
    static constexpr uint8_t m_maxNumNativeRoi = 3;   //!< Number of native ROI with different dQP supported by VDEnc HW

    EncodeAllocator*         m_allocator            = nullptr;
    CodechalHwInterfaceNext*     m_hwInterface          = nullptr;
    AvcBasicFeature*         m_basicFeature         = nullptr;
    AvcEncodeBRC*            m_brcFeature           = nullptr;
    AvcVdencStreamInFeature* m_vdencStreamInFeature = nullptr;
    PCODEC_AVC_ENCODE_PIC_PARAMS m_picParam = nullptr;

    SupportedModes   m_supportedModes;
    bool             m_isNativeRoi = false;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencRoiInterface)
};

}  // namespace encode
#endif  //<! __CODECHAL_AVC_VDENC_ROI_INTERFACE_H__
