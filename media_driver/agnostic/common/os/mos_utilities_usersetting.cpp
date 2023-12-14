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
//! \file        mos_utilities_usersetting.cpp
//! \brief       This module implements the MOS wrapper functions
//!

#include "mos_os.h"
#include "mos_util_user_interface.h"
#include "media_user_settings_mgr.h"

#define __MAX_MULTI_STRING_COUNT         128

char s_xmlFilePath[MOS_USER_CONTROL_MAX_DATA_SIZE] = {};

/*****************************************************************************
|
|                           USER FEATURE Functions
|
*****************************************************************************/
void MosUtilities::MosFreeUserFeatureValueString(PMOS_USER_FEATURE_VALUE_STRING pUserString)
{
    if (pUserString != nullptr)
    {
        if (pUserString->uSize > 0)
        {
            if (pUserString->pStringData)
            {
                MOS_FreeMemAndSetNull(pUserString->pStringData);
                MosAtomicDecrement(m_mosMemAllocFakeCounter);
            }
            pUserString->uSize = 0;
        }
    }
}

MOS_STATUS MosUtilities::MosWriteOneUserFeatureKeyToXML(PMOS_USER_FEATURE_VALUE pUserFeature)
{
    char                            sOutBuf[MOS_USER_CONTROL_MAX_DATA_SIZE];
    char                            ValueType[MAX_USER_FEATURE_FIELD_LENGTH];
    char                            KeyPath[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(pUserFeature);

    switch (pUserFeature->Type)
    {
    case MOS_USER_FEATURE_TYPE_USER:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "UFINT\\%s",
            pUserFeature->pcPath);
        break;
    case MOS_USER_FEATURE_TYPE_SYSTEM:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "UFEXT\\%s",
            pUserFeature->pcPath);
        break;
    default:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "%s",pUserFeature->pcPath);
        break;
     }

    switch (pUserFeature->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "bool");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "dword");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "qword");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "string");
        break;
    default:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "unknown");
        break;
     }

    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "    <Key name=\"%s\" type=\"%s\" location=\"%s\" defaultval=\"%s\" description=\"%s\" />\n",
        pUserFeature->pValueName,
        ValueType,
        KeyPath,
        pUserFeature->DefaultValue,
        pUserFeature->pcDescription);
    MosAppendFileFromPtr(
        s_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return eStatus;
}

MOS_STATUS MosUtilities::MosWriteOneUserFeatureGroupToXML(MOS_USER_FEATURE_VALUE   UserFeatureFilter)
{
    char                                sOutBuf[MAX_USER_FEATURE_FIELD_LENGTH];
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    // Group Header Start
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "  <Group name=\"%s\">\n",
        UserFeatureFilter.pcGroup);
    eStatus = MosAppendFileFromPtr(
        s_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));

    // Group User Feature Keys
    eStatus = MosGetItemFromMosUserFeatureDescField(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        &MosWriteOneUserFeatureKeyToXML,
        &UserFeatureFilter);

    // Group Header End
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "  </Group>\n");
    eStatus = MosAppendFileFromPtr(
        s_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return eStatus;
}

MOS_STATUS MosUtilities::MosGenerateUserFeatureKeyXML(MOS_CONTEXT_HANDLE mosCtx)
{
    char                                sOutBuf[MAX_USER_FEATURE_FIELD_LENGTH];
    uint32_t                            uiIndex=0;
    MOS_USER_FEATURE_VALUE              UserFeatureFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    const char * const                  FilterGroups[] = { "Codec", "Decode", "Encode", "CP", "General", "MOS",
                                                           "Report", "VP", "Media", "Secure HEVC Encode", "MDF"};
    uint32_t                            FilterGroupsCount = sizeof(FilterGroups) / sizeof(FilterGroups[0]);
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    // Check if XML dump is enabled by User Feature Key
    MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MosUserFeatureReadValueID(
                    nullptr,
                    __MOS_USER_FEATURE_KEY_XML_AUTOGEN_ID,
                    &UserFeatureData,
                    mosCtx);
    if (UserFeatureData.u32Data == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = s_xmlFilePath;
    eStatus = MosUserFeatureReadValueID(
                    nullptr,
                    __MOS_USER_FEATURE_KEY_XML_FILEPATH_ID,
                    &UserFeatureData,
                    mosCtx);
    // User Feature Key Header Start
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "<UserFeatureKeys>\n");
    eStatus = MosWriteFileFromPtr(
        UserFeatureData.StringData.pStringData,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    // User Feature Key Groups
    for (uiIndex = 0; uiIndex < FilterGroupsCount; uiIndex++)
    {
        UserFeatureFilter.pcGroup = FilterGroups[uiIndex];
        eStatus = MosWriteOneUserFeatureGroupToXML(UserFeatureFilter);
        if (eStatus!= MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to Write One User FeatureGroup To XML");
        }
    }

    // User Feature Key Header End
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "</UserFeatureKeys>\n");
    eStatus = MosAppendFileFromPtr(
        UserFeatureData.StringData.pStringData,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return    eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureSetMultiStringValue(
    PMOS_USER_FEATURE_VALUE_DATA     pFeatureData,
    uint32_t                         dwSize)
{
    PMOS_USER_FEATURE_VALUE_STRING  pStrings;
    uint32_t                        uiNumStrings;
    uint32_t                        ui;
    char                            *pData;
    char                            *pCurData;
    uint32_t                        dwLen;
    uint32_t                        dwPos;

    MOS_OS_CHK_NULL_RETURN(pFeatureData);
    MOS_OS_ASSERT(dwSize);

    pStrings = pFeatureData->MultiStringData.pStrings;
    pData = pFeatureData->MultiStringData.pMultStringData;
    dwPos = 0;
    uiNumStrings = 0;

    MOS_OS_ASSERT(pStrings);
    MOS_OS_ASSERT(pData);

    // Find number of strings in the multi string array
    do
    {
        pCurData = pData + dwPos;
        dwLen = (uint32_t)strlen(pCurData);
        if (dwLen == 0)
        {
            MOS_OS_NORMALMESSAGE("Invalid user feature key entry.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        uiNumStrings++;
        dwPos += dwLen + 1;

        if (dwPos >= (dwSize - 1))
        {
            // last entry
            break;
        }
    } while (true);

    // Check the size of MultiStringData
    if (pFeatureData->MultiStringData.uCount < uiNumStrings)
    {
        MOS_OS_NORMALMESSAGE("pFeatureValue->MultiStringData.uCount is smaller than the actual necessary number.");
        return MOS_STATUS_UNKNOWN;
    }

    // Populate Array
    dwPos = 0;
    for (ui = 0; ui < uiNumStrings; ui++)
    {
        pCurData = pData + dwPos;
        dwLen = (uint32_t)strlen(pCurData);
        MOS_OS_ASSERT(dwLen > 0);
        pStrings[ui].pStringData = pCurData;
        pStrings[ui].uSize = dwLen;

        dwPos += dwLen + 1;
    }

    pFeatureData->MultiStringData.uCount = uiNumStrings;
    pFeatureData->MultiStringData.uSize = dwPos;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosCopyUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA pSrcData,
    PMOS_USER_FEATURE_VALUE_DATA pDstData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                            ui;
    PMOS_USER_FEATURE_VALUE_STRING      pSrcString = nullptr;
    PMOS_USER_FEATURE_VALUE_STRING      pDstString = nullptr;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------
    MOS_OS_ASSERT(pSrcData);
    MOS_OS_ASSERT(pDstData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------

    switch(ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        pDstData->bData = pSrcData->bData;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        pDstData->i32Data = pSrcData->i32Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        pDstData->i64Data = pSrcData->i64Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
        pDstData->u32Data = pSrcData->u32Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        pDstData->u64Data = pSrcData->u64Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        pDstData->fData = pSrcData->fData;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        if ((pSrcData->StringData.pStringData != nullptr) && (strlen(pSrcData->StringData.pStringData) != 0))
        {
            pDstData->StringData.uMaxSize = pSrcData->StringData.uMaxSize;
            pDstData->StringData.uSize = pSrcData->StringData.uSize;
            if (pDstData->StringData.pStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            eStatus = MosSecureMemcpy(
                pDstData->StringData.pStringData,
                pDstData->StringData.uSize,
                pSrcData->StringData.pStringData,
                pSrcData->StringData.uSize);
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        if ((pSrcData->MultiStringData.pMultStringData != nullptr) && (strlen(pSrcData->MultiStringData.pMultStringData) != 0))
        {
            pDstData->MultiStringData.uCount = pSrcData->MultiStringData.uCount;
            pDstData->MultiStringData.uMaxSize = pSrcData->MultiStringData.uMaxSize;
            pDstData->MultiStringData.uSize = pSrcData->MultiStringData.uSize;
            if (pDstData->MultiStringData.pMultStringData != nullptr)
            {
                eStatus = MosSecureMemcpy(
                    pDstData->MultiStringData.pMultStringData,
                    pDstData->MultiStringData.uSize,
                    pSrcData->MultiStringData.pMultStringData,
                    pSrcData->MultiStringData.uSize);

                for (ui = 0; ui < pSrcData->MultiStringData.uCount; ui++)
                {
                    pSrcString = &pSrcData->MultiStringData.pStrings[ui];
                    pDstString = &pDstData->MultiStringData.pStrings[ui];

                    MOS_OS_CHK_NULL(pSrcString);
                    MOS_OS_CHK_NULL(pDstString);
                    pDstString->uMaxSize = pSrcString->uMaxSize;
                    pDstString->uSize = pSrcString->uSize;
                    if (pDstString->pStringData != nullptr)
                    {
                        eStatus = MosSecureMemcpy(
                            pDstString->pStringData,
                            pDstString->uSize+1,
                            pSrcString->pStringData,
                            pSrcString->uSize+1);
                    }// if
                }// for
            }
        }// if
        break;
    default:
        break;
    }
finish:
    return eStatus;
}

MOS_STATUS MosUtilities::MosAssignUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA    pDstData,
    const char                      *pData,
    MOS_USER_FEATURE_VALUE_TYPE     ValueType
)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    uint32_t                        dwUFSize = 0;

    //------------------------------
    MOS_OS_ASSERT(pData);
    MOS_OS_ASSERT(pDstData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------

    switch(ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        pDstData->bData = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        pDstData->i32Data = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        pDstData->i64Data = atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
        pDstData->u32Data = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        pDstData->u64Data = atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        pDstData->fData = (float)atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        pDstData->StringData.uMaxSize = MOS_USER_CONTROL_MAX_DATA_SIZE;
        if ((pData != nullptr) && (strlen(pData) != 0))
        {
            pDstData->StringData.uSize = (uint32_t)strlen(pData) + 1;
            if (pDstData->StringData.uSize > pDstData->StringData.uMaxSize)
            {
                pDstData->StringData.uSize = pDstData->StringData.uMaxSize;
            }
            pDstData->StringData.pStringData = (char *)MOS_AllocAndZeroMemory(strlen(pData) + 1);
            if (pDstData->StringData.pStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            MosAtomicIncrement(m_mosMemAllocFakeCounter);
            eStatus = MosSecureStrcpy(
                pDstData->StringData.pStringData,
                pDstData->StringData.uSize,
                (char *)pData);
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:

        pDstData->MultiStringData.uCount = MOS_USER_MAX_STRING_COUNT;
        pDstData->MultiStringData.uMaxSize = MOS_USER_CONTROL_MAX_DATA_SIZE;
        pDstData->MultiStringData.pStrings = (PMOS_USER_FEATURE_VALUE_STRING)MOS_AllocAndZeroMemory(sizeof(MOS_USER_FEATURE_VALUE_STRING) * __MAX_MULTI_STRING_COUNT);
        if (pDstData->MultiStringData.pStrings == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            pDstData->MultiStringData.pMultStringData = nullptr;
            pDstData->MultiStringData.uSize           = 0;
            pDstData->MultiStringData.uCount          = 0;
            return MOS_STATUS_NULL_POINTER;
        }
        MosAtomicIncrement(m_mosMemAllocFakeCounter);
        if ((pData != nullptr) && (strlen(pData) != 0))
        {
            if (!pDstData->MultiStringData.pMultStringData)
            {
                MosAtomicIncrement(m_mosMemAllocFakeCounter);
            }
            MOS_SafeFreeMemory(pDstData->MultiStringData.pMultStringData);
            pDstData->MultiStringData.pMultStringData = (char *)MOS_AllocAndZeroMemory(strlen(pData) + 1);
            if (pDstData->MultiStringData.pMultStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            eStatus = MosSecureMemcpy(
                pDstData->MultiStringData.pMultStringData,
                strlen(pData),
                (char *)pData,
                strlen(pData));
            if ((eStatus = MosUserFeatureSetMultiStringValue(
                pDstData,
                dwUFSize)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to set multi string value.");
                return eStatus;
            }
        }
        break;
    default:
        break;
    }
    return eStatus;
}
MOS_STATUS MosUtilities::MosUserFeatureSetDefaultValues(
    PMOS_USER_FEATURE_VALUE_WRITE_DATA pWriteValues,
    uint32_t                           uiNumOfValues)
{
    uint32_t                ui;
    PMOS_USER_FEATURE_VALUE pUserFeature = nullptr;
    uint32_t                ValueID      = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_STATUS              eStatus      = MOS_STATUS_UNKNOWN;

    //--------------------------------------------------
    MOS_OS_CHK_NULL_RETURN(pWriteValues);
    //--------------------------------------------------
    for (ui = 0; ui < uiNumOfValues; ui++)
    {
        ValueID = pWriteValues[ui].ValueID;
        pUserFeature = MosUtilUserInterface::GetValue(ValueID);
        MOS_OS_CHK_NULL_RETURN(pUserFeature);
        // Copy the write data into corresponding user feature value
        MosCopyUserFeatureValueData(
            &pWriteValues[ui].Value,
            &pUserFeature->Value,
            pUserFeature->ValueType);
    }
    eStatus = MOS_STATUS_SUCCESS;
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKey(PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey);
    //------------------------------

    eStatus = MosAssignUserFeatureValueData(
        &pUserFeatureKey->Value,
        pUserFeatureKey->DefaultValue,
        pUserFeatureKey->ValueType);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        MosUtilUserInterface::AddEntry(pUserFeatureKey->ValueID, pUserFeatureKey);
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureData(PMOS_USER_FEATURE_VALUE_DATA pData,MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                    ui;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    if (pData == nullptr)
    {
        return eStatus;
    }
    //------------------------------

    switch (ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        MosFreeUserFeatureValueString(&pData->StringData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        for (ui = 0; ui < pData->MultiStringData.uCount; ui++)
        {
            MosFreeUserFeatureValueString(&pData->MultiStringData.pStrings[ui]);
        }
        MOS_SafeFreeMemory(pData->MultiStringData.pStrings);
        MosAtomicDecrement(m_mosMemAllocFakeCounter);
        pData->MultiStringData.pStrings = nullptr;
        pData->MultiStringData.pMultStringData = nullptr;
        pData->MultiStringData.uSize = 0;
        pData->MultiStringData.uCount = 0;
        break;
    default:
        break;
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKey(PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    //------------------------------
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey);
    //------------------------------

    MosUtilUserInterface::DelEntry(pUserFeatureKey->ValueID);
    eStatus = MosDestroyUserFeatureData(
        &pUserFeatureKey->Value,
        pUserFeatureKey->ValueType);

    return eStatus;
}

MOS_STATUS MosUtilities::MosIsCorrectDefaultValueType(
    const char                  *pData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                    dwLen;
    uint32_t                    ui;
    int32_t                     IntVal;
    MOS_STATUS                  eStatus = MOS_STATUS_INVALID_PARAMETER;

    dwLen = (uint32_t)strlen(pData);
    //------------------------------
    MOS_OS_ASSERT(pData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------
    switch (ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        if ((!strcmp(pData, "0")) || (!strcmp(pData, "1")))
        {
            eStatus = MOS_STATUS_SUCCESS;
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        eStatus = MOS_STATUS_SUCCESS;
        for (ui = 0; ui<dwLen; ui++)
        {
            IntVal = pData[ui] - '0';
            if ((0 > IntVal) || (9 < IntVal))
            {
                if ((((ui == 0)&&(pData[ui] - '-') != 0)) && ((pData[ui] - '.') != 0))
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    break;
                }
            }
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        eStatus = MOS_STATUS_SUCCESS;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        eStatus = MOS_STATUS_SUCCESS;
        break;
    default:
        break;
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosIsCorrectUserFeatureDescField(PMOS_USER_FEATURE_VALUE pUserFeatureKey, uint32_t maxKeyID)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey);
    if ((pUserFeatureKey->ValueID <= __MOS_USER_FEATURE_KEY_INVALID_ID) ||
        (pUserFeatureKey->ValueID >= maxKeyID))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pValueName);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pcPath);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pcWritePath);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pcGroup);

    if ((pUserFeatureKey->pcDescription != nullptr) &&
        (strlen(pUserFeatureKey->pcDescription) > MAX_USER_FEATURE_FIELD_LENGTH))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    eStatus = MosIsCorrectDefaultValueType(
        pUserFeatureKey->DefaultValue,
        pUserFeatureKey->ValueType);
    return eStatus;
}

MOS_STATUS MosUtilities::MosGetItemFromMosUserFeatureDescField(
    MOS_USER_FEATURE_VALUE      *descTable,
    uint32_t                    numOfItems,
    uint32_t                    maxId,
    MOS_STATUS                  (*CallbackFunc)(PMOS_USER_FEATURE_VALUE),
    PMOS_USER_FEATURE_VALUE     pUserFeatureKeyFilter)
{
    uint32_t    uiIndex = 0;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_OS_CHK_NULL_RETURN(CallbackFunc);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKeyFilter);
    MOS_OS_CHK_NULL_RETURN(descTable);
    //------------------------------

    for (uiIndex = __MOS_USER_FEATURE_KEY_INVALID_ID; uiIndex < numOfItems; uiIndex++)
    {
        if (MosIsCorrectUserFeatureDescField(&descTable[uiIndex], maxId) != MOS_STATUS_SUCCESS)
        {
            continue;
        }

        if ((pUserFeatureKeyFilter->ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID) && (pUserFeatureKeyFilter->ValueID != descTable[uiIndex].ValueID))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pValueName != nullptr) && (strcmp(pUserFeatureKeyFilter->pValueName, descTable[uiIndex].pValueName) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcPath != nullptr) && (strcmp(pUserFeatureKeyFilter->pcPath, descTable[uiIndex].pcPath) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcWritePath != nullptr) && (strcmp(pUserFeatureKeyFilter->pcWritePath, descTable[uiIndex].pcWritePath) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcGroup != nullptr) && (strcmp(pUserFeatureKeyFilter->pcGroup, descTable[uiIndex].pcGroup) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->Type != MOS_USER_FEATURE_TYPE_INVALID) && (pUserFeatureKeyFilter->Type != descTable[uiIndex].Type))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID) && (pUserFeatureKeyFilter->ValueType != descTable[uiIndex].ValueType))
        {
            continue;
        }
        eStatus = (*CallbackFunc)(&descTable[uiIndex]);

    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId)
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    eStatus = MosGetItemFromMosUserFeatureDescField(
        descTable,
        numOfItems,
        maxId,
        &MosDeclareUserFeatureKey,
        &UserFeatureKeyFilter);
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysForAllDescFields()
{
    MOS_OS_CHK_STATUS_RETURN(MosDeclareUserFeatureKeysFromDescFields(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId)
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    eStatus = MosGetItemFromMosUserFeatureDescField(
        descTable,
        numOfItems,
        maxId,
        &MosDestroyUserFeatureKey,
        &UserFeatureKeyFilter);
    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysForAllDescFields()
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;

    MOS_OS_CHK_STATUS_RETURN(MosDestroyUserFeatureKeysFromDescFields(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueInit(uint32_t uiNumValues)
{
    // Check if memory is allocated
    if (uiNumValues == 0)
    {
        MOS_OS_ASSERTMESSAGE("pUserFeature->uiNumValues is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

void MosUtilities::MosUserFeatureCallback(
        PTP_CALLBACK_INSTANCE Instance,
        void                  *pvParameter,
        PTP_WAIT              Wait,
        TP_WAIT_RESULT        WaitResult)
{
    PMOS_USER_FEATURE_NOTIFY_DATA  pNotifyData;
    MOS_UNUSED(Instance);
    MOS_UNUSED(Wait);
    MOS_UNUSED(WaitResult);

    MOS_OS_ASSERT(pvParameter);

    pNotifyData = (PMOS_USER_FEATURE_NOTIFY_DATA)pvParameter;
    pNotifyData->bTriggered = true;
}
MOS_STATUS MosUtilities::MosUserFeatureReadValueBinary(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    void        *pvData;
    uint32_t    dwUFSize;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_BINARY);

    pvData = pFeatureValue->Value.BinaryData.pBinaryData;
    if (!pvData)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->BinaryData.pBinaryData is NULL.");
        return MOS_STATUS_NULL_POINTER;
    }

    dwUFSize = pFeatureValue->Value.BinaryData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->BinaryData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_BINARY,
                  nullptr,
                  pvData,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.BinaryData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.BinaryData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read binary user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }

    pFeatureValue->Value.BinaryData.uSize = dwUFSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFSize;
    char        pcTmpStr[MOS_USER_CONTROL_MAX_DATA_SIZE];

    //--------------------------------------------------
    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_STRING);
    //--------------------------------------------------

    MosZeroMemory(pcTmpStr, MOS_USER_CONTROL_MAX_DATA_SIZE);
    dwUFSize = pFeatureValue->Value.StringData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->StringData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_SZ,
                  nullptr,
                  pcTmpStr,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.StringData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.StringData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read single string user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }
    if (strlen(pcTmpStr) > 0)
    {
        if (!pFeatureValue->Value.StringData.pStringData)
        {
            MosAtomicIncrement(m_mosMemAllocFakeCounter);
        }

        if (pFeatureValue->Value.StringData.uSize < strlen(pcTmpStr) + 1)
        {
            pFeatureValue->Value.StringData.pStringData =
                (char *)MOS_ReallocMemory(pFeatureValue->Value.StringData.pStringData, strlen(pcTmpStr) + 1);
            pFeatureValue->Value.StringData.uSize = strlen(pcTmpStr) + 1;
        }

        MOS_OS_CHK_NULL_RETURN(pFeatureValue->Value.StringData.pStringData);

        MosZeroMemory(pFeatureValue->Value.StringData.pStringData, pFeatureValue->Value.StringData.uSize);

        MosSecureMemcpy(pFeatureValue->Value.StringData.pStringData, pFeatureValue->Value.StringData.uSize, pcTmpStr, strlen(pcTmpStr));
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueMultiString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFSize;
    char        pcTmpStr[MOS_USER_CONTROL_MAX_DATA_SIZE];

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING);

    if (!pFeatureValue->Value.MultiStringData.pStrings)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->MultiStringData.pStrings is NULL.");
        return MOS_STATUS_NULL_POINTER;
    }
    MosZeroMemory(pcTmpStr, MOS_USER_CONTROL_MAX_DATA_SIZE);
    dwUFSize = pFeatureValue->Value.MultiStringData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->MultiStringData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_MULTI_SZ,
                  nullptr,
                  pcTmpStr,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.MultiStringData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.MultiStringData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read single string user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }

    if (strlen(pcTmpStr) > 0)
    {
        if (!pFeatureValue->Value.MultiStringData.pMultStringData)
        {
            MosAtomicIncrement(m_mosMemAllocFakeCounter);
        }
        MOS_SafeFreeMemory(pFeatureValue->Value.MultiStringData.pMultStringData);
        pFeatureValue->Value.MultiStringData.pMultStringData = (char *)MOS_AllocAndZeroMemory(strlen(pcTmpStr) + 1);
        if (pFeatureValue->Value.MultiStringData.pMultStringData == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NULL_POINTER;
        }
        MosSecureMemcpy(
            pFeatureValue->Value.MultiStringData.pMultStringData,
            strlen(pcTmpStr),
            pcTmpStr,
            strlen(pcTmpStr));

    if((eStatus = MosUserFeatureSetMultiStringValue(
            &pFeatureValue->Value,
        dwUFSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set multi string value.");
        return eStatus;
    }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValuePrimitive(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFType = 0;
    uint32_t    dwUFSize;
    void        *pvData = nullptr;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);

    switch(pFeatureValue->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        dwUFType    = RRF_RT_UF_DWORD;
        dwUFSize    = sizeof(uint32_t);
        pvData      = &pFeatureValue->Value.fData;
        break;

    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        dwUFType    = RRF_RT_UF_QWORD;
        dwUFSize    = sizeof(uint64_t);
        pvData      = &pFeatureValue->Value.u64Data;
        break;

    default:
        MOS_OS_ASSERTMESSAGE("Invalid primitive value type.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  dwUFType,
                  nullptr,
                  pvData,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // This error case can be hit if the user feature key does not exist.
        MOS_OS_NORMALMESSAGE("Failed to read primitive user feature value \"%s\".", pFeatureValue->pValueName);
        return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS          eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_STRING);
    MOS_OS_ASSERT(pDataValue);

    if((eStatus = MosUserFeatureSetValueEx(
                      UFKey,
                      pFeatureValue->pValueName,
                      0,
                      UF_SZ,
                      (uint8_t*)pDataValue->StringData.pStringData,
                      pDataValue->StringData.uSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write string user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueMultiString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    PMOS_USER_FEATURE_VALUE_STRING  pStringData;
    uint8_t                         *pData;
    uint8_t                         *pCurData;
    uint32_t                        dwDataSize;
    uint32_t                        dwAvailableSize;
    uint32_t                        ui;
    MOS_STATUS                      eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING);
    MOS_OS_ASSERT(pDataValue);
    MOS_OS_ASSERT(pDataValue->MultiStringData.uCount > 0);

    pData       = nullptr;
    dwDataSize  = 0;

    for (ui = 0; ui < pDataValue->MultiStringData.uCount; ui++)
    {
        pStringData = &pDataValue->MultiStringData.pStrings[ui];
        dwDataSize += pStringData->uSize;
        dwDataSize += 1;                                                        // for \0
    }
    dwDataSize += 1;                                                            // for \0 at the very end (see MULTI_SZ spec)

    // Allocate memory to store data
    pData = (uint8_t*)MOS_AllocAndZeroMemory(dwDataSize);
    if(pData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
        return MOS_STATUS_NO_SPACE;
    }

    // Copy data from original string array
    pCurData        = pData;
    dwAvailableSize = dwDataSize;
    for (ui = 0; ui < pDataValue->MultiStringData.uCount; ui++)
    {
        pStringData = &pDataValue->MultiStringData.pStrings[ui];
        eStatus = MosSecureMemcpy(pCurData, dwAvailableSize, pStringData->pStringData, pStringData->uSize);
        if(eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to copy memory.");
            goto finish;
        }
        pCurData += pStringData->uSize;
        pCurData++;                                                             // \0 is already added since we zeroed the memory
                                                                                // Very last \0 is already added since we zeroed the memory
        dwAvailableSize -= pStringData->uSize + 1;
    }
    // Write the user feature MULTI_SZ entry
    if((eStatus = MosUserFeatureSetValueEx(
                       UFKey,
                       pFeatureValue->pValueName,
                       0,
                       UF_MULTI_SZ,
                       pData,
                       dwDataSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write multi string user feature value.");
    }

finish:
    MOS_FreeMemory(pData);
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueBinary(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS      eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_BINARY);
    MOS_OS_ASSERT(pDataValue);

    if((eStatus = MosUserFeatureSetValueEx(
                       UFKey,
                       pFeatureValue->pValueName,
                       0,
                       UF_BINARY,
                       (uint8_t*)pDataValue->BinaryData.pBinaryData,
                       pDataValue->BinaryData.uSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write binary user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuePrimitive(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFType = UF_NONE;
    uint32_t    dwUFSize = 0;
    void        *pvData = nullptr;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    MOS_OS_ASSERT(pDataValue);

    switch(pFeatureValue->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        dwUFType    = UF_DWORD;
        dwUFSize    = sizeof(uint32_t);
        pvData      = &pDataValue->fData;
        break;

    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        dwUFType    = UF_QWORD;
        dwUFSize    = sizeof(uint64_t);
        pvData      = &pDataValue->u64Data;
        break;

    default:
        MOS_OS_ASSERTMESSAGE("Invalid primitive value type.");
        return MOS_STATUS_UNKNOWN;
    }

    if((eStatus = MosUserFeatureSetValueEx(
                        UFKey,
                        pFeatureValue->pValueName,
                        0,
                        dwUFType,
                        (uint8_t*)pvData,
                        dwUFSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write primitive user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueFromMapID(
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_USER_FEATURE_KEY_PATH_INFO  *ufInfo)
{
    void                        *ufKey           = nullptr;
    PMOS_USER_FEATURE_VALUE     pUserFeature    = nullptr;
    int32_t                     iDataFlag       = MOS_USER_FEATURE_VALUE_DATA_FLAG_NONE_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_STATUS                  eStatus         = MOS_STATUS_SUCCESS;

    //--------------------------------------------------
    MOS_OS_ASSERT(pValueData);
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);
    //--------------------------------------------------
    iDataFlag = pValueData->i32DataFlag;

    pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (nullptr == pUserFeature)
    {
        MOS_OS_NORMALMESSAGE("Cannot found the user feature key.");
        return MOS_STATUS_NULL_POINTER;
    }

    // Open the user feature
    // Assigned the pUserFeature to ufKey for future reading
    ufKey = pUserFeature;
    if((eStatus = MosUserFeatureOpen(
                       pUserFeature->Type,
                       pUserFeature->pcPath,
                       KEY_READ,
                       &ufKey,
                       ufInfo)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Failed to open user feature for concurrency.");
        if ((eStatus = MosUserFeatureOpen(
                 pUserFeature->Type,
                 pUserFeature->pcPath,
                 KEY_READ,
                 &ufKey,
                 ufInfo)) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to open user feature for reading eStatus:%d.", eStatus);
            eStatus = MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
            goto finish;
        }
    }

    // Initialize Read Value
    if((eStatus = MosUserFeatureReadValueInit(pUserFeature->uiNumOfValues)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to initialize user feature read value eStatus:%d.",eStatus);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

#if !(_DEBUG || _RELEASE_INTERNAL)
    // For release build, don't read debug only keys, but return default directly
    if (pUserFeature->EffctiveRange == MOS_USER_FEATURE_EFFECT_DEBUGONLY)
    {
        eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        goto finish;
    }
#endif

    // Read the Values from user feature
    switch(pUserFeature->ValueType)
    {
       case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
           eStatus = MosUserFeatureReadValueBinary(ufKey, pUserFeature);
           break;
       case MOS_USER_FEATURE_VALUE_TYPE_STRING:
           eStatus = MosUserFeatureReadValueString(ufKey, pUserFeature);
           break;
       case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
           eStatus = MosUserFeatureReadValueMultiString(ufKey, pUserFeature);
           break;
       default:
           eStatus = MosUserFeatureReadValuePrimitive(ufKey, pUserFeature);
           break;
    }

    if(eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Failed to read value from user feature eStatus:%d.", eStatus);
        eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    }
finish:
    if ((pUserFeature != nullptr) &&
        ((eStatus == MOS_STATUS_SUCCESS) ||
        (iDataFlag != MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE)))
    {
        // Use the User Feature Value or default value in corresponding user feature key Desc Fields
        // when User Feature Key read successfully or no input custom default value
        MosCopyUserFeatureValueData(
            &pUserFeature->Value,
            pValueData,
            pUserFeature->ValueType);
    }
    MosUserFeatureCloseKey(ufKey);  // Closes the key if not nullptr
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueID(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_CONTEXT_HANDLE              mosCtx)
{
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo = Mos_GetDeviceUfPathInfo((PMOS_CONTEXT)mosCtx);
    return MosUserFeatureReadValueFromMapID(
        ValueID,
        pValueData,
        ufInfo);
}
MOS_STATUS MosUtilities::MosUserFeatureReadValueID(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo)
{
    return MosUserFeatureReadValueFromMapID(
        ValueID,
        pValueData,
        ufInfo);
}

const char* MosUtilities::MosUserFeatureLookupValueName(uint32_t ValueID)
{
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);

    PMOS_USER_FEATURE_VALUE pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (pUserFeature)
    {
        return pUserFeature->pValueName;
    }
    else
    {
        return nullptr;
    }
}

const char *MosUtilities::MosUserFeatureLookupReadPath(uint32_t ValueID)
{
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);

    PMOS_USER_FEATURE_VALUE pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (pUserFeature)
    {
        return pUserFeature->pcPath;
    }
    else
    {
        return nullptr;
    }
}

const char *MosUtilities::MosUserFeatureLookupWritePath(uint32_t ValueID)
{
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);

    PMOS_USER_FEATURE_VALUE pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (pUserFeature)
    {
        return pUserFeature->pcWritePath;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesTblID(
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues,
    MOS_USER_FEATURE_KEY_PATH_INFO          *ufInfo)
{
    uint32_t                            ui;
    PMOS_USER_FEATURE_VALUE             pFeatureValue      = nullptr;
    void                                *UFKey              = nullptr;
    PMOS_USER_FEATURE_VALUE_WRITE_DATA  pUserWriteData     = nullptr;
    PMOS_USER_FEATURE_VALUE             pUserFeature       = nullptr;
    uint32_t                            ValueID            = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_STATUS                          eStatus            = MOS_STATUS_SUCCESS;
    char                                WritePathWithPID[MAX_PATH];
    int32_t                             pid;
    uint64_t                            ulTraceData         = 0;
    bool                                isValid             = false;

    //--------------------------------------------------
    MOS_OS_CHK_NULL_RETURN(pWriteValues);
    //--------------------------------------------------

    MosZeroMemory(WritePathWithPID, MAX_PATH);

    pid = MosGetPid();

    for (ui = 0; ui < uiNumOfValues; ui++)
    {
        ValueID = pWriteValues[ui].ValueID;

        pUserFeature = MosUtilUserInterface::GetValue(ValueID);

        MOS_OS_CHK_NULL_RETURN(pUserFeature);
        // Open the user feature
        // Assigned the pUserFeature to UFKey for future reading
        UFKey = pUserFeature;

        //append write path with pid
        sprintf_s(WritePathWithPID, MAX_PATH, "%s\\%d", pUserFeature->pcWritePath, pid);

        // Trace data in case opening user feature for write fails
        switch (pUserFeature->ValueType)
        {
            case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
            case MOS_USER_FEATURE_VALUE_TYPE_INT32:
            case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
            case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
                ulTraceData = pWriteValues[ui].Value.u32Data;
                isValid = true;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_INT64:
            case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
                ulTraceData = pWriteValues[ui].Value.u64Data;
                isValid = true;
                break;
            default:
                MOS_OS_NORMALMESSAGE("Unknown value type %d", pUserFeature->ValueType);
        }

        if (isValid)
        {
            MosTraceDataDictionary(pUserFeature->pValueName, &ulTraceData, sizeof(ulTraceData));
        }

        //try to open Write path with pid first
        if ((eStatus = MosUserFeatureOpen(
                 pUserFeature->Type,
                 WritePathWithPID,
                 KEY_WRITE,
                 &UFKey,
                 ufInfo)) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to open user feature for concurrency.");
            if ((eStatus = MosUserFeatureOpen(
                     pUserFeature->Type,
                     pUserFeature->pcWritePath,
                     KEY_WRITE,
                     &UFKey,
                     ufInfo)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_NORMALMESSAGE("Failed to open user feature for writing.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
                goto finish;
            }
        }

        //------------------------------------
        MOS_OS_ASSERT(pUserFeature->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
        //------------------------------------
        switch(pUserFeature->ValueType)
        {
        case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
            if ((eStatus = MosUserFeatureWriteValueBinary(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write binary value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_STRING:
            if ((eStatus = MosUserFeatureWriteValueString(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write string value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
            if ((eStatus = MosUserFeatureWriteValueMultiString(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write multi string value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        default:
            if ((eStatus = MosUserFeatureWriteValuePrimitive(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write primitive data value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
        }
        MosUserFeatureCloseKey(UFKey);      // Closes the key if not nullptr
    }

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MosUserFeatureCloseKey(UFKey);      // Closes the key if not nullptr
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesID(
    PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues,
    MOS_CONTEXT_HANDLE                      mosCtx)
{
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo = Mos_GetDeviceUfPathInfo((PMOS_CONTEXT)mosCtx);

    return MosUserFeatureWriteValuesTblID(
        pWriteValues,
        uiNumOfValues,
        ufInfo);
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesID(
    PMOS_USER_FEATURE_INTERFACE        pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA pWriteValues,
    uint32_t                           uiNumOfValues,
    MOS_USER_FEATURE_KEY_PATH_INFO *   ufInfo)
{
    return MosUserFeatureWriteValuesTblID(
        pWriteValues,
        uiNumOfValues,
        ufInfo);
}

