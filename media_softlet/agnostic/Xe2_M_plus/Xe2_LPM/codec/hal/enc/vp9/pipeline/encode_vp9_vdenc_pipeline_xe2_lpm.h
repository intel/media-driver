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
//! \file     encode_vp9_vdenc_pipeline_xe2_lpm.h
//! \brief    Defines the interface for vp9 vdenc encode pipeline
//!
#ifndef __ENCODE_VP9_VDENC_PIPELINE_XE2_LPM_H__
#define __ENCODE_VP9_VDENC_PIPELINE_XE2_LPM_H__

#include "encode_vp9_vdenc_pipeline_xe2_lpm_base.h"
#include "encode_vp9_vdenc_feature_manager_xe2_lpm.h"

namespace encode
{
class Vp9VdencPipelineXe2_Lpm : public Vp9VdencPipelineXe2_Lpm_Base
{
public:
    //!
    //! \brief  Vp9VdencPipelineXe2_Lpm constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Vp9VdencPipelineXe2_Lpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    //!
    //! \brief  Vp9VdencPipelineXe2_Lpm destructor
    //!
    virtual ~Vp9VdencPipelineXe2_Lpm() {}

    //!
    //! \brief  Initialize the VP9 VDENC pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) override;

protected:
    //!
    //! \brief  Create VP9 VDENC feature manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureManager() override;

MEDIA_CLASS_DEFINE_END(encode__Vp9VdencPipelineXe2_Lpm)
};

}  // namespace encode
#endif  // !__ENCODE_VP9_VDENC_PIPELINE_XE2_LPM_H__
