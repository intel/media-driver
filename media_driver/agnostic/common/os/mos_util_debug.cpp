/*
* Copyright (c) 2013-2021, Intel Corporation
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
#include "mos_utilities.h"
#include "mos_util_debug_next.h"

extern uint8_t MosUltFlag;

//!
//! \brief HLT file name template
//!
const char * const MosLogPathTemplate = MOS_LOG_PATH_TEMPLATE;

//!
//! \brief DDI dump file name template
//!
const char * const DDILogPathTemplate = "%s\\ddi_dump_%d.%s";

const char * const MOS_LogLevelName[MOS_MESSAGE_LVL_COUNT] = {
    "",          // DISABLED
    "CRITICAL",
    "NORMAL  ",
    "VERBOSE ",
    "ENTER   ",
    "EXIT    ",
    "ENTER   ",  // ENTER VERBOSE
    "EXIT    ",  // EXIT VERBOSE
};

const char * const MOS_ComponentName[MOS_COMPONENT_COUNT] = {
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

//!
//! \brief Mos params global structure initalized when UMD device created
//!        and de-initialized when UMD device destroyed.
//!
MOS_MESSAGE_PARAMS g_MosMsgParams;
MOS_MESSAGE_PARAMS g_MosMsgParams_DDI_Dump;

extern const MOS_USER_FEATURE_VALUE_ID pcComponentUserFeatureKeys[MOS_COMPONENT_COUNT][3] = {
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

    {// CPLIB
    __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG_ID
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
    __MOS_USER_FEATURE_KEY_MESSAGE_MCPY_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MCPY_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MCPY_TAG_ID
    }
};

extern const uint8_t subComponentCount[MOS_COMPONENT_COUNT] = {
    MOS_SUBCOMP_COUNT,
    MOS_HW_SUBCOMP_COUNT,
    MOS_CODEC_SUBCOMP_COUNT,
    MOS_VP_SUBCOMP_COUNT,
    MOS_CP_SUBCOMP_COUNT,
    MOS_DDI_SUBCOMP_COUNT,
    MOS_CM_SUBCOMP_COUNT
};

//!
//! \brief    Set debug message level for a sub-component within a component
//! \details  Set debug message level for a sub-component within a component
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \param    MOS_MESSAGE_LEVEL msgLevel
//!           [in] Message level that the sub component allows
//! \return   void
//!
void MOS_SetSubCompMessageLevel(MOS_COMPONENT_ID compID, uint8_t subCompID, MOS_MESSAGE_LEVEL msgLevel)
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

    g_MosMsgParams.components[compID].subComponents[subCompID].uiMessageLevel = msgLevel;
}

//!
//! \brief    Set debug message level for a particular component
//! \details  Set debug message level for a particular component
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    MOS_MESSAGE_LEVEL msgLevel
//!           [in] Message level that the component allows
//! \return   void
//!
void MOS_SetCompMessageLevel(MOS_COMPONENT_ID compID, MOS_MESSAGE_LEVEL msgLevel)
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
    g_MosMsgParams.components[compID].component.uiMessageLevel = msgLevel;
}

//!
//! \brief    Set debug message level for all components
//! \details  Set all component to the same msg level
//! \param    MOS_MESSAGE_LEVEL msgLevel
//!           [in] Message level that all components allow
//! \return   void
//!
void MOS_SetCompMessageLevelAll(MOS_MESSAGE_LEVEL msgLevel)
{
    if (msgLevel >= MOS_MESSAGE_LVL_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid msg level %d.", msgLevel);
        return;
    }

    uint32_t i = 0;

    for(i = 0; i < MOS_COMPONENT_COUNT; i++)
    {
        MOS_SetCompMessageLevel((MOS_COMPONENT_ID)i, msgLevel);
    }
}

//!
//! \brief    Enable/disable asserts for a sub-component within a component
//! \details  Enable/disable asserts for a sub-component within a component
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \param    int32_t iFlag
//!           [in] Enable/disable flag
//! \return   void
//!
void MOS_SubCompAssertEnableDisable(MOS_COMPONENT_ID compID, uint8_t subCompID, int32_t bEnable)
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

    g_MosMsgParams.components[compID].subComponents[subCompID].bAssertEnabled = bEnable;
}

//!
//! \brief    Enable or disable asserts of a particular component
//! \details  Enable or disable asserts of a particular component
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    int32_t bEnable
//!           [in] Enable/disable flag
//! \return   void
//!
void MOS_CompAssertEnableDisable(MOS_COMPONENT_ID compID, int32_t bEnable)
{
    return MosUtilDebug::MosCompAssertEnableDisable(compID, bEnable);
}

//!
//! \brief    Set debug message level and asserts for a component and its sub-components.
//! \details  Set debug message level and asserts for a component and its sub-components.
//!              This includes registering all sub-components.
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    [in] mosCtx
//!           os device ctx handle
//! \return   void
//!
void MOS_MessageInitComponent(MOS_COMPONENT_ID compID, MOS_CONTEXT_HANDLE mosCtx)
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

    MessageKey         = pcComponentUserFeatureKeys[compID][0];
    BySubComponentsKey = pcComponentUserFeatureKeys[compID][1];
    SubComponentsKey   = pcComponentUserFeatureKeys[compID][2];

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MOS_UserFeature_ReadValue_ID(
                  nullptr,
                  MessageKey,
                  &UserFeatureData,
                  mosCtx);

    // If the user feature key was not found, create it with the default value.
    if (eStatus  != MOS_STATUS_SUCCESS)
    {
        MOS_ZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
        UserFeatureWriteData.Value.u32Data = UserFeatureData.u32Data;
        UserFeatureWriteData.ValueID = MessageKey;
        MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);
    }

    uiCompUserFeatureSetting = UserFeatureData.u32Data;

    // Extract the 3-bit message level and 1-bit assert flag setting for this component.
    MOS_SetCompMessageLevel(compID, (MOS_MESSAGE_LEVEL) (uiCompUserFeatureSetting & 0x7));
    MOS_CompAssertEnableDisable(compID, (uiCompUserFeatureSetting >> 3) & 0x1);

    // Check if sub-components should be set seperately.
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MOS_UserFeature_ReadValue_ID(
        nullptr,
        BySubComponentsKey,
        &UserFeatureData,
        mosCtx);
    // If the user feature key was not found, create it with default (0) value.
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_ZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
        UserFeatureWriteData.Value.u32Data = UserFeatureData.u32Data;
        UserFeatureWriteData.ValueID = BySubComponentsKey;
        MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);
    }

    g_MosMsgParams.components[compID].bBySubComponent = UserFeatureData.u32Data;

    // Set sub components:
    if (g_MosMsgParams.components[compID].bBySubComponent)
    {
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        eStatus = MOS_UserFeature_ReadValue_ID(
            nullptr,
            SubComponentsKey,
            &UserFeatureData,
            mosCtx);
        // If the user feature key was not found, create it with default (0) value.
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_ZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
            UserFeatureWriteData.Value.u64Data = UserFeatureData.u64Data;
            UserFeatureWriteData.ValueID = SubComponentsKey;
            MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);
        }

        uiSubCompUserFeatureSetting = UserFeatureData.u64Data;

        for(i = 0; i < subComponentCount[compID]; i++)
        {
            // Extract the 3-bit message level and 1-bit assert flag setting for each sub-comp
            // from the user feature key and populate to the MOS message params structure
            MOS_SetSubCompMessageLevel(compID, i, (MOS_MESSAGE_LEVEL)(uiSubCompUserFeatureSetting & 0x7));
            MOS_SubCompAssertEnableDisable(compID, i, (uiSubCompUserFeatureSetting >> 3) & 0x1);

            uiSubCompUserFeatureSetting = (uiSubCompUserFeatureSetting >> 4);
        }
    }
}

//!
//! \brief    Initialize or refresh the DDI Dump facility
//! \details  Initialize or refresh the DDI Dump facility
//!           Called during MOS init
//! \param    [in] mosCtx
//!           os device ctx handle
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DDIDumpInit(MOS_CONTEXT_HANDLE mosCtx)
{
    char                                        fileNamePrefix[MOS_MAX_HLT_FILENAME_LEN];
    MOS_USER_FEATURE_VALUE_DATA                 UserFeatureData;
    char                                        cDDIDumpFilePath[MOS_MAX_HLT_FILENAME_LEN] = { 0 };
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    g_MosMsgParams_DDI_Dump.bUseHybridLogTrace = false;
    g_MosMsgParams_DDI_Dump.pLogFile = nullptr;
    g_MosMsgParams_DDI_Dump.pTraceFile = nullptr;

    //Check if DDI dump is enabled
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_DDI_DUMP_ENABLE_ID,
        &UserFeatureData,
        mosCtx);

    if (UserFeatureData.i32Data == 1)
    {
        //Check for the DDI dump path from the user feature key
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        eStatus = MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_DDI_DUMP_DIRECTORY_ID,
            &UserFeatureData,
            mosCtx);

        // set-up DDI dump file name
        MOS_SecureStringPrint(cDDIDumpFilePath, MOS_MAX_HLT_FILENAME_LEN, MOS_MAX_HLT_FILENAME_LEN - 1, DDILogPathTemplate,
            (UserFeatureData.StringData.uSize > 0) ? UserFeatureData.StringData.pStringData : fileNamePrefix, MosUtilities::MosGetPid(), "log");

        eStatus = MosUtilities::MosSecureFileOpen(&g_MosMsgParams_DDI_Dump.pLogFile, cDDIDumpFilePath, "w");
        if (MOS_FAILED(eStatus))
        {
            MOS_OS_NORMALMESSAGE("Failed to open log file '%s'.", cDDIDumpFilePath);
            g_MosMsgParams_DDI_Dump.pLogFile = nullptr;
        }
     }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Initialize the MOS message params structure and HLT.
//! \details  Initialize the MOS message params structure and HLT,
//!           to be called during device creation
//! \param    [in] mosCtx
//!           os device ctx handle
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
void MOS_MessageInit(MOS_CONTEXT_HANDLE mosCtx)
{
    return MosUtilDebug::MosMessageInit(mosCtx);
}

//!
//! \brief    Close file handles and frees resources
//! \details  Close file handles and frees resources
//!           Called during MOS close
//! \return   void
//!
void MOS_HLTClose()
{
    if(g_MosMsgParams.pTraceFile != nullptr)
    {
        fclose(g_MosMsgParams.pTraceFile);
        g_MosMsgParams.pTraceFile = nullptr;

        MOS_OS_NORMALMESSAGE("Trace file is closed.");
    }

    if(g_MosMsgParams.pLogFile != nullptr)
    {
        MOS_OS_NORMALMESSAGE("Log file is closing, total services %d.",
            g_MosMsgParams.uiCounter);

        fclose(g_MosMsgParams.pLogFile);
        g_MosMsgParams.pLogFile = nullptr;
    }
}

// !
//! \brief    Close file handles and frees resources
//! \details  Close file handles and frees resources
//!           Called during MOS close
//! \return   void
//!
void MOS_DDIDumpClose()
{
    if (g_MosMsgParams_DDI_Dump.pLogFile != nullptr)
    {
        fclose(g_MosMsgParams_DDI_Dump.pLogFile);
        g_MosMsgParams_DDI_Dump.pLogFile = nullptr;
        MOS_OS_NORMALMESSAGE("Encode DDI Dump file closing");
    }
}

//!
//! \brief    Frees the MOS message buffer and MOS message parameters structure
//! \details  Frees the MOS message buffer and MOS message parameters structure,
//!           to be called during device destruction
//! \return   void
//!
void MOS_MessageClose()
{
    return MosUtilDebug::MosMessageClose();
}

//!
//! \brief    Checks whether debug messages should be printed.
//! \details  Determines by the print level, component and sub-component IDs
//!           whether the debug message should be printed.
//! \param    MOS_MESSAGE_LEVEL level
//!           [in] Level of the message
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \param    const char  *message
//!           [in] pointer to the message format string
//! \return   int32_t
//!
int32_t MOS_ShouldPrintMessage(
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
    if (g_MosMsgParams.components[compID].component.uiMessageLevel < level)
    {
        return false;
    }

    if (g_MosMsgParams.components[compID].bBySubComponent                               &&
        g_MosMsgParams.components[compID].subComponents[subCompID].uiMessageLevel < level)
    {
        return false;
    }

    return true;

}

#if MOS_ASSERT_ENABLED
//!
//! \brief    Checks whether assert should be hit.
//! \details  Determines by the component and sub-component IDs
//!           whether an assert should be hit.
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \return   int32_t
//!
int32_t MOS_ShouldAssert(MOS_COMPONENT_ID compID, uint8_t subCompID)
{
    if (compID    >= MOS_COMPONENT_COUNT      ||
        subCompID >= MOS_MAX_SUBCOMPONENT_COUNT)
    {
        MOS_OS_ASSERTMESSAGE("Invalid compoent ID %d, subCompID %d", compID, subCompID);
        return false;
    }

    if (!g_MosMsgParams.components[compID].component.bAssertEnabled)
    {
        return false;
    }

    if (g_MosMsgParams.components[compID].bBySubComponent                       &&
       !g_MosMsgParams.components[compID].subComponents[subCompID].bAssertEnabled)
    {
        return false;
    }

    return true;

}
#endif // MOS_ASSERT_ENABLED

#endif // MOS_MESSAGES_ENABLED
