/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_fullenc.h
//! \brief    Defines for vdenc full-enc feature
//!
#ifndef __HEVC_VDENC_FULL_ENC_H__
#define __HEVC_VDENC_FULL_ENC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_hevc_vdenc_preenc.h"

namespace encode
{
    class HevcVdencFullEnc : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting
    {
#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
        enum EncodeFullencDef0
        {
            SINGLE_PASS_ENC = 0,
            PRE_ENC_PASS,
            FULL_ENC_PASS
        };

        typedef struct mfxI16PAIR
        {
            short x, y;  // integer pair
        } mfxI16PAIR;

        typedef mfxI16PAIR I16PAIR;

        struct EncodeFullencDef1
        {
            I16PAIR  mv;
            uint16_t intraSAD;
            uint16_t interSAD;
        };

    public:
        HevcVdencFullEnc(
            MediaFeatureManager *featureManager,
            EncodeAllocator *allocator,
            CodechalHwInterfaceNext *hwInterface,
            void *constSettings);

        virtual ~HevcVdencFullEnc();

        //!
        //! \brief  Init full-enc basic features related parameter
        //!
        //! \param  [in] settings
        //!         Pointer to settings
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init(void *settings) override;

        //!
        //! \brief  Update full-enc features related parameter
        //!
        //! \param  [in] params
        //!         Pointer to parameters
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Update(void *params) override;
#if USE_CODECHAL_DEBUG_TOOL
        MOS_STATUS EncodeFullencFuntion0();
#endif
        MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    protected:
        PMOS_INTERFACE      m_osInterface    = nullptr;           //!< Os Inteface
        EncodeBasicFeature  *m_basicFeature  = nullptr;         //!< EncodeBasicFeature
        HevcVdencPreEnc     *m_preEncFeature = nullptr;

        PMOS_RESOURCE       EncodeFullencMember0 = nullptr;
        PMOS_RESOURCE       EncodeFullencMember1 = nullptr;

        uint32_t m_encodeMode = 0;

        uint8_t  EncodeFullencMember2 = 0;  //0:32x32, 1:16x16, 2:8x8
        uint8_t  EncodeFullencMember3 = 0;
        uint8_t  EncodeFullencMember4 = 0;
        uint32_t EncodeFullencMember5 = 0;

        EncodeAllocator *m_allocator = nullptr;

        MOS_STATUS UpdateTrackedBufferParameters();
        MOS_STATUS UpdatePreEncSize();

#if USE_CODECHAL_DEBUG_TOOL
        FILE *pfile0 = nullptr;
        FILE *pfile1 = nullptr;
#endif
    MEDIA_CLASS_DEFINE_END(encode__HevcVdencFullEnc)
    };

}  // namespace encode

#endif  // !__HEVC_VDENC_FULL_ENC_H__

