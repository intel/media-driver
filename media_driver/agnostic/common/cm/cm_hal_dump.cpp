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

#if (MDF_COMMAND_BUFFER_DUMP || MDF_CURBE_DATA_DUMP || MDF_SURFACE_STATE_DUMP)
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
//! \param    [in] destBuf
//!           dest buffer
//! \param    [in] buflen
//!           length of buffer
//! \param    [in] srcBuf
//!           pointer to surface of buffer
//! \param    [in] uNum
//!           number of dword to dump
//! \return   number of bytes written
//!
static uint32_t HalCm_CopyHexDwordLine(char  *destBuf, size_t buflen, uint32_t *srcBuf, uint32_t uNum)
{
    uint32_t bytesWritten = 0;
    for (uint32_t i = 0; i < uNum; ++i) {
        bytesWritten += PlatformSNPrintf(destBuf + bytesWritten, buflen - bytesWritten, "%08x ", srcBuf[i]);
    }
    bytesWritten += PlatformSNPrintf(destBuf + bytesWritten, buflen - bytesWritten, "\n");

    return bytesWritten;
}

//!
//! \brief    Get dump file name and counter
//! \param    [in] fileNamePrefix[]
//!           dump file name
//! \param    [in] timeStampFlag
//!           flag to control if need time stamp
//! \param    [in] counter
//!           dump file counter
//! \param    [in] pValueName
//!           pointer to value name, used for user feature read
//! \param    [in] outputDir
//!           pointer to output dir
//! \param    [in] outputFile
//!           pointer output file prefix
//! \return   number of bytes written
//!
int32_t GetFileNameAndCounter(char fileNamePrefix[], bool timeStampFlag, int32_t counter,
                              const char *pValueName, const char *outputDir,const char *outputFile)
{
    GetLogFileLocation(outputDir, fileNamePrefix);
    PlatformSNPrintf(fileNamePrefix + strlen(fileNamePrefix), 
                     MOS_MAX_HLT_FILENAME_LEN - strlen(fileNamePrefix), PLATFORM_DIR_SEPERATOR);

    if (timeStampFlag)
    {
        SYSTEMTIME systime;
        GetLocalTime(&systime);
        PlatformSNPrintf(fileNamePrefix + strlen(fileNamePrefix), 
                        MOS_MAX_HLT_FILENAME_LEN - strlen(fileNamePrefix), "%s_%d_%d_%d_%d_%d_%d_%d.txt",
                        outputFile, systime.wMonth, systime.wDay, systime.wHour, 
                        systime.wMinute, systime.wSecond, systime.wMilliseconds, counter);
    }
    else
    {
        //check if command buffer or surface state counter, and get it
        if (!strcmp(outputFile,"Command_Buffer"))
        {
            counter = GetCommandBufferDumpCounter(__MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_COUNTER);
        }
        else if (!strcmp(outputFile,"Surface_State_Dump"))
        {
            counter = GetSurfaceStateDumpCounter(__MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_COUNTER);
        }
        else if (!strcmp(outputFile, "Interface_Descriptor_Data_Dump"))
        {
            counter = GetInterfaceDescriptorDataDumpCounter(__MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_COUNTER);
        }
       
        PlatformSNPrintf(fileNamePrefix + strlen(fileNamePrefix), 
                         MOS_MAX_HLT_FILENAME_LEN - strlen(fileNamePrefix), "%s_%d.txt", outputFile, counter);
    }
    return counter;

}

#endif

#if MDF_COMMAND_BUFFER_DUMP

#define HALCM_COMMAND_BUFFER_OUTPUT_DIR         "HALCM_Command_Buffer_Dumps"
#define HALCM_COMMAND_BUFFER_OUTPUT_FILE        "Command_Buffer"

//!
//! \brief    Read Register key to check if dump flag enabled
//! \param    [in] state
//!           Pointer to cm hal state
//! \return   CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_InitDumpCommandBuffer(PCM_HAL_STATE state)
{
    MOS_USER_FEATURE        userFeature;
    char                    fileName[MOS_MAX_HLT_FILENAME_LEN];
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  userFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(state);

    MOS_ZeroMemory(&userFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&userFeature, sizeof(userFeature));

    // Check if command buffer dump was enabled in user feature settings.
    userFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    userFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    userFeature.pValues = &userFeatureValue;
    userFeature.uiNumValues = 1;
    userFeatureValue.bData = false;

    GetLogFileLocation(HALCM_COMMAND_BUFFER_OUTPUT_DIR, fileName);

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &userFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_ENABLE,
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read command buffer user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (userFeatureValue.bData)
    {
       
        eStatus = MOS_CreateDirectory(fileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create output directory. Status = %d", eStatus);
            goto finish;
        }
        // Setup member function and variable.
        state->dumpCommandBuffer = userFeatureValue.bData?true: false;
        if (userFeatureValue.bData == 17)
        {
            state->enableCMDDumpTimeStamp = true;
        }
        else
        {
            state->enableCMDDumpTimeStamp = false;
        }
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}

//!
//! \brief    Dump command buffer to file
//! \param    [in] state
//!           pointer to cm hal state
//! \param    [in] cmdBuffer
//!           pointer to command buffer
//! \param    [in] offsetSurfaceState
//!           offset to surface state
//! \param    [in] sizeOfSurfaceState
//!           size of surface state
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_DumpCommadBuffer(PCM_HAL_STATE state, PMOS_COMMAND_BUFFER cmdBuffer, int offsetSurfaceState, size_t sizeOfSurfaceState)
{

    int32_t        commandBufferNumber = 0;
    int32_t         hr = CM_FAILURE;
    MOS_STATUS      eStatus = MOS_STATUS_UNKNOWN;
    char            *outputBuffer = nullptr;
    // Each hex value should have 9 chars.
    uint32_t        bytesWritten = 0;
    uint32_t        numberOfDwords = 0;
    uint32_t        sizeToAllocate = 0;
    char            fileName[MOS_MAX_HLT_FILENAME_LEN];
    uint32_t        offset = 0;

    PMOS_INTERFACE osInterface = state->osInterface;
    PRENDERHAL_STATE_HEAP stateHeap   = state->renderHal->pStateHeap;

    MOS_OS_ASSERT(state);
    MOS_OS_ASSERT(cmdBuffer);

   //Check if use timestamp in cmd buffer dump file
    commandBufferNumber = GetFileNameAndCounter(fileName, state->enableCMDDumpTimeStamp,
                          commandBufferNumber,
                          __MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_COUNTER, 
                          HALCM_COMMAND_BUFFER_OUTPUT_DIR,
                          HALCM_COMMAND_BUFFER_OUTPUT_FILE);

    //get the command buffer header size
    offset = GetCommandBufferHeaderDWords(osInterface);

    numberOfDwords = cmdBuffer->iOffset / sizeof(uint32_t) - offset;
    sizeToAllocate = numberOfDwords * (SIZE_OF_DWORD_PLUS_ONE)+2 +   //length of command buffer line
        stateHeap->iCurrentSurfaceState *
        (SIZE_OF_DWORD_PLUS_ONE * 
        state->renderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize() / sizeof(uint32_t) + 2); //length of surface state lines
                                                                                                                          // Alloc output buffer.
    outputBuffer = (char *)MOS_AllocAndZeroMemory(sizeToAllocate);
    if (!outputBuffer) {
        MOS_OS_NORMALMESSAGE("Failed to allocate memory for command buffer dump");
        return MOS_STATUS_NO_SPACE;
    }    

    // write command buffer dwords.
    bytesWritten += HalCm_CopyHexDwordLine(outputBuffer, sizeToAllocate - bytesWritten,
                                          (uint32_t *)cmdBuffer->pCmdBase + offset, numberOfDwords);
    MOS_OS_CHK_STATUS(MOS_WriteFileFromPtr((const char *)fileName, outputBuffer, bytesWritten));
    commandBufferNumber++;

    //Record command buffer dump counter
    if (!state->enableCMDDumpTimeStamp)
    {        
        RecordCommandBufferDumpCounter(commandBufferNumber,
                                       __MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_COUNTER_ID);
    }  
    hr = CM_SUCCESS;
finish:
    // Free the memory.
    if (outputBuffer)
    {
        MOS_FreeMemAndSetNull(outputBuffer);
    }
    return hr;
}

#endif

#if MDF_CURBE_DATA_DUMP

#define HALCM_CURBE_DATA_OUTPUT_DIR         "HALCM_Curbe_Data_Dumps"
#define HALCM_CURBE_DATA_OUTPUT_FILE        "Curbe_Data"

//!
//! \brief    Read Register key to check if Curbe Data dump flag enabled
//! \param    [in] state
//!           Pointer to cm hal state
//! \return   CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_InitDumpCurbeData(PCM_HAL_STATE state)
{
    MOS_USER_FEATURE        userFeature;
    char                    fileName[MOS_MAX_HLT_FILENAME_LEN];
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  userFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(state);

    MOS_ZeroMemory(&userFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&userFeature, sizeof(userFeature));

    // Check if curbe data dump was enabled in user feature settings.
    userFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    userFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    userFeature.pValues = &userFeatureValue;
    userFeature.uiNumValues = 1;
    userFeatureValue.bData = false;

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &userFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_CURBE_DUMP_ENABLE,
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read curbe data dump user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (userFeatureValue.bData)
    {
        GetLogFileLocation(HALCM_CURBE_DATA_OUTPUT_DIR, fileName);
        eStatus = MOS_CreateDirectory(fileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create curbe data output directory. Status = %d", eStatus);
            goto finish;
        }
        // Setup member function and variable.
        state->dumpCurbeData = userFeatureValue.bData ? true: false;
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}

//!
//! \brief    Dump Curbe Data to file
//! \param    [in] state
//!           pointer to cm hal state
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_DumpCurbeData(PCM_HAL_STATE state)
{
    static uint32_t curbeDataNumber = 0;
    int32_t         hr = CM_FAILURE;
    MOS_STATUS      eStatus = MOS_STATUS_UNKNOWN;
    char            *outputBuffer = nullptr;
    uint32_t        bytesWritten = 0;
    char            fileName[MOS_MAX_HLT_FILENAME_LEN];
    uint32_t        numberOfDwords = 0;
    uint32_t        sizeToAllocate = 0;
    PMOS_INTERFACE osInterface = state->osInterface;
    uint32_t        *curbeData = nullptr;
    PRENDERHAL_STATE_HEAP stateHeap = state->renderHal->pStateHeap;

    MOS_OS_ASSERT(state);

    // Set the file name.
    GetLogFileLocation(HALCM_CURBE_DATA_OUTPUT_DIR, fileName);

    PlatformSNPrintf(fileName + strlen(fileName), MOS_MAX_HLT_FILENAME_LEN - strlen(fileName),
                     PLATFORM_DIR_SEPERATOR);
    PlatformSNPrintf(fileName + strlen(fileName), MOS_MAX_HLT_FILENAME_LEN - strlen(fileName),
                     "%s_%d.txt", HALCM_CURBE_DATA_OUTPUT_FILE, (int)curbeDataNumber);

    // write curbe data dwords.
    if (state->dshEnabled)
    {
        numberOfDwords = stateHeap->pCurMediaState->pDynamicState->Curbe.dwSize / sizeof(uint32_t);
        sizeToAllocate = numberOfDwords*SIZE_OF_DWORD_PLUS_ONE+2;
        outputBuffer = (char *)MOS_AllocAndZeroMemory(sizeToAllocate);
        curbeData = (uint32_t *)MOS_AllocAndZeroMemory(stateHeap->pCurMediaState->pDynamicState->Curbe.dwSize);
        stateHeap->pCurMediaState->pDynamicState->memoryBlock.ReadData(curbeData,
            stateHeap->pCurMediaState->pDynamicState->Curbe.dwOffset,
            stateHeap->pCurMediaState->pDynamicState->Curbe.dwSize);

        bytesWritten += HalCm_CopyHexDwordLine(outputBuffer,
                          sizeToAllocate - bytesWritten,
                          curbeData,
                          numberOfDwords);
    }
    else
    {
        numberOfDwords = stateHeap->pCurMediaState->iCurbeOffset / sizeof(uint32_t);
        sizeToAllocate = numberOfDwords*SIZE_OF_DWORD_PLUS_ONE+2;
        outputBuffer = (char *)MOS_AllocAndZeroMemory(sizeToAllocate);
        bytesWritten += HalCm_CopyHexDwordLine(outputBuffer,
                          sizeToAllocate - bytesWritten,
                          (uint32_t*)(stateHeap->pGshBuffer + stateHeap->pCurMediaState->dwOffset + stateHeap->dwOffsetCurbe),
                          numberOfDwords);
    }

    MOS_OS_CHK_STATUS(MOS_WriteFileFromPtr((const char *)fileName, outputBuffer, bytesWritten));

    curbeDataNumber++;

    hr = CM_SUCCESS;

finish:
    // Free the memory.
    if (outputBuffer)
    {
        MOS_FreeMemAndSetNull(outputBuffer);
    }
    
    if (curbeData)
    {
        MOS_FreeMemAndSetNull(curbeData);
    }

    return hr;

}

#endif

#if MDF_SURFACE_CONTENT_DUMP

//!
//! \brief    Read Register key to check if Surface content flag enabled
//! \param    [in] state
//!           Pointer to cm hal state
//! \return   CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_InitSurfaceDump(PCM_HAL_STATE state)
{
    MOS_USER_FEATURE        userFeature;
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  userFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(state);

    MOS_ZeroMemory(&userFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&userFeature, sizeof(userFeature));

    // Check if surface content dump was enabled in user feature settings.
    userFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    userFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    userFeature.pValues = &userFeatureValue;
    userFeature.uiNumValues = 1;
    userFeatureValue.bData = false;

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &userFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_DUMP_ENABLE,
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read surface content dump user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (userFeatureValue.bData)
    {
        // Setup member function and variable.
        state->dumpSurfaceContent = userFeatureValue.bData ? true: false;
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}

#endif


#if MDF_SURFACE_STATE_DUMP
#define HALCM_SURFACE_STATE_OUTPUT_DIR         "HALCM_Surface_State_Dumps"
#define HALCM_SURFACE_STATE_OUTPUT_FILE        "Surface_State_Dump"

int32_t HalCm_InitDumpSurfaceState(PCM_HAL_STATE state)
{
    MOS_USER_FEATURE        userFeature;
    char                    fileName[MOS_MAX_HLT_FILENAME_LEN];
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  userFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(state);

    MOS_ZeroMemory(&userFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&userFeature, sizeof(userFeature));

    // Check if command buffer dump was enabled in user feature settings.
    userFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    userFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    userFeature.pValues = &userFeatureValue;
    userFeature.uiNumValues = 1;
    userFeatureValue.bData = false;

    GetLogFileLocation(HALCM_SURFACE_STATE_OUTPUT_DIR, fileName);

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &userFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_ENABLE,
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read surface state user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (userFeatureValue.bData)
    {
        eStatus = MOS_CreateDirectory(fileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create output directory. Status = %d", eStatus);
            goto finish;
        }
        // Setup member function and variable.
        state->dumpSurfaceState = userFeatureValue.bData ? true : false;

        if (userFeatureValue.bData == 17)
        {
            state->enableSurfaceStateDumpTimeStamp = true;
        }
        else
        {
            state->enableSurfaceStateDumpTimeStamp = false;
        }
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}

//!
//! \brief    Dump surface state to file
//! \param    [in] state
//!           pointer to cm hal state
//! \param    [in] offsetSurfaceState
//!           offset to surface state
//! \param    [in] sizeOfSurfaceState
//!           size of surface state
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_DumpSurfaceState(PCM_HAL_STATE state,  int offsetSurfaceState, size_t sizeOfSurfaceState)
{
    int32_t         hr = CM_FAILURE;
    MOS_STATUS      eStatus = MOS_STATUS_UNKNOWN;

    PMOS_INTERFACE osInterface = state->osInterface;
    PRENDERHAL_STATE_HEAP stateHeap = state->renderHal->pStateHeap;

    char            filename[MOS_MAX_HLT_FILENAME_LEN];
    int32_t         surfacestatedumpNumber = 0;
    uint32_t        surfacebytesWritten = 0;
    char            *surfaceoutputBuffer = nullptr;
    uint32_t        surfacesizeToAllocate = 0;
   
   //Check if use timestamp in cmd buffer dump file
    surfacestatedumpNumber = GetFileNameAndCounter(filename, state->enableSurfaceStateDumpTimeStamp,
        surfacestatedumpNumber,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_COUNTER,
        HALCM_SURFACE_STATE_OUTPUT_DIR,
        HALCM_SURFACE_STATE_OUTPUT_FILE);
    
    //calculate surface state dump allocation size
    surfacesizeToAllocate = stateHeap->iCurrentSurfaceState *
        (SIZE_OF_DWORD_PLUS_ONE * 
        state->renderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize() / sizeof(uint32_t) + 2);
    
    //allocate surface output buffer
    surfaceoutputBuffer = (char *)MOS_AllocAndZeroMemory(surfacesizeToAllocate);
    if (!surfaceoutputBuffer) {
        MOS_OS_NORMALMESSAGE("Failed to allocate memory for surface state dump");
        return MOS_STATUS_NO_SPACE;
    }

    //write all surface states
    for (int32_t index = 0; index < stateHeap->iCurrentSurfaceState; ++index) {
        PRENDERHAL_SURFACE_STATE_ENTRY entry = stateHeap->pSurfaceEntry + index;
        void *surfaceState = (char*)entry->pSurfaceState;
        //the address of surface states are 32bit or uint32_t aligned.
        surfacebytesWritten += HalCm_CopyHexDwordLine(surfaceoutputBuffer + surfacebytesWritten, 
                               surfacesizeToAllocate - surfacebytesWritten, (uint32_t*)surfaceState,
                               sizeOfSurfaceState / sizeof(uint32_t));
    }

    //Write to file
    MOS_OS_CHK_STATUS(MOS_WriteFileFromPtr((const char *)filename, surfaceoutputBuffer, 
                      surfacebytesWritten));

    surfacestatedumpNumber++;

    if (!state->enableSurfaceStateDumpTimeStamp)
    {
        RecordSurfaceStateDumpCounter(surfacestatedumpNumber, 
                                      __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_COUNTER_ID);
    }

    hr = CM_SUCCESS;

finish:
    // Free the memory.
    if (surfaceoutputBuffer)
    {
        MOS_FreeMemAndSetNull(surfaceoutputBuffer);
    }

    return hr;

}
#endif

#if MDF_INTERFACE_DESCRIPTOR_DATA_DUMP
#define HALCM_INTERFACE_DESCRIPTOR_DATA_OUTPUT_DIR         "HALCM_Interface_Descriptor_Data_Dumps"
#define HALCM_INTERFACE_DESCRIPTOR_DATA_OUTPUT_FILE        "Interface_Descriptor_Data_Dump"

int32_t HalCm_InitDumpInterfaceDescriporData(PCM_HAL_STATE state)
{
    MOS_USER_FEATURE        userFeature;
    char                    fileName[MOS_MAX_HLT_FILENAME_LEN];
    MOS_STATUS              eStatus;
    MOS_USER_FEATURE_VALUE  userFeatureValue;
    int32_t                 hr = CM_FAILURE;

    MOS_OS_ASSERT(state);

    MOS_ZeroMemory(&userFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    MOS_ZeroMemory(&userFeature, sizeof(userFeature));

    // Check if command buffer dump was enabled in user feature settings.
    userFeature.Type = MOS_USER_FEATURE_TYPE_USER;
    userFeature.pPath = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    userFeature.pValues = &userFeatureValue;
    userFeature.uiNumValues = 1;
    userFeatureValue.bData = false;

    GetLogFileLocation(HALCM_INTERFACE_DESCRIPTOR_DATA_OUTPUT_DIR, fileName);

    eStatus = MOS_UserFeature_ReadValue(
        nullptr,
        &userFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_DUMP,
        MOS_USER_FEATURE_VALUE_TYPE_INT32);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to read interface descriptor data dump user feature key. Status = %d", eStatus);
        goto finish;
    }
    if (userFeatureValue.bData)
    {
        eStatus = MOS_CreateDirectory(fileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create output directory. Status = %d", eStatus);
            goto finish;
        }
        // Setup member function and variable.
        state->dumpIDData = userFeatureValue.bData ? true : false;
        if (userFeatureValue.bData == 17)
        {
            state->enableIDDumpTimeStamp = true;
        }
        else
        {
            state->enableIDDumpTimeStamp = false;
        }
        
    }
    hr = CM_SUCCESS;
finish:
    return hr;
}

//!
//! \brief    Dump interface descriptor data to file
//! \param    [in] state
//!           pointer to cm hal state
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason
//!
int32_t HalCm_DumpInterfaceDescriptorData(PCM_HAL_STATE state)
{
    uint32_t        IDDNumber = 0; 
    int32_t         hr = CM_FAILURE;
    MOS_STATUS      eStatus = MOS_STATUS_UNKNOWN;
    char            *outputBuffer = nullptr;
    uint32_t        bytesWritten = 0;
    char            fileName[MOS_MAX_HLT_FILENAME_LEN];
    uint32_t        numberOfDwords = 0;
    uint32_t        sizeToAllocate = 0;
    PMOS_INTERFACE osInterface = state->osInterface;
    uint32_t        *InterfaceDescriptorData = nullptr;
    PRENDERHAL_STATE_HEAP stateHeap = state->renderHal->pStateHeap;

    MOS_OS_ASSERT(state);

    // Set the file name.
    GetLogFileLocation(HALCM_INTERFACE_DESCRIPTOR_DATA_OUTPUT_DIR, fileName);

    //Check if use timestamp in cmd buffer dump file
    IDDNumber = GetFileNameAndCounter(fileName, state->enableIDDumpTimeStamp,
        IDDNumber,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_COUNTER,
        HALCM_INTERFACE_DESCRIPTOR_DATA_OUTPUT_DIR,
        HALCM_INTERFACE_DESCRIPTOR_DATA_OUTPUT_FILE);

    // write interface descriptor data dwords.
    if (state->dshEnabled)
    {
        numberOfDwords = stateHeap->pCurMediaState->pDynamicState->MediaID.dwSize / sizeof(uint32_t);
        sizeToAllocate = numberOfDwords * SIZE_OF_DWORD_PLUS_ONE + 2;
        outputBuffer = (char *)MOS_AllocAndZeroMemory(sizeToAllocate);
        InterfaceDescriptorData = (uint32_t *)MOS_AllocAndZeroMemory(stateHeap->pCurMediaState->pDynamicState->MediaID.dwSize);
        stateHeap->pCurMediaState->pDynamicState->memoryBlock.ReadData(InterfaceDescriptorData,
            stateHeap->pCurMediaState->pDynamicState->MediaID.dwOffset,
            stateHeap->pCurMediaState->pDynamicState->MediaID.dwSize);

        bytesWritten += HalCm_CopyHexDwordLine(outputBuffer,
            sizeToAllocate - bytesWritten,
            InterfaceDescriptorData,
            numberOfDwords);
    }
    else
    {
        numberOfDwords = stateHeap->dwSizeMediaID / sizeof(uint32_t);
        sizeToAllocate = numberOfDwords * SIZE_OF_DWORD_PLUS_ONE + 2;
        outputBuffer = (char *)MOS_AllocAndZeroMemory(sizeToAllocate);
        bytesWritten += HalCm_CopyHexDwordLine(outputBuffer,
            sizeToAllocate - bytesWritten,
            (uint32_t*)(stateHeap->pGshBuffer + stateHeap->pCurMediaState->dwOffset + stateHeap->dwOffsetMediaID),
            numberOfDwords);
    }

    MOS_OS_CHK_STATUS(MOS_WriteFileFromPtr((const char *)fileName, outputBuffer, bytesWritten));

    IDDNumber++;
    if (!state->enableIDDumpTimeStamp)
    {
        RecordInterfaceDescriptorDataDumpCounter(IDDNumber,
            __MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_COUNTER_ID);
    }

    hr = CM_SUCCESS;

finish:
    // Free the memory.
    if (outputBuffer)
    {
        MOS_FreeMemAndSetNull(outputBuffer);
    }

    if (InterfaceDescriptorData)
    {
        MOS_FreeMemAndSetNull(InterfaceDescriptorData);
    }

    return hr;

}
#endif