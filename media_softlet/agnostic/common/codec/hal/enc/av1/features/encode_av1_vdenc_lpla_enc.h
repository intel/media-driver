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
//! \file     encode_av1_vdenc_lpla_enc.h
//! \brief    Defines for encode av1 lowpower lookahead(Encode Pass) feature
//!

#ifndef __ENCODE_AV1_VDENC_LPLA_ENC_H__
#define __ENCODE_AV1_VDENC_LPLA_ENC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "encode_basic_feature.h"
#include "encode_pipeline.h"
#include "encode_av1_brc_update_packet.h"
#include "encode_av1_brc_init_packet.h"
#include "encode_lpla.h"

namespace encode
{
    class AV1VdencLplaEnc : public MediaFeature, public mhw::vdbox::huc::Itf::ParSetting
    {
    public:
        AV1VdencLplaEnc(
            MediaFeatureManager *featureManager,
            EncodeAllocator     *allocator,
            CodechalHwInterfaceNext *hwInterface,
            void                *constSettings);

        virtual ~AV1VdencLplaEnc();

        //!
        //! \brief  Init lpla encode pass features related parameter
        //! \param  [in] settings
        //!         Pointer to settings
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init(void *settings) override;

        //!
        //! \brief  Update cqp basic features related parameter
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Update(void *params) override;

        //!
        //! \brief    Setup/configure encoder based on sequence parameter set
        //! \details  It is invoked when the encoder receives a new sequence parameter set and it would
        //!           set up and configure the encoder state that used for the sequence
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetSequenceStructs();

        MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);

    protected:
        CODEC_AV1_ENCODE_SEQUENCE_PARAMS *m_av1SeqParams = nullptr;  //!< Pointer to sequence parameter
        CODEC_AV1_ENCODE_PICTURE_PARAMS  *m_av1PicParams = nullptr;  //!< Pointer to picture parameter

        uint32_t             m_targetBufferFulness      = 0;            //!< Target encode buffer fulness in bits, used by BRC and calculated from initial buffer fulness, target frame size (from DDI) and average frame size
        mutable bool         m_initDeltaQP              = true;         //!< Initial delta QP
        mutable uint32_t     m_prevQpModulationStrength = 0;            //!< Previous QP Modulation strength
        uint32_t             m_prevTargetFrameSize      = 0;            //!< Target frame size of previous frame.
        uint32_t             m_averageFrameSize         = 0;            //!< Average frame size based on targed bitrate and frame rate, in unit of bits
        EncodeLPLA *         m_lplaHelper               = nullptr;      //!< Lookahead helper

        EncodeAllocator      *m_allocator               = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__AV1VdencLplaEnc)
    };
} // encode

#endif
