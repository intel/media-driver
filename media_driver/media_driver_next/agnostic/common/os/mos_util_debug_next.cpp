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
//! \file     mos_util_debug_next.cpp
//! \brief    Common OS debug across different platform
//! \details  Common OS debug across different platform
//!

#include "mos_util_debug_next.h"

#if MOS_MESSAGES_ENABLED
#include "mos_utilities_next.h"

//!
//! \brief HLT file name template
//!
const char * const MosUtilDebug::m_mosLogPathTemplate = MOS_LOG_PATH_TEMPLATE;

//!
//! \brief DDI dump file name template
//!
const char * const MosUtilDebug::m_DdiLogPathTemplate = "%s\\ddi_dump_%d.%s";

const char * const MosUtilDebug::m_mosLogLevelName[MOS_MESSAGE_LVL_COUNT] = {
    "",          // DISABLED
    "CRITICAL",
    "NORMAL  ",
    "VERBOSE ",
    "ENTER   ",
    "EXIT    ",
    "ENTER   ",  // ENTER VERBOSE
    "EXIT    ",  // EXIT VERBOSE
};

const char * const MosUtilDebug::m_mosComponentName[MOS_COMPONENT_COUNT] = {
    "[MOS]:  ",
    "[MHW]:  ",
    "[CODEC]:",
    "[VP]:   ",
    "[CP]:   ",
    MOS_COMPONENT_NAME_DDI_STRING,
    "[CM]:   "
};

MOS_MESSAGE_PARAMS MosUtilDebug::m_mosMsgParams = {};
MOS_MESSAGE_PARAMS MosUtilDebug::m_mosMsgParamsDdiDump = {};

const MOS_USER_FEATURE_VALUE_ID MosUtilDebug::m_pcComponentUserFeatureKeys[MOS_COMPONENT_COUNT][3] = {
    {
    __MOS_USER_FEATURE_KEY_MESSAGE_OS_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_OS_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_OS_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_HW_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_HW_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_HW_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_CODEC_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CODEC_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CODEC_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_VP_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_VP_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_VP_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_DDI_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_DDI_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_DDI_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_CM_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CM_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CM_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_SCALABILITY_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_SCALABILITY_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_SCALABILITY_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_MMC_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MMC_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MMC_TAG_ID
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_BLT_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_BLT_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_BLT_TAG_ID
    }
};

const uint8_t MosUtilDebug::m_subComponentCount[MOS_COMPONENT_COUNT] = {
    MOS_SUBCOMP_COUNT,
    MOS_HW_SUBCOMP_COUNT,
    MOS_CODEC_SUBCOMP_COUNT,
    MOS_VP_SUBCOMP_COUNT,
    MOS_CP_SUBCOMP_COUNT,
    MOS_DDI_SUBCOMP_COUNT,
    MOS_CM_SUBCOMP_COUNT
};

void MosUtilDebug::MosSetSubCompMessageLevel(MOS_COMPONENT_ID compID, uint8_t subCompID, MOS_MESSAGE_LEVEL msgLevel)
{
    if (compID >= MOS_COMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid component %d.", compID);
        return;
    }

    if (subCompID >= MOS_MAX_SUBCOMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid sub-component %d.", subCompID);
        return;
    }

    if (msgLevel >= MOS_MESSAGE_LVL_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid msg level %d.", msgLevel);
        return;
    }

    m_mosMsgParams.components[compID].subComponents[subCompID].uiMessageLevel = msgLevel;
}

void MosUtilDebug::MosSetCompMessageLevel(MOS_COMPONENT_ID compID, MOS_MESSAGE_LEVEL msgLevel)
{
    if (compID >= MOS_COMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid component %d.", compID);
        return;
    }

    if (msgLevel >= MOS_MESSAGE_LVL_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid msg level %d.", msgLevel);
        return;
    }
    m_mosMsgParams.components[compID].component.uiMessageLevel = msgLevel;
}

void MosUtilDebug::MosSetCompMessageLevelAll(MOS_MESSAGE_LEVEL msgLevel)
{
    if (msgLevel >= MOS_MESSAGE_LVL_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid msg level %d.", msgLevel);
        return;
    }

    uint32_t i = 0;

    for(i = 0; i < MOS_COMPONENT_COUNT; i++)
    {
        MosSetCompMessageLevel((MOS_COMPONENT_ID)i, msgLevel);
    }
}

void MosUtilDebug::MosSubCompAssertEnableDisable(MOS_COMPONENT_ID compID, uint8_t subCompID, int32_t bEnable)
{
    if (compID >= MOS_COMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid component %d.", compID);
        return;
    }

    if (subCompID >= MOS_MAX_SUBCOMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid sub-component %d.", subCompID);
        return;
    }

    m_mosMsgParams.components[compID].subComponents[subCompID].bAssertEnabled = bEnable;
}

void MosUtilDebug::MosCompAssertEnableDisable(MOS_COMPONENT_ID compID, int32_t bEnable)
{
    if (compID >= MOS_COMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid component %d.", compID);
        return;
    }

    m_mosMsgParams.components[compID].component.bAssertEnabled = bEnable;
}

void MosUtilDebug::MosMessageInitComponent(MOS_COMPONENT_ID compID)
{
    uint32_t                                    uiCompUserFeatureSetting = 0;
    uint64_t                                    uiSubCompUserFeatureSetting = 0;
    uint8_t                                     i = 0;
    MOS_USER_FEATURE_VALUE_ID                   MessageKey = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_USER_FEATURE_VALUE_ID                   BySubComponentsKey = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_USER_FEATURE_VALUE_ID                   SubComponentsKey = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_USER_FEATURE_VALUE_DATA                 UserFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA           UserFeatureWriteData;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    if (compID >= MOS_COMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid component %d.", compID);
        return;
    }

    MessageKey         = m_pcComponentUserFeatureKeys[compID][0];
    BySubComponentsKey = m_pcComponentUserFeatureKeys[compID][1];
    SubComponentsKey   = m_pcComponentUserFeatureKeys[compID][2];

    MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MosUtilities::MosUserFeatureReadValueID(
                  nullptr,
                  MessageKey,
                  &UserFeatureData);

    // If the user feature key was not found, create it with the default value.
    if (eStatus  != MOS_STATUS_SUCCESS)
    {
        MosUtilities::MosZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
        UserFeatureWriteData.Value.u32Data = UserFeatureData.u32Data;
        UserFeatureWriteData.ValueID = MessageKey;
        MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData,1);
    }

    uiCompUserFeatureSetting = UserFeatureData.u32Data;

    // Extract the 3-bit message level and 1-bit assert flag setting for this component.
    MosSetCompMessageLevel(compID, (MOS_MESSAGE_LEVEL) (uiCompUserFeatureSetting & 0x7));
    MosCompAssertEnableDisable(compID, (uiCompUserFeatureSetting >> 3) & 0x1);

    // Check if sub-components should be set seperately.
    MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        BySubComponentsKey,
        &UserFeatureData);
    // If the user feature key was not found, create it with default (0) value.
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MosUtilities::MosZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
        UserFeatureWriteData.Value.u32Data = UserFeatureData.u32Data;
        UserFeatureWriteData.ValueID = BySubComponentsKey;
        MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData,1);
    }

    m_mosMsgParams.components[compID].bBySubComponent = UserFeatureData.u32Data;

    // Set sub components:
    if (m_mosMsgParams.components[compID].bBySubComponent)
    {
        MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        eStatus = MosUtilities::MosUserFeatureReadValueID(
            nullptr,
            SubComponentsKey,
            &UserFeatureData);
        // If the user feature key was not found, create it with default (0) value.
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MosUtilities::MosZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
            UserFeatureWriteData.Value.u64Data = UserFeatureData.u64Data;
            UserFeatureWriteData.ValueID = SubComponentsKey;
            MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData, 1);
        }

        uiSubCompUserFeatureSetting = UserFeatureData.u64Data;

        for(i = 0; i < m_subComponentCount[compID]; i++)
        {
            // Extract the 3-bit message level and 1-bit assert flag setting for each sub-comp
            // from the user feature key and populate to the MOS message params structure
            MosSetSubCompMessageLevel(compID, i, (MOS_MESSAGE_LEVEL)(uiSubCompUserFeatureSetting & 0x7));
            MosSubCompAssertEnableDisable(compID, i, (uiSubCompUserFeatureSetting >> 3) & 0x1);

            uiSubCompUserFeatureSetting = (uiSubCompUserFeatureSetting >> 4);
        }
    }
}

MOS_STATUS MosUtilDebug::MosHLTInit()
{
    uint32_t                                    nPID = 0;
    char                                        hltFileName[MOS_MAX_HLT_FILENAME_LEN] = {0};
    MOS_USER_FEATURE_VALUE                      UserFeatureValue = __NULL_USER_FEATURE_VALUE__;
    char                                        fileNamePrefix[MOS_MAX_HLT_FILENAME_LEN];
    int32_t                                     bUseHybridLogTrace = false;
    MOS_USER_FEATURE_VALUE_DATA                 UserFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA           UserFeatureWriteData;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    if (m_mosMsgParams.uiCounter != 0)
    {
        MOS_OS_NORMALMESSAGE("HLT settings already set.");
        return MOS_STATUS_UNKNOWN;
    }

    m_mosMsgParams.bUseHybridLogTrace = false;
    m_mosMsgParams.pLogFile           = nullptr;
    m_mosMsgParams.pTraceFile         = nullptr;

    // Check if HLT should be enabled.
    MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED_ID,
        &UserFeatureData);
    // If the user feature key was not found, create it with the default value.
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MosUtilities::MosZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
        UserFeatureWriteData.Value.u32Data = UserFeatureData.u32Data;
        UserFeatureWriteData.ValueID = __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED_ID;
        MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData, 1);
    }

    bUseHybridLogTrace = MosUtilities::m_mosUltFlag ? 1 : UserFeatureData.u32Data;

    // Dumping memory mapped regions to trace file disabled for now
    // Need to add new user feature key or derive from the above key.
    m_mosMsgParams.bEnableMaps = 0;

    if (!bUseHybridLogTrace)
    {
        MOS_OS_NORMALMESSAGE("HLT not enabled.");
        return MOS_STATUS_SUCCESS;               //[SH]: Check this.
    }

    nPID = MosUtilities::m_mosUltFlag ? 0 : MosUtilities::MosGetPid();

    // Get logfile directory.
    MosLogFileNamePrefix(fileNamePrefix);
    MosUtilities::MosSecureStringPrint(hltFileName, MOS_MAX_HLT_FILENAME_LEN, MOS_MAX_HLT_FILENAME_LEN-1, m_mosLogPathTemplate, fileNamePrefix, nPID, "log");

#if defined(LINUX) || defined(ANDROID)
    eStatus = MosUtilities::MosCreateDirectory(fileNamePrefix);
    if (MOS_FAILED(eStatus))
    {
        MOS_OS_NORMALMESSAGE("Failed to create output directory. Status = %d", eStatus);
    }
#endif

    eStatus = MosUtilities::MosSecureFileOpen(&m_mosMsgParams.pLogFile, hltFileName, "w");

    if (MOS_FAILED(eStatus))
    {
        MOS_OS_NORMALMESSAGE("Failed to open log file '%s'.", hltFileName);
        m_mosMsgParams.pLogFile = nullptr;
    }

    if(m_mosMsgParams.pLogFile == nullptr)
    {
        return MOS_STATUS_HLT_INIT_FAILED;
    }

    // Only if logfile init succeeded, bUseHybridLogTrace is set.
    m_mosMsgParams.bUseHybridLogTrace = true;

    // Output preface information
    MosHltpPreface(m_mosMsgParams.pLogFile);
    MOS_OS_NORMALMESSAGE("HLT initialized successfuly (%s).", hltFileName);

    //[SH]: Trace and log are enabled with the same key right now. This can be changed to enable/disable them independently.
    MosUtilities::MosSecureStringPrint(hltFileName, MOS_MAX_HLT_FILENAME_LEN, MOS_MAX_HLT_FILENAME_LEN-1, m_mosLogPathTemplate, fileNamePrefix, nPID, "hlt");

    eStatus = MosUtilities::MosSecureFileOpen(&m_mosMsgParams.pTraceFile, hltFileName, "w");

    if (MOS_FAILED(eStatus))
    {
        MOS_OS_NORMALMESSAGE("Failed to open trace file '%s'.", hltFileName);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilDebug::MosDDIDumpInit()
{
    char                                        fileNamePrefix[MOS_MAX_HLT_FILENAME_LEN];
    MOS_USER_FEATURE_VALUE_DATA                 UserFeatureData;
    char                                        cDDIDumpFilePath[MOS_MAX_HLT_FILENAME_LEN] = { 0 };
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    m_mosMsgParamsDdiDump.bUseHybridLogTrace = false;
    m_mosMsgParamsDdiDump.pLogFile = nullptr;
    m_mosMsgParamsDdiDump.pTraceFile = nullptr;

    //Check if DDI dump is enabled
    MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_DDI_DUMP_ENABLE_ID,
        &UserFeatureData);

    if (UserFeatureData.i32Data == 1)
    {
        //Check for the DDI dump path from the user feature key
        MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        eStatus = MosUtilities::MosUserFeatureReadValueID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_DDI_DUMP_DIRECTORY_ID,
            &UserFeatureData);

        // set-up DDI dump file name
        MosUtilities::MosSecureStringPrint(cDDIDumpFilePath, MOS_MAX_HLT_FILENAME_LEN, MOS_MAX_HLT_FILENAME_LEN - 1, m_DdiLogPathTemplate,
            (UserFeatureData.StringData.uSize > 0) ? UserFeatureData.StringData.pStringData : fileNamePrefix, MosUtilities::MosGetPid(), "log");

        eStatus = MosUtilities::MosSecureFileOpen(&m_mosMsgParamsDdiDump.pLogFile, cDDIDumpFilePath, "w");
        if (MOS_FAILED(eStatus))
        {
            MOS_OS_NORMALMESSAGE("Failed to open log file '%s'.", cDDIDumpFilePath);
            m_mosMsgParamsDdiDump.pLogFile = nullptr;
        }
     }

    return MOS_STATUS_SUCCESS;
}

void MosUtilDebug::MosMessageInit()
{
    uint8_t                                     i = 0;
    MOS_USER_FEATURE_VALUE_DATA                 UserFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA           UserFeatureWriteData;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    if(m_mosMsgParams.uiCounter == 0)   // first time only
    {
        // Set all sub component messages to critical level by default.
        MosSetCompMessageLevelAll(MOS_MESSAGE_LVL_CRITICAL);

        // Set print level and asserts for each component
        for(i = 0; i < MOS_COMPONENT_COUNT; i++)
        {
            MosMessageInitComponent((MOS_COMPONENT_ID)i);
        }

        // Check if MOS messages are enabled
        MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        eStatus = MosUtilities::MosUserFeatureReadValueID(
            nullptr,
            __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED_ID,
            &UserFeatureData);
        // If the user feature key was not found, create it with default value.
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MosUtilities::MosZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
            UserFeatureWriteData.Value.i32Data = UserFeatureData.i32Data;
            UserFeatureWriteData.ValueID = __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED_ID;
            MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData, 1);
        }

        m_mosMsgParams.bUseOutputDebugString = UserFeatureData.i32Data;

        if (MosUtilities::m_mosUltFlag)
        {
            MosSetCompMessageLevelAll(MOS_MESSAGE_LVL_DISABLED);
            MosSetCompMessageLevel(MOS_COMPONENT_OS, MOS_MESSAGE_LVL_VERBOSE);
            m_mosMsgParams.bUseOutputDebugString = 0;
            m_mosMsgParams.components[MOS_COMPONENT_OS].bBySubComponent = 0;
            MosCompAssertEnableDisable(MOS_COMPONENT_CM, 0);
        }

        MosHLTInit();
        MosDDIDumpInit();

        // all above action should not be covered by memninja since its destroy is behind memninja counter report to test result.
        MosMemAllocCounter     = 0;
        MosMemAllocFakeCounter = 0;
        MosMemAllocCounterGfx  = 0;
        MOS_OS_VERBOSEMESSAGE("MemNinja leak detection begin");
    }

    // uiCounter's thread safety depends on global_lock in VPG_Initialize
    m_mosMsgParams.uiCounter++;     // Will be zero initially since it is part of a global structure.

}

void MosUtilDebug::MosHLTClose()
{
    if(m_mosMsgParams.pTraceFile != nullptr)
    {
        fclose(m_mosMsgParams.pTraceFile);
        m_mosMsgParams.pTraceFile = nullptr;

        MOS_OS_NORMALMESSAGE("Trace file is closed.");
    }

    if(m_mosMsgParams.pLogFile != nullptr)
    {
        MOS_OS_NORMALMESSAGE("Log file is closing, total services %d.",
            m_mosMsgParams.uiCounter);

        fclose(m_mosMsgParams.pLogFile);
        m_mosMsgParams.pLogFile = nullptr;
    }
}

void  MosUtilDebug::MosDDIDumpClose()
{
    if (m_mosMsgParamsDdiDump.pLogFile != nullptr)
    {
        fclose(m_mosMsgParamsDdiDump.pLogFile);
        m_mosMsgParamsDdiDump.pLogFile = nullptr;
        MOS_OS_NORMALMESSAGE("Encode DDI Dump file closing");
    }
}

void MosUtilDebug::MosMessageClose()
{
    // uiCounter's thread safety depends on global_lock in VPG_Terminate
    if(m_mosMsgParams.uiCounter == 1)
    {
        MosDDIDumpClose();
        MosHLTClose();
        MosUtilities::MosZeroMemory(&m_mosMsgParams, sizeof(MOS_MESSAGE_PARAMS));
    }
    else
    {
        m_mosMsgParams.uiCounter--;
    }
}

int32_t MosUtilDebug::MosShouldPrintMessage(
    MOS_MESSAGE_LEVEL  level,
    MOS_COMPONENT_ID   compID,
    uint8_t            subCompID,
    const char * const message)
{
    if (message == nullptr)
    {
        return false;
    }

    if (compID    >= MOS_COMPONENT_COUNT      ||
        subCompID >= MOS_MAX_SUBCOMPONENT_COUNT ||
        level >= MOS_MESSAGE_LVL_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid compoent ID %d, subCompID %d, and msg level %d.", compID, subCompID, level);
        return false;
    }

    // Check if message level set for comp (and if needed for subcomp) is equal or greater than requested level
    if (m_mosMsgParams.components[compID].component.uiMessageLevel < level)
    {
        return false;
    }

    if (m_mosMsgParams.components[compID].bBySubComponent                               &&
        m_mosMsgParams.components[compID].subComponents[subCompID].uiMessageLevel < level)
    {
        return false;
    }

    return true;

}
#endif // MOS_MESSAGES_ENABLED

#if MOS_ASSERT_ENABLED

int32_t MosUtilDebug::MosShouldAssert(MOS_COMPONENT_ID compID, uint8_t subCompID)
{
    if (compID    >= MOS_COMPONENT_COUNT      ||
        subCompID >= MOS_MAX_SUBCOMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid compoent ID %d, subCompID %d", compID, subCompID);
        return false;
    }

    if (!m_mosMsgParams.components[compID].component.bAssertEnabled)
    {
        return false;
    }

    if (m_mosMsgParams.components[compID].bBySubComponent                       &&
       !m_mosMsgParams.components[compID].subComponents[subCompID].bAssertEnabled)
    {
        return false;
    }

    return true;

}
#endif // MOS_ASSERT_ENABLED

