/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_pipeline_adapter.h
//! \brief    Defines the interface to adapt to encode pipeline
//!

#ifndef __ENCODE_PIPELINE_ADAPTER_H__
#define __ENCODE_PIPELINE_ADAPTER_H__

#include "codechal_common.h"

class EncoderPipelineAdapter: public Codechal
{
public:
    //!
    //! \brief    Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //!
    EncoderPipelineAdapter(
        CodechalHwInterfaceNext    *hwInterface,
        CodechalDebugInterface *debugInterface) : Codechal(hwInterface, debugInterface)
    {
        m_apogeiosEnable = true;
    };

    uint32_t                        m_mvOffset = 0;                 //!< MV data offset, in 64 byte
    uint32_t                        m_mbcodeBottomFieldOffset = 0;  //!< MB code offset frame/TopField - zero, BottomField - nonzero
    uint32_t                        m_mvBottomFieldOffset = 0;      //!< MV data offset frame/TopField - zero, BottomField - nonzero

    bool                            m_vdencEnabled = false;         //!< Vdenc enabled flag

MEDIA_CLASS_DEFINE_END(EncoderPipelineAdapter)
};
#endif // !__ENCODE_PIPELINE_ADAPTER_H__

