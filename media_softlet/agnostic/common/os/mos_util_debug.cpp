/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     mos_util_debug.cpp
//! \brief    Common OS debug across different platform
//! \details  Common OS debug across different platform
//!

#include "mos_util_debug.h"

#if MOS_MESSAGES_ENABLED
#include "media_user_setting.h"

//!
//! \brief DDI dump file name template
//!
#define DDILogPathTemplate  "%s\\ddi_dump_%d.%s"

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
    "[CM]:   ",
    "[CPLIB]:",
    "[SCAL]: ",
    "[MMC]:  ",
    "[MCPY]: "
};

const char* MosUtilDebug::m_pcComponentUserFeatureKeys[MOS_COMPONENT_COUNT][3] = {
    {
    __MOS_USER_FEATURE_KEY_MESSAGE_OS_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_OS,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_OS_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_MHW_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MHW,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MHW_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_CODEC_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CODEC,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CODEC_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_VP_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_VP,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_VP_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_DDI_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_DDI,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_DDI_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_CM_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CM,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CM_TAG
    },

    {// CPLIB
    __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_SCALABILITY_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_SCALABILITY,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_SCALABILITY_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_MMC_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MMC,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MMC_TAG
    },

    {
    __MOS_USER_FEATURE_KEY_MESSAGE_MCPY_TAG,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MCPY,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MCPY_TAG
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

MOS_MESSAGE_PARAMS MosUtilDebug::m_mosMsgParams            = {};

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

void MosUtilDebug::MosMessageInitComponent(MOS_COMPONENT_ID compID, MediaUserSettingSharedPtr userSettingPtr)
{
    uint32_t                                    uiCompUserFeatureSetting = 0;
    uint64_t                                    uiSubCompUserFeatureSetting = 0;
    uint8_t                                     i = 0;
    const char                                  *messageKey = nullptr;
    const char                                  *bySubComponentsKey = nullptr;
    const char                                  *subComponentsKey = nullptr;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    if (compID >= MOS_COMPONENT_COUNT )
    {
        MOS_OS_ASSERTMESSAGE("Invalid component %d.", compID);
        return;
    }

    messageKey         = m_pcComponentUserFeatureKeys[compID][0];
    bySubComponentsKey = m_pcComponentUserFeatureKeys[compID][1];
    subComponentsKey   = m_pcComponentUserFeatureKeys[compID][2];

    eStatus = ReadUserSetting(
        userSettingPtr,
        uiCompUserFeatureSetting,
        messageKey,
        MediaUserSetting::Group::Device);
    // If the user feature key was not found, create it with the default value.
    if (eStatus  != MOS_STATUS_SUCCESS)
    {
        ReportUserSetting(
            userSettingPtr,
            messageKey,
            uiCompUserFeatureSetting,
            MediaUserSetting::Group::Device);
    }

    // Extract the 3-bit message level and 1-bit assert flag setting for this component.
    MosSetCompMessageLevel(compID, (MOS_MESSAGE_LEVEL) (uiCompUserFeatureSetting & 0x7));
    MosCompAssertEnableDisable(compID, (uiCompUserFeatureSetting >> 3) & 0x1);

    // Check if sub-components should be set seperately.
    eStatus = ReadUserSetting(
        userSettingPtr,
        m_mosMsgParams.components[compID].bBySubComponent,
        bySubComponentsKey,
        MediaUserSetting::Group::Device);

    // If the user feature key was not found, create it with default (0) value.
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        ReportUserSetting(
            userSettingPtr,
            bySubComponentsKey,
            m_mosMsgParams.components[compID].bBySubComponent,
            MediaUserSetting::Group::Device);
    }

    // Set sub components:
    if (m_mosMsgParams.components[compID].bBySubComponent)
    {
        // Check if sub-components should be set seperately.
        eStatus = ReadUserSetting(
            userSettingPtr,
            uiSubCompUserFeatureSetting,
            subComponentsKey,
            MediaUserSetting::Group::Device);

        // If the user feature key was not found, create it with default (0) value.
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ReportUserSetting(
                userSettingPtr,
                subComponentsKey,
                uiSubCompUserFeatureSetting,
                MediaUserSetting::Group::Device);
        }

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

MOS_STATUS MosUtilDebug::MosHLTInit(MediaUserSettingSharedPtr userSettingPtr)
{
    uint32_t                                    nPID = 0;
    char                                        hltFileName[MOS_MAX_HLT_FILENAME_LEN] = {0};
    char                                        fileNamePrefix[MOS_MAX_HLT_FILENAME_LEN];
    int32_t                                     bUseHybridLogTrace = false;
    int32_t                                     bEnableFlush = false;
    int32_t                                     bEnableMemoryFootPrint = false;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    if (m_mosMsgParams.uiCounter != 0 )
    {
        MOS_OS_NORMALMESSAGE("HLT settings already set.");
        return MOS_STATUS_UNKNOWN;
    }

    
    eStatus = ReadUserSetting(
        userSettingPtr,
        bEnableFlush,
        __MOS_USER_FEATURE_KEY_FLUSH_LOG_FILE_BEFORE_SUBMISSION,
        MediaUserSetting::Group::Device);


    ReportUserSetting(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_FLUSH_LOG_FILE_BEFORE_SUBMISSION,
        bEnableFlush,
        MediaUserSetting::Group::Device);

    if (!bEnableFlush)
    {
        MOS_OS_NORMALMESSAGE("HLT flush is not enabled.");
    }

    m_mosMsgParams.bUseHybridLogTrace = false;
    m_mosMsgParams.pLogFile           = nullptr;
    m_mosMsgParams.pTraceFile         = nullptr;
    m_mosMsgParams.bEnableFlush       = bEnableFlush;

    // disable memory foot print
    eStatus = ReadUserSetting(
        userSettingPtr,
        bEnableMemoryFootPrint,
        __MOS_USER_FEATURE_KEY_ENABLE_MEMORY_FOOT_PRINT,
        MediaUserSetting::Group::Device);

    ReportUserSetting(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_ENABLE_MEMORY_FOOT_PRINT,
        bEnableMemoryFootPrint,
        MediaUserSetting::Group::Device);

    if (bEnableMemoryFootPrint)
    {
        MOS_OS_NORMALMESSAGE("Mos memory foot print is enabled.");
    }

    m_mosMsgParams.bEnableMemoryFootPrint = bEnableMemoryFootPrint;

    // Check if HLT should be enabled.
    eStatus = ReadUserSetting(
        userSettingPtr,
        bUseHybridLogTrace,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED,
        MediaUserSetting::Group::Device);
    // If the user feature key was not found, create it with the default value.
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        ReportUserSetting(
            userSettingPtr,
            __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED,
            bUseHybridLogTrace,
            MediaUserSetting::Group::Device);
    }

    bUseHybridLogTrace = (MosUtilities::m_mosUltFlag && *MosUtilities::m_mosUltFlag) ? 1 : bUseHybridLogTrace;

    // Dumping memory mapped regions to trace file disabled for now
    // Need to add new user feature key or derive from the above key.
    m_mosMsgParams.bEnableMaps = 0;

    if (!bUseHybridLogTrace)
    {
        MOS_OS_NORMALMESSAGE("HLT not enabled.");
        return MOS_STATUS_SUCCESS;               //[SH]: Check this.
    }

    nPID = (MosUtilities::m_mosUltFlag && *MosUtilities::m_mosUltFlag) ? 0 : MosUtilities::MosGetPid();

    // Get logfile directory.
    MosLogFileNamePrefix(fileNamePrefix, userSettingPtr);
    MosUtilities::MosSecureStringPrint(hltFileName, MOS_MAX_HLT_FILENAME_LEN, MOS_MAX_HLT_FILENAME_LEN - 1, MOS_LOG_PATH_TEMPLATE, fileNamePrefix, nPID, MosUtilities::MosGetCurrentThreadId(), "log");

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
    MosUtilities::MosSecureStringPrint(hltFileName, MOS_MAX_HLT_FILENAME_LEN, MOS_MAX_HLT_FILENAME_LEN - 1, MOS_LOG_PATH_TEMPLATE, fileNamePrefix, nPID, MosUtilities::MosGetCurrentThreadId(), "hlt");

    eStatus = MosUtilities::MosSecureFileOpen(&m_mosMsgParams.pTraceFile, hltFileName, "w");

    if (MOS_FAILED(eStatus))
    {
        MOS_OS_NORMALMESSAGE("Failed to open trace file '%s'.", hltFileName);
    }

    return MOS_STATUS_SUCCESS;
}

void MosUtilDebug::MosMessageInit(MediaUserSettingSharedPtr userSettingPtr)
{
    uint8_t                                     i = 0;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    if(m_mosMsgParams.uiCounter == 0)   // first time only
    {
        eStatus = ReadUserSetting(
            userSettingPtr,
            m_mosMsgParams.bDisableAssert,
            __MOS_USER_FEATURE_KEY_DISABLE_ASSERT,
            MediaUserSetting::Group::Device);
        // If the user feature key was not found, create it with default value.
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ReportUserSetting(
                userSettingPtr,
                __MOS_USER_FEATURE_KEY_DISABLE_ASSERT,
                m_mosMsgParams.bDisableAssert,
                MediaUserSetting::Group::Device);
        }

        // Set all sub component messages to critical level by default.
        MosSetCompMessageLevelAll(MOS_MESSAGE_LVL_CRITICAL);

        // Set print level and asserts for each component
        for(i = 0; i < MOS_COMPONENT_COUNT; i++)
        {
            MosMessageInitComponent((MOS_COMPONENT_ID)i, userSettingPtr);
        }

        // Check if MOS messages are enabled
        eStatus = ReadUserSetting(
            userSettingPtr,
            m_mosMsgParams.bUseOutputDebugString,
            __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED,
            MediaUserSetting::Group::Device);

        // If the user feature key was not found, create it with default value.
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            eStatus = ReportUserSetting(
                userSettingPtr,
                __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED,
                m_mosMsgParams.bUseOutputDebugString,
                MediaUserSetting::Group::Device);
        }

        if (MosUtilities::m_mosUltFlag && (*MosUtilities::m_mosUltFlag))
        {
            MosSetCompMessageLevelAll(MOS_MESSAGE_LVL_DISABLED);
            MosSetCompMessageLevel(MOS_COMPONENT_OS, MOS_MESSAGE_LVL_CRITICAL);
            MosSetCompMessageLevel(MOS_COMPONENT_VP, MOS_MESSAGE_LVL_CRITICAL);
            m_mosMsgParams.bUseOutputDebugString = 1;
            m_mosMsgParams.components[MOS_COMPONENT_OS].bBySubComponent = 0;
            MosCompAssertEnableDisable(MOS_COMPONENT_CM, 0);
            MosCompAssertEnableDisable(MOS_COMPONENT_VP, 1);
        }

        MosHLTInit(userSettingPtr);

        // all above action should not be covered by memninja since its destroy is behind memninja counter report to test result.
        if (MosUtilities::m_mosMemAllocCounter &&
            MosUtilities::m_mosMemAllocCounterGfx &&
            MosUtilities::m_mosMemAllocFakeCounter)
        {
            *MosUtilities::m_mosMemAllocCounter     = 0;
            *MosUtilities::m_mosMemAllocFakeCounter = 0;
            *MosUtilities::m_mosMemAllocCounterGfx  = 0;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("MemNinja count pointers are nullptr");
        }

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

void MosUtilDebug::MosMessageClose()
{
    // uiCounter's thread safety depends on global_lock in VPG_Terminate
    if(m_mosMsgParams.uiCounter == 1)
    {
        MosHLTClose();
        MosUtilities::MosZeroMemory(&m_mosMsgParams, sizeof(MOS_MESSAGE_PARAMS));
    }
    else
    {
        m_mosMsgParams.uiCounter--;
    }
}

void MosUtilDebug::MosHLTFlush()
{
    if (!m_mosMsgParams.bEnableFlush)
    {
        return;
    }

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    if (m_mosMsgParams.pLogFile != nullptr)
    {
#if COMMON_DLL_SEPARATION_SUPPORT
        // Every DLL has its own C Runtime (CRT),
        // and fflush is not safe across dlls.
        // When common dll separation is enabled, We should call back into common dll for all DDI dlls.
        MosUtilities::MosFlushToFileInCommon(m_mosMsgParams.pLogFile);
#else
        fflush(m_mosMsgParams.pLogFile);
#endif
    }
    if (m_mosMsgParams.pTraceFile != nullptr)
    {
#if COMMON_DLL_SEPARATION_SUPPORT
        // Every DLL has its own C Runtime (CRT),
        // and fflush is not safe across dlls.
        // When common dll separation is enabled, We should call back into common dll for all DDI dlls.
        MosUtilities::MosFlushToFileInCommon(m_mosMsgParams.pTraceFile);
#else
        fflush(m_mosMsgParams.pTraceFile);
#endif
    }
}

bool MosUtilDebug::EnableMemoryFootPrint()
{
    return m_mosMsgParams.bEnableMemoryFootPrint;
}

void MosUtilDebug::MosMessage(
    MOS_MESSAGE_LEVEL level,
    MOS_COMPONENT_ID  compID,
    uint8_t           subCompID,
    const PCCHAR      functionName,
    int32_t           lineNum,
    const PCCHAR      message,
    ...)
{
    va_list var_args;
    va_start(var_args, message);
    MosMessageInternal(level, compID, subCompID, functionName, lineNum, message, var_args);
    va_end(var_args);
    return;
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

    // If trace is enabled, return true, otherwise continue checking if log is enabled
    if (MosUtilities::MosShouldTraceEventMsg(level, compID))
    {
        return true;
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

#if MOS_ASSERT_ENABLED

int32_t MosUtilDebug::MosShouldAssert(MOS_COMPONENT_ID compID, uint8_t subCompID)
{
    if (m_mosMsgParams.bDisableAssert)
    {
        return false;
    }

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

#endif  // MOS_MESSAGES_ENABLED
