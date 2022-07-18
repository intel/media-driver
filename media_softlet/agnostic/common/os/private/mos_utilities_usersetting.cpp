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

#include "mos_utilities.h"

/*****************************************************************************
|
|                           USER FEATURE Functions
|
*****************************************************************************/
void MosUtilities::MosFreeUserFeatureValueString(PMOS_USER_FEATURE_VALUE_STRING pUserString)
{
    return;
}

MOS_STATUS MosUtilities::MosWriteOneUserFeatureKeyToXML(PMOS_USER_FEATURE_VALUE pUserFeature)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosWriteOneUserFeatureGroupToXML(MOS_USER_FEATURE_VALUE   UserFeatureFilter)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosGenerateUserFeatureKeyXML(MOS_CONTEXT_HANDLE mosCtx)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureSetMultiStringValue(
    PMOS_USER_FEATURE_VALUE_DATA     pFeatureData,
    uint32_t                         dwSize)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosCopyUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA pSrcData,
    PMOS_USER_FEATURE_VALUE_DATA pDstData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosAssignUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA    pDstData,
    const char                      *pData,
    MOS_USER_FEATURE_VALUE_TYPE     ValueType
)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureSetDefaultValues(
    PMOS_USER_FEATURE_VALUE_WRITE_DATA pWriteValues,
    uint32_t                           uiNumOfValues)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKey(PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureData(PMOS_USER_FEATURE_VALUE_DATA pData,MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKey(PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosIsCorrectDefaultValueType(
    const char                  *pData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosIsCorrectUserFeatureDescField(PMOS_USER_FEATURE_VALUE pUserFeatureKey, uint32_t maxKeyID)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosGetItemFromMosUserFeatureDescField(
    MOS_USER_FEATURE_VALUE      *descTable,
    uint32_t                    numOfItems,
    uint32_t                    maxId,
    MOS_STATUS                  (*CallbackFunc)(PMOS_USER_FEATURE_VALUE),
    PMOS_USER_FEATURE_VALUE     pUserFeatureKeyFilter)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysForAllDescFields()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysForAllDescFields()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueInit(uint32_t uiNumValues)
{
    return MOS_STATUS_SUCCESS;
}

void MosUtilities::MosUserFeatureCallback(
        PTP_CALLBACK_INSTANCE Instance,
        void                  *pvParameter,
        PTP_WAIT              Wait,
        TP_WAIT_RESULT        WaitResult)
{
    return;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueBinary(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueMultiString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValuePrimitive(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueMultiString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueBinary(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuePrimitive(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueFromMapID(
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_USER_FEATURE_KEY_PATH_INFO  *ufInfo)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueID(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_CONTEXT_HANDLE              mosCtx)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueID(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo)
{
    return MOS_STATUS_SUCCESS;
}

const char* MosUtilities::MosUserFeatureLookupValueName(uint32_t ValueID)
{
    return nullptr;
}

const char *MosUtilities::MosUserFeatureLookupReadPath(uint32_t ValueID)
{
    return nullptr;
}

const char *MosUtilities::MosUserFeatureLookupWritePath(uint32_t ValueID)
{
    return nullptr;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesTblID(
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues,
    MOS_USER_FEATURE_KEY_PATH_INFO          *ufInfo)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesID(
    PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues,
    MOS_CONTEXT_HANDLE                      mosCtx)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesID(
    PMOS_USER_FEATURE_INTERFACE        pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA pWriteValues,
    uint32_t                           uiNumOfValues,
    MOS_USER_FEATURE_KEY_PATH_INFO *   ufInfo)
{
    return MOS_STATUS_SUCCESS;
}

