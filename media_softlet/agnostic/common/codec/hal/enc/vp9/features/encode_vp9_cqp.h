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
//! \file     encode_vp9_cqp.h
//! \brief    Defines the common interface for vp9 encode cqp features
//!
#ifndef __ENCODE_VP9_CQP_H__
#define __ENCODE_VP9_CQP_H__

#include "encode_vp9_basic_feature.h"

namespace encode
{

class Vp9EncodeCqp : public MediaFeature, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9EncodeCqp feature constructor
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
    Vp9EncodeCqp(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings);

    //!
    //! \brief  Vp9EncodeCqp feature destructor
    //!
    virtual ~Vp9EncodeCqp() {}

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
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

protected:

    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources() override;

    EncodeAllocator *     m_allocator    = nullptr;
    Vp9BasicFeature *     m_basicFeature = nullptr;

    // Deblocking
    MOS_RESOURCE m_resDeblockingFilterLineBuffer       = {0};  //!< Deblocking filter line buffer
    MOS_RESOURCE m_resDeblockingFilterTileLineBuffer   = {0};  //!< Deblocking filter tile line buffer
    MOS_RESOURCE m_resDeblockingFilterTileColumnBuffer = {0};  //!< Deblocking filter tile column buffer

MEDIA_CLASS_DEFINE_END(encode__Vp9EncodeCqp)
};

}  // namespace encode

#endif  // __ENCODE_VP9_CQP_H__