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
//! \file     encode_hevc_vdenc_feature_manager.h
//! \brief    Defines the common interface for henvc vdenc feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_HEVC_VDENC_FEATURE_MANAGER_H__
#define __ENCODE_HEVC_VDENC_FEATURE_MANAGER_H__
#include <vector>
#include "media_hevc_feature_defs.h"
#include "encode_feature_manager.h"
#include "encode_hevc_basic_feature.h"
#include "encode_hevc_cqp.h"
#include "encode_hevc_vdenc_roi.h"
#include "encode_hevc_tile.h"
#include "encode_hevc_dss.h"
#include "encode_recycle_resource.h"
#include "encode_hevc_vdenc_scc.h"

namespace encode
{

class EncodeHevcVdencFeatureManager : public EncodeFeatureManager
{
public:
    //!
    //! \brief  EncodeHevcVdencFeatureManager constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
     EncodeHevcVdencFeatureManager(EncodeAllocator *allocator,
                             CodechalHwInterfaceNext *hwInterface,
                             TrackedBuffer       *trackedBuf,
                             RecycleResource     *recycleBuf):
                             m_allocator(allocator),
                             m_hwInterface(hwInterface),
                             m_recycleResource(recycleBuf),
                             m_trackedBuf(trackedBuf) {};

    //!
    //! \brief  EncodeHevcVdencFeatureManager deconstructor
    //!
    virtual ~EncodeHevcVdencFeatureManager() {}

    //!
    //! \brief  Check the conflict between features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckFeatures(void *params) override;
    //!
    //! \brief  Create feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateConstSettings() override;

protected:

    //!
    //! \brief  Create features
    //! \param  [in] constsettings
    //!         feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatures(void *constSettings) override;

    //!
    //! \brief  Map target usage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MapTargetUsage(uint8_t &targetUsage);

    //!
    //! \brief  Validate Random Access params
    //! \param  [in] hevcSeqParams
    //!         sequese parameters from DDI
    //! \param  [in] hevcPictureParams
    //!         picture parameters from DDI
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateRandomAccess(
                PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
                PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
                PCODEC_HEVC_ENCODE_SLICE_PARAMS    slcParams);

    //!
    //! \brief  Validate SCC status
    //! \param  [in] hevcPictureParams
    //!         picture parameters from DDI
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateSCC(PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams);

    //!
    //! \brief  Validate ACQP status
    //! \param  [in] hevcSeqParams
    //!         sequese parameters from DDI
    //! \param  [in] hevcPictureParams
    //!         picture parameters from DDI
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateACQP(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams);

    MOS_STATUS ValidatePassNum(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams);

    //!
    //! \brief  Check Platform Capabilities
    //! \param  [in] hevcSeqParams
    //!         sequese parameters from DDI
    //! \param  [in] hevcPictureParams
    //!         picture parameters from DDI
    //! \param  [in] hevcSliceParams
    //!         slice parameters from DDI
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckPlatformCapability(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams);

    EncodeAllocator *m_allocator = nullptr;

    CodechalHwInterfaceNext *m_hwInterface = nullptr;
    RecycleResource     *m_recycleResource = nullptr;
    TrackedBuffer       *m_trackedBuf = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeHevcVdencFeatureManager)
};


}
#endif // !__ENCODE_HEVC_VDENC_FEATURE_MANAGER_H__
