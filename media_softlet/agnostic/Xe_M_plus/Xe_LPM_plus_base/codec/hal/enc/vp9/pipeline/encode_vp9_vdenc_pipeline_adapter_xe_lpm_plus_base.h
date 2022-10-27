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
//! \file     encode_vp9_vdenc_pipeline_adapter_xe_lpm_plus_base.h
//! \brief    Defines the interface to adapt to vp9 vdenc encode pipeline
//!

#ifndef __ENCODE_VP9_VDENC_PIPELINE_ADAPTER_XE_LPM_PLUS_BASE_H__
#define __ENCODE_VP9_VDENC_PIPELINE_ADAPTER_XE_LPM_PLUS_BASE_H__

#include "encode_pipeline_adapter.h"
#include "encode_vp9_vdenc_pipeline_xe_lpm_plus_base.h"

class EncodeVp9VdencPipelineAdapterXe_Lpm_Plus_Base : public EncoderPipelineAdapter
{
public:
    //!
    //! \brief    EncodeVp9VdencPipelineAdapterXe_Lpm_Plus_Base constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //!
    EncodeVp9VdencPipelineAdapterXe_Lpm_Plus_Base(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    //!
    //! \brief  EncodeVp9VdencPipelineAdapterXe_Lpm_Plus_Base destructor
    //!
    virtual ~EncodeVp9VdencPipelineAdapterXe_Lpm_Plus_Base() {}

    //!
    //! \brief    Allocate and intialize the VP9 VDENC pipeline.
    //! \param    [in] codecHalSettings
    //!           Settings used to finalize the creation of the VP9 VDENC pipeline
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS Allocate(CodechalSetting *codecHalSettings);

    //!
    //! \brief    Performs the operation requested by the codec function.
    //! \param    [in] params
    //!           Parameters need to perform the requested function. The parameter structure
    //!           changes based on codec function.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS Execute(void *params);

    //!
    //! \brief    Gets available status for executed pictures.
    //! \details  All pictures for which EndFrame() has been called are eligable
    //!           for status reporting. Once a successful or error status is reported out by
    //!           CodecHal, it is discarded.
    //! \param    [out] status
    //!           Array to store statuses up to a maximum of wNumStatus, valid pointer
    //!           must be passed in to GetStatusReport()
    //! \param    [in] numStatus
    //!           The size of the pCodecStatus array
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus);

    //!
    //! \brief  Destroy VP9 VDENC pipeline
    //!
    //! \return void
    //!
    virtual void Destroy();

protected:
    std::shared_ptr<encode::Vp9VdencPipelineXe_Lpm_Plus_Base> m_encoder = nullptr;

MEDIA_CLASS_DEFINE_END(EncodeVp9VdencPipelineAdapterXe_Lpm_Plus_Base)
};
#endif // !__ENCODE_VP9_VDENC_PIPELINE_ADAPTER_XE_LPM_PLUS_BASE_H__

