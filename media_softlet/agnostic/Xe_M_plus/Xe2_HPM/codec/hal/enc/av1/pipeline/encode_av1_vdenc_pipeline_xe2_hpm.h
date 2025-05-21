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
//! \file     encode_av1_vdenc_pipeline_xe2_hpm.h
//! \brief    Defines the interface for av1 vdenc encode pipeline Xe2_HPM
//!
#ifndef __ENCODE_AV1_VDENC_PIPELINE_XE2_HPM_H__
#define __ENCODE_AV1_VDENC_PIPELINE_XE2_HPM_H__

#include "encode_av1_vdenc_pipeline_xe_lpm_plus_base.h"
#include "encode_av1_vdenc_feature_manager.h"

namespace encode
{
class Av1VdencPipelineXe2_Hpm : public Av1VdencPipelineXe_Lpm_Plus_Base
{
public:
    //!
    //! \brief  Av1VdencPipelineXe2_Hpm constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Av1VdencPipelineXe2_Hpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface) : Av1VdencPipelineXe_Lpm_Plus_Base(hwInterface, debugInterface) {}

    MOS_STATUS Init(void *settings) override;

protected:
    MOS_STATUS CreateFeatureManager() override;
    MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPipelineXe2_Hpm)
};

}  // namespace encode
#endif  // __ENCODE_AV1_VDENC_PIPELINE_XE2_HPM_H__
