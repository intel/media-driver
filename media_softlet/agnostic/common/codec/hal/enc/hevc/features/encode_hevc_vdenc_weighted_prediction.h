/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_weighted_prediction.h
//! \brief    Defines for hevc weighted prediction feature
//!
#ifndef __ENCODE_HEVC_VDENC_WEIGHTED_PREDICTION_H__
#define __ENCODE_HEVC_VDENC_WEIGHTED_PREDICTION_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "encode_huc_brc_update_packet.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"

namespace encode
{
    class HevcVdencWeightedPred : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
    {
    public:
        HevcVdencWeightedPred(
            MediaFeatureManager *featureManager,
            EncodeAllocator *allocator,
            CodechalHwInterfaceNext *hwInterface,
            void *constSettings);

        //!
        //! \brief  Update weighted prediction features related parameter
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Update(void *params) override;

        //!
        //! \brief  Set HuC dmem  of BRC update for weighted prediction
        //!
        //! \param    [in] isFirstPass
        //!           Whether is first pass or not
        //! \param    [out] dmem
        //!           Reference of CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetHucBrcUpdateDmemBuffer(
            bool isFirstPass,
            VdencHevcHucBrcUpdateDmem &dmem);

        //!
        //! \brief  Set HuC Constant data of BRC update for weighted prediction
        //!
        //! \param    [in] hevcSlcParams
        //!           Reference of HEVC slice parameters
        //! \param    [in] sliceIndex
        //!           Index of slice
        //! \param    [in] weightOffsetStateCmdSize
        //!           Command size of weight offset state
        //! \param    [in, out] sliceLocation
        //!           the location of slice in command
        //! \param    [out] constantData
        //!           Reference of CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G12
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetHucBrcUpdateConstData(
            const CODEC_HEVC_ENCODE_SLICE_PARAMS &hevcSliceParams,
            uint32_t sliceIndex,
            uint32_t weightOffsetStateCmdSize,
            uint32_t &sliceLocation,
            VdencHevcHucBrcConstantData &constantData);

        MHW_SETPAR_DECL_HDR(VDENC_WEIGHTSOFFSETS_STATE);

        MHW_SETPAR_DECL_HDR(HCP_WEIGHTOFFSET_STATE);

        MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    protected:
        PCODEC_HEVC_ENCODE_SLICE_PARAMS m_hevcSliceParams = nullptr;   //!< Pointer to slice parameter
        bool m_bEnableGPUWeightedPrediction = false;
        HevcBasicFeature *m_basicFeature  = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__HevcVdencWeightedPred)
    };

}  // namespace encode

#endif  // !__ENCODE_HEVC_VDENC_WEIGHTED_PREDICTION_H__
