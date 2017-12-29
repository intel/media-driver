/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mhw_vdbox_huc_g9_skl.cpp
//! \brief    Constructs VdBox Huc commands on Gen9 BXT+ platforms

#include "mhw_vdbox_huc_g9_skl.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_skl.h"

void MhwVdboxHucInterfaceG9Skl::InitMmioRegisters()
{
    MmioRegistersHuc *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

    mmioRegisters->hucUKernelHdrInfoRegOffset = 0x0D014;
    mmioRegisters->hucStatusRegOffset         = 0x0D000;
    mmioRegisters->hucStatus2RegOffset        = 0x0D3B0;
}

MOS_STATUS MhwVdboxHucInterfaceG9Skl::GetHucStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MhwVdboxHucInterfaceGeneric<mhw_vdbox_huc_g9_skl, mhw_mi_g9_X>::GetHucStateCommandSize(mode, commandsSize, patchListSize, params);

    uint32_t            maxSize = *commandsSize;
    uint32_t            patchListMaxSize = *patchListSize;
    uint32_t            standard = CodecHal_GetStandardFromMode(mode);

    if (standard == CODECHAL_HEVC)
    {
        if (mode != CODECHAL_ENCODE_MODE_HEVC && params->bShortFormat)
        {
            maxSize += mhw_vdbox_vdenc_g9_skl::VD_PIPELINE_FLUSH_CMD::byteSize;

            patchListMaxSize += PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD);
        }
    }
    else if (standard == CODECHAL_CENC)
    {
        maxSize = mhw_vdbox_vdenc_g9_skl::VD_PIPELINE_FLUSH_CMD::byteSize;

        patchListMaxSize += PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD);
    }
    else if (standard == CODECHAL_VP9)
    {
        if (mode == CODECHAL_ENCODE_MODE_VP9 && params->bHucDummyStream)
        {
            maxSize += mhw_vdbox_vdenc_g9_skl::VD_PIPELINE_FLUSH_CMD::byteSize;

            patchListMaxSize += PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD);
        }
    }
    else if (standard == CODECHAL_AVC)
    {
        maxSize += mhw_vdbox_vdenc_g9_skl::VD_PIPELINE_FLUSH_CMD::byteSize;

        patchListMaxSize += PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD);
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported standard.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

