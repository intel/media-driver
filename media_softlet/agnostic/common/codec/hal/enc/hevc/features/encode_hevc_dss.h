/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     encode_hevc_dss.h
//! \brief    Defines the common interface for hevc encode dynamic slice feature
//!

#ifndef __ENCODE_HEVC_DSS_H__
#define __ENCODE_HEVC_DSS_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "encode_basic_feature.h"
#include "encode_pipeline.h"
#include "encode_status_report.h"

#define ENCODE_HEVC_MIN_DSS_PIC_WIDTH    480
#define ENCODE_HEVC_MIN_DSS_PIC_HEIGHT   320

namespace encode
{
    class HevcEncodeDss : public MediaFeature
    {
    public:
        HevcEncodeDss(MediaFeatureManager *featureManager, EncodeAllocator *allocator, CodechalHwInterfaceNext *hwInterface, void *constSettings);

        ~HevcEncodeDss() {}

        //!
        //! \brief  Init cqp basic features related parameter
        //! \param  [in] settings
        //!         Pointer to settings
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init(void *settings);

        //!
        //! \brief  Update cqp basic features related parameter
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Update(void *params);

        //!
        //! \brief    Add command to read the HCP status
        //!
        //! \param    [in] vdboxIndex
        //!           Index of vdbox
        //! \param    [in] statusReport
        //!           Encode status report
        //! \param    [in, out] cmdBuffer
        //!           Command buffer
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ReadHcpStatus(
            MHW_VDBOX_NODE_IND  vdboxIndex,
            MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS ReadSliceSizeForSinglePipe(EncodePipeline *pipeline, MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS ReadSliceSize(EncodePipeline *pipeline, MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS CopyDataBlock(
            PMOS_RESOURCE       sourceSurface,
            uint32_t            sourceOffset,
            PMOS_RESOURCE       destSurface,
            uint32_t            destOffset,
            uint32_t            copySize,
            MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS GetDssBuffer(PMOS_RESOURCE &resSliceCountBuffer, PMOS_RESOURCE &resVDEncModeTimerBuffer);

    protected:
        //! \brief  Allocate feature related resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AllocateResources();

        // Parameters passed from application
        const CODEC_HEVC_ENCODE_PICTURE_PARAMS * m_hevcPicParams   = nullptr;  //!< Pointer to picture parameter
        const CODEC_HEVC_ENCODE_SEQUENCE_PARAMS *m_hevcSeqParams   = nullptr;  //!< Pointer to sequence parameter
        const CODEC_HEVC_ENCODE_SLICE_PARAMS *   m_hevcSliceParams = nullptr;  //!< Pointer to slice parameter
        EncodeAllocator *                        m_allocator       = nullptr;
        CodechalHwInterfaceNext *                m_hwInterface     = nullptr;
        EncodeBasicFeature *                     m_basicFeature    = nullptr;  //!< EncodeBasicFeature
        MOS_RESOURCE                             m_resSliceReport[CODECHAL_ENCODE_STATUS_NUM] = {};
        PMOS_RESOURCE                            m_resSliceCountBuffer     = nullptr;  //!< Resource of slice count buffer
        PMOS_RESOURCE                            m_resVDEncModeTimerBuffer = nullptr;  //!< Resource of Vdenc mode timer buffer
        HEVC_TILE_STATS_INFO                     m_hevcTileStatsOffset = {};

        std::shared_ptr<mhw::vdbox::hcp::Itf>   m_hcpItf   = nullptr;
        std::shared_ptr<mhw::mi::Itf>           m_miItf    = nullptr;
        std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__HevcEncodeDss)
    };

}
#endif
