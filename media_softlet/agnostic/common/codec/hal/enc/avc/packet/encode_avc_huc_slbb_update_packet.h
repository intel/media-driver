/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_avc_huc_slbb_update_packet.h
//! \brief    Defines the implementation of AVC HuC SLBB Update packet
//!

#ifndef __ENCODE_AVC_HUC_SLBB_UPDATE_PACKET_H__
#define __ENCODE_AVC_HUC_SLBB_UPDATE_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "encode_utils.h"
#include "encode_avc_vdenc_pipeline.h"
#include "encode_avc_basic_feature.h"
#include "encode_huc_slbb_update_pkt.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "codec_def_common_encode.h"

namespace encode
{

class AVCHucSLBBUpdatePkt : public HucSLBBUpdatePkt
{
public:
    //!
    //! \brief  Constructor
    //!
    AVCHucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief  Destructor
    //!
    virtual ~AVCHucSLBBUpdatePkt();

    //!
    //! \brief  Initialize the AVCHucSLBBUpdatePkt
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Submit the AVC HuC SLBB Update packet
    //! \param  [in] commandBuffer
    //!         Pointer to command buffer
    //! \param  [in] packetPhase
    //!         Packet phase
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

    //!
    //! \brief  Calculate Command Size
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested patch list size
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

protected:
    //!
    //! \brief  Allocate Resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief  Construct batch buffer for AVC-specific SLBB updates
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ConstructBatchBuffer() override;
    //!
    //! \brief  Set DMEM buffer with AVC-specific data
    //! \details Implements base class pure virtual function for AVC codec
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmem() const override;

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpInput() override;
    virtual MOS_STATUS DumpOutput() override;
#endif

    //!
    //! \brief  Set HUC_IMEM_STATE command parameters
    //! \param  [in] params
    //!         Pointer to HUC_IMEM_STATE parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);

    //!
    //! \brief  Set HUC_VIRTUAL_ADDR_STATE command parameters
    //! \param  [in] params
    //!         Pointer to HUC_VIRTUAL_ADDR_STATE parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

    //!
    //! \brief  Set HUC_IMEM_ADDR command parameters (PPGTT mode)
    //! \details Override to set integrityEnable for PXP-protected kernel loads.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(HUC_IMEM_ADDR);

    //!
    //! \brief  HuC AVC SLBB Update DMEM structure
    //! \details Compact structure layout matching avc_slbb_update.h specification (64 bytes total):
    //!          Section 1: SLBB Size (2 bytes)
    //!          Section 2: Command Offsets (10 bytes, 5 uint16_t fields)
    //!          Section 3: Command Metadata (4 bytes, 2 uint16_t fields)
    //!          Section 4: Frame/GOP Parameters (8 bytes, 4 uint16_t fields)
    //!          Section 5: Encoding Parameters (8 bytes, 8 uint8_t fields)
    //!          Section 6: Reserved Padding (32 bytes)
    //!
    struct HucAvcSlbbUpdateDmem
    {
        // Section 1: SLBB Size (2 bytes)
        uint16_t SlbbSize;                    //!< Size of second level batch buffer in bytes

        // Section 2: Command Offsets (10 bytes)
        uint16_t MfxAvcImgStateOffset;        //!< Byte offset to MFX_AVC_IMG_STATE command (Group1)
        uint16_t VdencAvcCostStateOffset;     //!< Byte offset to VDENC_AVC_COST_STATE command (Group1, via VDENC_CMD3)
        uint16_t VdencAvcImgStateOffset;      //!< Byte offset to VDENC_AVC_IMG_STATE command (Group1)
        uint16_t MfxAvcSliceStateOffset;      //!< Byte offset to MFX_AVC_SLICE_STATE command (Group2, first slice)
        uint16_t VdencAvcSliceStateOffset;    //!< Byte offset to VDENC_AVC_SLICE_STATE command (Group2, first slice)

        // Section 3: Command Metadata (4 bytes)
        uint16_t SliceNum;      //!< Number of slices in the frame (for multi-slice support)
        uint16_t MaxPassNum;    //!< Maximum number of BRC passes (reserved for future use)

        // Section 4: Frame/GOP Parameters (8 bytes)
        uint16_t PicWidthInMb;   //!< Picture width in macroblocks
        uint16_t PicHeightInMb;  //!< Picture height in macroblocks
        uint16_t GopPicSize;     //!< GOP picture size
        uint16_t GopRefDist;     //!< GOP reference distance (1 = IPPP, >1 = IPBB...)

        // Section 5: Encoding Parameters (8 bytes)
        uint8_t FrameType;            //!< Frame type (0=P_FRAME, 1=B_FRAME, 2=I_FRAME per AVC BRC constants)
        uint8_t QpValue;              //!< Quantization parameter value (0-51 for AVC)
        uint8_t TargetUsage;          //!< Target usage (1=best quality, 4=balanced, 7=best speed)
        uint8_t RateControlMethod;    //!< Rate control method (0=CQP, 1=CBR, 2=VBR, etc.)
        uint8_t PictureCodingType;    //!< Picture coding type for encoding decisions
        uint8_t Level;                //!< AVC level_idc (e.g., 10=1.0, 20=2.0, 31=3.1, etc.)
        uint8_t RefPicFlag;           //!< Reference picture flag (0=not ref, 1=ref)
        uint8_t LowDelayMode;         //!< Low delay mode flag (0=disabled, 1=enabled)

        // Section 6: Encoding Feature Flags + Reserved Padding (32 bytes)
        uint8_t EnableSliceLevelRateCtrl;  //!< Slice-level rate control enable flag
        uint8_t EnableRollingIntraRefresh; //!< Rolling intra refresh mode (0=disabled, 1=column, 2=row)
        uint8_t Wa_18011246551;             //!< Workaround flag for Wa_18011246551
        uint8_t NumRefIdxL0ActiveMinus1;   //!< Number of active L0 references minus 1
        uint8_t SliceQp;                   //!< QpY + slice_qp_delta
        uint8_t QpMin;                     //!< Minimum QP for CMD3 cost table lookup
        uint8_t Is10Bit422;                //!< 1 if 10-bit 4:2:2 profile (XL platform override)
        uint8_t EntropyCodingModeFlag;     //!< 0=CAVLC, 1=CABAC (for EncMbConfCheckThrLUT)
        int8_t  SignedQp;                  //!< Signed QP value (m_QP, can be negative for 10-bit 4:2:2)
        uint8_t AdaptiveTUEnabled;         //!< AdaptiveTU mode flag: 0=disabled, 1=enabled (process TU7 SLBB via Region 2->3)
        uint8_t UseExtendedThreshold;
        uint8_t ExtendedPlatform;          //!< 0 = base (HuC programs all SLBB fields); 1 = extended platform (HuC preserves driver pre-fills)
        uint8_t Reserved8[20];             //!< Reserved for future use, must be zero-initialized
    };

    //!
    //! \brief  AVC HuC SLBB Update kernel descriptor
    //!
    static constexpr uint32_t m_vdboxHucAvcSlbbUpdateKernelDescriptor = 22;

    //! \brief  Selects whether the HuC kernel should preserve driver pre-fills for SLBB fields on extended platforms.
    virtual bool IsExtendedPlatform() const { return false; }

    //!
    //! \brief  PXP (Protected Xe Path) mode flag
    //! \details Set in Init() when osCpInterface reports HMEnabled. Selects the
    //!          integrity-protected SLBB update kernel and enables HuC integrity
    //!          verification on the PPGTT IMEM load path.
    //!
    bool m_isPxp = false;

    //!
    //! \brief  Pointer to AVC basic feature
    //!
    AvcBasicFeature *m_basicFeature = nullptr;

    //!
    //! \brief  Shared pointer to feature manager
    //!
    std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

MEDIA_CLASS_DEFINE_END(encode__AVCHucSLBBUpdatePkt)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_HUC_SLBB_UPDATE_PACKET_H__