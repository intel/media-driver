/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_predication_packet.cpp
//! \brief    Defines the interface for decode predication sub packet
//!
#include "decode_predication_packet_g12.h"
#include "decode_common_feature_defs.h"

namespace decode
{

DecodePredicationPktG12::DecodePredicationPktG12(DecodePipeline *pipeline, CodechalHwInterface *hwInterface)
    : DecodeSubPacket(pipeline, *hwInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS DecodePredicationPktG12::Init()
{
    DECODE_CHK_NULL(m_pipeline);
    DECODE_CHK_NULL(m_hwInterface);

    m_miInterface = m_hwInterface->GetMiInterface();
    DECODE_CHK_NULL(m_miInterface);

    MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);

    m_predication = dynamic_cast<DecodePredication *>(
        featureManager->GetFeature(DecodeFeatureIDs::decodePredication));
    DECODE_CHK_NULL(m_predication);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePredicationPktG12::Prepare()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePredicationPktG12::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
{
    if (!m_predication->m_predicationEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS condBBEndParams;
    MOS_ZeroMemory(&condBBEndParams, sizeof(condBBEndParams));

    // Skip current frame if presPredication is not equal to zero
    if (m_predication->m_predicationNotEqualZero)
    {
        auto                   mmioRegistersMfx = m_hwInterface->SelectVdboxAndGetMmioRegister(MHW_VDBOX_NODE_1, &cmdBuffer);
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        // load presPredication to general purpose register0
        MHW_MI_STORE_REGISTER_MEM_PARAMS loadRegisterMemParams;
        MOS_ZeroMemory(&loadRegisterMemParams, sizeof(loadRegisterMemParams));
        loadRegisterMemParams.presStoreBuffer = &m_predication->m_resPredication->OsResource;
        loadRegisterMemParams.dwOffset        = (uint32_t)m_predication->m_predicationResOffset;
        loadRegisterMemParams.dwRegister      = mmioRegistersMfx->generalPurposeRegister0LoOffset;
        DECODE_CHK_STATUS(m_miInterface->AddMiLoadRegisterMemCmd(
            &cmdBuffer,
            &loadRegisterMemParams));
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterImmParams;
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData     = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister0HiOffset;
        DECODE_CHK_STATUS(m_miInterface->AddMiLoadRegisterImmCmd(
            &cmdBuffer,
            &loadRegisterImmParams));

        // load 0 to general purpose register4
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData     = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister4LoOffset;
        DECODE_CHK_STATUS(m_miInterface->AddMiLoadRegisterImmCmd(
            &cmdBuffer,
            &loadRegisterImmParams));
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData     = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister4HiOffset;
        DECODE_CHK_STATUS(m_miInterface->AddMiLoadRegisterImmCmd(
            &cmdBuffer,
            &loadRegisterImmParams));

        //perform the add operation
        MHW_MI_MATH_PARAMS miMathParams;
        MHW_MI_ALU_PARAMS  miAluParams[4];
        MOS_ZeroMemory(&miMathParams, sizeof(miMathParams));
        MOS_ZeroMemory(&miAluParams, sizeof(miAluParams));
        // load     srcA, reg0
        miAluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[0].Operand1  = MHW_MI_ALU_SRCA;
        miAluParams[0].Operand2  = MHW_MI_ALU_GPREG0;
        // load     srcB, reg4
        miAluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[1].Operand1  = MHW_MI_ALU_SRCB;
        miAluParams[1].Operand2  = MHW_MI_ALU_GPREG4;
        // add      srcA, srcB
        miAluParams[2].AluOpcode = MHW_MI_ALU_ADD;
        miAluParams[2].Operand1  = MHW_MI_ALU_SRCB;
        miAluParams[2].Operand2  = MHW_MI_ALU_GPREG4;
        // store      reg0, ZF
        miAluParams[3].AluOpcode    = MHW_MI_ALU_STORE;
        miAluParams[3].Operand1     = MHW_MI_ALU_GPREG0;
        miAluParams[3].Operand2     = MHW_MI_ALU_ZF;
        miMathParams.pAluPayload    = miAluParams;
        miMathParams.dwNumAluParams = 4;  // four ALU commands needed for this substract opertaion. see following ALU commands.
        DECODE_CHK_STATUS(m_miInterface->AddMiMathCmd(
            &cmdBuffer,
            &miMathParams));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer = &m_predication->m_predicationBuffer->OsResource;
        storeRegParams.dwOffset        = 0;
        storeRegParams.dwRegister      = mmioRegistersMfx->generalPurposeRegister0LoOffset;
        DECODE_CHK_STATUS(m_miInterface->AddMiStoreRegisterMemCmd(
            &cmdBuffer,
            &storeRegParams));

        condBBEndParams.presSemaphoreBuffer = &m_predication->m_predicationBuffer->OsResource;
        condBBEndParams.dwOffset            = 0;
        condBBEndParams.dwValue             = 0;
        condBBEndParams.bDisableCompareMask = true;
        DECODE_CHK_STATUS(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &condBBEndParams));
    }
    else
    {
        // Skip current frame if presPredication is equal to zero
        condBBEndParams.presSemaphoreBuffer = &m_predication->m_resPredication->OsResource;
        condBBEndParams.dwOffset            = (uint32_t)m_predication->m_predicationResOffset;
        condBBEndParams.bDisableCompareMask = true;
        condBBEndParams.dwValue             = 0;
        DECODE_CHK_STATUS(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &condBBEndParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePredicationPktG12::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    commandBufferSize      = 0;
    requestedPatchListSize = 0;
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
