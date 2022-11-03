/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_hevc_slice_packet_xe_m_base.h
//! \brief    Defines the implementation of hevc decode slice packet
//!

#ifndef __DECODE_HEVC_SLICE_PACKET_XE_M_BASE_H__
#define __DECODE_HEVC_SLICE_PACKET_XE_M_BASE_H__

#include "media_cmd_packet.h"
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "decode_hevc_basic_feature.h"
#include "codechal_hw.h"

namespace decode
{

class HevcDecodeSlcPktXe_M_Base : public DecodeSubPacket
{
public:
    HevcDecodeSlcPktXe_M_Base(HevcPipeline *pipeline, CodechalHwInterface *hwInterface)
        : DecodeSubPacket(pipeline, *hwInterface), m_hevcPipeline(pipeline)
    {
        m_hwInterface = hwInterface;
        if (m_hwInterface != nullptr)
        {
            m_miInterface  = m_hwInterface->GetMiInterface();
            m_osInterface  = m_hwInterface->GetOsInterface();
            m_hcpInterface  = hwInterface->GetHcpInterface();
        }
    }
    virtual ~HevcDecodeSlcPktXe_M_Base();

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Execute hevc slice packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx) = 0;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

protected:
    virtual MOS_STATUS ValidateSubTileIdx(const HevcTileCoding::SliceTileInfo &sliceTileInfo,
                                          uint32_t subTileIdx);

    virtual MOS_STATUS SetHcpSliceStateParams(MHW_VDBOX_HEVC_SLICE_STATE &sliceStateParams,
                                              uint32_t sliceIdx, uint32_t subTileIdx);

    virtual MOS_STATUS SetRefIdxParams(MHW_VDBOX_HEVC_REF_IDX_PARAMS &refIdxParams,
                                       uint32_t sliceIdx);

    virtual MOS_STATUS SetWeightOffsetParams(
        MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS &weightOffsetParams, uint32_t sliceIdx);
    virtual MOS_STATUS AddWeightOffset(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx);

    virtual MOS_STATUS SetBsdObjParams(MHW_VDBOX_HCP_BSD_PARAMS &bsdObjParams,
                                       uint32_t sliceIdx, uint32_t subTileIdx);
    virtual MOS_STATUS AddBsdObj(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx,
                                 uint32_t subTileIdx);
    MOS_STATUS AddHcpCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx);

    //!
    //! \brief  Calculate slcie level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();

    HevcPipeline *         m_hevcPipeline     = nullptr;
    MhwVdboxHcpInterface * m_hcpInterface     = nullptr;
    HevcBasicFeature *     m_hevcBasicFeature = nullptr;
    DecodeAllocator *      m_allocator        = nullptr;
    CodechalHwInterface   *m_hwInterface      = nullptr;
    MhwMiInterface        *m_miInterface      = nullptr;

    // Parameters passed from application
    PCODEC_HEVC_PIC_PARAMS       m_hevcPicParams = nullptr;      //!< Pointer to picture parameter
    PCODEC_HEVC_SLICE_PARAMS     m_hevcSliceParams = nullptr;    //!< Pointer to slice parameter
    PCODEC_HEVC_EXT_PIC_PARAMS   m_hevcRextPicParams = nullptr;  //!< Extended pic params for Rext
    PCODEC_HEVC_EXT_SLICE_PARAMS m_hevcRextSliceParams = nullptr;//!< Extended slice params for Rext
    PCODEC_HEVC_SCC_PIC_PARAMS   m_hevcSccPicParams = nullptr;   //!< Pic params for SCC

    uint32_t m_sliceStatesSize      = 0;  //!< Slice state command size
    uint32_t m_slicePatchListSize   = 0;  //!< Slice patch list size
MEDIA_CLASS_DEFINE_END(decode__HevcDecodeSlcPktXe_M_Base)
};

}  // namespace decode
#endif
