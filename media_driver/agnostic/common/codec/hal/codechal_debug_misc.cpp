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
    if (m_enableHwDebugHooks &&
        m_goldenReferenceExist &&
        m_goldenReferences.size() > 0 &&
        vResource.size() > 0)
    {
        MOS_COMMAND_BUFFER cmdBuffer{};
        std::vector<uint32_t *> vSemaData;
        MHW_GENERIC_PROLOG_PARAMS genericPrologParams{};
        genericPrologParams.pOsInterface  = m_osInterface;
        genericPrologParams.pvMiInterface = m_miInterface;

        CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
        CODECHAL_DEBUG_CHK_STATUS(Mhw_SendGenericPrologCmd(
            &cmdBuffer,
            &genericPrologParams));
        LockSemaResource(vSemaData, vResource);
        // for CRC mismatch detection
        for (uint32_t i = 0; i < vResource.size(); i++)
        {
            CODECHAL_DEBUG_CHK_STATUS(m_hwInterface->SendHwSemaphoreWaitCmd(
                &vResource[i],
                m_goldenReferences[frameNum][i],
                MHW_MI_SAD_EQUAL_SDD,
                &cmdBuffer));
        }
        StoreNumFrame((MhwMiInterface*)m_miInterface, frameCntRes, frameNum, &cmdBuffer);

        SubmitDummyWorkload(&cmdBuffer, false);
        //Get Decode output
        std::vector<uint32_t> data = {CalculateCRC(buf, size)};
        CODECHAL_DEBUG_CHK_STATUS(FillSemaResource(vSemaData, data));
    }
    return MOS_STATUS_SUCCESS;
}

#endif // USE_CODECHAL_DEBUG_TOOL
