/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_huc_brc_update_packet_xe_lpm_plus.h
//! \brief    Defines the implementation of xe_lpm_plus huc update packet 
//!

#ifndef __CODECHAL_HUC_BRC_UPDATE_PACKET_XE_LPM_PLUS_H__
#define __CODECHAL_HUC_BRC_UPDATE_PACKET_XE_LPM_PLUS_H__

#include "encode_huc_brc_update_packet.h"

namespace encode
{
    class HucBrcUpdatePktXe_Lpm_Plus : public HucBrcUpdatePkt
    {
    public:
        HucBrcUpdatePktXe_Lpm_Plus(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
            HucBrcUpdatePkt(pipeline, task, hwInterface)
        {
        }

        virtual ~HucBrcUpdatePktXe_Lpm_Plus(){};

        MOS_STATUS Init() override;

    protected:
        virtual MOS_STATUS ConstructGroup1Cmds() override;
        virtual MOS_STATUS ConstructGroup2Cmds() override;
        virtual MOS_STATUS ConstructGroup3Cmds() override;
        virtual MOS_STATUS SetConstDataHuCBrcUpdate() const override;

        MOS_STATUS AddAllCmds_HCP_PAK_INSERT_OBJECT_SLICE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        uint32_t m_alignSize[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = {0};

    MEDIA_CLASS_DEFINE_END(encode__HucBrcUpdatePktXe_Lpm_Plus)
    };

}  // namespace encode
#endif
