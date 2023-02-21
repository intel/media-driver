/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_huc_s2l_packet.h
//! \brief    Defines the implementation of huc S2L packet
//!

#ifndef __DECODE_HUC_S2L_PACKET_H__
#define __DECODE_HUC_S2L_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "decode_utils.h"
#include "decode_hevc_pipeline.h"
#include "decode_hevc_basic_feature.h"

#include "mhw_vdbox_huc_cmdpar.h"
#include "mhw_vdbox_huc_itf.h"
#include "mhw_cmdpar.h"

namespace decode
{
    struct HucHevcS2lPicBss
    {
        uint32_t    pic_width_in_min_cbs_y;
        uint32_t    pic_height_in_min_cbs_y;
        uint8_t     log2_min_luma_coding_block_size_minus3;
        uint8_t     log2_diff_max_min_luma_coding_block_size;
        uint16_t    chroma_format_idc                           : 2;  //!< range 0..3
        uint16_t    separate_colour_plane_flag                  : 1;
        uint16_t    bit_depth_luma_minus8                       : 4;
        uint16_t    bit_depth_chroma_minus8                     : 4;
        uint16_t    log2_max_pic_order_cnt_lsb_minus4           : 4;  //!< range 0..12
        uint16_t    sample_adaptive_offset_enabled_flag         : 1;
        uint8_t     num_short_term_ref_pic_sets;                      //!< range 0..64
        uint8_t     long_term_ref_pics_present_flag             : 1;
        uint8_t     num_long_term_ref_pics_sps                  : 6;  //!< range 0..32
        uint8_t     sps_temporal_mvp_enable_flag                : 1;

        uint8_t     num_ref_idx_l0_default_active_minus1        : 4;  //!< range 0..15
        uint8_t     num_ref_idx_l1_default_active_minus1        : 4;  //!< range 0..15
        int8_t      pic_init_qp_minus26;                              //!< range -62..25
        uint8_t     dependent_slice_segments_enabled_flag       : 1;
        uint8_t     cabac_init_present_flag                     : 1;
        uint8_t     pps_slice_chroma_qp_offsets_present_flag    : 1;
        uint8_t     weighted_pred_flag                          : 1;
        uint8_t     weighted_bipred_flag                        : 1;
        uint8_t     output_flag_present_flag                    : 1;
        uint8_t     tiles_enabled_flag                          : 1;
        uint8_t     entropy_coding_sync_enabled_flag            : 1;
        uint8_t     loop_filter_across_slices_enabled_flag      : 1;
        uint8_t     deblocking_filter_override_enabled_flag     : 1;
        uint8_t     pic_disable_deblocking_filter_flag          : 1;
        uint8_t     lists_modification_present_flag             : 1;
        uint8_t     slice_segment_header_extension_present_flag : 1;
        uint8_t     high_precision_offsets_enabled_flag         : 1;
        uint8_t     chroma_qp_offset_list_enabled_flag          : 1;
        uint8_t                                                 : 1;

        int32_t     CurrPicOrderCntVal;
        int32_t     PicOrderCntValList[CODEC_MAX_NUM_REF_FRAME_HEVC];
        uint8_t     RefPicSetStCurrBefore[8];
        uint8_t     RefPicSetStCurrAfter[8];
        uint8_t     RefPicSetLtCurr[8];
        uint16_t    RefFieldPicFlag;
        uint16_t    RefBottomFieldFlag;
        int8_t      pps_beta_offset_div2;
        int8_t      pps_tc_offset_div2;
        uint16_t    StRPSBits;

        uint8_t     num_tile_columns_minus1;
        uint8_t     num_tile_rows_minus1;
        uint16_t    column_width[HEVC_NUM_MAX_TILE_COLUMN];
        uint16_t    row_height[HEVC_NUM_MAX_TILE_ROW];

        uint16_t    NumSlices;
        uint8_t     num_extra_slice_header_bits;
        int8_t      RefIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC];

        struct
        {
            uint8_t     reserve_0;
            uint16_t    reserve_1;
            uint32_t    reserve_2;
            uint32_t    reserve_3;
        } reserve;
    };

    struct HucHevcS2lSliceBss
    {
        uint32_t    BSNALunitDataLocation;
        uint32_t    SliceBytesInBuffer;

        struct
        {
            uint32_t    reserve_0;
            uint32_t    reserve_1;
            uint32_t    reserve_2;
            uint32_t    reserve_3;
        } reserve;
    };

    class HucS2lPkt : public DecodeHucBasic, public mhw::vdbox::huc::Itf::ParSetting
    {
    public:
        HucS2lPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext*hwInterface)
            : DecodeHucBasic(pipeline, task, hwInterface)
        {
            if (pipeline != nullptr)
            {
                m_statusReport = pipeline->GetStatusReportInstance();
                m_hevcPipeline = dynamic_cast<HevcPipeline *>(pipeline);
            }
        }

        virtual ~HucS2lPkt() {}

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
        //! \brief  destroy interal resource, should be invoked for each frame
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS FreeResource();

        virtual MOS_STATUS Destroy() override;

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
        virtual MOS_STATUS CalculateCommandSize(
            uint32_t &commandBufferSize,
            uint32_t &requestedPatchListSize) override;

        //!
        //! \brief  Get Packet Name
        //! \return std::string
        //!
        virtual std::string GetPacketName() override
        {
            return "S2L";
        }

        virtual MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
        virtual MHW_SETPAR_DECL_HDR(HUC_IND_OBJ_BASE_ADDR_STATE);
        virtual MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

        virtual MOS_STATUS AddCmd_HUC_STREAM_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, CODEC_HEVC_SLICE_PARAMS sliceParams);
        virtual MOS_STATUS AddCmd_HUC_START(MOS_COMMAND_BUFFER &cmdBuffer, bool laststreamobject);
        virtual MOS_STATUS AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer);
        virtual MOS_STATUS AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer);

    protected:
        //!
        //! \brief  Calculate Command Buffer Size
        //!
        //! \return uint32_t
        //!         Command buffer size calculated
        //!
        virtual uint32_t CalculateCommandBufferSize();

        //!
        //! \brief  Calculate Patch List Size
        //!
        //! \return uint32_t
        //!         Patchlist size calculated
        //!
        virtual uint32_t CalculatePatchListSize();

        virtual MOS_STATUS SetHucDmemPictureBss(HucHevcS2lPicBss &hucHevcS2LPicBss);
        virtual MOS_STATUS SetHucDmemSliceBss(
            HucHevcS2lSliceBss (&hucHevcS2LSliceBss)[CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6]);
        MOS_STATUS AddHucCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t index, CODEC_HEVC_SLICE_PARAMS &sliceParams);

#if USE_CODECHAL_DEBUG_TOOL
        virtual MOS_STATUS DumpHucS2l();
#endif

        static constexpr uint32_t m_vdboxHucHevcS2lKernelDescriptor = 1; //!< Huc HEVC S2L kernel descriptor

        HevcPipeline *             m_hevcPipeline      = nullptr; //!< Pointer to hevc pipeline
        HevcBasicFeature *         m_hevcBasicFeature  = nullptr; //!< Pointer to hevc basic feature
        PCODEC_HEVC_PIC_PARAMS     m_hevcPicParams     = nullptr; //!< Pointer to picture parameter
        PCODEC_HEVC_EXT_PIC_PARAMS m_hevcRextPicParams = nullptr; //!< Extended pic params for Rext
        PCODEC_HEVC_SLICE_PARAMS   m_hevcSliceParams   = nullptr; //!< Pointer to slice parameter
        PCODEC_HEVC_SCC_PIC_PARAMS m_hevcSccPicParams  = nullptr; //!< Pic params for SCC

        MOS_BUFFER*                m_s2lDmemBuffer     = nullptr; //!< Resource of current DMEM buffer
        MOS_BUFFER*                m_s2lControlTempMVRegionBuffer = nullptr;  //!< Point to RegionBuffer which controls temporal MV Buffer
        uint32_t                   m_dmemBufferSize    = 0;       //!< Size of DMEM buffer
        uint32_t                   m_dmemTransferSize  = 0;       //!< Transfer size of current DMEM buffer

        uint32_t                   m_pictureStatesSize    = 0;
        uint32_t                   m_picturePatchListSize = 0;
        uint32_t                   m_sliceStatesSize      = 0;
        uint32_t                   m_slicePatchListSize   = 0;

        MEDIA_CLASS_DEFINE_END(decode__HucS2lPkt)
    };

}  // namespace decode
#endif
