/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_hevc_cqp.h
//! \brief    Defines the common interface for hevc encode cqp features
//!
#ifndef __ENCODE_HEVC_CQP_H__
#define __ENCODE_HEVC_CQP_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "encode_basic_feature.h"

namespace encode
{
class HevcEncodeCqp : public MediaFeature, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    HevcEncodeCqp(MediaFeatureManager *featureManager, EncodeAllocator *allocator, CodechalHwInterfaceNext *hwInterface, void *constSettings);

    ~HevcEncodeCqp() {}

    //!
    //! \brief  Init cqp basic features related parameter
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
    //! \brief    Check if RDOQ enabled
    //!
    //! \return   bool
    //!           true if rdoq enabled, else rdoq disabled.
    //!
    bool IsRDOQEnabled() { return m_rdoqEnable; }

    MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

protected:

    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources() override;

    //! \brief  set feature refer to const settings.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetConstSettings() override;

    //!
    //! \brief    Verify slice SAO state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS VerifySliceSAOState();

    virtual void UpdateRDOQCfg();

    EncodeAllocator *m_allocator = nullptr;
    EncodeBasicFeature   *m_basicFeature = nullptr;  //!< EncodeBasicFeature
    MOS_CONTEXT_HANDLE  m_mosCtx = nullptr;

    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf = nullptr;

    static const uint8_t m_hevcSAOStreamoutSizePerLCU = 16;

    int m_picQPY = 0;
    int m_slcQP  = 0;

    //Deblocking
    bool     m_SliceDeblockingFilterDisabled = false;
    uint32_t m_SliceTcOffsetDiv2             = 0;
    uint32_t m_SliceBettaOffsetDiv2          = 0;
    PMOS_RESOURCE m_resDeblockingFilterTileRowStoreScratchBuffer = nullptr;    //!< De-blocking filter tile row store Scratch data buffer
    PMOS_RESOURCE m_resDeblockingFilterColumnRowStoreScratchBuffer = nullptr;  //!< De-blocking filter column row Store scratch data buffer
    PMOS_RESOURCE m_resDeblockingFilterRowStoreScratchBuffer = nullptr;        //!< Handle of De-block row store surface

    //SAO
    bool m_saoEnable = false;
    PMOS_RESOURCE   m_resSAOLineBuffer = nullptr;                    //!< SAO line data buffer
    PMOS_RESOURCE   m_resSAOTileLineBuffer = nullptr;                //!< SAO tile line data buffer
    PMOS_RESOURCE   m_resSAOTileColumnBuffer = nullptr;              //!< SAO tile column data buffer
    PMOS_RESOURCE   m_resSAOStreamOutBuffer = nullptr;               //!< SAO stream-out buffer
    MOS_RESOURCE    m_vdencSAORowStoreBuffer = { 0 };                //!< SAO RowStore buffer

    //TransformSkip
    bool m_transformSkipEnable = false;

    //RDOQ
    bool     m_rdoqEnable           = false;
    uint32_t m_rdoqIntraTuThreshold = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    bool     m_rdoqIntraTuOverride          = false;  //!< Override RDOQ intra TU or not
    bool     m_rdoqIntraTuDisableOverride   = false;  //!< Override RDOQ intra TU disable
    uint16_t m_rdoqIntraTuThresholdOverride = 0;      //!< Override RDOQ intra TU threshold
#endif

MEDIA_CLASS_DEFINE_END(encode__HevcEncodeCqp)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_CQP_H__
