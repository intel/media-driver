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
//! \file     decode_mpeg2_slice_packet_xe3p_lpm_base.cpp
//! \brief    Defines the interface for vvc decode slice packet for Xe3P_LPM_Base
//!

#include "decode_vvc_slice_packet_xe3p_lpm_base.h"

namespace decode
{
    MOS_STATUS VvcDecodeSlcPktXe3P_Lpm_Base::CalculateSliceStateCommandSize()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        uint32_t maxSize = 0, patchListMaxSize = 0;

        DECODE_CHK_NULL(m_vvcpItf);
        DECODE_CHK_NULL(m_miItf);
        DECODE_CHK_NULL(m_vdencItf);

        // MI_BATCH_BUFFER_START is added here because each slice needs to jump to pic-level cmd in 2nd level BB
        maxSize =
            m_vvcpItf->MHW_GETSIZE_F(VVCP_SLICE_STATE)() +
            m_vvcpItf->MHW_GETSIZE_F(VVCP_REF_IDX_STATE)() * 2 +
            m_vvcpItf->MHW_GETSIZE_F(VVCP_WEIGHTOFFSET_STATE)() * 2 +
            m_vvcpItf->MHW_GETSIZE_F(VVCP_BSD_OBJECT)() +
            m_vvcpItf->MHW_GETSIZE_F(VVCP_TILE_CODING)() +
            m_vvcpItf->MHW_GETSIZE_F(VVCP_VD_CONTROL_STATE)() * 2 +
            m_vvcpItf->MHW_GETSIZE_F(VVCP_PIPE_MODE_SELECT)() +
            m_miItf->MHW_GETSIZE_F(MFX_WAIT)() * 2 +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() +
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)();

        patchListMaxSize =
            PATCH_LIST_COMMAND(VVCP_SLICE_STATE_CMD) +
            PATCH_LIST_COMMAND(VVCP_REF_IDX_STATE_CMD) * 2 +
            PATCH_LIST_COMMAND(VVCP_WEIGHTOFFSET_STATE_CMD) * 2 +
            PATCH_LIST_COMMAND(VVCP_BSD_OBJECT_CMD) +
            PATCH_LIST_COMMAND(VVCP_TILE_CODING_CMD) +
            PATCH_LIST_COMMAND(VVCP_VD_CONTROL_STATE_CMD) * 2 +
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(VVCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD);

        m_tileStateSize = m_vvcpItf->MHW_GETSIZE_F(VVCP_TILE_CODING)() +
                            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
        m_tilePatchListSize = PATCH_LIST_COMMAND(VVCP_TILE_CODING_CMD);

        m_sliceStatesSize    = maxSize;
        m_slicePatchListSize = patchListMaxSize;

        return eStatus;
    }

}  // namespace decode
