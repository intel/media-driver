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
//! \file     decode_avc_slice_packet.h
//! \brief    Defines the implementation of avc decode slice packet
//!

#ifndef __DECODE_AVC_SLICE_PACKET_H__
#define __DECODE_AVC_SLICE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_avc_pipeline.h"
#include "decode_utils.h"
#include "decode_avc_basic_feature.h"
#include "mhw_vdbox_mfx_itf.h"

namespace decode
{
//!
//! \struct   AvcRefListWrite
//! \brief    Average reference list write
//!
struct AvcRefListWrite
{
    union
    {
        struct
        {
            uint8_t bottomField : 1;
            uint8_t frameStoreID : 4;
            uint8_t fieldPicFlag : 1;
            uint8_t longTermFlag : 1;
            uint8_t nonExisting : 1;
        };
        struct
        {
            uint8_t value;
        };
    } Ref[32];
};

class AvcDecodeSlcPkt : public DecodeSubPacket, public mhw::vdbox::mfx::Itf::ParSetting
{
public:
    AvcDecodeSlcPkt(AvcPipeline *pipeline, CodechalHwInterfaceNext*hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_avcPipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_mfxItf = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
            m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~AvcDecodeSlcPkt(){};

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
    //! \brief  Execute avc slice packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx) = 0;

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

    bool m_firstValidSlice = true; //!< Indicate the first valid slice if error slices are skipped
protected:
    MOS_STATUS AddCmd_AVC_SLICE_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MOS_STATUS SET_AVC_SLICE_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MOS_STATUS AddCmd_AVC_PHANTOM_SLICE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MOS_STATUS AddCmd_AVC_SLICE_WEIGHT_OFFSET(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MOS_STATUS AddCmd_AVC_SLICE_REF_IDX(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MOS_STATUS AddCmd_AVC_BSD_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MOS_STATUS AddCmd_AVC_SLICE_Addr(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MOS_STATUS SetAndAddAvcSliceState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx);
    MHW_SETPAR_DECL_HDR(MFD_AVC_BSD_OBJECT);
    MHW_SETPAR_DECL_HDR(MFX_AVC_WEIGHTOFFSET_STATE);
    MHW_SETPAR_DECL_HDR(MFX_AVC_REF_IDX_STATE);

    //!
    //! \brief  Calculate slice level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();

    AvcPipeline *         m_avcPipeline     = nullptr;
    AvcBasicFeature *     m_avcBasicFeature = nullptr;
    DecodeAllocator *     m_allocator       = nullptr;

    // Parameters passed from application
    CODEC_AVC_PIC_PARAMS *  m_avcPicParams   = nullptr;  //!< Pointer to AVC picture parameter
    CODEC_AVC_SLICE_PARAMS *m_avcSliceParams = nullptr;  //!< Pointer to AVC slices parameter

    uint32_t                              m_sliceStatesSize    = 0;  //!< Slice state command size
    uint32_t                              m_slicePatchListSize = 0;  //!< Slice patch list size
    uint32_t                              m_curSliceNum        = 0;
    uint32_t                              m_listID             = 0;
    std::shared_ptr<mhw::vdbox::mfx::Itf> m_mfxItf             = nullptr;

    uint32_t                m_IndirectBsdDataLength                     = 0;
    uint32_t                m_IndirectBsdDataStartAddress               = 0;
    bool                    m_LastsliceFlag                             = false;
    uint32_t                m_FirstMacroblockMbBitOffset                = 0;
    uint32_t                m_FirstMbByteOffsetOfSliceDataOrSliceHeader = 0;
    bool                    m_decodeInUse                               = false;
    PCODEC_AVC_SLICE_PARAMS m_pAvcSliceParams                           = nullptr;

MEDIA_CLASS_DEFINE_END(decode__AvcDecodeSlcPkt)
};

}  // namespace decode
#endif
