/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     mos_utilities_inner.cpp
//! \brief    Common OS service across different platform, mos only
//! \details  Common OS service across different platform, mos only
//!

#include "mos_os.h"

bool                 MosUtilities::m_enableAddressDump = false;

MOS_STATUS MosUtilities::MosUtilitiesInit(MediaUserSettingSharedPtr userSettingPtr)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_FUNCTION_ENTER;

    eStatus = MosOsUtilitiesInit(userSettingPtr);

#if (_DEBUG || _RELEASE_INTERNAL)
    //Initialize MOS simulate random alloc memorflag
#if COMMON_DLL_SEPARATION_SUPPORT
    MosInitAllocFailSimulateFlagInCommon(userSettingPtr);
#else
    MosInitAllocMemoryFailSimulateFlag(userSettingPtr);
#endif

    eStatus = ReadUserSettingForDebug(
        userSettingPtr,
        MosUtilities::m_enableAddressDump,
        "Resource Addr Dump Enable",
        MediaUserSetting::Group::Device);
#endif

    return eStatus;
}

MOS_STATUS MosUtilities::MosUtilitiesClose(MediaUserSettingSharedPtr userSettingPtr)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    // MOS_OS_Utilitlies_Close must be called right before end of function
    // Because Memninja will calc mem leak here.
    // Any memory allocation release after MosOsUtilitiesClose() will be treated as mem leak.
    eStatus = MosOsUtilitiesClose(userSettingPtr);

#if (_DEBUG || _RELEASE_INTERNAL)
    //Reset Simulate Alloc Memory Fail flags
#if COMMON_DLL_SEPARATION_SUPPORT
    MosInitAllocFailSimulateFlagInCommon(userSettingPtr);
#else
    MosInitAllocMemoryFailSimulateFlag(userSettingPtr);
#endif
#endif

    return eStatus;
}

/*****************************************************************************
|
|                           USER FEATURE Functions
|
*****************************************************************************/

MOS_STATUS MosUtilities::MosUserFeatureOpen(
    MOS_USER_FEATURE_TYPE           KeyType,
    const char                     *pSubKey,
    uint32_t                        dwAccess,
    void                          **pUFKey,
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo)
{
    MOS_STATUS eStatus;
    void      *RootKey = 0;

    MOS_OS_ASSERT(pSubKey);
    MOS_OS_ASSERT(pUFKey);

    if (KeyType == MOS_USER_FEATURE_TYPE_USER)
    {
        RootKey = (void *)UFKEY_INTERNAL;
    }
    else if (KeyType == MOS_USER_FEATURE_TYPE_SYSTEM)
    {
        RootKey = (void *)UFKEY_EXTERNAL;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Invalid Key Type %d.", KeyType);
        return MOS_STATUS_UNKNOWN;
    }

    if ((eStatus = MosUserFeatureOpenKey(
             RootKey,
             pSubKey,
             0,
             dwAccess,
             pUFKey,
             ufInfo)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to open user feature key %s.", pSubKey);
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureEnableNotification(
    PMOS_USER_FEATURE_INTERFACE   pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA pNotification,
    MOS_CONTEXT_HANDLE            mosCtx)
{
    PMOS_USER_FEATURE_NOTIFY_DATA_COMMON pNotifyCommon;
    int32_t                              bResult;
    MOS_STATUS                           eStatus;
    MOS_UNUSED(pOsUserFeatureInterface);

    //---------------------------------------
    MOS_OS_ASSERT(pNotification);
    MOS_OS_ASSERT(pNotification->NotifyType != MOS_USER_FEATURE_NOTIFY_TYPE_INVALID);
    MOS_OS_ASSERT(pNotification->pPath);
    //---------------------------------------

    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo = Mos_GetDeviceUfPathInfo((PMOS_CONTEXT)mosCtx);

    // Reset the triggered flag
    pNotification->bTriggered = false;

    if (pNotification->pHandle == nullptr)
    {
        // Allocate private data as well
        pNotification->pHandle = MOS_AllocAndZeroMemory(sizeof(MOS_USER_FEATURE_NOTIFY_DATA));
        if (pNotification->pHandle == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NO_SPACE;
        }
    }
    pNotifyCommon = (PMOS_USER_FEATURE_NOTIFY_DATA_COMMON)pNotification->pHandle;

    // Open User Feature for Reading
    if (pNotifyCommon->UFKey == 0)
    {
        if ((eStatus = MosUserFeatureOpen(
                 pNotification->Type,
                 pNotification->pPath,
                 KEY_READ,
                 &pNotifyCommon->UFKey,
                 ufInfo)) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to open user feature for reading.");
            return MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
        }
    }

    // Create Event for notification
    if (pNotifyCommon->hEvent == nullptr)
    {
        pNotifyCommon->hEvent = MosCreateEventEx(
            nullptr,
            nullptr,
            0);
        if (pNotifyCommon->hEvent == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NO_SPACE;
        }
    }

    // Unregister wait event if already registered
    if (pNotifyCommon->hWaitEvent)
    {
        if ((bResult = MosUnregisterWaitEx(pNotifyCommon->hWaitEvent)) == false)
        {
            MOS_OS_ASSERTMESSAGE("Unable to unregiser wait event.");
            return MOS_STATUS_EVENT_WAIT_UNREGISTER_FAILED;
        }
        pNotifyCommon->hWaitEvent = nullptr;
    }

    // Register a Callback
    if ((eStatus = MosUserFeatureNotifyChangeKeyValue(
             pNotifyCommon->UFKey,
             false,
             pNotifyCommon->hEvent,
             true)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Unable to setup user feature key notification.");
        return MOS_STATUS_UNKNOWN;
    }

    // Create a wait object
    if ((bResult = MosUserFeatureWaitForSingleObject(
             &pNotifyCommon->hWaitEvent,
             pNotifyCommon->hEvent,
             (void *)MosUserFeatureCallback,
             pNotification)) == false)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create a wait object.");
        return MOS_STATUS_EVENT_WAIT_REGISTER_FAILED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureDisableNotification(
    PMOS_USER_FEATURE_INTERFACE   pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA pNotification)
{
    PMOS_USER_FEATURE_NOTIFY_DATA_COMMON pNotifyDataCommon;
    int32_t                              bResult;
    MOS_STATUS                           eStatus;
    MOS_UNUSED(pOsUserFeatureInterface);

    //---------------------------------------
    MOS_OS_ASSERT(pNotification);
    //---------------------------------------

    if (pNotification->pHandle)
    {
        pNotifyDataCommon = (PMOS_USER_FEATURE_NOTIFY_DATA_COMMON)
                                pNotification->pHandle;

        if (pNotifyDataCommon->hWaitEvent)
        {
            if ((bResult = MosUnregisterWaitEx(pNotifyDataCommon->hWaitEvent)) == false)
            {
                MOS_OS_ASSERTMESSAGE("Unable to unregiser wait event.");
                return MOS_STATUS_EVENT_WAIT_UNREGISTER_FAILED;
            }
        }
        if (pNotifyDataCommon->UFKey)
        {
            if ((eStatus = MosUserFeatureCloseKey(pNotifyDataCommon->UFKey)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("User feature key close failed.");
                return eStatus;
            }
        }
        if (pNotifyDataCommon->hEvent)
        {
            MosCloseHandle(pNotifyDataCommon->hEvent);
        }

        // Free Notify Data Memory
        MOS_FreeMemory(pNotifyDataCommon);
        pNotification->pHandle = nullptr;
    }
    return MOS_STATUS_SUCCESS;
}

const uint32_t MosUtilities::GetRegAccessDataType(MOS_USER_FEATURE_VALUE_TYPE type)
{
    switch (type)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        return UF_DWORD;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        return UF_QWORD;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        return UF_SZ;
    default:
        return UF_NONE;
    }
}

MOS_STATUS MosUtilities::StrToMediaUserSettingValue(
    std::string                &strValue,
    MOS_USER_FEATURE_VALUE_TYPE type,
    MediaUserSetting::Value    &dstValue)
{
    uint32_t base = 10;
    if (strValue.size() > 2 && strValue.at(0) == '0' && (strValue.at(0) == 'x' || strValue.at(0) == 'X'))  // 0x or 0X
    {
        base = 16;
    }
    switch (type)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL: {
        bool value = (std::stoul(strValue, nullptr, base) != 0) ? true : false;
        dstValue   = value;
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT: {
        float value = std::stof(strValue, nullptr);
        dstValue    = value;
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32: {
        uint32_t value = std::stoul(strValue, nullptr, base);
        dstValue       = value;
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_INT32: {
        int32_t value = std::stoi(strValue, nullptr, base);
        dstValue      = value;
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64: {
        uint64_t value = std::stoull(strValue, nullptr, base);
        dstValue       = value;
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_INT64: {
        int64_t value = std::stoll(strValue, nullptr, base);
        dstValue      = value;
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
    case MOS_USER_FEATURE_VALUE_TYPE_STRING: {
        dstValue = strValue;
        break;
    }
    default:
        MOS_OS_NORMALMESSAGE("Invalid data type");
        return MOS_STATUS_UNKNOWN;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::DataToMediaUserSettingValue(
    uint8_t                    *data,
    size_t                      dataSize,
    MediaUserSetting::Value    &dstValue,
    MOS_USER_FEATURE_VALUE_TYPE type)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(data);

    auto sizeIsValid = [&](size_t requiredSize) -> bool {
        if (requiredSize > dataSize)
        {
            status = MOS_STATUS_INVALID_PARAMETER;
            return false;
        }
        return true;
    };

    switch (type)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL: {
        if (sizeIsValid(sizeof(uint8_t)))
        {
            dstValue = (*data > 0) ? true : false;
        }
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT: {
        if (sizeIsValid(sizeof(float)))
        {
            dstValue = *((float *)data);
        }
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32: {
        if (sizeIsValid(sizeof(uint32_t)))
        {
            dstValue = *((uint32_t *)data);
        }
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_INT32: {
        if (sizeIsValid(sizeof(int32_t)))
        {
            dstValue = *((int32_t *)data);
        }
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64: {
        if (sizeIsValid(sizeof(uint64_t)))
        {
            dstValue = *((uint64_t *)data);
        }
        break;
    }
    case MOS_USER_FEATURE_VALUE_TYPE_INT64: {
        if (sizeIsValid(sizeof(int64_t)))
        {
            dstValue = *((int64_t *)data);
        }
        break;
    }
    default:
        MOS_OS_NORMALMESSAGE("Invalid data type");
        return MOS_STATUS_UNKNOWN;
    }
    return status;
}

/*****************************************************************************
|
|                           Export Functions
|
*****************************************************************************/
MOS_FUNC_EXPORT void MosUtilities::MosSetUltFlag(uint8_t ultFlag)
{
    if (MosUtilities::m_mosUltFlag != nullptr)
    {
        *MosUtilities::m_mosUltFlag = ultFlag;
    }
}
MOS_FUNC_EXPORT int32_t MosUtilities::MosGetMemNinjaCounter()
{
    return m_mosMemAllocCounterNoUserFeature;
}

MOS_FUNC_EXPORT int32_t MosUtilities::MosGetMemNinjaCounterGfx()
{
    return m_mosMemAllocCounterNoUserFeatureGfx;
}

#ifdef __cplusplus
extern "C" {
#endif

    MOS_FUNC_EXPORT int32_t MOS_GetMemNinjaCounter()
    {
        return MosUtilities::MosGetMemNinjaCounter();
    }

    MOS_FUNC_EXPORT int32_t MOS_GetMemNinjaCounterGfx()
    {
        return MosUtilities::MosGetMemNinjaCounterGfx();
    }

    MOS_FUNC_EXPORT void MOS_SetUltFlag(uint8_t ultFlag)
    {
        MosUtilities::MosSetUltFlag(ultFlag);
    }

#ifdef __cplusplus
}
#endif