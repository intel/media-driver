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
//! \file     codechal_decoder.cpp
//! \brief    Implements the decode interface for CodecHal.
//! \details  The decode interface is further sub-divided by standard, this file is for the base interface which is shared by all decode standards.
//!

#include "codechal_debug.h"

#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif

#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_config_manager.h"
#include "codechal_encoder_base.h"
#include <iomanip>

MOS_STATUS CodechalDebugInterface::Initialize(
    CodechalHwInterface *hwInterface,
    CODECHAL_FUNCTION    codecFunction,
    MediaCopyWrapper    *mediaCopyWrapper)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(hwInterface);
    m_hwInterface   = hwInterface;
    m_codecFunction = codecFunction;
    m_osInterface   = m_hwInterface->GetOsInterface();
    m_cpInterface   = m_hwInterface->GetCpInterface();
    m_miInterface   = m_hwInterface->GetMiInterface();

    CODECHAL_DEBUG_CHK_NULL(m_osInterface);
    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    CODECHAL_DEBUG_CHK_STATUS(InitializeUserSetting());

    //dump loctaion is codechaldump
    MediaDebugInterface::SetOutputFilePath();

    m_configMgr = MOS_New(CodecDebugConfigMgr, this, m_codecFunction, m_outputFilePath);
    CODECHAL_DEBUG_CHK_NULL(m_configMgr);
    CODECHAL_DEBUG_CHK_STATUS(m_configMgr->ParseConfig(m_osInterface->pOsContext));

    MediaDebugInterface::InitDumpLocation();

    if (m_hwInterface->GetPlatform().eProductFamily < IGFX_DG2)
    {
        m_dumpYUVSurface = m_dumpYUVSurfaceLegacy;
        m_dumpBuffer     = m_dumpBufferLegacy;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_ENABLE_HW_DEBUG_HOOKS_DEBUG,
            MediaUserSetting::Group::Device, 0, true);
        m_enableHwDebugHooks = outValue.Get<bool>();
    }
    CheckGoldenReferenceExist();
    if (m_enableHwDebugHooks && m_goldenReferenceExist)
    {
        LoadGoldenReference();
    }

    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_FRAME_NUMBER_TO_STOP_DEBUG,
            MediaUserSetting::Group::Device,
            -1,
            true);
        m_stopFrameNumber = outValue.Get<int32_t>();
    }

    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_SW_CRC_DEBUG,
            MediaUserSetting::Group::Device,
            0,
            true);
        m_swCRC = outValue.Get<bool>();
    }
#endif

    SetFastDumpConfig(mediaCopyWrapper);

    return MOS_STATUS_SUCCESS;
}

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

MOS_STATUS CodechalDebugInterface::DetectCorruptionHw(void *hwInterface, PMOS_RESOURCE frameCntRes, uint32_t curIdx, uint32_t frameCrcOffset, std::vector<MOS_RESOURCE> &vStatusBuffer, PMOS_COMMAND_BUFFER pCmdBuffer, uint32_t frameNum)
{
    if (m_enableHwDebugHooks &&
        m_goldenReferenceExist &&
        m_goldenReferences.size() > 0 &&
        vStatusBuffer.size() > 0)
    {
        for (uint32_t i = 0; i < vStatusBuffer.size(); i++)
        {
            MEDIA_DEBUG_CHK_STATUS(((CodechalHwInterface*)hwInterface)->SendHwSemaphoreWaitCmd(
                &vStatusBuffer[i],
                m_goldenReferences[curIdx][i],
                MHW_MI_SAD_EQUAL_SDD,
                pCmdBuffer,
                frameCrcOffset));
        }
        StoreNumFrame((MhwMiInterface*)m_miInterface, frameCntRes, frameNum, pCmdBuffer);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::StoreNumFrame(PMHW_MI_INTERFACE pMiInterface, PMOS_RESOURCE pResource, int32_t frameNum, PMOS_COMMAND_BUFFER pCmdBuffer)
{
    MHW_MI_STORE_DATA_PARAMS storeDataParams{};
    storeDataParams.pOsResource      = pResource;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue          = frameNum;
    MEDIA_DEBUG_CHK_STATUS(pMiInterface->AddMiStoreDataImmCmd(pCmdBuffer, &storeDataParams));
    return MOS_STATUS_SUCCESS;
}

#define FIELD_TO_OFS(field_name) ofs << print_shift << std::setfill(' ') << std::setw(25) << std::left << std::string(#field_name) + ": " << (int64_t)report->field_name << std::endl;
#define PTR_TO_OFS(ptr_name) ofs << print_shift << std::setfill(' ') << std::setw(25) << std::left << std::string(#ptr_name) + ": " << report->ptr_name << std::endl;
MOS_STATUS CodechalDebugInterface::DumpEncodeStatusReport(const EncodeStatusReport *report)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(report);

    const char *bufferName = "EncodeStatusReport_Parsed";
    const char *attrName   = MediaDbgAttr::attrStatusReport;
    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    const char *  filePath = CreateFileName(bufferName, attrName, MediaDbgExtType::txt);
    std::ofstream ofs(filePath);

    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }
    std::string print_shift = "";
    sizeof(report->CodecStatus);
    FIELD_TO_OFS(CodecStatus);
    FIELD_TO_OFS(StatusReportNumber);
    FIELD_TO_OFS(CurrOriginalPic.FrameIdx);
    FIELD_TO_OFS(CurrOriginalPic.PicFlags);
    FIELD_TO_OFS(CurrOriginalPic.PicEntry);
    FIELD_TO_OFS(Func);
    PTR_TO_OFS(  pCurrRefList);
    ofs << std::endl;

    FIELD_TO_OFS(bSequential);
    FIELD_TO_OFS(bitstreamSize);
    FIELD_TO_OFS(QpY);
    FIELD_TO_OFS(SuggestedQpYDelta);
    FIELD_TO_OFS(NumberPasses);
    FIELD_TO_OFS(AverageQp);
    FIELD_TO_OFS(HWCounterValue.IV);
    FIELD_TO_OFS(HWCounterValue.Count);
    PTR_TO_OFS(  hwctr);
    FIELD_TO_OFS(QueryStatusFlags);

    print_shift = "    ";
    FIELD_TO_OFS(PanicMode);
    FIELD_TO_OFS(SliceSizeOverflow);
    FIELD_TO_OFS(NumSlicesNonCompliant);
    FIELD_TO_OFS(LongTermReference);
    FIELD_TO_OFS(FrameSkipped);
    FIELD_TO_OFS(SceneChangeDetected);
    print_shift = "";
    ofs << std::endl;

    FIELD_TO_OFS(MAD);
    FIELD_TO_OFS(loopFilterLevel);
    FIELD_TO_OFS(LongTermIndication);
    FIELD_TO_OFS(NextFrameWidthMinus1);
    FIELD_TO_OFS(NextFrameHeightMinus1);
    FIELD_TO_OFS(NumberSlices);

    FIELD_TO_OFS(PSNRx100[0]);
    FIELD_TO_OFS(PSNRx100[1]);
    FIELD_TO_OFS(PSNRx100[2]);

    FIELD_TO_OFS(NumberTilesInFrame);
    FIELD_TO_OFS(UsedVdBoxNumber);
    FIELD_TO_OFS(SizeOfSliceSizesBuffer);
    PTR_TO_OFS(  pSliceSizes);
    FIELD_TO_OFS(SizeOfTileInfoBuffer);
    PTR_TO_OFS(  pHEVCTileinfo);
    FIELD_TO_OFS(NumTileReported);
    ofs << std::endl;

    FIELD_TO_OFS(StreamId);
    PTR_TO_OFS(  pLookaheadStatus);
    ofs.close();

    return MOS_STATUS_SUCCESS;
}
#undef FIELD_TO_OFS
#undef PTR_TO_OFS

#endif // USE_CODECHAL_DEBUG_TOOL


