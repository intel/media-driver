/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     decode_vvc_pipeline_xe3_lpm_base.h
//! \brief    Defines the interface for avc decode pipeline
//!
#ifndef __DECODE_VVC_PIPELINE_XE3_LPM_BASE_H__
#define __DECODE_VVC_PIPELINE_XE3_LPM_BASE_H__

#include "decode_vvc_pipeline.h"
#include "decode_huc_packet_creator.h"

namespace decode
{

class VvcPipelineXe3_Lpm_Base : public VvcPipeline, public HucPacketCreator
{
public:
    //!
    //! \brief  DecodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    VvcPipelineXe3_Lpm_Base(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface  *debugInterface);

    MOS_STATUS Init(void *settings) override;

    virtual ~VvcPipelineXe3_Lpm_Base(){}
    CmdPacket               *m_vvcDecodeS2LPkt = nullptr;
    CodechalHwInterfaceNext *m_hwInterface     = nullptr;
    MEDIA_CLASS_DEFINE_END(decode__VvcPipelineXe3_Lpm_Base)
};

}  // namespace decode
#endif  // !__DECODE_VVC_PIPELINE_XE3_LPM_BASE_H__
