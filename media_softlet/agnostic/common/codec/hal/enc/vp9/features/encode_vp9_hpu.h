/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_vp9_hpu.h
//! \brief    Defines the common interface for vp9 encode hpu (header probabiloity update) features
//!
#ifndef __ENCODE_VP9_HPU_H__
#define __ENCODE_VP9_HPU_H__

#include "encode_vp9_basic_feature.h"
#include "media_vp9_packet_defs.h"

namespace encode
{

class Vp9EncodeHpu : public MediaFeature, public mhw::vdbox::huc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9EncodeHpu feature constructor
    //!
    //! \param  [in] featureManager
    //!         Pointer to MediaFeatureManager
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] constSettings
    //!         Pointer to const settings
    //!
    Vp9EncodeHpu(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings);

    //!
    //! \brief  Vp9EncodeHpu feature destructor
    //!
    virtual ~Vp9EncodeHpu() {}

    //!
    //! \brief  Init CQP basic features related parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update CQP basic features related parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Set regions for huc prob
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegions(
        mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const;

    //!
    //! \brief  Get probability buffer
    //! \param  [in] idx
    //!         Index of the probability buffer
    //! \param  [out] buffer
    //!         Reference to the buffer get from Brc feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetProbabilityBuffer(
        uint32_t       idx,
        PMOS_RESOURCE &buffer);

    //!
    //! \brief  Get huc probability dmem buffer
    //! \param  [in] idx
    //!         Index of the huc probability dmem buffer
    //! \param  [out] buffer
    //!         Reference to the buffer get from Brc feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHucProbDmemBuffer(
        uint32_t       idx,
        PMOS_RESOURCE &buffer);

    //!
    //! \brief  Set Last Pass flag
    //! \param  [in] bool
    //!         Last Pass
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetIsLastPass(bool isLastPass);

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

protected:

    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources() override;

    static constexpr uint32_t m_probabilityCounterBufferSize = 193 * CODECHAL_CACHELINE_SIZE;
    static const uint32_t     m_probDmem[320];

    EncodeAllocator *     m_allocator    = nullptr;
    Vp9BasicFeature *     m_basicFeature = nullptr;

    // HuC Prob resoruces/buffers
    MOS_RESOURCE m_resProbabilityDeltaBuffer             = {0};                  //!< Probability delta buffer
    MOS_RESOURCE m_resProbabilityCounterBuffer           = {0};                  //!< Probability counter buffer
    MOS_RESOURCE m_resHucProbDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][3]; //!< VDENC HuC Prob DMEM buffer
    MOS_RESOURCE m_resHucProbOutputBuffer                = {0};                  //!< HuC Prob output buffer
    MOS_RESOURCE m_resProbBuffer[CODEC_VP9_NUM_CONTEXTS] = {0};                  //!< Probability buffer

    mutable bool m_isLastPass = false;
MEDIA_CLASS_DEFINE_END(encode__Vp9EncodeHpu)
};

}  // namespace encode

#endif  // __ENCODE_VP9_CQP_H__
