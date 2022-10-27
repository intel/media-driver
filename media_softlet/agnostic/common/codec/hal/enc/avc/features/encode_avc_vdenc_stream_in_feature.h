/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_vdenc_stream_in_feature.h
//! \brief    Defines the common feature interface for encode AVC VDEnc Stream-in
//!
#ifndef __ENCODE_AVC_VDENC_STREAM_IN_FEATURE_H__
#define __ENCODE_AVC_VDENC_STREAM_IN_FEATURE_H__

#include "codechal_debug.h"
#include "encode_allocator.h"
#include "encode_avc_basic_feature.h"
#include "mhw_vdbox_vdenc_itf.h"

namespace encode
{
struct AvcVdencStreamInState
{
    union
    {
        //!< DWORD 0
        struct
        {
            uint32_t RegionOfInterestSelection : MOS_BITFIELD_RANGE(0, 7);   //!< Region of Interest (ROI) Selection
            uint32_t ForceIntra                : MOS_BITFIELD_RANGE(8, 8);   //!< ForceIntra
            uint32_t ForceSkip                 : MOS_BITFIELD_RANGE(9, 9);   //!< ForceSkip
            uint32_t Reserved22                : MOS_BITFIELD_RANGE(10, 31); //!< Reserved
        };
        uint32_t Value;
    } DW0;
    union
    {
        //!< DWORD 1
        struct
        {
            uint32_t QpPrimeY         : MOS_BITFIELD_RANGE(0, 7);   //!< QpPrimeY
            uint32_t TargetSizeInWord : MOS_BITFIELD_RANGE(8, 15);  //!< TargetSizeInWord
            uint32_t MaxSizeInWord    : MOS_BITFIELD_RANGE(16, 23); //!< MaxSizeInWord
            uint32_t Reserved8        : MOS_BITFIELD_RANGE(24, 31); //!< Reserved
        };
        uint32_t Value;
    } DW1;
    union
    {
        //!< DWORD 2
        struct
        {
            uint32_t FwdPredictorX : MOS_BITFIELD_RANGE(0, 15);  //!< Fwd Predictor.X
            uint32_t FwdPredictorY : MOS_BITFIELD_RANGE(16, 31); //!< Fwd Predictor.Y
        };
        uint32_t Value;
    } DW2;
    union
    {
        //!< DWORD 3
        struct
        {
            uint32_t BwdPredictorX : MOS_BITFIELD_RANGE(0, 15);  //!< Bwd Predictor.X
            uint32_t BwdPredictorY : MOS_BITFIELD_RANGE(16, 31); //!< Bwd Predictor.Y
        };
        uint32_t Value;
    } DW3;
    union
    {
        //!< DWORD 4
        struct
        {
            uint32_t FwdRefId0   : MOS_BITFIELD_RANGE(0, 3);  //!< Fwd RefID0
            uint32_t BwdRefId0   : MOS_BITFIELD_RANGE(4, 7);  //!< Bwd RefID0
            uint32_t Reserved24  : MOS_BITFIELD_RANGE(8, 31); //!< Reserved
        };
        uint32_t Value;
    } DW4;

    uint32_t Reserved11[11];  //!< Reserved

    AvcVdencStreamInState()
    {
        ENCODE_ASSERT(AvcVdencStreamInState::byteSize == CODECHAL_CACHELINE_SIZE);

        DW0.Value = 0;
        DW1.Value = 0;
        DW2.Value = 0;
        DW3.Value = 0;
        DW4.Value = 0;
        MOS_ZeroMemory(&Reserved11, sizeof(Reserved11));
    }

    static const size_t byteSize;
};


class AvcVdencStreamInFeature : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::huc::Itf::ParSetting
{
public:
    //!
    //! \brief  AvcVDEncStreamIn constructor
    //!
    AvcVdencStreamInFeature(
        MediaFeatureManager* featureManager,
        EncodeAllocator* allocator,
        CodechalHwInterfaceNext *hwInterface,
        void* constSettings);

    //!
    //! \brief  AvcVDEncStreamIn destructor
    //!
    virtual ~AvcVdencStreamInFeature();

    //!
    //! \brief  Init AVC VDEnc Stream-in instance
    //! \param  [in] basicFeature
    //!         Pointer to basic feature
    //! \param  [in] allocator
    //!         Pointer to allocator
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void* setting) override;

    //!
    //! \brief  Update VDEnc Stream-in buffer for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void* setting) override;

    //!
    //! \brief  Get VDEnc Stream-in buffer base locked addrress
    //! \return AvcVdencStreamInState*
    //!         pointer to stream in buffer locked address
    //!
    virtual AvcVdencStreamInState* Lock();

    //!
    //! \brief  Unlock VDEnc Stream-in buffer base locked addrress
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Unlock();

    //!
    //! \brief  Enable VDEnc Stream-in feature
    //!         Should be called only if VDEnc Stream-in will be used
    //!
    MOS_STATUS Enable();

    //!
    //! \brief  Reset VDEnc Stream-in after frame programming is done
    //!
    void Reset();

    //!
    //! \brief  Clear VDEnc Stream-in buffer data
    //!
    MOS_STATUS Clear();

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Dump(CodechalDebugInterface *itf, const char* bufName);
#endif

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

protected:

    CodechalHwInterfaceNext *m_hwInterface    = nullptr;
    AvcBasicFeature     *m_basicFeature   = nullptr;  //!< AVC paramter
    EncodeAllocator     *m_allocator      = nullptr;  //!< Encode allocator
    PMOS_RESOURCE        m_streamInBuffer = nullptr;  //!< Stream in buffer

    bool     m_updated     = false;  //!< Indicate stream in buffer updated
    bool     m_enabled     = false;  //!< Indicate stream in enabled for current frame or not
    uint32_t m_widthInMb   = 0;
    uint32_t m_heightInMb  = 0;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencStreamInFeature)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_VDENC_STREAM_IN_FEATURE_H__
