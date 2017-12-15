/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_hal_dump.cpp  
//! \brief     Functions related to dump generated hw commands/curbe data  
//!

#include "cm_hal.h"
#include "renderhal_platform_interface.h"

#if (MDF_COMMAND_BUFFER_DUMP || MDF_CURBE_DATA_DUMP)
#if defined(ANDROID) || defined(LINUX)
#define PlatformSNPrintf snprintf
#define PLATFORM_DIR_SEPERATOR   "/"
#else
#define PlatformSNPrintf sprintf_s
#define PLATFORM_DIR_SEPERATOR   "\\"
#endif

#define SIZE_OF_DWORD_PLUS_ONE                  (2*sizeof(uint32_t) +1)
//!
//! \brief    Dump Hex Dword to dest buffer
//! \param    [in] pDestBuf
//!           dest buffer
//! \param    [in] buflen
//!           length of buffer
//! \param    [in] pSrcBuf
//!           pointer to surface of buffer
//! \param    [in] uNum
//!           number of dword to dump
//! \return   number of bytes written
//!
static uint32_t HalCm_CopyHexDwordLine(char  *pDestBuf, size_t buflen, uint32_t *pSrcBuf, uint32_t uNum)
{
    uint32_t dwBytesWritten = 0;
    for (uint32_t i = 0; i < uNum; ++i) {
        dwBytesWritten += PlatformSNPrintf(pDestBuf + dwBytesWritten, buflen - dwBytesWritten, "%08x ", pSrcBuf[i]);
    }
    dwBytesWritten += PlatformSNPrintf(pDestBuf + dwBytesWritten, buflen - dwBytesWritten, "\n");

    return dwBytesWritten;
}
#endif


#if MDF_COMMAND_BUFFER_DUMP

#define HALCM_COMMAND_BUFFER_OUTPUT_DIR         "HALCM_Command_Buffer_Dumps"
#define HALCM_COMMAND_BUFFER_OUTPUT_FILE        "Command_Buffer"


//!
//! \brief    Read Register key to check if dump flag enabled
//! \param    [in] pState
//!           Pointer to cm hal state
//! \return   CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_InitDumpCommandBuffer(PCM_HAL_STATE pState)
{
    MOS_USER_FEATURE        UserFeature;
    char                    sFileName[MOS_MAX_HLT_FILENAME_LEN];
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  UserFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(pState);

    MOS_ZeroMemory(&UserFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&UserFeature, sizeof(UserFeature));

    // Check if command buffer dump was enabled in user feature settings.
    UserFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues = &UserFeatureValue;
    UserFeature.uiNumValues = 1;
    UserFeatureValue.bData = false;

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &UserFeature,
        "Dump Command Buffer Enable",
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read command buffer user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (UserFeatureValue.bData)
    {
        PlatformSNPrintf(sFileName, MOS_MAX_HLT_FILENAME_LEN, HALCM_COMMAND_BUFFER_OUTPUT_DIR);
        eStatus = MOS_CreateDirectory(sFileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create output directory. Status = %d", eStatus);
            goto finish;
        }
        // Setup member function and variable.
        pState->bDumpCommandBuffer = UserFeatureValue.bData?true: false;
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}



//!
//! \brief    Dump command buffer to file
//! \param    [in] pState
//!           pointer to cm hal state
//! \param    [in] pCmdBuffer
//!           pointer to command buffer
//! \param    [in] offsetSurfaceState
//!           offset to surface state
//! \param    [in] sizeOfSurfaceState
//!           size of surface state
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_DumpCommadBuffer(PCM_HAL_STATE pState, PMOS_COMMAND_BUFFER pCmdBuffer, int offsetSurfaceState, size_t sizeOfSurfaceState)
{
    static uint32_t dwCommandBufferNumber = 0;
    int32_t         hr = CM_FAILURE;
    MOS_STATUS      eStatus = MOS_STATUS_UNKNOWN;
    char            *pOutputBuffer = nullptr;
    // Each hex value should have 9 chars.
    uint32_t        dwBytesWritten = 0;
    uint32_t        dwNumberOfDwords = 0;
    uint32_t        dwSizeToAllocate = 0;
    char            sFileName[MOS_MAX_HLT_FILENAME_LEN];

    PMOS_INTERFACE pOsInterface = pState->pOsInterface;
    PRENDERHAL_STATE_HEAP pStateHeap   = pState->pRenderHal->pStateHeap;

    MOS_OS_ASSERT(pState);
    MOS_OS_ASSERT(pCmdBuffer);

    // Set the file name.
    PlatformSNPrintf(sFileName, MOS_MAX_HLT_FILENAME_LEN, HALCM_COMMAND_BUFFER_OUTPUT_DIR);
    PlatformSNPrintf(sFileName + strlen(sFileName), MOS_MAX_HLT_FILENAME_LEN - strlen(sFileName), PLATFORM_DIR_SEPERATOR);
    PlatformSNPrintf(sFileName + strlen(sFileName), MOS_MAX_HLT_FILENAME_LEN - strlen(sFileName), "%s_Pid%d_Tid%ld_%d.txt", HALCM_COMMAND_BUFFER_OUTPUT_FILE, CmGetCurProcessId(), CmGetCurThreadId(), dwCommandBufferNumber);

    dwNumberOfDwords = pCmdBuffer->iOffset / sizeof(uint32_t);

    dwSizeToAllocate = dwNumberOfDwords * (SIZE_OF_DWORD_PLUS_ONE) + 2 +   //length of command buffer line 
        pStateHeap->iCurrentSurfaceState * 
        (SIZE_OF_DWORD_PLUS_ONE * pState->pRenderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize() / sizeof(uint32_t) + 2); //length of surface state lines

    // Alloc output buffer.
    pOutputBuffer = (char *)MOS_AllocAndZeroMemory(dwSizeToAllocate);
    if (!pOutputBuffer) {
        MOS_OS_NORMALMESSAGE("Failed to allocate memory for command buffer dump");
        return MOS_STATUS_NO_SPACE;
    }
    // write command buffer dwords.
    dwBytesWritten += HalCm_CopyHexDwordLine(pOutputBuffer, dwSizeToAllocate - dwBytesWritten, (uint32_t *)pCmdBuffer->pCmdBase, dwNumberOfDwords);

    //write all surface states 
    for (int32_t dwIndex = 0; dwIndex < pStateHeap->iCurrentSurfaceState; ++dwIndex) {
        PRENDERHAL_SURFACE_STATE_ENTRY pEntry = pStateHeap->pSurfaceEntry + dwIndex;
        void *pSurfaceState = (char*)pEntry->pSurfaceState;
        //the address of surface states are 32bit or uint32_t aligned.
        dwBytesWritten += HalCm_CopyHexDwordLine(pOutputBuffer + dwBytesWritten, dwSizeToAllocate - dwBytesWritten, (uint32_t*)pSurfaceState, sizeOfSurfaceState / sizeof(uint32_t));
    }

    MOS_OS_CHK_STATUS(MOS_WriteFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));

    dwCommandBufferNumber++;

    hr = CM_SUCCESS;

finish:
    // Free the memory.
    if (pOutputBuffer)
    {
        MOS_FreeMemAndSetNull(pOutputBuffer);
    }

    return hr;

}

#endif


#if MDF_CURBE_DATA_DUMP

#define HALCM_CURBE_DATA_OUTPUT_DIR         "HALCM_Curbe_Data_Dumps"
#define HALCM_CURBE_DATA_OUTPUT_FILE        "Curbe_Data"

//!
//! \brief    Read Register key to check if Curbe Data dump flag enabled
//! \param    [in] pState
//!           Pointer to cm hal state
//! \return   CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_InitDumpCurbeData(PCM_HAL_STATE pState)
{
    MOS_USER_FEATURE        UserFeature;
    char                    sFileName[MOS_MAX_HLT_FILENAME_LEN];
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  UserFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(pState);

    MOS_ZeroMemory(&UserFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&UserFeature, sizeof(UserFeature));

    // Check if curbe data dump was enabled in user feature settings.
    UserFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues = &UserFeatureValue;
    UserFeature.uiNumValues = 1;
    UserFeatureValue.bData = false;

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_CURBE_DUMP_ENABLE,
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read curbe data dump user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (UserFeatureValue.bData)
    {
        PlatformSNPrintf(sFileName, MOS_MAX_HLT_FILENAME_LEN, HALCM_CURBE_DATA_OUTPUT_DIR);
        eStatus = MOS_CreateDirectory(sFileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create curbe data output directory. Status = %d", eStatus);
            goto finish;
        }
        // Setup member function and variable.
        pState->bDumpCurbeData = UserFeatureValue.bData ? true: false;
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}

//!
//! \brief    Dump Curbe Data to file
//! \param    [in] pState
//!           pointer to cm hal state
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_DumpCurbeData(PCM_HAL_STATE pState)
{
    static uint32_t dwCurbeDataNumber = 0;
    int32_t         hr = CM_FAILURE;
    MOS_STATUS      eStatus = MOS_STATUS_UNKNOWN;
    char            *pOutputBuffer = nullptr;
    uint32_t        dwBytesWritten = 0;
    char            sFileName[MOS_MAX_HLT_FILENAME_LEN];
    uint32_t        dwNumberOfDwords = 0;
    uint32_t        dwSizeToAllocate = 0;
    PMOS_INTERFACE pOsInterface = pState->pOsInterface;
    PRENDERHAL_STATE_HEAP pStateHeap = pState->pRenderHal->pStateHeap;

    MOS_OS_ASSERT(pState);

    // Set the file name.
    PlatformSNPrintf(sFileName, MOS_MAX_HLT_FILENAME_LEN, HALCM_CURBE_DATA_OUTPUT_DIR);
    PlatformSNPrintf(sFileName + strlen(sFileName), MOS_MAX_HLT_FILENAME_LEN - strlen(sFileName), PLATFORM_DIR_SEPERATOR);
    PlatformSNPrintf(sFileName + strlen(sFileName), MOS_MAX_HLT_FILENAME_LEN - strlen(sFileName), "%s_Pid%d_Tid%ld_%d.txt", HALCM_CURBE_DATA_OUTPUT_FILE, CmGetCurProcessId(), CmGetCurThreadId(), dwCurbeDataNumber);

    // write curbe data dwords.
    if (pState->bDynamicStateHeap)
    {
        dwNumberOfDwords = pStateHeap->pCurMediaState->pDynamicState->Curbe.dwSize;
        dwSizeToAllocate = dwNumberOfDwords*SIZE_OF_DWORD_PLUS_ONE+2;
        pOutputBuffer = (char *)MOS_AllocAndZeroMemory(dwSizeToAllocate);
        dwBytesWritten += HalCm_CopyHexDwordLine(pOutputBuffer, 
                          dwSizeToAllocate - dwBytesWritten,
                          (uint32_t*)pStateHeap->pCurMediaState->pDynamicState->pMemoryBlock->pDataPtr + pStateHeap->pCurMediaState->pDynamicState->Curbe.dwOffset,
                          dwNumberOfDwords);
    }
    else
    {
        dwNumberOfDwords = pStateHeap->pCurMediaState->iCurbeOffset;
        dwSizeToAllocate = dwNumberOfDwords*SIZE_OF_DWORD_PLUS_ONE+2;
        pOutputBuffer = (char *)MOS_AllocAndZeroMemory(dwSizeToAllocate);
        dwBytesWritten += HalCm_CopyHexDwordLine(pOutputBuffer,
                          dwSizeToAllocate - dwBytesWritten,
                          (uint32_t*)(pStateHeap->pGshBuffer + pStateHeap->pCurMediaState->dwOffset + pStateHeap->dwOffsetCurbe),
                          dwNumberOfDwords);
    }

    MOS_OS_CHK_STATUS(MOS_WriteFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));

    dwCurbeDataNumber++;

    hr = CM_SUCCESS;

finish:
    // Free the memory.
    if (pOutputBuffer)
    {
        MOS_FreeMemAndSetNull(pOutputBuffer);
    }

    return hr;

}

#endif

#if MDF_SURFACE_CONTENT_DUMP

//!
//! \brief    Read Register key to check if Surface content flag enabled
//! \param    [in] pState
//!           Pointer to cm hal state
//! \return   CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_InitSurfaceDump(PCM_HAL_STATE pState)
{
    MOS_USER_FEATURE        UserFeature;
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  UserFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(pState);

    MOS_ZeroMemory(&UserFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&UserFeature, sizeof(UserFeature));

    // Check if surface content dump was enabled in user feature settings.
    UserFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues = &UserFeatureValue;
    UserFeature.uiNumValues = 1;
    UserFeatureValue.bData = false;

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_DUMP_ENABLE,
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read surface content dump user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (UserFeatureValue.bData)
    {
        // Setup member function and variable.
        pState->bDumpSurfaceContent = UserFeatureValue.bData ? true: false;
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}

#endif
