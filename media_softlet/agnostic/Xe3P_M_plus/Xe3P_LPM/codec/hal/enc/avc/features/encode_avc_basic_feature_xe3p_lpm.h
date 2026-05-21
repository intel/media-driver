/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_avc_basic_feature_xe3p_lpm.h
//! \brief    Defines the common interface for encode avc Xe3P_LPM basic feature
//!
#ifndef __ENCODE_AVC_BASIC_FEATURE_XE3P_LPM_H__
#define __ENCODE_AVC_BASIC_FEATURE_XE3P_LPM_H__

#include "encode_avc_basic_feature.h"

namespace encode
{

class AvcBasicFeatureXe3P_Lpm : public AvcBasicFeature
{
public:
    AvcBasicFeatureXe3P_Lpm(EncodeAllocator *allocator,
                           CodechalHwInterfaceNext *hwInterface,
                           TrackedBuffer *trackedBuf,
                           RecycleResource *recycleBuf,
                           MediaCopyWrapper *mediaCopyWrapper,
                           void *constSettings = nullptr);

    virtual ~AvcBasicFeatureXe3P_Lpm();

    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief    Get the minimum AVC QP value for Xe3P_LPM platform
    //! \return   uint8_t Returns CODEC_AVC_MIN_QP5 (5) for VBR rate control,
    //!                   CODEC_AVC_MIN_QP10 (10) for other rate control methods
    //!
    virtual uint8_t GetMinAvcQp() override;

    virtual void UpdateMinMaxQp() override;

    //!
    //! \brief    Get VDENC read batch buffer for original TU
    //! \return   Pointer to VDENC BRC image states read buffer array for original TU
    //!
    PMOS_RESOURCE GetVdencReadBatchBufferOrigin(uint32_t index)
    {
        if (index < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
        {
            return m_vdencBrcImageStatesReadBufferOrigin[index];
        }
        return nullptr;
    }

    //!
    //! \brief    Get VDENC read batch buffer for TU7
    //! \return   Pointer to VDENC BRC image states read buffer array for TU7
    //!
    PMOS_RESOURCE GetVdencReadBatchBufferTU7(uint32_t index)
    {
        if (index < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
        {
            return m_vdencBrcImageStatesReadBufferTU7[index];
        }
        return nullptr;
    }

    //!
    //! \brief    Get VDEnc 2nd level batch buffer
    //! \details  Returns pointer to the VDEnc 2nd level batch buffer for SLBB update output
    //! \param    [in] recycledBufIdx
    //!           Recycled buffer index
    //! \return   MHW_BATCH_BUFFER*
    //!           Pointer to the VDEnc 2nd level batch buffer
    //!
    MHW_BATCH_BUFFER* GetVdenc2ndLevelBatchBuffer(uint32_t recycledBufIdx);
    MHW_BATCH_BUFFER* GetVdenc2ndLevelBatchBufferTU7(uint32_t recycledBufIdx);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(VDENC_AVC_IMG_STATE);

protected:
    PMOS_RESOURCE m_vdencBrcImageStatesReadBufferOrigin[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};  //!< Read-only VDENC+PAK IMG STATE buffer for original TU
    PMOS_RESOURCE m_vdencBrcImageStatesReadBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};     //!< Read-only VDENC+PAK IMG STATE buffer for TU7
    //!< VDEnc 2nd level batch buffer for SLBB update output
    MHW_BATCH_BUFFER m_vdenc2ndLevelBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};
    //!< VDEnc 2nd level batch buffer for TU7 SLBB update output
    MHW_BATCH_BUFFER m_vdenc2ndLevelBatchBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};

    CodechalHwInterfaceNext *m_hwInterface = nullptr;  //!< Pointer to hardware interface

    //!
    //! \brief    Get VDENC one slice state size
    //! \return   Slice state size in bytes
    //!
    uint32_t GetVdencOneSliceStateSize();

MEDIA_CLASS_DEFINE_END(encode__AvcBasicFeatureXe3P_Lpm)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_BASIC_FEATURE_XE3P_LPM_H__
