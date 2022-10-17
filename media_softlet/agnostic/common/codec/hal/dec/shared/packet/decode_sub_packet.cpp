/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_sub_packet.cpp
//! \brief    Defines the common interface for decode sub packet
//! \details  The decocode sub packet interface is further sub-divided by different packet usages,
//!           this file is for the base interface which is shared by all decode packets.
//!

#include "decode_sub_packet.h"
#include "decode_pipeline.h"

namespace decode
{

DecodeSubPacket::DecodeSubPacket(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : m_pipeline(pipeline), m_hwInterface(hwInterface)
{
    if (m_pipeline != nullptr)
    {
        m_featureManager = m_pipeline->GetFeatureManager();
    }

    if (m_hwInterface != nullptr)
    {
        m_osInterface   = hwInterface->GetOsInterface();
    }
}

}
