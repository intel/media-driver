/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     decode_vvc_picture_packet.h
//! \brief    Defines the implementation of VVC decode picture packet
//!

#ifndef __DECODE_VVC_PICTURE_PACKET_H__
#define __DECODE_VVC_PICTURE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_vvc_pipeline.h"
#include "decode_utils.h"
#include "decode_vvc_basic_feature.h"
#include "decode_common_feature_defs.h"
#include "mhw_vdbox_vvcp_itf.h"

using namespace mhw::vdbox::vvcp;

namespace decode
{
    class VvcDecodePicPkt : public DecodeSubPacket, public Itf::ParSetting
    {
    public:
        //!
        //! \brief  VvcDecodePicPkt constructor
        //!
        VvcDecodePicPkt(VvcPipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : DecodeSubPacket(pipeline, hwInterface), m_vvcPipeline(pipeline)
        {
            if (hwInterface != nullptr)
            {
                m_hwInterface = dynamic_cast<CodechalHwInterfaceNext *>(hwInterface);
                if (m_hwInterface != nullptr)
                {
                    m_vvcpItf     = std::static_pointer_cast<mhw::vdbox::vvcp::Itf>(m_hwInterface->GetVvcpInterfaceNext());
                }
                m_miItf       = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
            }
        }

        //!
        //! \brief  VvcDecodePicPkt deconstructor
        //!
        virtual ~VvcDecodePicPkt();

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
        //! \brief  Execute VVC picture packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer);

        //!
        //! \brief  Init VVC state commands
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS InitVvcState(MOS_COMMAND_BUFFER& cmdBuffer);

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

        PMHW_BATCH_BUFFER GetPicLvlBB()
        {
            return m_curPicLvlBatchBuffer;
        }

        static const uint32_t m_alfBufferSize               = 4608;         // Max 8*144DWs, size in Bytes
        static const uint32_t m_chromaQpBufferSize          = 256;          // Max 4*16DWs, size in Bytes
        static const uint32_t m_scalingListBufferSize       = 1400;         // Max 350DWs, size in Bytes

    protected:
        virtual MOS_STATUS AllocateFixedResources();
        virtual MOS_STATUS AllocateVariableResources();
        virtual MOS_STATUS AllocatePicLvlBB();

        MOS_STATUS SetDataBuffers() const;
        MOS_STATUS SetScalingListDataBuffer(uint8_t* buffer, uint32_t size) const;
        MOS_STATUS FixVvcpPipeBufAddrParams() const;

        //! \brief    Set Rowstore Cache offset
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SetRowstoreCachingOffsets();

        //!
        //! \brief  Free resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS FreeResources();

#if USE_CODECHAL_DEBUG_TOOL
        //!
        //! \brief  Dump resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpResources() const;
#endif

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();

        // VVCP MHW functions
        MOS_STATUS AddAllCmds_VVCP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer);
        MOS_STATUS AddAllCmds_VVCP_SURFACE_STATE(MOS_COMMAND_BUFFER &cmdBuffer);

        MHW_SETPAR_DECL_HDR(VVCP_VD_CONTROL_STATE);
        MHW_SETPAR_DECL_HDR(VVCP_PIPE_MODE_SELECT);
        MHW_SETPAR_DECL_HDR(VVCP_SURFACE_STATE);
        MHW_SETPAR_DECL_HDR(VVCP_PIPE_BUF_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(VVCP_IND_OBJ_BASE_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(VVCP_PIC_STATE);
        MHW_SETPAR_DECL_HDR(VVCP_DPB_STATE);

        //Interfaces
        VvcPipeline             *m_vvcPipeline     = nullptr;
        VvcBasicFeature         *m_vvcBasicFeature = nullptr;
        DecodeAllocator         *m_allocator       = nullptr;
        DecodeMemComp           *m_mmcState        = nullptr;
        CodecVvcPicParams       *m_vvcPicParams    = nullptr;
        MOS_SURFACE             m_refSurface[vvcMaxNumRefFrame];
        std::shared_ptr<Itf>    m_vvcpItf          = nullptr;
        CodechalHwInterfaceNext* m_hwInterface      = nullptr;

        //Internal buffers
        BufferArray             *m_alfBufferArray           = nullptr;
        BufferArray             *m_scalingListBufferArray   = nullptr;
        PMOS_BUFFER             m_apsAlfBuffer              = nullptr;
        PMOS_BUFFER             m_apsScalingListBuffer      = nullptr;
        BufferArray             *m_chromaQpBufferArray      = nullptr;
        PMOS_BUFFER             m_chromaQpBuffer            = nullptr;
        PMOS_BUFFER             m_vcedLineBuffer            = nullptr;
        PMOS_BUFFER             m_vcmvLineBuffer            = nullptr;
        PMOS_BUFFER             m_vcprLineBuffer            = nullptr;
        PMOS_BUFFER             m_vclfYLineBuffer           = nullptr;
        PMOS_BUFFER             m_vclfYTileRowBuffer        = nullptr;
        PMOS_BUFFER             m_vclfYTileColumnBuffer     = nullptr;
        PMOS_BUFFER             m_vclfULineBuffer           = nullptr;
        PMOS_BUFFER             m_vclfUTileRowBuffer        = nullptr;
        PMOS_BUFFER             m_vclfUTileColumnBuffer     = nullptr;
        PMOS_BUFFER             m_vclfVLineBuffer           = nullptr;
        PMOS_BUFFER             m_vclfVTileRowBuffer        = nullptr;
        PMOS_BUFFER             m_vclfVTileColumnBuffer     = nullptr;
        PMOS_BUFFER             m_vcSaoYLineBuffer          = nullptr;
        PMOS_BUFFER             m_vcSaoYTileRowBuffer       = nullptr;
        PMOS_BUFFER             m_vcSaoYTileColumnBuffer    = nullptr;
        PMOS_BUFFER             m_vcSaoULineBuffer          = nullptr;
        PMOS_BUFFER             m_vcSaoUTileRowBuffer       = nullptr;
        PMOS_BUFFER             m_vcSaoUTileColumnBuffer    = nullptr;
        PMOS_BUFFER             m_vcSaoVLineBuffer          = nullptr;
        PMOS_BUFFER             m_vcSaoVTileRowBuffer       = nullptr;
        PMOS_BUFFER             m_vcSaoVTileColumnBuffer    = nullptr;
        PMOS_BUFFER             m_vcAlfLineBuffer           = nullptr;
        PMOS_BUFFER             m_vcAlfTileRowBuffer        = nullptr;
        PMOS_BUFFER             m_vcAlfYTileColumnBuffer    = nullptr;
        PMOS_BUFFER             m_vcAlfUTileColumnBuffer    = nullptr;
        PMOS_BUFFER             m_vcAlfVTileColumnBuffer    = nullptr;

        uint32_t                m_prevFrmWidth              = 0;    //!< Frame width of the previous frame
        uint32_t                m_prevFrmHeight             = 0;    //!< Frame height of the previous frame
        uint32_t                m_pictureStatesSize         = 0;    //!< Picture states size
        uint32_t                m_picturePatchListSize      = 0;    //!< Picture patch list size
        mutable uint8_t         m_curVvcpSurfStateId        = 0;

        BatchBufferArray        *m_picLevelBBArray     = nullptr;   //!< Point to picture level batch buffer
        PMHW_BATCH_BUFFER       m_curPicLvlBatchBuffer = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__VvcDecodePicPkt)
    };

}  // namespace decode

#endif
