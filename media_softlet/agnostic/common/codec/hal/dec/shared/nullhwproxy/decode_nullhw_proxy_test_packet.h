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
//! \file     decode_nullhw_proxy_test_packet.h
//!
#ifndef __DECODE_NULLHW_PROXY_TEST_PACKET_H__
#define __DECODE_NULLHW_PROXY_TEST_PACKET_H__
#include "decode_allocator.h"
#include "decode_utils.h"
#include "decode_pipeline.h"
#include "mhw_mi_itf.h"
namespace decode
{
#if (_DEBUG || _RELEASE_INTERNAL)
class DecodeNullHWProxyTestPkt
{
private:
    DecodeNullHWProxyTestPkt() {}
    MOS_BUFFER       *m_dataStoreBuf       = nullptr;
    BatchBufferArray *m_secondLevelBBArray = nullptr;
    PMHW_BATCH_BUFFER m_batchBuf           = nullptr;
    DecodeAllocator  *m_allocator          = nullptr;
    int32_t           m_isNullHWEnabled    = 0;
    int32_t           m_repeatCount        = 0;
    MOS_COMMAND_BUFFER m_statusCheckCmdBuffer = {};
    std::shared_ptr<mhw::mi::Itf> m_miItf;
    MOS_RESOURCE                  m_conditionalBbEndDummy;  //!> Dummy Resource for conditional batch buffer end WA
public:
    ~DecodeNullHWProxyTestPkt() 
    { 
        Destory();
    }
    DecodeNullHWProxyTestPkt(const DecodeNullHWProxyTestPkt &) = delete;
    DecodeNullHWProxyTestPkt &operator=(const DecodeNullHWProxyTestPkt &) = delete;
    static DecodeNullHWProxyTestPkt* Instance()
    {
        static DecodeNullHWProxyTestPkt instance;
        return &instance;
    }
    MOS_STATUS Init(DecodePipeline* pipeline, MOS_INTERFACE *osInterface);
    MOS_STATUS Destory();
    MOS_STATUS Init2ndLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase);
    MOS_STATUS Pack2ndLevelCmds(DecodePipeline *pipeline, MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddNullHwProxyCmd(DecodePipeline *pipeline, MOS_INTERFACE *osInterface, PMOS_COMMAND_BUFFER cmdBuffer);
    MOS_STATUS SendCondBbEndCmd(
        PMOS_RESOURCE resource,
        uint32_t      offset,
        uint32_t      compData,
        bool          disableCompMask,
        bool          enableEndCurrentBatchBuffLevel,
        uint32_t      compareOperation,
        PMOS_COMMAND_BUFFER cmdBuffer);
};
#endif
}
#endif
