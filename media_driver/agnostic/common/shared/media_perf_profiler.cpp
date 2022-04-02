/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     media_perf_profiler.cpp
//! \brief    Defines data structures and interfaces for media performance profiler.
//! \details  
//!

#include "media_perf_profiler.h"

#define UMD_PERF_LOG            8
#define NAME_LEN                60
#define LOCAL_STRING_SIZE       64
#define OFFSET_OF(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER )

typedef enum _UMD_PERF_MODE
{
    UMD_PERF_MODE_TIMING_ONLY      = 0,
    UMD_PERF_MODE_WITH_MEMORY_INFO = 4
} UMD_PERF_MODE;

#pragma pack(push)
#pragma pack(8)
struct PerfEntry 
{
    uint32_t    nodeIndex;                  //!< Perf node index
    uint32_t    processId;                  //!< Process Id
    uint32_t    instanceId;                 //!< Instance Id
    uint32_t    engineTag;                  //!< Engine tag
    uint32_t    perfTag;                    //!< Performance tag
    uint32_t    timeStampBase;              //!< HW timestamp base
    uint32_t    beginRegisterValue[8];      //!< Begin register value
    uint32_t    endRegisterValue[8];        //!< End register value
    uint32_t    beginCpuTime[2];            //!< Begin CPU Time Stamp
    uint32_t    reserved[14];               //!< Reserved[14]
    uint64_t    beginTimeClockValue;        //!< Begin timestamp
    uint64_t    endTimeClockValue;          //!< End timestamp
};
#pragma pack(pop)

struct NodeHeader 
{
    uint32_t osPlatform  : 3;
    uint32_t genPlatform : 3; 
    uint32_t eventType   : 4;
    uint32_t perfMode    : 3;
    uint32_t genAndroid  : 4;
    uint32_t genPlatform_ext : 2;
    uint32_t reserved    : 13;
};

#define BASE_OF_NODE(perfDataIndex) (sizeof(NodeHeader) + (sizeof(PerfEntry) * perfDataIndex))

#define CHK_STATUS_RETURN(_stmt)                   \
{                                                  \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);   \
    if (stmtStatus != MOS_STATUS_SUCCESS)          \
    {                                              \
        return stmtStatus;                         \
    }                                              \
}

#define CHK_NULL_RETURN(_ptr)                      \
{                                                  \
    if ((_ptr) == nullptr)                         \
    {                                              \
        return MOS_STATUS_NULL_POINTER;            \
    }                                              \
}

#define CHK_STATUS_UNLOCK_MUTEX_RETURN(_stmt)      \
{                                                  \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);   \
    if (stmtStatus != MOS_STATUS_SUCCESS)          \
    {                                              \
        MosUtilities::MosUnlockMutex(m_mutex);     \
        return stmtStatus;                         \
    }                                              \
}

#define CHK_NULL_UNLOCK_MUTEX_RETURN(_ptr)         \
{                                                  \
    if ((_ptr) == nullptr)                         \
    {                                              \
        MosUtilities::MosUnlockMutex(m_mutex);     \
        return MOS_STATUS_NULL_POINTER;            \
    }                                              \
}

MediaPerfProfiler* MediaPerfProfiler::Instance()
{
    static MediaPerfProfiler instance;
    if (!instance.m_mutex && instance.m_profilerEnabled)
    {
        MOS_OS_ASSERTMESSAGE("Create MediaPerfProfiler failed!");
        return nullptr;
    }
    else
    {
        return &instance;
    }

}

void MediaPerfProfiler::Destroy(MediaPerfProfiler* profiler, void* context, MOS_INTERFACE *osInterface)
{
    PERF_UTILITY_PRINT;

    if (profiler->m_profilerEnabled == 0 || profiler->m_mutex == nullptr)
    {
        return;
    }

    MediaPerfProfilerNext *profilerNext = MediaPerfProfilerNext::Instance();
    if(profilerNext && profilerNext->m_profilerEnabled && profilerNext->m_mutex)
    {
        MosUtilities::MosLockMutex(profilerNext->m_mutex);

        if(profilerNext->m_ref > 0)
        {
            profilerNext->m_ref--;
        }

        MosUtilities::MosUnlockMutex(profilerNext->m_mutex);
    }

    MosUtilities::MosLockMutex(profiler->m_mutex);

    if(profiler->m_ref > 0)
    {
        profiler->m_ref--;
    }

    osInterface->pfnWaitAllCmdCompletion(osInterface);

    profiler->m_contextIndexMap.erase(context);

    MosUtilities::MosLockMutex(profilerNext->m_mutex);
    if (profiler->m_ref == 0 && profilerNext->m_ref == 0)
    {
        MosUtilities::MosUnlockMutex(profilerNext->m_mutex);
        if (profiler->m_initialized == true)
        {
            if(profiler->m_enableProfilerDump)
            {
                profiler->SavePerfData(osInterface);
            }

            osInterface->pfnFreeResource(
                osInterface,
                &profiler->m_perfStoreBuffer);

            profiler->m_initialized = false;
        }

        MosUtilities::MosUnlockMutex(profiler->m_mutex);
    }
    else
    {
        MosUtilities::MosUnlockMutex(profilerNext->m_mutex);
        MosUtilities::MosUnlockMutex(profiler->m_mutex);
    }

    PERF_UTILITY_PRINT;

    //Destroy APO perf profiler here without writting bin file again. Destroy() in APO class is not called before media softlet build done.
    if (!profilerNext || profilerNext->m_profilerEnabled == 0 || profilerNext->m_mutex == nullptr)
    {
        return;
    }

    MosUtilities::MosLockMutex(profilerNext->m_mutex);

    osInterface->pfnWaitAllCmdCompletion(osInterface);

    profilerNext->m_contextIndexMap.erase(context);

    MosUtilities::MosLockMutex(profiler->m_mutex);
    if (profiler->m_ref == 0 && profilerNext->m_ref == 0)
    {
        MosUtilities::MosUnlockMutex(profiler->m_mutex);
        if (profilerNext->m_initialized == true)
        {

            osInterface->pfnFreeResource(
                osInterface,
                &profilerNext->m_perfStoreBuffer);

            profilerNext->m_initialized = false;
        }

        MosUtilities::MosUnlockMutex(profilerNext->m_mutex);
    }
    else
    {
        MosUtilities::MosUnlockMutex(profiler->m_mutex);
        MosUtilities::MosUnlockMutex(profilerNext->m_mutex);
    }

}

MOS_STATUS MediaPerfProfiler::StoreData(
    MhwMiInterface *miInterface, 
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset,
    uint32_t value)
{
    if (m_miItf)
    {
        CHK_STATUS_RETURN(MediaPerfProfilerNext::StoreData(m_miItf, cmdBuffer, offset, value));
    }
    else
    {
        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));

        storeDataParams.pOsResource         = &m_perfStoreBuffer;
        storeDataParams.dwResourceOffset    = offset;
        storeDataParams.dwValue             = value;

        CHK_STATUS_RETURN(miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreRegister(
    MOS_INTERFACE *osInterface,
    MhwMiInterface *miInterface, 
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset,
    uint32_t reg)
{
    if (m_miItf)
    {
        CHK_STATUS_RETURN(MediaPerfProfilerNext::StoreRegister(osInterface, m_miItf, cmdBuffer, offset, reg));
    }
    else
    {
        MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegMemParams;
        MOS_ZeroMemory(&storeRegMemParams, sizeof(storeRegMemParams));

        storeRegMemParams.presStoreBuffer = &m_perfStoreBuffer;
        storeRegMemParams.dwOffset        = offset;
        storeRegMemParams.dwRegister      = reg;

        MEDIA_FEATURE_TABLE* skuTable = osInterface->pfnGetSkuTable(osInterface);
        if(skuTable && MEDIA_IS_SKU(skuTable, FtrMemoryRemapSupport))
        {
            storeRegMemParams.dwOption = CCS_HW_FRONT_END_MMIO_REMAP;
        }

        CHK_STATUS_RETURN(miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegMemParams));
    };

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreTSByPipeCtrl(
    MhwMiInterface *miInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset)
{
    if (m_miItf)
    {
        CHK_STATUS_RETURN(MediaPerfProfilerNext::StoreTSByPipeCtrl(m_miItf, cmdBuffer, offset));
    }
    else
    {
        MHW_PIPE_CONTROL_PARAMS PipeControlParams;

        MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
        PipeControlParams.dwResourceOffset = offset;
        PipeControlParams.dwPostSyncOp     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        PipeControlParams.dwFlushMode      = MHW_FLUSH_READ_CACHE;
        PipeControlParams.presDest         = &m_perfStoreBuffer;

        CHK_STATUS_RETURN(miInterface->AddPipeControl(
            cmdBuffer,
            NULL,
            &PipeControlParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreTSByMiFlush(
    MhwMiInterface *miInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset)
{
    if (m_miItf)
    {
        CHK_STATUS_RETURN(MediaPerfProfilerNext::StoreTSByMiFlush(m_miItf, cmdBuffer, offset));
    }
    else
    {
        MHW_MI_FLUSH_DW_PARAMS FlushDwParams;

        MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
        FlushDwParams.postSyncOperation             = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        FlushDwParams.dwResourceOffset              = offset;
        FlushDwParams.pOsResource                   = &m_perfStoreBuffer;

        CHK_STATUS_RETURN(miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &FlushDwParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreDataNext(
    MhwMiInterface* miInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset,
    uint32_t value)
{
    std::shared_ptr<mhw::mi::Itf> miItf = std::static_pointer_cast<mhw::mi::Itf>(miInterface->GetNewMiInterface());

    if (miItf == nullptr)
    {
        return (StoreData(miInterface, cmdBuffer, offset, value));
    }

    auto &storeDataParams            = miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
    storeDataParams                  = {};
    storeDataParams.pOsResource      = &m_perfStoreBuffer;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = value;

    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreRegisterNext(
    MOS_INTERFACE* osInterface,
    MhwMiInterface* miInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset,
    uint32_t reg)
{
    std::shared_ptr<mhw::mi::Itf> miItf = std::static_pointer_cast<mhw::mi::Itf>(miInterface->GetNewMiInterface());

    if (miItf == nullptr)
    {
        return (StoreRegister(osInterface, miInterface, cmdBuffer, offset, reg));
    }

    auto& storeRegMemParams           = miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    storeRegMemParams                 = {};
    storeRegMemParams.presStoreBuffer = &m_perfStoreBuffer;
    storeRegMemParams.dwOffset        = offset;
    storeRegMemParams.dwRegister      = reg;

    MEDIA_FEATURE_TABLE* skuTable = osInterface->pfnGetSkuTable(osInterface);
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrMemoryRemapSupport))
    {
        storeRegMemParams.dwOption = CCS_HW_FRONT_END_MMIO_REMAP;
    }

    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreTSByPipeCtrlNext(
    MhwMiInterface* miInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset)
{
    std::shared_ptr<mhw::mi::Itf> miItf = std::static_pointer_cast<mhw::mi::Itf>(miInterface->GetNewMiInterface());

    if (miItf == nullptr)
    {
        return (StoreTSByPipeCtrl(miInterface, cmdBuffer, offset));
    }

    auto& PipeControlParams            = miItf->MHW_GETPAR_F(PIPE_CONTROL)();
    PipeControlParams                  = {};
    PipeControlParams.dwResourceOffset = offset;
    PipeControlParams.dwPostSyncOp     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    PipeControlParams.dwFlushMode      = MHW_FLUSH_READ_CACHE;
    PipeControlParams.presDest         = &m_perfStoreBuffer;

    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(PIPE_CONTROL)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreTSByMiFlushNext(
    MhwMiInterface* miInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t offset)
{
    std::shared_ptr<mhw::mi::Itf> miItf = std::static_pointer_cast<mhw::mi::Itf>(miInterface->GetNewMiInterface());

    if (miItf == nullptr)
    {
        return (StoreTSByMiFlush(miInterface, cmdBuffer, offset));
    }

    auto& FlushDwParams             = miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    FlushDwParams                   = {};
    FlushDwParams.postSyncOperation = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    FlushDwParams.dwResourceOffset  = offset;
    FlushDwParams.pOsResource       = &m_perfStoreBuffer;

    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::AddPerfCollectStartCmd(void* context,
    MOS_INTERFACE *osInterface,
    MhwMiInterface *miInterface,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    if (m_initialized == false)
    {
        return status;
    }

    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(miInterface);
    CHK_NULL_RETURN(cmdBuffer);
    CHK_NULL_RETURN(m_mutex);

    uint32_t perfDataIndex = 0;

    MosUtilities::MosLockMutex(m_mutex);

    perfDataIndex = m_perfDataIndex;
    m_perfDataIndex++;

    if (BASE_OF_NODE(perfDataIndex) + sizeof(PerfEntry) > m_bufferSize)
    {
        MosUtilities::MosUnlockMutex(m_mutex);
        MOS_OS_ASSERTMESSAGE("Reached maximum perf data buffer size, please increase it in Performance\\Perf Profiler Buffer Size");
        return MOS_STATUS_NOT_ENOUGH_BUFFER;
    }

    m_contextIndexMap[context] = perfDataIndex;
    m_miItf = std::static_pointer_cast<mhw::mi::Itf>(miInterface->GetNewMiInterface());

    bool             rcsEngineUsed = false;
    MOS_GPU_CONTEXT  gpuContext;

    gpuContext     = osInterface->pfnGetGpuContext(osInterface);
    rcsEngineUsed = MOS_RCS_ENGINE_USED(gpuContext);

    if (m_multiprocess)
    {
        CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreDataNext(
            miInterface,
            cmdBuffer,
            BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, processId),
            MosUtilities::MosGetPid()));
    }

    CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreDataNext(
        miInterface,
        cmdBuffer,
        BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, perfTag),
        osInterface->pfnGetPerfTag(osInterface)));

    CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreDataNext(
        miInterface,
        cmdBuffer,
        BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, engineTag),
        GpuContextToGpuNode(gpuContext)));

    if (m_timerBase != 0)
    {
        CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreDataNext(
            miInterface,
            cmdBuffer,
            BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, timeStampBase),
            m_timerBase));
    }

    int8_t regIndex = 0;
    for (regIndex = 0; regIndex < 8; regIndex++)
    {
        if (m_registers[regIndex] != 0)
        {
            CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreRegisterNext(
                osInterface,
                miInterface,
                cmdBuffer,
                BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, beginRegisterValue[regIndex]),
                m_registers[regIndex]));
        }
    }

    uint64_t beginCPUTimestamp = MOS_GetCurTime();
    uint32_t timeStamp[2];
    MOS_SecureMemcpy(timeStamp, 2*sizeof(uint32_t), &beginCPUTimestamp, 2*sizeof(uint32_t));

    for (int i = 0; i < 2; i++)
    {
        CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreDataNext(
            miInterface,
            cmdBuffer,
            BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, beginCpuTime[i]),
            timeStamp[i]));
    }

    // The address of timestamp must be 8 bytes aligned.
    uint32_t offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, beginTimeClockValue);
    offset = MOS_ALIGN_CEIL(offset, 8);

    if (rcsEngineUsed)
    {
        CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreTSByPipeCtrlNext(
            miInterface,
            cmdBuffer,
            offset));
    }
    else
    {
        CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreTSByMiFlushNext(
            miInterface,
            cmdBuffer,
            offset));
    }

    MosUtilities::MosUnlockMutex(m_mutex);

    return status;
}

MOS_STATUS MediaPerfProfiler::AddPerfCollectEndCmd(void* context,
    MOS_INTERFACE *osInterface,
    MhwMiInterface *miInterface,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS       status        = MOS_STATUS_SUCCESS;

    if (m_initialized == false)
    {
        return status;
    }

    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(miInterface);
    CHK_NULL_RETURN(cmdBuffer);

    MOS_GPU_CONTEXT  gpuContext;
    bool             rcsEngineUsed = false;
    uint32_t         perfDataIndex = 0;

    MosUtilities::MosLockMutex(m_mutex);
    m_miItf = std::static_pointer_cast<mhw::mi::Itf>(miInterface->GetNewMiInterface());

    gpuContext     = osInterface->pfnGetGpuContext(osInterface);
    rcsEngineUsed = MOS_RCS_ENGINE_USED(gpuContext);

    perfDataIndex = m_contextIndexMap[context];

    int8_t regIndex = 0;
    for (regIndex = 0; regIndex < 8; regIndex++)
    {
        if (m_registers[regIndex] != 0)
        {
            CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreRegisterNext(
                osInterface,
                miInterface,
                cmdBuffer,
                BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, endRegisterValue[regIndex]),
                m_registers[regIndex]));
        }
    }

    // The address of timestamp must be 8 bytes aligned.
    uint32_t offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, endTimeClockValue);
    offset = MOS_ALIGN_CEIL(offset, 8);

    if (rcsEngineUsed)
    {
        CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreTSByPipeCtrlNext(
            miInterface,
            cmdBuffer,
            offset));
    }
    else
    {
        CHK_STATUS_UNLOCK_MUTEX_RETURN(StoreTSByMiFlushNext(
            miInterface,
            cmdBuffer,
            offset));
    }

    MosUtilities::MosUnlockMutex(m_mutex);
    return status;
}

MOS_STATUS MediaPerfProfiler::SavePerfData(MOS_INTERFACE *osInterface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CHK_NULL_RETURN(osInterface);

    MediaPerfProfilerNext *profilerNext = MediaPerfProfilerNext::Instance();
    CHK_NULL_RETURN(profilerNext);

    if (m_perfDataIndex > 0 || profilerNext->m_perfDataIndex > 0)
    {
        MOS_LOCK_PARAMS     LockFlagsNoOverWrite;
        MOS_ZeroMemory(&LockFlagsNoOverWrite, sizeof(MOS_LOCK_PARAMS));

        LockFlagsNoOverWrite.WriteOnly = 1;
        LockFlagsNoOverWrite.NoOverWrite = 1;

        uint8_t* pData = (uint8_t*)osInterface->pfnLockResource(
            osInterface,
            &m_perfStoreBuffer,
            &LockFlagsNoOverWrite);

        CHK_NULL_RETURN(pData);

        uint8_t* pDataNext = (uint8_t*)osInterface->pfnLockResource(
            osInterface,
            &profilerNext->m_perfStoreBuffer,
            &LockFlagsNoOverWrite);

        if (pDataNext)
        {
            //Append perf data written by APO perf Profiler into legacy one to avoid file overritting.
            MosUtilities::MosSecureMemcpy(
                pData+BASE_OF_NODE(m_perfDataIndex),
                BASE_OF_NODE(profilerNext->m_perfDataIndex)-sizeof(NodeHeader),
                pDataNext+sizeof(NodeHeader),
                BASE_OF_NODE(profilerNext->m_perfDataIndex)-sizeof(NodeHeader));
        }

        if (m_multiprocess)
        {
            int32_t pid = MosUtilities::MosGetPid();
            tm      localtime = { 0 };
            MosUtilities::MosGetLocalTime(&localtime);
            char outputFileName[MOS_MAX_PATH_LENGTH + 1];

            MOS_SecureStringPrint(outputFileName, MOS_MAX_PATH_LENGTH + 1, MOS_MAX_PATH_LENGTH + 1, "%s-pid%d-%04d%02d%02d%02d%02d%02d.bin",
                m_outputFileName, pid, localtime.tm_year + 1900, localtime.tm_mon + 1, localtime.tm_mday, localtime.tm_hour, localtime.tm_min, localtime.tm_sec);

            MosUtilities::MosWriteFileFromPtr(outputFileName, pData, BASE_OF_NODE(m_perfDataIndex)+BASE_OF_NODE(profilerNext->m_perfDataIndex)-sizeof(NodeHeader));
        }
        else
        {
            MosUtilities::MosWriteFileFromPtr(m_outputFileName, pData, BASE_OF_NODE(m_perfDataIndex)+BASE_OF_NODE(profilerNext->m_perfDataIndex)-sizeof(NodeHeader));
        }

        osInterface->pfnUnlockResource(
            osInterface,
            &m_perfStoreBuffer);

        osInterface->pfnUnlockResource(
            osInterface,
            &profilerNext->m_perfStoreBuffer);
    }

    return status;
}
