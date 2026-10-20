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
//! \file     decode_vvc_slice_packet_xe3p_lpm_base.h
//! \brief    Defines the implementation of vvc decode slice packet for Xe3P_LPM_Base
//!

#ifndef __DECODE_VVC_SLICE_PACKET_XE3P_LPM_BASE_H__
#define __DECODE_VVC_SLICE_PACKET_XE3P_LPM_BASE_H__

#include "media_cmd_packet.h"
#include "decode_vvc_pipeline.h"
#include "decode_utils.h"
#include "decode_vvc_basic_feature.h"
#include "decode_vvc_slice_packet.h"

namespace decode
{
    class VvcDecodeSlcPktXe3P_Lpm_Base : public VvcDecodeSlicePkt
    {
    public:
        VvcDecodeSlcPktXe3P_Lpm_Base(VvcPipeline *pipeline, CodechalHwInterfaceNext* hwInterface)
            : VvcDecodeSlicePkt(pipeline, hwInterface)
        {
        }
        virtual ~VvcDecodeSlcPktXe3P_Lpm_Base() {};

        uint32_t GetSliceStatesSize()
        {
            return m_sliceStatesSize;
        }

        uint32_t GetSlicePatchListSize()
        {
            return m_slicePatchListSize;
        }

        uint32_t GetTileStatesSize()
        {
            return m_tileStateSize;
        }

        uint32_t GetTilePatchListSize()
        {
            return m_tilePatchListSize;
        }

    protected:
        //!
        //! \brief  Calculate slcie level command Buffer Size
        //!
        //! \return uint32_t
        //!         Command buffer size calculated
        //!
        MOS_STATUS CalculateSliceStateCommandSize() override;

    MEDIA_CLASS_DEFINE_END(decode__VvcDecodeSlcPktXe3P_Lpm_Base)
    };

}  // namespace decode

#endif  // __DECODE_VVC_SLICE_PACKET_XE3P_LPM_BASE_H__
