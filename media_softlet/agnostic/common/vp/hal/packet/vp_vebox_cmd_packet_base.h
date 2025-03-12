/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_vebox_cmd_packet.h
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#ifndef __VP_VEBOX_CMD_PACKET_BASE_H__
#define __VP_VEBOX_CMD_PACKET_BASE_H__

#include "vp_cmd_packet.h"
#include "vp_vebox_common.h"

namespace vp {

class VpVeboxCmdPacketBase : virtual public VpCmdPacket
{
public:
    VpVeboxCmdPacketBase(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc);
    virtual ~VpVeboxCmdPacketBase();
    virtual MOS_STATUS SetSfcCSCParams(PSFC_CSC_PARAMS cscParams)       = 0;
    virtual MOS_STATUS SetVeboxCSCParams(PVEBOX_CSC_PARAMS cscParams)   = 0;
    virtual MOS_STATUS SetVeboxBeCSCParams(PVEBOX_CSC_PARAMS cscParams) = 0;
    virtual MOS_STATUS SetVeboxFeCSCParams(PVEBOX_CSC_PARAMS cscParams) = 0;
    virtual MOS_STATUS SetDiParams(PVEBOX_DI_PARAMS diParams)           = 0;
    virtual MOS_STATUS SetDnParams(PVEBOX_DN_PARAMS pDnParams)          = 0;
    virtual MOS_STATUS SetHdrParams(PVEBOX_HDR_PARAMS hdrParams)        = 0;
    virtual MOS_STATUS SetProcampParams(PVEBOX_PROCAMP_PARAMS pProcampParams) = 0;
    virtual MOS_STATUS SetSfcRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams)   = 0;
    virtual MOS_STATUS SetScalingParams(PSFC_SCALING_PARAMS scalingParams)    = 0;
    virtual MOS_STATUS SetSteParams(PVEBOX_STE_PARAMS pSteParams)             = 0;
    virtual MOS_STATUS SetTccParams(PVEBOX_TCC_PARAMS pTccParams)             = 0;
    virtual MOS_STATUS SetCgcParams(PVEBOX_CGC_PARAMS veboxCgcParams)         = 0;


    virtual MOS_STATUS UpdateCscParams(FeatureParamCsc &params) = 0;
    virtual MOS_STATUS UpdateDenoiseParams(FeatureParamDenoise &params) = 0;
    virtual MOS_STATUS UpdateTccParams(FeatureParamTcc &params) = 0;
    virtual MOS_STATUS UpdateSteParams(FeatureParamSte &params) = 0;
    virtual MOS_STATUS UpdateProcampParams(FeatureParamProcamp &params) = 0;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxCmdPacketBase)
};

}
#endif // !__VP_VEBOX_CMD_PACKET_H__
