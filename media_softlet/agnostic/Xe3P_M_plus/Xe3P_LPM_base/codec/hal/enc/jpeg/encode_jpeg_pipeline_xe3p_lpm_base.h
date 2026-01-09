/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_jpeg_pipeline_xe3p_lpm_base.h
//! \brief    Defines the interface for jpeg encode pipeline Xe3P_LPM_Base
//!
#ifndef __ENCODE_JPEG_PIPELINE_XE3P_LPM_BASE_H__
#define __ENCODE_JPEG_PIPELINE_XE3P_LPM_BASE_H__

#include "encode_jpeg_pipeline.h"

namespace encode {

class JpegPipelineXe3P_Lpm_Base : public JpegPipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    JpegPipelineXe3P_Lpm_Base(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface) : JpegPipeline(hwInterface, debugInterface) {}

    virtual ~JpegPipelineXe3P_Lpm_Base() {}

    MOS_STATUS InitMmcState();

MEDIA_CLASS_DEFINE_END(encode__JpegPipelineXe3P_Lpm_Base)
};

}
#endif // !__ENCODE_JPEG_PIPELINE_XE3P_LPM_BASE_H__
