/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     decode_jpeg_pipeline_xe2_hpm.h
//! \brief    Defines the interface for jpeg decode pipeline
//!
#ifndef __DECODE_JPEG_PIPELINE_XE2_HPM_H__
#define __DECODE_JPEG_PIPELINE_XE2_HPM_H__

#include "decode_jpeg_pipeline_xe_lpm_plus_base.h"

namespace decode
{

class JpegPipelineXe2_Hpm : public JpegPipelineXe_Lpm_Plus_Base
{
public:
    //!
    //! \brief  DecodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    JpegPipelineXe2_Hpm(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface  *debugInterface);

    virtual ~JpegPipelineXe2_Hpm(){};

protected:

#ifdef _MMC_SUPPORTED
    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState() override;
#endif

MEDIA_CLASS_DEFINE_END(decode__JpegPipelineXe2_Hpm)
};

}  // namespace decode
#endif  // !__DECODE_JPEG_PIPELINE_XE2_HPM_H__
