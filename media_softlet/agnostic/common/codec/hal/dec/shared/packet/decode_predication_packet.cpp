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
#include "decode_predication_packet.h"
#include "decode_common_feature_defs.h"

namespace decode
{

DecodePredicationPkt::DecodePredicationPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : DecodeSubPacket(pipeline, hwInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS DecodePredicationPkt::Init()
{
    DECODE_CHK_NULL(m_pipeline);
    DECODE_CHK_NULL(m_hwInterface);

    m_miItf = m_hwInterface->GetMiInterfaceNext();
    DECODE_CHK_NULL(m_miItf);

    MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);

    m_predication = dynamic_cast<DecodePredication*>(
                    featureManager->GetFeature(DecodeFeatureIDs::decodePredication));
    DECODE_CHK_NULL(m_predication);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePredicationPkt::Prepare()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePredicationPkt::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
{
    if (!m_predication->m_predicationEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS  condBBEndParams;
    MOS_ZeroMemory(&condBBEndParams, sizeof(condBBEndParams));

    // Skip current frame if presPredication is not equal to zero
    if (m_predication->m_predicationNotEqualZero)
    {
        auto mmioRegistersMfx = m_hwInterface->SelectVdAndGetMmioReg(MHW_VDBOX_NODE_1, &cmdBuffer);
        auto &flushDwParams    = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams          = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        // load presPredication to general purpose register0
        auto &miLoadRegMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = &m_predication->m_resPredication->OsResource;
        miLoadRegMemParams.dwOffset        = (uint32_t)m_predication->m_predicationResOffset;
        miLoadRegMemParams.dwRegister      = mmioRegistersMfx->generalPurposeRegister0LoOffset;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(&cmdBuffer));

        auto &miLoadRegImmParams      = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister0HiOffset;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(&cmdBuffer));

        // load 0 to general purpose register4
        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister4LoOffset;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(&cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister4HiOffset;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(&cmdBuffer));

        //perform the add operation
        mhw::mi::MHW_MI_ALU_PARAMS miAluParams[4] = {};
        // load     srcA, reg0
        miAluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[0].Operand1 = MHW_MI_ALU_SRCA;
        miAluParams[0].Operand2 = MHW_MI_ALU_GPREG0;
        // load     srcB, reg4
        miAluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[1].Operand1 = MHW_MI_ALU_SRCB;
        miAluParams[1].Operand2 = MHW_MI_ALU_GPREG4;
        // add      srcA, srcB
        miAluParams[2].AluOpcode = MHW_MI_ALU_ADD;
        miAluParams[2].Operand1 = MHW_MI_ALU_SRCB;
        miAluParams[2].Operand2 = MHW_MI_ALU_GPREG4;
        // store      reg0, ZF
        miAluParams[3].AluOpcode = MHW_MI_ALU_STORE;
        miAluParams[3].Operand1 = MHW_MI_ALU_GPREG0;
        miAluParams[3].Operand2 = MHW_MI_ALU_ZF;

        auto &miMathParams = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams       = {};
        miMathParams.pAluPayload = miAluParams;
        miMathParams.dwNumAluParams = 4;  // four ALU commands needed for this substract opertaion. see following ALU commands.
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_MATH)(&cmdBuffer));


        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = &m_predication->m_predicationBuffer->OsResource;
        miStoreRegMemParams.dwOffset        = 0;
        miStoreRegMemParams.dwRegister      = mmioRegistersMfx->generalPurposeRegister0LoOffset;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        auto &miConditionalBatchBufferEndParams               = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams                     = {};
        miConditionalBatchBufferEndParams.dwOffset            = 0;
        miConditionalBatchBufferEndParams.dwValue             = 0;
        miConditionalBatchBufferEndParams.bDisableCompareMask = true;
        miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_predication->m_predicationBuffer->OsResource;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));
    }
    else
    {
        // Skip current frame if presPredication is equal to zero
        auto &miConditionalBatchBufferEndParams               = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams                     = {};
        miConditionalBatchBufferEndParams.dwOffset            = (uint32_t)m_predication->m_predicationResOffset;
        miConditionalBatchBufferEndParams.dwValue             = 0;
        miConditionalBatchBufferEndParams.bDisableCompareMask = true;
        miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_predication->m_resPredication->OsResource;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePredicationPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    commandBufferSize = 0;
    requestedPatchListSize = 0;
    return MOS_STATUS_SUCCESS;
}

}
