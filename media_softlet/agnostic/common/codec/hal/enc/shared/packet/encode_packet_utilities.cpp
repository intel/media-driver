/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_packet_utilities.cpp
//! \brief    Defines encode packrt utilities.
//!

#include "encode_packet_utilities.h"
#include "codec_def_common_encode.h"

namespace encode {
    MOS_STATUS PacketUtilities::Init()
    {
#if USE_CODECHAL_DEBUG_TOOL
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "Fake Header Size Enable",
            MediaUserSetting::Group::Sequence);
        m_enableFakeHrdSize = outValue.Get<uint32_t>();

        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "Fake IFrame Header Size",
            MediaUserSetting::Group::Sequence);
        m_fakeIFrameHrdSize = outValue.Get<uint32_t>();

        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "Fake PBFrame Header Size",
            MediaUserSetting::Group::Sequence);
        m_fakePBFrameHrdSize = outValue.Get<uint32_t>();
#endif

        return MOS_STATUS_SUCCESS;
    }

    PacketUtilities::PacketUtilities(CodechalHwInterfaceNext *hwInterface, MediaFeatureManager *featureManager)
    {
        m_hwInterface       = hwInterface;
        m_featureManager    = featureManager;

        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        m_miItf          = hwInterface->GetMiInterfaceNext();
        auto osInterface = hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_NO_STATUS_RETURN(osInterface);
        m_userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);
        if (!m_userSettingPtr)
        {
            ENCODE_NORMALMESSAGE("Initialize m_userSettingPtr instance failed!");
        }
    }

    PacketUtilities::~PacketUtilities()
    {
    }

    MOS_STATUS PacketUtilities::AddMemCopyCmd(PMOS_COMMAND_BUFFER cmdBuf, PMOS_RESOURCE pDst, PMOS_RESOURCE pSrc, uint32_t size)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuf);
        ENCODE_CHK_NULL_RETURN(pDst);
        ENCODE_CHK_NULL_RETURN(pSrc);
        ENCODE_CHK_NULL_RETURN(m_miItf);

        auto &copyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        copyMemMemParams             = {};
        copyMemMemParams.presSrc     = pSrc;
        copyMemMemParams.presDst     = pDst;
        for (uint32_t i = 0; i < size; i = i + 4)
        {
            copyMemMemParams.dwSrcOffset = i;
            copyMemMemParams.dwDstOffset = i;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuf));
        }
        return eStatus;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS PacketUtilities::ModifyEncodedFrameSizeWithFakeHeaderSize(
        PMOS_COMMAND_BUFFER     cmdBuffer,
        uint32_t                fakeHeaderSizeInByte,
        PMOS_RESOURCE           resBrcUpdateCurbe,
        uint32_t                targetSizePos,
        PMOS_RESOURCE           resPakStat,
        uint32_t                slcHrdSizePos)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_featureManager);
        EncodeBasicFeature* basicFeature = dynamic_cast<EncodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(basicFeature);

        //calculate slice headers size
        PCODEC_ENCODER_SLCDATA slcData = basicFeature->m_slcData;
        ENCODE_CHK_NULL_RETURN(slcData);
        uint32_t totalSliceHeaderSize = 0;
        for (uint32_t slcCount = 0; slcCount < basicFeature->m_numSlices; slcCount++)
        {
            totalSliceHeaderSize += (slcData->BitSize + 7) >> 3;
            slcData++;
        }

        uint32_t firstHdrSz = 0;
        for (uint32_t i = 0; i < basicFeature->m_NumNalUnits; i++)
        {
            firstHdrSz += basicFeature->m_nalUnitParams[i]->uiSize;
        }

        totalSliceHeaderSize += firstHdrSz;

        ENCODE_CHK_STATUS_RETURN(AddBufferWithIMMValue(
            cmdBuffer,
            resBrcUpdateCurbe,
            targetSizePos,
            fakeHeaderSizeInByte - totalSliceHeaderSize,
            true));

        ENCODE_CHK_STATUS_RETURN(AddBufferWithIMMValue(
            cmdBuffer,
            resPakStat,
            slcHrdSizePos,
            fakeHeaderSizeInByte * 8,
            true));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS PacketUtilities::ModifyEncodedFrameSizeWithFakeHeaderSizeAVC(
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t            fakeHeaderSizeInByte,
        PMOS_RESOURCE       *resBrcUpdateCurbe,
        uint32_t            targetSizePos,
        PMOS_RESOURCE       resPakStat,
        uint32_t            slcHrdSizePos)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_featureManager);
        EncodeBasicFeature *basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(basicFeature);

        //calculate all frame headers size, including 1st slice header
        uint32_t totalHeaderSize = uint32_t(basicFeature->m_bsBuffer.pCurrent - basicFeature->m_bsBuffer.pBase);

        // change encoded frame size for next frame and next pass
        for (int i = 0; i < 2; i++)
        {
            if (resBrcUpdateCurbe[i] == nullptr)
                continue;
            ENCODE_CHK_STATUS_RETURN(AddBufferWithIMMValue(
                cmdBuffer,
                resBrcUpdateCurbe[i],
                sizeof(uint32_t) * 5,
                fakeHeaderSizeInByte - totalHeaderSize,
                true));
        }

        // change headers size (U16)
        ENCODE_CHK_STATUS_RETURN(SetBufferWithIMMValueU16(
            cmdBuffer,
            resPakStat,
            0,
            fakeHeaderSizeInByte * 8,
            0));  // second or first word in dword
            
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS PacketUtilities::AddBufferWithIMMValue(
        PMOS_COMMAND_BUFFER     cmdBuffer,
        PMOS_RESOURCE           presStoreBuffer,
        uint32_t                offset,
        uint32_t                value,
        bool                    bAdd)
    {
        MHW_MI_STORE_REGISTER_MEM_PARAMS    StoreRegParams;
        MHW_MI_STORE_DATA_PARAMS            StoreDataParams;
        MHW_MI_LOAD_REGISTER_REG_PARAMS     LoadRegRegParams;
        MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        if (m_vdboxIndex > m_hwInterface->GetMaxVdboxIndex())
        {
            ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        auto pMmioRegisters = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(m_vdboxIndex);

        auto &parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush       = {};
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer);

        MOS_ZeroMemory(&LoadRegRegParams, sizeof(LoadRegRegParams));

        auto &registerMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = presStoreBuffer;
        registerMemParams.dwOffset        = offset;
        registerMemParams.dwRegister      = pMmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        auto &loadRegImmParams      = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        loadRegImmParams            = {};
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister0HiOffset;
        loadRegImmParams.dwData     = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        loadRegImmParams            = {};
        loadRegImmParams.dwData     = value;
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        loadRegImmParams            = {};
        loadRegImmParams.dwData     = 0;
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {0};
        int                        aluCount     = 0;

        // load     srcA, reg0
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
        ++aluCount;
        // load     srcB, reg4
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
        ++aluCount;
        if (bAdd)
        {
            // add      srcA, srcB
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
        }
        else
        {
            // sub      srcA, srcB
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_SUB;
        }
        ++aluCount;
        // store      reg0, ACCU
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
        ++aluCount;

        auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams                = {};
        miMathParams.dwNumAluParams = aluCount;
        miMathParams.pAluPayload    = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));

        // update the value
        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = presStoreBuffer;
        miStoreRegMemParams.dwOffset        = offset;
        miStoreRegMemParams.dwRegister      = pMmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));
        return eStatus;
    }

    MOS_STATUS PacketUtilities::SetBufferWithIMMValueU16(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       presStoreBuffer,
        uint32_t            offset,
        uint32_t            value,
        bool                bSecond)
    {
        MHW_MI_STORE_REGISTER_MEM_PARAMS StoreRegParams;
        MHW_MI_LOAD_REGISTER_IMM_PARAMS  LoadRegisterImmParams;
        MHW_MI_MATH_PARAMS               MiMathParams;
        MHW_MI_ALU_PARAMS                MiAluParams[4];  // is used twice
        MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        if (m_vdboxIndex > m_hwInterface->GetMaxVdboxIndex())
        {
            ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        auto pMmioRegisters = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(m_vdboxIndex);

        auto &parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush       = {};
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer);

        auto &registerMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = presStoreBuffer;
        registerMemParams.dwOffset        = offset;
        registerMemParams.dwRegister      = pMmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        auto &loadRegImmParams      = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        loadRegImmParams            = {};
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister0HiOffset;
        loadRegImmParams.dwData     = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        uint32_t mask = bSecond ? 0xffff : 0xffff0000;
        value         = bSecond ? value << 16 : value;

        loadRegImmParams            = {};
        loadRegImmParams.dwData     = mask;
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        loadRegImmParams            = {};
        loadRegImmParams.dwData     = 0;
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {0};
        int                        aluCount     = 0;

        // load    srcA, reg0
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
        aluCount++;
        // load    srcB, reg4
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
        aluCount++;
        // and     srcA, srcB
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_AND;
        aluCount++;
        // store   reg0, ACCU
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
        aluCount++;

        auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams                = {};
        miMathParams.dwNumAluParams = aluCount;
        miMathParams.pAluPayload    = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));

        loadRegImmParams            = {};
        loadRegImmParams.dwData     = value;
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        loadRegImmParams            = {};
        loadRegImmParams.dwData     = 0;
        loadRegImmParams.dwRegister = pMmioRegisters->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        // load    srcA, reg0
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
        aluCount++;
        // load    srcB, reg4
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
        aluCount++;

        // or      srcA, srcB
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_OR;
        aluCount++;

        // store   reg0, ACCU
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
        aluCount++;

        miMathParams                = {};
        miMathParams                = {};
        miMathParams.dwNumAluParams = aluCount;
        miMathParams.pAluPayload    = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));

        // update the value
        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = presStoreBuffer;
        miStoreRegMemParams.dwOffset        = offset;
        miStoreRegMemParams.dwRegister      = pMmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        return eStatus;
    }

    bool PacketUtilities::GetFakeHeaderSettings(uint32_t &fakeHeaderSizeInByte, bool isIframe)
    {
        ENCODE_FUNC_CALL();

        fakeHeaderSizeInByte = isIframe ? m_fakeIFrameHrdSize : m_fakePBFrameHrdSize;

        return m_enableFakeHrdSize;
    }
#endif
}
