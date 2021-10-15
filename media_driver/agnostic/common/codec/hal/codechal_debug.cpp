/*
* Copyright (c) 2011-2021, Intel Corporation
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
//! \file     codechal_debug.cpp
//! \brief    Defines the debug interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_config_manager.h"
#include "codechal_encoder_base.h"
#include <iomanip>

CodechalDebugInterface::CodechalDebugInterface()
{
    memset(&m_currPic, 0, sizeof(CODEC_PICTURE));
    memset(m_fileName, 0, sizeof(m_fileName));
    memset(m_path, 0, sizeof(m_path));
}

CodechalDebugInterface::~CodechalDebugInterface()
{
    if (nullptr != m_configMgr)
    {
        MOS_Delete(m_configMgr);
    }
}

void CodechalDebugInterface::CheckGoldenReferenceExist()
{
    std::ifstream crcGoldenRefStream(m_crcGoldenRefFileName);
    m_goldenReferenceExist = crcGoldenRefStream.good() ? true : false;
}

MOS_STATUS CodechalDebugInterface::Initialize(
    CodechalHwInterface *hwInterface,
    CODECHAL_FUNCTION    codecFunction)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(hwInterface);
    m_hwInterface   = hwInterface;
    m_codecFunction = codecFunction;
    m_osInterface   = m_hwInterface->GetOsInterface();
    m_cpInterface   = m_hwInterface->GetCpInterface();
    m_miInterface   = m_hwInterface->GetMiInterface();

    //dump loctaion is codechaldump
    MediaDebugInterface::SetOutputFilePath();

    m_configMgr = MOS_New(CodecDebugConfigMgr, this, m_codecFunction, m_outputFilePath);
    CODECHAL_DEBUG_CHK_NULL(m_configMgr);
    CODECHAL_DEBUG_CHK_STATUS(m_configMgr->ParseConfig(m_osInterface->pOsContext));

    MediaDebugInterface::InitDumpLocation();

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = 0;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_ENABLE_HW_DEBUG_HOOKS_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_enableHwDebugHooks = userFeatureData.u32Data ? true : false;
    CheckGoldenReferenceExist();
    if (m_enableHwDebugHooks && m_goldenReferenceExist)
    {
        LoadGoldenReference();
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = -1;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_FRAME_NUMBER_TO_STOP_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_stopFrameNumber = userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = 0;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_SW_CRC_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_swCRC = userFeatureData.i32Data == 0 ? false : true;
#endif
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpHucDmem(
    PMOS_RESOURCE             dmemResource,
    uint32_t                  dmemSize,
    uint32_t                  hucPassNum,
    CodechalHucRegionDumpType dumpType)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrHuCDmem))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(dmemResource);
    if (Mos_ResourceIsNull(dmemResource))
    {
        return MOS_STATUS_NULL_POINTER;
    }

    std::string funcName = "";
    if (m_codecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        funcName = "DEC_Cenc_";
    }
    else
    {
        funcName = "ENC_";
    }

    std::string dmemName = MediaDbgBufferType::bufHucDmem;
    std::string passName = std::to_string(hucPassNum);
    switch (dumpType)
    {
    case hucRegionDumpInit:
        funcName = funcName + dmemName + "_InitPass" + passName;
        break;
    case hucRegionDumpUpdate:
        funcName = funcName + dmemName + "_UpdatePass" + passName;
        break;
    case hucRegionDumpRegionLocked:
        funcName = funcName + dmemName + "_RegionLocked" + passName;
        break;
    case hucRegionDumpCmdInitializer:
        funcName = funcName + dmemName + "_CmdInitializerPass" + passName;
        break;
    case hucRegionDumpPakIntegrate:
        funcName = funcName + dmemName + "_PakIntPass" + passName;
        break;
    case hucRegionDumpHpu:
        funcName = funcName + dmemName + "_HpuPass" + passName;
        break;
    case hucRegionDumpHpuSuperFrame:
        funcName = funcName + dmemName + "_HpuPass" + passName + "_SuperFramePass";
        break;
    case hucRegionDumpBackAnnotation:
        funcName = funcName + dmemName + "_BackAnnotationPass" + passName;
        break;
    default:
        funcName = funcName + dmemName + "_Pass" + passName;
        break;
    }

    return DumpBuffer(dmemResource, nullptr, funcName.c_str(), dmemSize);
}

MOS_USER_FEATURE_VALUE_ID CodechalDebugInterface::SetOutputPathKey()
{
    return __MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_OUTPUT_DIRECTORY_ID;
}

MOS_USER_FEATURE_VALUE_ID CodechalDebugInterface::InitDefaultOutput()
{
    m_outputFilePath.append(MEDIA_DEBUG_CODECHAL_DUMP_OUTPUT_FOLDER);
    return SetOutputPathKey();
}

MOS_STATUS CodechalDebugInterface::DetectCorruptionSw(std::vector<MOS_RESOURCE> &vResource, PMOS_RESOURCE frameCntRes, uint8_t *buf, uint32_t &size, uint32_t frameNum)
{
    if (m_enableHwDebugHooks &&
        m_goldenReferenceExist &&
        m_goldenReferences.size() > 0 &&
        vResource.size() > 0)
    {
        MOS_COMMAND_BUFFER cmdBuffer = {};
        std::vector<uint32_t *> vSemaData;
        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
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
        StoreNumFrame(m_miInterface, frameCntRes, frameNum, &cmdBuffer);

        SubmitDummyWorkload(&cmdBuffer, false);
        //Get Decode output
        std::vector<uint32_t> data = {CalculateCRC(buf, size)};
        CODECHAL_DEBUG_CHK_STATUS(FillSemaResource(vSemaData, data));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpHucRegion(
    PMOS_RESOURCE             region,
    uint32_t                  regionOffset,
    uint32_t                  regionSize,
    uint32_t                  regionNum,
    const char *              regionName,
    bool                      inputBuffer,
    uint32_t                  hucPassNum,
    CodechalHucRegionDumpType dumpType)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrHucRegions))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_ASSERT(regionNum < 16);
    CODECHAL_DEBUG_CHK_NULL(region);

    if (Mos_ResourceIsNull(region))
    {
        return MOS_STATUS_NULL_POINTER;
    }

    std::string funcName = "";
    if (m_codecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        funcName = "DEC_CENC_";
    }
    else
    {
        funcName = "ENC_";
    }

    std::string bufName       = MediaDbgBufferType::bufHucRegion;
    std::string inputName     = (inputBuffer) ? "Input_" : "Output_";
    std::string regionNumName = std::to_string(regionNum);
    std::string passName      = std::to_string(hucPassNum);
    switch (dumpType)
    {
    case hucRegionDumpInit:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_InitPass" + passName;
        break;
    case hucRegionDumpUpdate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_UpdatePass" + passName;
        break;
    case hucRegionDumpRegionLocked:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_RegionLockedPass" + passName;
        break;
    case hucRegionDumpCmdInitializer:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_CmdInitializerPass" + passName;
        break;
    case hucRegionDumpPakIntegrate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_PakIntPass" + passName;
        break;
    case hucRegionDumpHpu:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_HpuPass" + passName;
        break;
    case hucRegionDumpBackAnnotation:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_BackAnnotationPass" + passName;
        break;
    default:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_Pass" + passName;
        break;
    }

    return DumpBuffer(region, nullptr, funcName.c_str(), regionSize, regionOffset);
}

MOS_STATUS CodechalDebugInterface::DumpEncodeStatusReport(void* report)
{
    CODECHAL_DEBUG_ASSERTMESSAGE("WARNING: DumpEncodeStatusReport(void* report) is used. Make sure that pointer contains ::EncodeStatusReport or similar struct");
    return DumpEncodeStatusReport((EncodeStatusReport*)report);
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
