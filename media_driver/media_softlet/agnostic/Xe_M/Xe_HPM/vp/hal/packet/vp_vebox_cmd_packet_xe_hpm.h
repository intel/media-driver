/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     vp_vebox_cmd_packet_xe_hpm.h
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#ifndef __VP_VEBOX_CMD_PACKET_XE_HPM_H__
#define __VP_VEBOX_CMD_PACKET_XE_HPM_H__

#include "vp_vebox_cmd_packet_xe_xpm_base.h"

namespace vp {

class VpVeboxCmdPacketXe_Hpm : virtual public VpVeboxCmdPacketXe_Xpm_Base
{
public:
    VpVeboxCmdPacketXe_Hpm(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc, bool disbaleSfcDithering);

    virtual ~VpVeboxCmdPacketXe_Hpm();

    virtual MOS_STATUS AddVeboxDndiState() override;

    virtual MOS_STATUS QueryStatLayoutGNE(
        VEBOX_STAT_QUERY_TYPE QueryType,
        uint32_t             *pQuery,
        uint8_t              *pStatSlice0Base,
        uint8_t              *pStatSlice1Base) override;

    virtual MOS_STATUS CheckTGNEValid(
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr,
        uint32_t *pQuery);

    virtual MOS_STATUS UpdateDnHVSParameters(
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr) override;

    virtual MOS_STATUS SetupDNTableForHVS(
        PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams) override;

    MOS_STATUS GNELumaConsistentCheck(
        uint32_t &dwGNELuma,
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr);

    // TGNE
    uint32_t dwGlobalNoiseLevel_Temporal  = 0;  //!< Global Temporal Noise Level for Y
    uint32_t dwGlobalNoiseLevelU_Temporal = 0;  //!< Global Temporal Noise Level for U
    uint32_t dwGlobalNoiseLevelV_Temporal = 0;  //!< Global Temporal Noise Level for V
    uint32_t curNoiseLevel_Temporal       = 0;  //!< Temporal Noise Level for Y
    uint32_t curNoiseLevelU_Temporal      = 0;  //!< Temporal Noise Level for U
    uint32_t curNoiseLevelV_Temporal      = 0;  //!< Temporal Noise Level for V
    bool     m_bTgneEnable                = true;
    bool     m_bTgneValid                 = false;

    MhwVeboxInterfaceG12::MHW_VEBOX_CHROMA_PARAMS veboxChromaParams = {};

MEDIA_CLASS_DEFINE_END(vp__VpVeboxCmdPacketXe_Hpm)
};

}
#endif // !__VP_VEBOX_CMD_PACKET_XE_HPM_H__
