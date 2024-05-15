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
//! \file     encode_hevc_vdenc_scc.h
//! \brief    Defines for hevc screen content coding feature
//!
#ifndef __ENCODE_HEVC_VDENC_SCC_H__
#define __ENCODE_HEVC_VDENC_SCC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "encode_basic_feature.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"

namespace encode
{
    class HevcVdencScc : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
    {
    public:
        HevcVdencScc(
            MediaFeatureManager *featureManager,
            EncodeAllocator *allocator,
            CodechalHwInterfaceNext *hwInterface,
            void *constSettings);

        virtual ~HevcVdencScc();

        //!
        //! \brief  Init scc basic features related parameter
        //!
        //! \param  [in] settings
        //!         Pointer to settings
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init(void *settings) override;

        //!
        //! \brief  Update scc features related parameter
        //!
        //! \param  [in] params
        //!         Pointer to parameters
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Update(void *params) override;

        //!
        //! \brief  Set huc dmem of brc update for scc
        //!
        //! \param  [in out] hucVdencBrcUpdateDmem
        //!         Reference of CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetHucBrcUpdateDmem(void* hucVdencBrcUpdateDmem);

        //!
        //! \brief  SetRecNotFilteredID parameters
        //!
        //! \param  [in, out] slotForRecNotFiltered
        //!         Slot ID for not filtered reconstructed surface
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetRecNotFilteredID(unsigned char &slotForRecNotFiltered);

        //!
        //! \brief    Check if SCC enabled
        //!
        //! \return   bool
        //!           true if SCC enabled, else SCC disabled.
        //!
        bool IsSCCEnabled() { return m_enableSCC; }

        virtual bool IsCompressFlagNeeded() { return true; }

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(VDENC_CMD2);

        MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

        MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(HCP_SURFACE_STATE);

        MHW_SETPAR_DECL_HDR(HCP_REF_IDX_STATE);

        MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    protected:
        MOS_STATUS                  AllocateEncResources();
        MOS_STATUS                  FreeEncResources();

        PMOS_INTERFACE              m_osInterface = nullptr;               //!< Os Inteface
        MOS_RESOURCE                m_vdencRecNotFilteredBuffer = {};

        bool                        m_enableLBCOnly = false;               //!< Enable LBC only for IBC
        bool                        m_enableSCC = false;                   //!< Flag to indicate if HEVC SCC is enabled.
        unsigned char               m_slotForRecNotFiltered = 0;           //!< Slot for not filtered reconstructed surface

        EncodeBasicFeature          *m_basicFeature = nullptr;             //!< EncodeBasicFeature
        bool                        m_mmcEnabled = false;
        MOS_CONTEXT_HANDLE          m_mosCtx = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__HevcVdencScc)
    };

}  // namespace encode

#endif  // !__ENCODE_HEVC_VDENC_SCC_H__
