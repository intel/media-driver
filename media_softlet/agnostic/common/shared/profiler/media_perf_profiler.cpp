/*
* Copyright (c) 2021-2023, Intel Corporation
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

#include <stddef.h>
#include "media_perf_profiler.h"
#include "media_skuwa_specific.h"
#include "mhw_itf.h"
#include "mhw_mi.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_mi_itf.h"
#include "mos_resource_defs.h"
#include "mos_util_debug.h"
#include "mos_utilities.h"
#include "mos_utilities_common.h"

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
    uint32_t    bitstreamSize;              //!< frame level: bitstreamSize
    uint32_t    SSEY;                       //!< frame level: SSEY 
    uint32_t    SSEU;                       //!< frame level: SSEU 
    uint32_t    SSEV;                       //!< frame level: SSEV
    union
    {
        uint32_t DWMeanSsimLayer1_YU;
        struct
        {
            uint32_t MeanSsimLayer1_Y : 12,  // [11:0]
                DW3_Res_15_12 : 4,           // [15:12]
                MeanSsimLayer1_U : 12,       // [27:16]
                DW3_Res_31_18 : 4;           // [31:28]
        };
    };
    union
    {
        uint32_t DWMeanSsimLayer1_V;
        struct
        {
            uint32_t MeanSsimLayer1_V : 12,  // [11:0]
                DW4_Res_15_12 : 4,           // [15:12]
                MeanSsimLayer1Part_Y : 12,   // [27:16]
                DW4_Res_31_18 : 4;           // [31:28]
        };
    };
    uint32_t    reserved[8];                //!< Reserved[8]
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

#define CHK_NULL_NO_STATUS_RETURN(_ptr)            \
{                                                  \
    if ((_ptr) == nullptr)                         \
    {                                              \
        return;                                    \
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

MediaPerfProfiler::MediaPerfProfiler()
{
    m_perfStoreBufferMap.clear();
    m_perfDataIndexMap.clear();
    m_refMap.clear();
    m_initializedMap.clear();

    m_profilerEnabled = 0;

    m_mutex = MosUtilities::MosCreateMutex();

    if (m_mutex)
    {
        // m_mutex is destroyed after MemNinja report, this will cause fake memory leak,
        // the following 2 lines is to circumvent Memninja counter validation and log parser
        MosUtilities::MosAtomicDecrement(MosUtilities::m_mosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(m_mutex, __FUNCTION__, __FILE__, __LINE__);
        PRINT_DESTROY_MEMORY(MT_MOS_DESTROY_MEMORY, MT_NORMAL, MT_MEMORY_PTR, (int64_t)(m_mutex), __FUNCTION__, __FILE__, __LINE__);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Create Mutex failed!");
    }
}

MediaPerfProfiler::~MediaPerfProfiler()
{
    if (m_mutex != nullptr)
    {
        MosUtilities::MosDestroyMutex(m_mutex);
        m_mutex = nullptr;
    }
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

    CHK_NULL_NO_STATUS_RETURN(profiler);
    CHK_NULL_NO_STATUS_RETURN(osInterface);

    if (profiler->m_profilerEnabled == 0 || profiler->m_mutex == nullptr)
    {
        return;
    }

    PMOS_CONTEXT pOsContext = osInterface->pOsContext;
    CHK_NULL_NO_STATUS_RETURN(pOsContext);
    MosUtilities::MosLockMutex(profiler->m_mutex);
    if (profiler->m_refMap[pOsContext] > 0)
    {
        profiler->m_refMap[pOsContext]--;
    }
    osInterface->pfnWaitAllCmdCompletion(osInterface);

    profiler->m_contextIndexMap.erase(context);

    if (profiler->m_refMap[pOsContext] == 0)
    {
        if (profiler->m_initializedMap[pOsContext] == true)
        {
            if(profiler->m_enableProfilerDump)
            {
                profiler->SavePerfData(osInterface);
            }

            osInterface->pfnFreeResource(
                osInterface,
                profiler->m_perfStoreBufferMap[pOsContext]);

            MOS_FreeMemAndSetNull(profiler->m_perfStoreBufferMap[pOsContext]);

            profiler->m_perfStoreBufferMap.erase(pOsContext);
            profiler->m_initializedMap.erase(pOsContext);
            profiler->m_refMap.erase(pOsContext);
            profiler->m_perfDataIndexMap.erase(pOsContext);
        }

        MosUtilities::MosUnlockMutex(profiler->m_mutex);
    }
    else
    {
        MosUtilities::MosUnlockMutex(profiler->m_mutex);
    }
}

MOS_STATUS MediaPerfProfiler::Initialize(void* context, MOS_INTERFACE *osInterface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(m_mutex);

    PMOS_CONTEXT pOsContext = osInterface->pOsContext;
    CHK_NULL_RETURN(pOsContext);
    MediaUserSettingSharedPtr userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);
    // Check whether profiler is enabled
    ReadUserSetting(
        userSettingPtr,
        m_profilerEnabled,
        __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE,
        MediaUserSetting::Group::Device);

    if (m_profilerEnabled == 0 || m_mutex == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    MosUtilities::MosLockMutex(m_mutex);

    m_contextIndexMap[context] = 0;

    if (m_initializedMap[pOsContext] == true)
    {
        MosUtilities::MosUnlockMutex(m_mutex);
        return status;
    }
    
    m_refMap[pOsContext]++;

    m_enableProfilerDump = MosUtilities::MosIsProfilerDumpEnabled();

    // Read output file name
    status = ReadUserSetting(
                userSettingPtr,
                m_outputFileName,
                __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_OUTPUT_FILE_NAME,
                MediaUserSetting::Group::Device);
    if (status != MOS_STATUS_SUCCESS)
    {
        MosUtilities::MosUnlockMutex(m_mutex);
        return status;
    }

    // Read buffer size
    ReadUserSetting(
            userSettingPtr,
            m_bufferSize,
            __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_BUFFER_SIZE_KEY,
            MediaUserSetting::Group::Device);

    m_timerBase = osInterface->pfnGetTsFrequency(osInterface);

    // Read multi processes support
    ReadUserSetting(
        userSettingPtr,
        m_multiprocess,
        __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE_MUL_PROC,
        MediaUserSetting::Group::Device);

    // Read multi header support
    ReadUserSetting(
        userSettingPtr,
        m_mergeheader,
        __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE_MER_HEADER,
        MediaUserSetting::Group::Device);

    // Read parallel execution support
    bool addPid = false;
    ReadUserSetting(
        userSettingPtr,
        addPid,
        __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_PARALLEL_EXEC,
        MediaUserSetting::Group::Device);

    if (addPid)
    {
        m_outputFileName += "-" + std::to_string(MosUtilities::MosGetPid());
    }
    // Read memory information register address
    int8_t regIndex = 0;
    for (regIndex = 0; regIndex < 8; regIndex++)
    {
        ReadUserSetting(
            userSettingPtr,
            m_registers[regIndex],
            m_registersKey[regIndex],
            MediaUserSetting::Group::Device);
    }

    // Read multi processes single binary flag
    ReadUserSetting(
        userSettingPtr,
        m_multiprocesssinglebin,
        __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_MUL_PROC_SINGLE_BIN,
        MediaUserSetting::Group::Device);

    PMOS_RESOURCE  pPerfStoreBuffer = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
    m_perfStoreBufferMap[pOsContext] = pPerfStoreBuffer;
    // Allocate the buffer which store the performance data
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type        = MOS_GFXRES_BUFFER;
    allocParams.TileType    = MOS_TILE_LINEAR;
    allocParams.Format      = Format_Buffer;
    allocParams.dwBytes     = m_bufferSize;
    allocParams.pBufName    = "PerfStoreBuffer";

    status = osInterface->pfnAllocateResource(
                                        osInterface,
                                        &allocParams,
                                        pPerfStoreBuffer);

    CHK_STATUS_UNLOCK_MUTEX_RETURN(status);

    CHK_STATUS_UNLOCK_MUTEX_RETURN(
        osInterface->pfnSkipResourceSync(pPerfStoreBuffer));

    PLATFORM platform = { IGFX_UNKNOWN };
    osInterface->pfnGetPlatform(osInterface, &platform);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly   = 1;

    NodeHeader* header = (NodeHeader*)osInterface->pfnLockResource(
            osInterface,
            pPerfStoreBuffer,
            &lockFlags);

    CHK_NULL_UNLOCK_MUTEX_RETURN(header);

    // Append the header info
    MOS_ZeroMemory(header, m_bufferSize);
    header->eventType   = UMD_PERF_LOG;

    uint32_t mappedPlatFormId = PlatFormIdMap(platform);
    header->genPlatform = (mappedPlatFormId - 8) & 0x7;
    header->genPlatform_ext = ((mappedPlatFormId - 8) >> 3) & 0x3;

    if (IsPerfModeWidthMemInfo(m_registers))
    {
        header->perfMode    = UMD_PERF_MODE_WITH_MEMORY_INFO;
    }
    else
    {
        header->perfMode    = UMD_PERF_MODE_TIMING_ONLY;
    }

    osInterface->pfnUnlockResource(
            osInterface,
            pPerfStoreBuffer);

    m_initializedMap[pOsContext] = true;

    MosUtilities::MosUnlockMutex(m_mutex);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreData(
    std::shared_ptr<mhw::mi::Itf> miItf,
    PMOS_COMMAND_BUFFER           cmdBuffer,
    MOS_CONTEXT_HANDLE            pOsContext,
    uint32_t                      offset,
    uint32_t                      value)
{
    CHK_NULL_RETURN(miItf);

    auto& storeDataParams            = miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
    storeDataParams                  = {};
    storeDataParams.pOsResource      = m_perfStoreBufferMap[(PMOS_CONTEXT)pOsContext];
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = value;
    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreRegister(
    MOS_INTERFACE                 *osInterface,
    std::shared_ptr<mhw::mi::Itf> miItf,
    PMOS_COMMAND_BUFFER           cmdBuffer,
    uint32_t                      offset,
    uint32_t                      reg)
{
    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(miItf);

    auto& storeRegMemParams           = miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    storeRegMemParams                 = {};
    storeRegMemParams.presStoreBuffer = m_perfStoreBufferMap[osInterface->pOsContext];
    storeRegMemParams.dwOffset        = offset;
    storeRegMemParams.dwRegister      = reg;

    MEDIA_FEATURE_TABLE* skuTable = osInterface->pfnGetSkuTable(osInterface);
    if(skuTable && MEDIA_IS_SKU(skuTable, FtrMemoryRemapSupport))
    {
        storeRegMemParams.dwOption = CCS_HW_FRONT_END_MMIO_REMAP;
    }

    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreTSByPipeCtrl(
    std::shared_ptr<mhw::mi::Itf> miItf,
    PMOS_COMMAND_BUFFER           cmdBuffer,
    MOS_CONTEXT_HANDLE            pOsContext,
    uint32_t                      offset)
{
    CHK_NULL_RETURN(miItf);

    auto& PipeControlParams            = miItf->MHW_GETPAR_F(PIPE_CONTROL)();
    PipeControlParams                  = {};
    PipeControlParams.dwResourceOffset = offset;
    PipeControlParams.dwPostSyncOp     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    PipeControlParams.dwFlushMode      = MHW_FLUSH_READ_CACHE;
    PipeControlParams.presDest         = m_perfStoreBufferMap[(PMOS_CONTEXT)pOsContext];

    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(PIPE_CONTROL)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::StoreTSByMiFlush(
    std::shared_ptr<mhw::mi::Itf> miItf,
    PMOS_COMMAND_BUFFER           cmdBuffer,
    MOS_CONTEXT_HANDLE            pOsContext,
    uint32_t                      offset)
{
    CHK_NULL_RETURN(miItf);

    auto& FlushDwParams             = miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    FlushDwParams                   = {};
    FlushDwParams.postSyncOperation = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    FlushDwParams.dwResourceOffset  = offset;
    FlushDwParams.pOsResource       = m_perfStoreBufferMap[(PMOS_CONTEXT)pOsContext];

    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::AddPerfCollectStartCmd(
    void                          *context,
    MOS_INTERFACE                 *osInterface,
    std::shared_ptr<mhw::mi::Itf> miItf,
    MOS_COMMAND_BUFFER            *cmdBuffer)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(miItf);
    CHK_NULL_RETURN(cmdBuffer);
    CHK_NULL_RETURN(m_mutex);

    PMOS_CONTEXT pOsContext = osInterface->pOsContext;
    CHK_NULL_RETURN(pOsContext);

    if (m_profilerEnabled == 0 || m_initializedMap[pOsContext] == false)
    {
        return status;
    }

    uint32_t perfDataIndex = 0;

    MosUtilities::MosLockMutex(m_mutex);

    perfDataIndex = m_perfDataIndexMap[pOsContext];
    m_perfDataIndexMap[pOsContext]++;
    m_contextIndexMap[context] = perfDataIndex;

    MosUtilities::MosUnlockMutex(m_mutex);

    bool             rcsEngineUsed = false;
    MOS_GPU_CONTEXT  gpuContext;

    gpuContext     = osInterface->pfnGetGpuContext(osInterface);
    rcsEngineUsed = MOS_RCS_ENGINE_USED(gpuContext);

    if (m_multiprocess)
    {
        CHK_STATUS_RETURN(StoreData(
            miItf,
            cmdBuffer,
            pOsContext,
            BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, processId),
            MosUtilities::MosGetPid()));
    }

    CHK_STATUS_RETURN(StoreData(
        miItf,
        cmdBuffer,
        pOsContext,
        BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, perfTag),
        osInterface->pfnGetPerfTag(osInterface)));

    CHK_STATUS_RETURN(StoreData(
        miItf,
        cmdBuffer,
        pOsContext,
        BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, engineTag),
        GpuContextToGpuNode(gpuContext)));

    if (m_timerBase != 0)
    {
        CHK_STATUS_RETURN(StoreData(
            miItf,
            cmdBuffer,
            pOsContext,
            BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, timeStampBase),
            m_timerBase));
    }

    int8_t regIndex = 0;
    for (regIndex = 0; regIndex < 8; regIndex++)
    {
        if (m_registers[regIndex] != 0)
        {
            CHK_STATUS_RETURN(StoreRegister(
                osInterface,
                miItf,
                cmdBuffer,
                BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, beginRegisterValue[regIndex]),
                m_registers[regIndex]));
        }
    }

    uint64_t beginCPUTimestamp = MosUtilities::MosGetCurTime();
    uint32_t timeStamp[2];
    MOS_SecureMemcpy(timeStamp, 2*sizeof(uint32_t), &beginCPUTimestamp, 2*sizeof(uint32_t));

    for (int i = 0; i < 2; i++)
    {
        CHK_STATUS_RETURN(StoreData(
            miItf,
            cmdBuffer,
            pOsContext,
            BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, beginCpuTime[i]),
            timeStamp[i]));
    }

    // The address of timestamp must be 8 bytes aligned.
    uint32_t offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, beginTimeClockValue);
    offset = MOS_ALIGN_CEIL(offset, 8);

    if (rcsEngineUsed)
    {
        CHK_STATUS_RETURN(StoreTSByPipeCtrl(
            miItf,
            cmdBuffer,
            pOsContext,
            offset));
    }
    else
    {
        CHK_STATUS_RETURN(StoreTSByMiFlush(
            miItf,
            cmdBuffer,
            pOsContext,
            offset));
    }

    return status;
}

MOS_STATUS MediaPerfProfiler::AddPerfCollectEndCmd(
    void                          *context,
    MOS_INTERFACE                 *osInterface,
    std::shared_ptr<mhw::mi::Itf> miItf,
    MOS_COMMAND_BUFFER            *cmdBuffer)
{
    MOS_STATUS       status        = MOS_STATUS_SUCCESS;

    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(miItf);
    CHK_NULL_RETURN(cmdBuffer);

    PMOS_CONTEXT pOsContext = osInterface->pOsContext;
    CHK_NULL_RETURN(pOsContext);

    if (m_profilerEnabled == 0 || m_initializedMap[pOsContext] == false)
    {
        return status;
    }

    MOS_GPU_CONTEXT  gpuContext;
    bool             rcsEngineUsed = false;
    uint32_t         perfDataIndex = 0;

    gpuContext     = osInterface->pfnGetGpuContext(osInterface);
    rcsEngineUsed = MOS_RCS_ENGINE_USED(gpuContext);

    perfDataIndex = m_contextIndexMap[context];

    int8_t regIndex = 0;
    for (regIndex = 0; regIndex < 8; regIndex++)
    {
        if (m_registers[regIndex] != 0)
        {
            CHK_STATUS_RETURN(StoreRegister(
                osInterface,
                miItf,
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
        CHK_STATUS_RETURN(StoreTSByPipeCtrl(
            miItf,
            cmdBuffer,
            pOsContext,
            offset));
    }
    else
    {
        CHK_STATUS_RETURN(StoreTSByMiFlush(
            miItf,
            cmdBuffer,
            pOsContext,
            offset));
    }

    return status;
}

MOS_STATUS MediaPerfProfiler::AddStoreBitstreamSizeCmd(
    void                           *context,
    MOS_INTERFACE                  *osInterface,
    std::shared_ptr<mhw::mi::Itf>& miItf,
    MOS_COMMAND_BUFFER             *cmdBuffer,
    uint32_t                       reg)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    
    if (m_profilerEnabled == 0)
    {
        return status;
    }

    CHK_NULL_RETURN(context);
    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(miItf);
    CHK_NULL_RETURN(cmdBuffer);

    PMOS_CONTEXT pOsContext = osInterface->pOsContext;
    CHK_NULL_RETURN(pOsContext);

    uint32_t perfDataIndex = m_contextIndexMap[context];

    CHK_STATUS_RETURN(StoreRegister(
        osInterface, 
        miItf, 
        cmdBuffer, 
        BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, bitstreamSize), 
        reg));

    return status;
}
    
MOS_STATUS MediaPerfProfiler::CopyMemData(
    std::shared_ptr<mhw::mi::Itf>& miItf,
    PMOS_COMMAND_BUFFER            cmdBuffer,
    MOS_CONTEXT_HANDLE             pOsContext,
    PMOS_RESOURCE                  presSrc,
    uint32_t                       dwSrcOffset,
    uint32_t                       dwDstOffset)
{
    CHK_NULL_RETURN(miItf);

    auto &miCpyMemMemParams = miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
    miCpyMemMemParams       = {};
    
    miCpyMemMemParams.presSrc     = presSrc;
    miCpyMemMemParams.dwSrcOffset = dwSrcOffset;
    miCpyMemMemParams.presDst     = m_perfStoreBufferMap[(PMOS_CONTEXT)pOsContext];
    miCpyMemMemParams.dwDstOffset = dwDstOffset;
    CHK_STATUS_RETURN(miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPerfProfiler::AddCopyQualityMetricCmd(
    void                           *context,
    MOS_INTERFACE                  *osInterface,
    std::shared_ptr<mhw::mi::Itf>& miItf,
    MOS_COMMAND_BUFFER             *cmdBuffer,
    UMD_QUALITY_METRIC_ITEM        item,
    PMOS_RESOURCE                  presSrc,
    uint32_t                       dwSrcOffset)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    uint32_t   offset = 0;

    if (m_profilerEnabled == 0)
    {
        return status;
    }

    CHK_NULL_RETURN(context);
    CHK_NULL_RETURN(osInterface);
    CHK_NULL_RETURN(miItf);
    CHK_NULL_RETURN(cmdBuffer);
    CHK_NULL_RETURN(presSrc);

    PMOS_CONTEXT pOsContext = osInterface->pOsContext;
    CHK_NULL_RETURN(pOsContext);

    uint32_t perfDataIndex = m_contextIndexMap[context];

    switch (item)
    {
        case UMD_QUALITY_ITEM_SSEY:
            offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, SSEY);
            break;
        case UMD_QUALITY_ITEM_SSEU:
            offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, SSEU);
            break;
        case UMD_QUALITY_ITEM_SSEV:
            offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, SSEV);
            break;
        case UMD_QUALITY_ITEM_MEAN_SSIM_YU:
            offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, DWMeanSsimLayer1_YU);
            break;
        case UMD_QUALITY_ITEM_MEAN_SSIM_V:
            offset = BASE_OF_NODE(perfDataIndex) + OFFSET_OF(PerfEntry, DWMeanSsimLayer1_V);
            break;
        default:
            status = MOS_STATUS_INVALID_PARAMETER;
            break;
    }

    if (status == MOS_STATUS_SUCCESS)
    {
        CHK_STATUS_RETURN(CopyMemData(miItf, cmdBuffer, pOsContext, presSrc, dwSrcOffset, offset));
    }

    return status;
}

MOS_STATUS MediaPerfProfiler::SavePerfData(MOS_INTERFACE *osInterface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CHK_NULL_RETURN(osInterface);

    PMOS_CONTEXT pOsContext = osInterface->pOsContext;
    CHK_NULL_RETURN(pOsContext);

    if (m_multiprocesssinglebin)
    {
        uint32_t        cnt                     = 0;
        MOS_LOCK_PARAMS LockFlagsNoOverWrite    = {};

        MOS_ZeroMemory(&LockFlagsNoOverWrite, sizeof(MOS_LOCK_PARAMS));
        LockFlagsNoOverWrite.WriteOnly = 1;
        LockFlagsNoOverWrite.NoOverWrite = 1;

        if (m_perfDataCombined == nullptr)
        {
            m_perfDataCombinedSize  = 96;

            for (auto iter = m_perfDataIndexMap.begin(); iter != m_perfDataIndexMap.end(); ++iter)
            {
                if (iter->second > 0)
                {
                    m_perfDataCombinedSize += BASE_OF_NODE(m_perfDataIndexMap[iter->first]) + 4;
                    cnt += 1;
                }
            }

            if (cnt == 0)
            {
                return status;
            }

            m_perfDataCombined = (uint32_t *)MOS_AllocAndZeroMemory(m_perfDataCombinedSize);
            CHK_NULL_RETURN(m_perfDataCombined);

            m_perfDataCombined[0] = 0x8086;
            m_perfDataCombined[2] = m_perfDataCombinedSize - (cnt * 4);
            m_perfDataCombined[3] = cnt;

            m_perfDataCombinedOffset = 96 + (cnt * 4);
        }

        if (m_perfDataIndexMap[pOsContext] > 0)
        {
            uint8_t* pData = (uint8_t*)osInterface->pfnLockResource(
                osInterface,
                m_perfStoreBufferMap[pOsContext],
                &LockFlagsNoOverWrite);

            CHK_NULL_RETURN(pData);
            MOS_SecureMemcpy(((uint8_t *)m_perfDataCombined) + m_perfDataCombinedOffset, BASE_OF_NODE(m_perfDataIndexMap[pOsContext]), pData, BASE_OF_NODE(m_perfDataIndexMap[pOsContext]));

            osInterface->pfnUnlockResource(
                osInterface,
                m_perfStoreBufferMap[pOsContext]);

            m_perfDataCombinedOffset += BASE_OF_NODE(m_perfDataIndexMap[pOsContext]);
            m_perfDataCombined[24 + m_perfDataCombinedIndex] = BASE_OF_NODE(m_perfDataIndexMap[pOsContext]);
            m_perfDataCombinedIndex ++;

            if (m_perfDataCombinedOffset == m_perfDataCombinedSize)
            {
                MosUtilities::MosWriteFileFromPtr(m_outputFileName.c_str(), m_perfDataCombined, m_perfDataCombinedSize);
                MOS_SafeFreeMemory(m_perfDataCombined);
                m_perfDataCombined = nullptr;
                m_perfDataCombinedIndex = 0;
                m_perfDataCombinedOffset = 0;
                m_perfDataCombinedSize = 0;
            }
        }

        return status;
    }
    else if (m_perfDataIndexMap[pOsContext] > 0)
    {
        MOS_LOCK_PARAMS     LockFlagsNoOverWrite;
        MOS_ZeroMemory(&LockFlagsNoOverWrite, sizeof(MOS_LOCK_PARAMS));

        LockFlagsNoOverWrite.WriteOnly = 1;
        LockFlagsNoOverWrite.NoOverWrite = 1;

        uint8_t* pData = (uint8_t*)osInterface->pfnLockResource(
            osInterface,
            m_perfStoreBufferMap[pOsContext],
            &LockFlagsNoOverWrite);

        CHK_NULL_RETURN(pData);

        if (m_multiprocess)
        {
            int32_t pid = MosUtilities::MosGetPid();
            tm      localtime = { 0 };
            MosUtilities::MosGetLocalTime(&localtime);
            char outputFileName[MOS_MAX_PATH_LENGTH + 1];

            MOS_SecureStringPrint(outputFileName, MOS_MAX_PATH_LENGTH + 1, MOS_MAX_PATH_LENGTH + 1, "%s-pid%d-context%p-%04d%02d%02d%02d%02d%02d.bin",
                m_outputFileName.c_str(), pid, pOsContext, localtime.tm_year + 1900, localtime.tm_mon + 1, localtime.tm_mday, localtime.tm_hour, localtime.tm_min, localtime.tm_sec);

            MosUtilities::MosWriteFileFromPtr(outputFileName, pData, BASE_OF_NODE(m_perfDataIndexMap[pOsContext]));
        }
        else if (m_mergeheader)
        {
            NodeHeader *header = reinterpret_cast<NodeHeader *>(pData);
            char outputFileName[MOS_MAX_PATH_LENGTH + 1];
            MOS_SecureStringPrint(outputFileName, MOS_MAX_PATH_LENGTH + 1, MOS_MAX_PATH_LENGTH + 1, "%s-header%u.bin", m_outputFileName.c_str(), *reinterpret_cast<uint32_t*>(header));
            HANDLE hFile = nullptr;
            if (MosUtilities::MosCreateFile(&hFile, outputFileName, 0) != MOS_STATUS_SUCCESS)
            {
                MosUtilities::MosWriteFileFromPtr(outputFileName, pData, BASE_OF_NODE(m_perfDataIndexMap[pOsContext]));
            }
            else
            {
                MosUtilities::MosCloseHandle(hFile);
                MosUtilities::MosAppendFileFromPtr(outputFileName, pData + sizeof(NodeHeader), BASE_OF_NODE(m_perfDataIndexMap[pOsContext]) - sizeof(NodeHeader));
            }
        }
        else
        {
            MosUtilities::MosWriteFileFromPtr(m_outputFileName.c_str(), pData, BASE_OF_NODE(m_perfDataIndexMap[pOsContext]));
        }

        osInterface->pfnUnlockResource(
            osInterface,
            m_perfStoreBufferMap[pOsContext]);
    }

    return status;
}

PerfGPUNode MediaPerfProfiler::GpuContextToGpuNode(MOS_GPU_CONTEXT context)
{
    PerfGPUNode node = PERF_GPU_NODE_UNKNOW;

    switch (context)
    {
        case MOS_GPU_CONTEXT_RENDER:
        case MOS_GPU_CONTEXT_RENDER2:
        case MOS_GPU_CONTEXT_RENDER3:
        case MOS_GPU_CONTEXT_RENDER4:
        case MOS_GPU_OVERLAY_CONTEXT:
        case MOS_GPU_CONTEXT_RENDER_RA:
            node = PERF_GPU_NODE_3D;
            break;
        case MOS_GPU_CONTEXT_COMPUTE:
        case MOS_GPU_CONTEXT_CM_COMPUTE:
        case MOS_GPU_CONTEXT_COMPUTE_RA:
            node = PERF_GPU_NODE_3D;
            break;
        case MOS_GPU_CONTEXT_VIDEO:
        case MOS_GPU_CONTEXT_VIDEO2:
        case MOS_GPU_CONTEXT_VIDEO3:
        case MOS_GPU_CONTEXT_VIDEO4:
        case MOS_GPU_CONTEXT_VIDEO5:
        case MOS_GPU_CONTEXT_VIDEO6:
        case MOS_GPU_CONTEXT_VIDEO7:
            node = PERF_GPU_NODE_VIDEO;
            break;
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO2:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO3:
            node = PERF_GPU_NODE_VIDEO2;
            break;
        case MOS_GPU_CONTEXT_VEBOX:
        case MOS_GPU_CONTEXT_VEBOX2:
            node = PERF_GPU_NODE_VE;
            break;
        case MOS_GPU_CONTEXT_BLT:
            node = PERF_GPU_NODE_BLT;
            break;
        case MOS_GPU_CONTEXT_TEE:
            node = PERF_GPU_NODE_TEE;
            break;
        default:
            node = PERF_GPU_NODE_UNKNOW;
            break;
    }

    return node;
}

uint32_t MediaPerfProfiler::PlatFormIdMap(PLATFORM platform)
{
    uint32_t perfPlatFormId = 0;

    if (GFX_GET_CURRENT_RENDERCORE(platform) > IGFX_GEN12LP_CORE)
    {
        perfPlatFormId = ((((uint32_t)(GFX_GET_CURRENT_RENDERCORE(platform)) >> 8) - 0xc) << 2) + (GFX_GET_CURRENT_RENDERCORE(platform) & 0x3) + (uint32_t)(IGFX_GEN12LP_CORE);
    }
    else
    {
        perfPlatFormId = (uint32_t)(GFX_GET_CURRENT_RENDERCORE(platform));
    }

    return perfPlatFormId;
}

bool MediaPerfProfiler::IsPerfModeWidthMemInfo(uint32_t *regs)
{
    int8_t index = 0;
    bool   ret   = false;

    for (index = 0; index < 8; index++)
    {
        if (regs[index] != 0)
        {
            ret = true;
            break;
        }
    }

    return ret;
}
