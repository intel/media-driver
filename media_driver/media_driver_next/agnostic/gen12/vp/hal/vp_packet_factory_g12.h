/*
* Copyright (c) 2018, Intel Corporation
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
#ifndef __VP_PACKET_FACTORY_G12_H__
#define __VP_PACKET_FACTORY_G12_H__

#include "media_cmd_packet.h"
#include "mhw_sfc.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include <vector>
#include "hw_filter.h"
#include "vp_packet_pipe.h"

namespace vp {

class PacketFactoryG12 : public PacketFactory
{
public:
    PacketFactoryG12();
    virtual ~PacketFactoryG12();
    virtual VpCmdPacket *CreateVeboxPacket();
    virtual VpCmdPacket *CreateRenderPacket();
};

}
#endif // !__VP_PACKET_PIPE_H__
