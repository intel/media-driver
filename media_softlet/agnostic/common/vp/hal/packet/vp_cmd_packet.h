/*
* Copyright (c) 2018-2021, Intel Corporation
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
#ifndef __VP_CMD_PACKET_H__
#define __VP_CMD_PACKET_H__

#include "media_cmd_packet.h"
#include "mhw_sfc.h"
#include "mhw_vebox_itf.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"
#include "vp_packet_shared_context.h"
#include "media_scalability.h"

namespace vp {

enum _PacketType
{
    VP_PIPELINE_PACKET_UNINITIALIZED  = 0,
    VP_PIPELINE_PACKET_VEBOX,
    VP_PIPELINE_PACKET_RENDER,
    VP_PIPELINE_PACKET_COMPUTE
};
using PacketType           = _PacketType;

class VpCmdPacket : virtual public CmdPacket
{
public:
    VpCmdPacket(MediaTask *task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc, PacketType packetId);
    virtual ~VpCmdPacket() {};

    void SetPacketSharedContext(VP_PACKET_SHARED_CONTEXT *context)
    {
        m_packetSharedContext = context;
    }

    // Need to remove vphal surface dependence from VpCmdPacket later.
    virtual MOS_STATUS PacketInit(
        VP_SURFACE                          *inputSurface,
        VP_SURFACE                          *outputSurface,
        VP_SURFACE                          *previousSurface,
        VP_SURFACE_SETTING                  &surfSetting,
        VP_EXECUTE_CAPS                     packetCaps) = 0;

    virtual MOS_STATUS PacketInitForReuse(
        VP_SURFACE         *inputSurface,
        VP_SURFACE         *outputSurface,
        VP_SURFACE         *previousSurface,
        VP_SURFACE_SETTING &surfSetting,
        VP_EXECUTE_CAPS     packetCaps)
    {
        VP_FUNC_CALL();
        m_packetResourcesPrepared = false;
        m_PacketCaps              = packetCaps;
        VP_RENDER_CHK_STATUS_RETURN(SetUpdatedExecuteResource(inputSurface, outputSurface, previousSurface, surfSetting));

        // need to update for DNDI case.
        m_DNDIFirstFrame = (!m_PacketCaps.bRefValid && (m_PacketCaps.bDN || m_PacketCaps.bDI));
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS Prepare()
    {
        return MOS_STATUS_SUCCESS;
    };

    virtual MOS_STATUS PrepareState()
    {
        return MOS_STATUS_SUCCESS;
    };

    virtual MOS_STATUS DumpOutput()
    {
        return MOS_STATUS_SUCCESS;
    };

    virtual MOS_STATUS SetUpdatedExecuteResource(
        VP_SURFACE                          *inputSurface,
        VP_SURFACE                          *outputSurface,
        VP_SURFACE                          *previousSurface,
        VP_SURFACE_SETTING &surfSetting)
    {
        VP_RENDER_ASSERTMESSAGE("Should not come here!");
        return MOS_STATUS_SUCCESS;
    }

    PacketType GetPacketId()
    {
        return m_PacketId;
    }

    void SetMediaScalability(MediaScalability *scalability)
    {
        m_scalability = scalability;
    }

    MediaScalability *&GetMediaScalability()
    {
        return m_scalability;
    }

    VP_SURFACE_SETTING &GetSurfSetting()
    {
        return m_surfSetting;
    }

    virtual bool ExtraProcessing()
    {
        return false;
    }

    MOS_STATUS UpdateFeature(SwFilterPipe *swFilter)
    {
        return MOS_STATUS_SUCCESS;
    }

    VP_EXECUTE_CAPS GetExecuteCaps()
    {
        return m_PacketCaps;
    }

protected:
    virtual MOS_STATUS VpCmdPacketInit();
    bool IsOutputPipeVebox()
    {
        return m_PacketCaps.bVebox && !m_PacketCaps.bSFC && !m_PacketCaps.bRender;
    }

    virtual MOS_STATUS SetMediaFrameTracking(RENDERHAL_GENERIC_PROLOG_PARAMS &genericPrologParams);

public:
    // HW intface to access MHW
    PVP_MHWINTERFACE    m_hwInterface = nullptr;
    VP_EXECUTE_CAPS     m_PacketCaps = {};
    PVpAllocator        &m_allocator;
    VPMediaMemComp      *m_mmc = nullptr;
    uint32_t            m_DNDIFirstFrame = 0;
    uint32_t            m_veboxHeapCurState = 0;

protected:
    PacketType                  m_PacketId = VP_PIPELINE_PACKET_UNINITIALIZED;
    VP_PACKET_SHARED_CONTEXT    *m_packetSharedContext    = nullptr;
    VP_SURFACE_SETTING          m_surfSetting;
    bool                        m_packetResourcesPrepared = false;
    VpFeatureReport             *m_report                 = nullptr;

private:
    MediaScalability *          m_scalability = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpCmdPacket)
};
}
#endif // !__VP_CMD_PACKET_H__
