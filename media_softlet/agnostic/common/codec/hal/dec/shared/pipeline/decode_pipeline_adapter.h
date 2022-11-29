/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     decode_pipeline_adapter.h
//! \brief    Defines the interface to adapt to decode pipeline
//!

#ifndef __DECODE_PIPELINE_ADAPTER_H__
#define __DECODE_PIPELINE_ADAPTER_H__

#include "codechal_common.h"
#include "codec_def_decode.h"

class DecodePipelineAdapter: public Codechal
{
public:
    //!
    //! \brief    Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //!
    DecodePipelineAdapter(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface *debugInterface) : Codechal(hwInterface, debugInterface)
    {
        m_apogeiosEnable = true;
    };

    virtual bool IsHybridDecoder() { return false; }

    virtual bool IsIncompletePicture() = 0;
    virtual bool IsIncompleteJpegScan() = 0;

#ifdef _DECODE_PROCESSING_SUPPORTED
    //!
    //! \brief  Indicates whether down sampling is supported
    //! \return If down sampling is supported by decode
    //!
    virtual bool IsDownSamplingSupported()= 0;
#endif

    virtual MOS_SURFACE* GetDummyReference() = 0;
    virtual CODECHAL_DUMMY_REFERENCE_STATUS GetDummyReferenceStatus() = 0;
    virtual void SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status) = 0;
    virtual uint32_t GetCompletedReport() = 0;
    virtual MOS_GPU_CONTEXT GetDecodeContext() = 0;
    virtual GPU_CONTEXT_HANDLE GetDecodeContextHandle() = 0;
    virtual MOS_STATUS SetDecodeFormat(bool isShortFormat ){ return MOS_STATUS_UNIMPLEMENTED; };

MEDIA_CLASS_DEFINE_END(DecodePipelineAdapter)
};
#endif // !__DECODE_PIPELINE_ADAPTER_H__

