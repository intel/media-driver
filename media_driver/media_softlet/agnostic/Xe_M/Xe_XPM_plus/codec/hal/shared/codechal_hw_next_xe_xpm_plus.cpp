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
//! \file     codechal_hw_next_xe_xpm_plus.cpp
//! \brief    Implements HW interface layer for PVC used on all OSs.
//! \details  Implements HW interface layer for PVC to be used on on all operating systems/DDIs, across CODECHAL components.
//!           This module must not contain any OS dependent code.
//!

#include "codechal_hw_next_xe_xpm_plus.h"
#include "media_interfaces_pvc.h"  //temporary include for getting avp interface
#include "codechal_hw_g12_X.h"
#include "mhw_render_g12_X.h"
#include "mhw_vdbox_hcp_hwcmd_g12_X.h"  // temporary include for calculating size of various hardware commands
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"

void CodechalHwInterfaceNextXe_Xpm_Plus::PrepareCmdSize(CODECHAL_FUNCTION codecFunction)
{
    InitCacheabilityControlSettings(codecFunction);

    m_isVdencSuperSliceEnabled = true;

    // Set platform dependent parameters
    m_sizeOfCmdBatchBufferEnd    = mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_sizeOfCmdMediaReset        = mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 8;
    m_vdencBrcImgStateBufferSize = 80 + mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize + 92 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencBatchBuffer1stGroupSize = mhw_vdbox_hcp_g12_X::HCP_PIPE_MODE_SELECT_CMD::byteSize + mhw_mi_g12_X::MFX_WAIT_CMD::byteSize * 2 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencBatchBuffer2ndGroupSize = 132 + mhw_vdbox_hcp_g12_X::HCP_PIC_STATE_CMD::byteSize + 248 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencReadBatchBufferSize =
        m_vdenc2ndLevelBatchBufferSize = m_vdencBatchBuffer1stGroupSize + m_vdencBatchBuffer2ndGroupSize + ENCODE_HEVC_VDENC_NUM_MAX_SLICES * (2 * mhw_vdbox_hcp_g12_X::HCP_WEIGHTOFFSET_STATE_CMD::byteSize + mhw_vdbox_hcp_g12_X::HCP_SLICE_STATE_CMD::byteSize + 3 * mhw_vdbox_hcp_g12_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize + 28 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize + 4 * ENCODE_VDENC_HEVC_PADDING_DW_SIZE);

    m_HucStitchCmdBatchBufferSize = 7 * 4 + 14 * 4 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    // HCP_WEIGHTOFFSET_STATE_CMD cmds is planned to be added in near future
    m_vdencBatchBufferPerSliceConstSize = mhw_vdbox_hcp_g12_X::HCP_SLICE_STATE_CMD::byteSize + mhw_vdbox_hcp_g12_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize  // 1st PakInsertObject cmd is not always inserted for each slice, 2nd PakInsertObject cmd is always inserted for each slice
                                          + 28 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    // Set to size of the BRC update command buffer, since it is larger than BRC Init/ PAK integration commands
    m_hucCommandBufferSize = mhw_vdbox_huc_g12_X::HUC_IMEM_STATE_CMD::byteSize + mhw_vdbox_huc_g12_X::HUC_PIPE_MODE_SELECT_CMD::byteSize + mhw_mi_g12_X::MFX_WAIT_CMD::byteSize * 3 + mhw_vdbox_huc_g12_X::HUC_DMEM_STATE_CMD::byteSize + mhw_vdbox_huc_g12_X::HUC_VIRTUAL_ADDR_STATE_CMD::byteSize + mhw_vdbox_huc_g12_X::HUC_STREAM_OBJECT_CMD::byteSize + mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize + mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize + mhw_vdbox_huc_g12_X::HUC_START_CMD::byteSize + mhw_vdbox_vdenc_g12_X::VD_PIPELINE_FLUSH_CMD::byteSize + mhw_mi_g12_X::MI_FLUSH_DW_CMD::byteSize + mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize * 2 + mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 2 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

}

CodechalHwInterfaceNextXe_Xpm_Plus::CodechalHwInterfaceNextXe_Xpm_Plus(
    PMOS_INTERFACE     osInterface,
    CODECHAL_FUNCTION  codecFunction,
    MhwInterfacesNext *mhwInterfacesNext,
    bool               disableScalability)
    : CodechalHwInterfaceNext(osInterface, codecFunction, mhwInterfacesNext, disableScalability)
{
    CODECHAL_HW_FUNCTION_ENTER;

    PrepareCmdSize(codecFunction);
    m_miInterface     = static_cast<MhwInterfacesPvc_Next*>(mhwInterfacesNext)->m_miInterface;
    m_renderInterface = static_cast<MhwInterfacesPvc_Next*>(mhwInterfacesNext)->m_renderInterface;
}

CodechalHwInterfaceNextXe_Xpm_Plus::~CodechalHwInterfaceNextXe_Xpm_Plus()
{
    if (m_renderInterface != nullptr)
    {
        MOS_Delete(m_renderInterface);
    }
}