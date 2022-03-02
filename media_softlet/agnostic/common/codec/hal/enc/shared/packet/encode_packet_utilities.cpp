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
namespace encode {
    MOS_STATUS PacketUtilities::Init()
    {
#if USE_CODECHAL_DEBUG_TOOL
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_FAKE_HEADER_SIZE_ID,
            &userFeatureData,
            m_hwInterface->GetOsInterface()->pOsContext);
        m_enableFakeHrdSize = (uint32_t)userFeatureData.u32Data;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_FAKE_IFRAME_HEADER_SIZE_ID,
            &userFeatureData,
            m_hwInterface->GetOsInterface()->pOsContext);
        m_fakeIFrameHrdSize = (uint32_t)userFeatureData.u32Data;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_FAKE_PBFRAME_HEADER_SIZE_ID,
            &userFeatureData,
            m_hwInterface->GetOsInterface()->pOsContext);
        m_fakePBFrameHrdSize = (uint32_t)userFeatureData.u32Data;
#endif

        return MOS_STATUS_SUCCESS;
    }

    PacketUtilities::PacketUtilities(CodechalHwInterface *hwInterface, MediaFeatureManager *featureManager)
    {
        m_hwInterface       = hwInterface;
        m_featureManager    = featureManager;

        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        m_miInterface       = hwInterface->GetMiInterface();
    }

    PacketUtilities::~PacketUtilities()
    {
        m_miInterface     = nullptr;
    }

    MOS_STATUS PacketUtilities::AddMemCopyCmd(PMOS_COMMAND_BUFFER cmdBuf, PMOS_RESOURCE pDst, PMOS_RESOURCE pSrc, uint32_t size)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuf);
        ENCODE_CHK_NULL_RETURN(pDst);
        ENCODE_CHK_NULL_RETURN(pSrc);
        ENCODE_CHK_NULL_RETURN(m_miInterface);
        MHW_MI_COPY_MEM_MEM_PARAMS cpyParams;
        cpyParams.presSrc = pSrc;
        cpyParams.presDst = pDst;
        for (uint32_t i = 0; i < size; i = i + 4)
        {
            cpyParams.dwSrcOffset = i;
            cpyParams.dwDstOffset = i;
            m_miInterface->AddMiCopyMemMemCmd(cmdBuf, &cpyParams);
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
        MHW_MI_LOAD_REGISTER_IMM_PARAMS     LoadRegisterImmParams;
        MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
        MHW_MI_MATH_PARAMS                  MiMathParams;
        MHW_MI_ALU_PARAMS                   MiAluParams[4];
        MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMfxInterface());

        if (m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex())
        {
            ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        auto pMmioRegistersMfx = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

        MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
        ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &FlushDwParams));

        MOS_ZeroMemory(&LoadRegRegParams, sizeof(LoadRegRegParams));

        MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
        MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));

        miLoadRegMemParams.presStoreBuffer = presStoreBuffer;
        miLoadRegMemParams.dwOffset = offset;
        miLoadRegMemParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));

        MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
        LoadRegisterImmParams.dwData = 0;
        LoadRegisterImmParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister0HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
            cmdBuffer,
            &LoadRegisterImmParams));

        MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
        LoadRegisterImmParams.dwData = value;
        LoadRegisterImmParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
            cmdBuffer,
            &LoadRegisterImmParams));
        MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
        LoadRegisterImmParams.dwData = 0;
        LoadRegisterImmParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
            cmdBuffer,
            &LoadRegisterImmParams));

        MOS_ZeroMemory(&MiMathParams, sizeof(MiMathParams));
        MOS_ZeroMemory(&MiAluParams, sizeof(MiAluParams));
        // load     srcA, reg0
        MiAluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
        MiAluParams[0].Operand1 = MHW_MI_ALU_SRCA;
        MiAluParams[0].Operand2 = MHW_MI_ALU_GPREG0;
        // load     srcB, reg4
        MiAluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
        MiAluParams[1].Operand1 = MHW_MI_ALU_SRCB;
        MiAluParams[1].Operand2 = MHW_MI_ALU_GPREG4;

        if (bAdd)
        {
            // add      srcA, srcB
            MiAluParams[2].AluOpcode = MHW_MI_ALU_ADD;
        }
        else
        {
            // sub      srcA, srcB
            MiAluParams[2].AluOpcode = MHW_MI_ALU_SUB;
        }

        // store      reg0, ACCU
        MiAluParams[3].AluOpcode = MHW_MI_ALU_STORE;
        MiAluParams[3].Operand1 = MHW_MI_ALU_GPREG0;
        MiAluParams[3].Operand2 = MHW_MI_ALU_ACCU;

        MiMathParams.pAluPayload = MiAluParams;
        MiMathParams.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
        ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiMathCmd(
            cmdBuffer,
            &MiMathParams));

        // update the value
        MOS_ZeroMemory(&StoreRegParams, sizeof(StoreRegParams));
        StoreRegParams.presStoreBuffer = presStoreBuffer;
        StoreRegParams.dwOffset = offset;
        StoreRegParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &StoreRegParams));

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