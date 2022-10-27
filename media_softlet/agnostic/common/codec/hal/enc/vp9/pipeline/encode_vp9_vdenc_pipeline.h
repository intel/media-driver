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
//! \file     encode_vp9_vdenc_pipeline.h
//! \brief    Defines the interface for vp9 vdenc encode pipeline
//!
#ifndef __ENCODE_VP9_VDENC_PIPELINE_H__
#define __ENCODE_VP9_VDENC_PIPELINE_H__

#include "encode_vp9_pipeline.h"

namespace encode {

class Vp9VdencPipeline : public Vp9Pipeline
{
public:
    //!
    //! \brief  Vp9VdencPipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Vp9VdencPipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    //!
    //! \brief  Vp9VdencPipeline destructor
    //!
    virtual ~Vp9VdencPipeline() {}

    //!
    //! \brief  Prepare internal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) override;

protected:
    //!
    //! \brief  Initialize the VP9 VDENC pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings) override;

    //!
    //! \brief  Uninitialize the VP9 VDENC pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Uninitialize() override;

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    //!
    //! \brief  Activates VDENC video packet and add it to activate packet list
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ActivateVdencVideoPackets();

    //!
    //! \brief  Create VP9 VDENC feature manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureManager() override;

MEDIA_CLASS_DEFINE_END(encode__Vp9VdencPipeline)
};

}
#endif // !__ENCODE_VP9_VDENC_PIPELINE_H__
