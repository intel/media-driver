/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     decode_pipeline_xe3_lpm_base.h
//! \brief    Defines the common interface for decode pipeline
//! \details  The decode pipeline interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#ifndef __DECODE_PIPELINE_XE3_LPM_BASE_H__
#define __DECODE_PIPELINE_XE3_LPM_BASE_H__

#include "decode_pipeline.h"
#include "codec_hw_xe3_lpm_base.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode {
    class DecodePipelineXe3LpmBase : public DecodePipeline
    {
    public:
        //!
        //! \brief  DecodePipeline constructor
        //! \param  [in] hwInterface
        //!         Pointer to CodechalHwInterface
        //! \param  [in] debugInterface
        //!         Pointer to CodechalDebugInterface
        //!
        DecodePipelineXe3LpmBase(
            CodechalHwInterfaceNext* hwInterface,
            CodechalDebugInterface* debugInterface) : DecodePipeline(hwInterface, debugInterface)
        {
            m_hwInterface = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base *>(hwInterface);
        }

        virtual ~DecodePipelineXe3LpmBase()
        {
        }

    protected:
        //!
        //! \brief  Create Post sub pipelines
        //! \param  [in] subPipelineManager
        //!         Point to subPipeline manager
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS CreatePostSubPipeLines(DecodeSubPipelineManager& subPipelineManager) override;

        MEDIA_CLASS_DEFINE_END(decode__DecodePipelineXe3LpmBase)
    };
}
#endif //_DECODE_PROCESSING_SUPPORTED
#endif //__DECODE_PIPELINE_XE3_LPM_BASE_H__
