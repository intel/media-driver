/*
* Copyright (c) 2011-2022, Intel Corporation
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
//! \file     codechal_debug_misc.cpp
//! \brief    Defines the debug interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_config_manager.h"
#include "codechal_encoder_base.h"
#include <iomanip>

MOS_STATUS CodechalDebugInterface::DetectCorruptionSw(std::vector<MOS_RESOURCE> &vResource, PMOS_RESOURCE frameCntRes, uint8_t *buf, uint32_t &size, uint32_t frameNum)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DetectCorruptionHw(CodechalHwInterface *hwInterface, PMOS_RESOURCE frameCntRes, uint32_t curIdx, uint32_t frameCrcOffset, std::vector<MOS_RESOURCE> &vStatusBuffer, PMOS_COMMAND_BUFFER pCmdBuffer, uint32_t frameNum)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::StoreNumFrame(PMHW_MI_INTERFACE pMiInterface, PMOS_RESOURCE pResource, int32_t frameNum, PMOS_COMMAND_BUFFER pCmdBuffer)
{
    return MOS_STATUS_SUCCESS;
}

#endif // USE_CODECHAL_DEBUG_TOOL
