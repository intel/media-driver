/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_command_buffer_os.cpp
//! \brief     Contains Class CmCommandBuffer  definitions
//!

#include "cm_command_buffer.h"
#include "cm_ish.h"
#include "cm_ssh.h"
#include "cm_media_state.h"
#include "cm_thread_space_rt.h"
#include "cm_mem.h"
#include "cm_kernel_ex.h"
#include "cm_group_space.h"

#include "mhw_render_g12_X.h"
#include "mhw_render_g11_X.h"
#include "mhw_mi_g12_X.h"

#include "mos_solo_generic.h"
#include "mhw_mmio_g9.h"

#include "cm_hal_g12.h"

MOS_STATUS CmCommandBuffer::AddFrameTracker(MOS_RESOURCE *resource, uint32_t offset, uint32_t tag)
{
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = resource;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue = tag;
    return m_miInterface->AddMiStoreDataImmCmd(&m_cmdBuf, &storeDataParams);
}

MOS_STATUS CmCommandBuffer::AddConditionalFrameTracker(MOS_RESOURCE *resource, uint32_t offset, uint32_t tag, CM_HAL_CONDITIONAL_BB_END_INFO *cbbInfo)
{
    MHW_MI_LOAD_REGISTER_REG_PARAMS loadRegRegParams;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegImmParams;
    MHW_MI_LOAD_REGISTER_MEM_PARAMS loadRegMemParams;

    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MHW_MI_MATH_PARAMS mathParams;
    MHW_MI_ALU_PARAMS aluParams[20];
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegMemParams;
    MHW_PIPE_CONTROL_PARAMS pipeCtrlParams;

    MOS_ZeroMemory(&mathParams, sizeof(mathParams));
    MOS_ZeroMemory(&aluParams, sizeof(aluParams));

    int aluCount = 0;

    aluParams[aluCount].AluOpcode = MHW_MI_ALU_AND;
    ++ aluCount;

    // store    reg1, CF
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG1;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_CF;
    ++ aluCount;
    // store    reg2, CF
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG2;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_CF;
    ++ aluCount;
    // store    reg3, CF
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG3;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_CF;
    ++ aluCount;

    mathParams.pAluPayload = aluParams;
    mathParams.dwNumAluParams = aluCount;
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiMathCmd(&m_cmdBuf, &mathParams));

    // miload   reg1, *(cbb_buffer + offset)
    MOS_ZeroMemory(&loadRegMemParams, sizeof(loadRegMemParams));
    loadRegMemParams.presStoreBuffer = &(m_cmhal->bufferTable[cbbInfo->bufferTableIndex].osResource);
    loadRegMemParams.dwOffset = cbbInfo->offset;
    loadRegMemParams.dwRegister = CS_GPR_REGISTER_INDEX(1);
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(&m_cmdBuf, &loadRegMemParams));

    // miload   reg8, *(tracker_resource + tracker_offset)
    MOS_ZeroMemory(&loadRegMemParams, sizeof(loadRegMemParams));
    loadRegMemParams.presStoreBuffer = resource;
    loadRegMemParams.dwOffset = offset;
    loadRegMemParams.dwRegister = CS_GPR_REGISTER_INDEX(8);
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(&m_cmdBuf, &loadRegMemParams));

    // miload   reg9, tag
    MOS_ZeroMemory(&loadRegImmParams, sizeof(loadRegImmParams));
    loadRegImmParams.dwData = tag;
    loadRegImmParams.dwRegister = CS_GPR_REGISTER_INDEX(9);
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&m_cmdBuf, &loadRegImmParams));

    if (!cbbInfo->disableCompareMask)
    {
        // miload   reg2, mask
        loadRegMemParams.presStoreBuffer = &(m_cmhal->bufferTable[cbbInfo->bufferTableIndex].osResource);
        loadRegMemParams.dwOffset = cbbInfo->offset + 4; // in mask mode, the mask stored in the next DW to the value
        loadRegMemParams.dwRegister = CS_GPR_REGISTER_INDEX(2);

        aluCount = 0;

        //load1 srca, reg1
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG1;
        ++ aluCount;
        //load srcb, reg2
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG2;
        ++ aluCount;
        //add
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_AND;
        ++ aluCount;
        //store reg1, accu
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG1;
        aluParams[aluCount].Operand2 = MHW_MI_ALU_ACCU;
        ++ aluCount;

        mathParams.pAluPayload = aluParams;
        mathParams.dwNumAluParams = aluCount;;
        CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiMathCmd(&m_cmdBuf, &mathParams));
    }

    // miload   reg2, cbb->value
    MOS_ZeroMemory(&loadRegImmParams, sizeof(loadRegImmParams));
    loadRegImmParams.dwData = cbbInfo->compareValue;
    loadRegImmParams.dwRegister = CS_GPR_REGISTER_INDEX(2);
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&m_cmdBuf, &loadRegImmParams));    //R2: user value 32bits

    // miload   reg3, 1
    MOS_ZeroMemory(&loadRegImmParams, sizeof(loadRegImmParams));
    loadRegImmParams.dwData = 1;
    loadRegImmParams.dwRegister = CS_GPR_REGISTER_INDEX(3);
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&m_cmdBuf, &loadRegImmParams));    //R3 = 1

    aluCount = 0;

    // load     srcB, reg1
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG1;
    ++ aluCount;
    // load     srcA, reg2
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG2;
    ++ aluCount;
    // sub      srcA, srcB
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_SUB;
    ++ aluCount;
    // storeinv reg4, CF --- if (compared > cbb->value) reg4 = -1; else reg4 = -2
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STOREINV;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG4;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_CF;
    ++ aluCount;
    mathParams.pAluPayload = aluParams;
    mathParams.dwNumAluParams = aluCount;
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiMathCmd(&m_cmdBuf, &mathParams));

    aluCount = 0;

    // load     srcA, reg3
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG3;
    ++ aluCount;
    // load     srcB, reg4
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG4;
    ++ aluCount;
    // add
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
    ++ aluCount;
    // store    reg10, accu --- if (compared > cbb->value) reg10 = 0; else reg10 = -1
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG10;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_ACCU;
    ++ aluCount;
    // load     srcA, reg8
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG8;
    ++ aluCount;
    // load     srcB, reg10
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG10;
    ++ aluCount;
    // and
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_AND;
    ++ aluCount;
    // store    reg11, accu --- if (compared > cbb->value) reg11 = 0; else reg11 = cur_tracker
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG11;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_ACCU;
    ++ aluCount;
    // storeinv reg12, reg10 --- if (compared > cbb->value) reg12 = -1; else reg12 = 0
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STOREINV;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG12;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG10;
    ++ aluCount;
    // load     srcA, reg9
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG9;
    ++ aluCount;
    // load     srcB, reg12
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG12;
    ++ aluCount;
    // and
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_AND;
    ++ aluCount;
    // store    reg13, accu --- if (compared > cbb->value) reg13 = tracker; else reg13 = 0
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG13;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_ACCU;
    ++ aluCount;
    // load     srcA, reg11
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG11;
    ++ aluCount;
    // load     srcB, reg13
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_GPREG13;
    ++ aluCount;
    // add
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
    ++ aluCount;
    // store    reg15, accu --- if (compared > cbb->value) reg15 = tracker; else reg15 = cur_tracker
    aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[aluCount].Operand1 = MHW_MI_ALU_GPREG15;
    aluParams[aluCount].Operand2 = MHW_MI_ALU_ACCU;
    ++ aluCount;

    mathParams.pAluPayload = aluParams;
    mathParams.dwNumAluParams = aluCount;
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiMathCmd(&m_cmdBuf, &mathParams));

    // Store reg13 to trackerResource
    MOS_ZeroMemory(&storeRegMemParams, sizeof(storeRegMemParams));
    storeRegMemParams.presStoreBuffer = resource;
    storeRegMemParams.dwOffset = offset;
    storeRegMemParams.dwRegister = CS_GPR_REGISTER_INDEX(15);
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&m_cmdBuf, &storeRegMemParams));

    // Insert a pipe control for synchronization
    MOS_ZeroMemory(&pipeCtrlParams, sizeof(pipeCtrlParams));
    pipeCtrlParams.bFlushRenderTargetCache = true;
    pipeCtrlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
    pipeCtrlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddPipeControl(&m_cmdBuf, nullptr, &pipeCtrlParams));

    pipeCtrlParams.dwFlushMode = MHW_FLUSH_READ_CACHE;
    CM_CHK_MOSSTATUS_RETURN(m_miInterface->AddPipeControl(&m_cmdBuf, nullptr, &pipeCtrlParams));

    return MOS_STATUS_SUCCESS;
}
