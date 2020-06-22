/*
* Copyright (c) 2019 - 2020, Intel Corporation
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
//! \file     vp_feature_manager.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __HW_FILTER_PIPE_H__
#define __HW_FILTER_PIPE_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include "policy.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_cmd_packet.h"
#include "vp_packet_pipe.h"
#include <vector>
#include "vp_resource_manager.h"

namespace vp
{
class HwFilterPipe
{
public:
    HwFilterPipe(VpInterface &vpInterface);
    virtual ~HwFilterPipe();
    void Clean();
    MOS_STATUS Initialize(SwFilterPipe &swFilterPipe, Policy &policy);
    MOS_STATUS AddHwFilter(HwFilter &hwFilter);
    MOS_STATUS InitPacketPipe(PacketPipe &packetPipe);
    MOS_STATUS UpdateResources();
    uint32_t HwFilterCount();
    // Get engine type of m_Pipe[index]
    EngineType GetEngineType(uint32_t index);

private:
    std::vector<HwFilter*>  m_Pipe;
    VpInterface             &m_vpInterface;
};
}
#endif // !__HW_FILTER_PIPE_H__
