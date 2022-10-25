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
//! \file     decode_mpeg2_mb_packet_xe_m_base.h
//! \brief    Defines the implementation of mpeg2 decode macroblock packet for Xe_M_Base
//!

#ifndef __DECODE_MPEG2_MB_PACKET_XE_M_BASE_H__
#define __DECODE_MPEG2_MB_PACKET_XE_M_BASE_H__

#include "media_cmd_packet.h"
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "decode_mpeg2_basic_feature.h"
#include "codechal_hw_g12_X.h"

namespace decode
{

    class Mpeg2DecodeMbPktXe_M_Base : public DecodeSubPacket
    {
    public:
        Mpeg2DecodeMbPktXe_M_Base(Mpeg2Pipeline* pipeline, CodechalHwInterface* hwInterface)
            : DecodeSubPacket(pipeline, *hwInterface), m_mpeg2Pipeline(pipeline)
        {
            m_hwInterface = hwInterface;
            if (m_hwInterface != nullptr)
            {
                m_miInterface  = m_hwInterface->GetMiInterface();
                m_osInterface  = hwInterface->GetOsInterface();
                m_mfxInterface = static_cast<CodechalHwInterfaceG12*>(hwInterface)->GetMfxInterface();
            }
        }
        virtual ~Mpeg2DecodeMbPktXe_M_Base() {};

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
        //! \brief  Execute mpeg2 slice packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MHW_BATCH_BUFFER& batchBuffer, uint32_t mbIdx) = 0;

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
            uint32_t& commandBufferSize,
            uint32_t& requestedPatchListSize) override;

        MOS_STATUS AddITObj(MHW_BATCH_BUFFER& batchBuffer, uint32_t mbIdx);
        MOS_STATUS InsertSkippedMacroblocks(
            MHW_BATCH_BUFFER& batchBuffer,
            uint32_t mbIdx,
            uint16_t nextMBStart,
            uint16_t skippedMBs);

    protected:
        //!
        //! \brief  Calculate macroblock level command Buffer Size
        //!
        //! \return uint32_t
        //!         Command buffer size calculated
        //!
        virtual MOS_STATUS CalculateMbStateCommandSize();

        MOS_STATUS SetMpeg2MbStateParams(MHW_VDBOX_MPEG2_MB_STATE& mpeg2MbState, uint32_t mbIdx);
        void PackMotionVectors(CODEC_PICTURE_FLAG pic_flag, PMHW_VDBOX_MPEG2_MB_STATE mpeg2MbState);

        Mpeg2Pipeline* m_mpeg2Pipeline = nullptr;
        MhwVdboxMfxInterface* m_mfxInterface = nullptr;
        Mpeg2BasicFeature* m_mpeg2BasicFeature = nullptr;
        DecodeAllocator* m_allocator = nullptr;

        CodechalHwInterface *m_hwInterface = nullptr;
        MhwMiInterface      *m_miInterface = nullptr;

        // Parameters passed from application
        CodecDecodeMpeg2PicParams* m_mpeg2PicParams = nullptr;      //!< Pointer to MPEG2 picture parameter

        uint32_t m_mbStatesSize = 0;  //!< MB state command size
        uint32_t m_mbPatchListSize = 0;  //!< MB patch list size

    private:
        //!
        //! \enum _MPEG2_IMT_TYPE
        //! \brief Mpeg2 image type
        //!
        enum Mpeg2ImtType
        {
            Mpeg2ImtNone = 0,    //!< triple GFXBlocks
            Mpeg2ImtFrameFrame = 1,    //!< triple
            Mpeg2ImtFieldField = 2,    //!< triple
            Mpeg2ImtFieldDualPrime = 3,    //!< triple
            Mpeg2ImtFrameFiled = 4,    //!< hex
            Mpeg2ImtFrameDualPrime = 5,    //!< hex
            Mpeg2Imt16x8 = 6     //!< hex
        };

        //!
        //! \enum  DecodeMotionType
        //! \brief Codechal decode motion type
        //!
        enum DecodeMotionType
        {
            CodechalDecodeMcField = 1,    //!< Field motion type
            CodechalDecodeMcFrame = 2,    //!< Frame motion type
            CodechalDecodeMc16x8 = 2,    //!< 16x8 motion type
            CodechalDecodeMcDmv = 3     //!< DMV motion type
        };

        //!
        //! \enum  DecodeMvPacking
        //! \brief For motion vector packing: the equivilant derefences of a [2][2][2] array mapped as a [8] array
        //!
        enum DecodeMvPacking
        {
            CodechalDecodeRstFirstForwHorz = 0, //!< first forward horizontal
            CodechalDecodeRstFirstForwVert = 1, //!< first forward vertical
            CodechalDecodeRstFirstBackHorz = 2, //!< first backward horizontal
            CodechalDecodeRstFirstBackVert = 3, //!< first backward vertical
            CodechalDecodeRstSecndForwHorz = 4, //!< second forward horizontal
            CodechalDecodeRstSecndForwVert = 5, //!< second forward vertical
            CodechalDecodeRstSecndBackHorz = 6, //!< second backward horizontal
            CodechalDecodeRstSecndBackVert = 7  //!< second backward vertical
        };
    MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodeMbPktXe_M_Base)
    };

}  // namespace decode
#endif
